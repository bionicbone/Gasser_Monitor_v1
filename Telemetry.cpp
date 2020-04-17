/*-------------------------------------------------------------------------------
 * While almost redesigned the concept for this Telemetry code was taken 
 * from the FrameLost Sensor by Tadango.
 * The FrameLost Sensor was redesigned by Reinhard to make the LQBB sensor.
 *-------------------------------------------------------------------------------*/

/*
TODO List:-
	- add sensor timing to allow "key sensors" to be sent more often than others
	- calculate "no data" skips timer based on number of sensors rather than hard code max 5 "SensorDataSkip"
	- tidy code, check flow etc.
*/ 



 #include "Config.h"
 #include "Telemetry.h"
 #include "Temperature.h"


#define SPORT_START 0x7E
#define SPORT_HEADER_DATA 0x10								// Data frame
#define SENSOR_PHYSICAL_ID 0x12								// (0xF2 on SPort Bus)
#define SPORT_HEADER_NODATA 0x00							// Signal no refresh required

bool			physIDnext = false;									// flag
uint16_t	sportCRC = 0;												// S.PORT CRC calculation
byte			lastSensorNumber = -1;							// Increaments after each sensor transmits, steps through sensor array
byte			nextSensorNumber = 0;								// Used to calculate the next sesnor number at the begining of send sensor process
unsigned long	lastSensorMillis = millis();		// Used to calcualte the sensor refresh rate
unsigned long	sensorRefreshRate = 0;					// Sensor refresh rate

// Helper for long / byte conversion
typedef union {
	char byteValue[4];
	long longValue;
} longHelper;

 // Required for Teensy V3.2 or v4.0 support
#if defined (__MK20DX256__)								// Teensy v3.2
#define SPORT 10													// Digital Teensy Serial2 pin for SPort (bidirectional mode)
volatile uint8_t *uartSingleTxRx = 0;	// Controls the Single Wire mode (Tx / Rx), 8 bit for Teensy v3.2, 32 bit for Teensy v4.0
Stream* sportFlusher;											// Must flush to write otherwise we crash!!
#elif defined(__IMXRT1062__)							// Teensy v4.0
#define SPORT 10													// Digital Teensy Serial2 pin for SPort (bidirectional mode)
volatile uint32_t *uartSingleTxRx = NULL;	// Controls the Single Wire mode (Tx / Rx), 8 bit for Teensy v3.2, 32 bit for Teensy v4.0
Stream* sportFlusher;											// Must flush to write otherwise we crash!!
#else
#error "Unsupported processor! 5V 16MHz Arduino ProMini, Nano or Teensy v3.2 required";
#endif

// Active Sensors and current value store
// Each sensor only has one value, LUA will set the types (RPM, Voltage, Amps, Temperature etc) and rename the sensors on discovery
// Each active sensor adds 24ms to the refresh rate, thus
// one sensor active the refresh rate will be 24ms, with all 32 sensors active the refresh rate will be 768ms
// *** Maintain void updateValue(byte sensorNumber) when new sensors are added ***
struct frSkyTeensySensorArray {
	bool			SensorActive[32]	= { true,   true,   true,   true,   true,   true,   true,   true,   true,   true,   false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  true,   true,   true,   true,   true,   true,   true };
	uint32_t	SensorID[32]			= { 0x5100, 0x5101, 0x5102, 0x5103, 0x5104, 0x5105, 0x5106, 0x5107, 0x5108, 0x5109, 0x510A, 0x510B, 0x510C, 0x510D, 0x510E, 0x510F, 0x5110, 0x5111, 0x5112, 0x5113, 0x5114, 0x5115, 0x5116, 0x5117, 0x5118, 0x5119, 0x511A, 0x511B, 0x511C, 0x511D, 0x511E, 0x511F };
	uint32_t	SensorValue[32]		= { 0 };
	bool			SensorDataChanged[32] = { {false} };
	byte			SensorDataSkip[32] = { 0 };						// Used to ensure each sensor sends regular updates even if the data does not change
};
frSkyTeensySensorArray frSkyTeensySensors;
const byte SENSOR_ARRAY_SIZE = sizeof(frSkyTeensySensors.SensorID) / sizeof(frSkyTeensySensors.SensorID[0]);



//		 Updates the values in the Sensor Array
// *** Must be maintained when new sensors are added ***
void updateValue(byte sensorNumber) {
	uint32_t totalFrames1K = totalFrames / 1000;
	switch (sensorNumber) {
	case 0:														// 5100 - Error - Last Error Number, 0 if none, 99 if paused
		break;
	case 1:														// 5101 - Error - Additional Information
		break;
	case 2:														// 5102 - SBUS - Total Frames / 1000
		if (totalFrames1K != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = totalFrames1K;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 3:														// 5103 - SBUS - Bad Frames as calculated from SBUS data @bionicbone method in last 100 frames
		if (badFramesPercentage100Result != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = badFramesPercentage100Result;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 4:														// 5104 - SBUS - End to End Quality as calculated from SBUS data @bionicbone method in last 100 frames
		if (overallE2EQuality != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = overallE2EQuality;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 5:														// 5105 RPM - Main Engine RPM
		if (mainRPMSensorDetectedRPM != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = mainRPMSensorDetectedRPM;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 6:														// 5106 - RPM - Clutch RPM
		if (clutchRPMSensorDetectedRPM != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = clutchRPMSensorDetectedRPM;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 7:														// 5107 - Temperature - Ambient
		if (ambientTemp != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = ambientTemp;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 8:														// 5108 - Temperature - Canopy
		if (canopyTemp != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = canopyTemp;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 9:														// 5109 - Temperature - Engine
		if (engineTemp != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = engineTemp;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 10:													// 510A - Use Next
		if (0 != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = 0;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;


	// 25 onwards used for temporary testing values
	case 25:														// 5119 - TEST DATA - SBUS Lost Frames as reported by Rx in last 100 frames
		if (lostFramesPercentage100Result != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = lostFramesPercentage100Result;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 26:														// 511A - TEST DATA - SBUS Longest frame hold as calculated from SBUS data @bionicbone method in last 100 frames
		if (channelsMaxHoldMillis100Resul != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = channelsMaxHoldMillis100Resul;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 27:														// 511B - TEST DATA - SBUS Wave
		if (wave1 != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = wave1;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 28:														// 511C - TEST DATA - SBUS low Refresh Rate over last 100 frames
		if (sbusFrameLowMicros != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = sbusFrameLowMicros;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 29:														// 511D - TEST DATA - SBUS high Refresh Rate over last 100 frames
		if (sbusFrameHighMicros != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = sbusFrameHighMicros;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 30:														// 511E - TEST DATA - 
		if (0 != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = 0;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 31:														// 511F - TEST DATA -
		if (0 != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = 0;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	}
}



void telemetry_ActivateTelemetry() {
	//Sport UART 8N1 57K6
#if defined (__AVR_ATmega328P__)
	sport.begin(57600);
#elif defined (__MK20DX256__)
#define sport Serial2
	sportFlusher = &Serial2;
	sport.begin(57600, SERIAL_8N1_RXINV_TXINV);
	// Teensy Serial2 needs to be single wire Tx/Rx
	uartSingleTxRx = &UART1_C3;
	UART1_C1 |= (UART_C1_LOOPS | UART_C1_RSRC);
#elif defined (__IMXRT1062__)
#define sport Serial2
	sportFlusher = &Serial2;
	sport.begin(57600, SERIAL_8N1_RXINV_TXINV);
	// Teensy Serial2 needs to be single wire Tx/Rx
	uartSingleTxRx = &LPUART4_CTRL;
	LPUART4_CTRL |= (LPUART_CTRL_LOOPS | LPUART_CTRL_RSRC);
	IOMUXC_LPUART4_TX_SELECT_INPUT = 2;
#endif
}


void telemetry_SendTelemetry() {
	// Read SPort bytes
	byte val = 0;
	while (sport.available()) {
		val = sport.read();
		
		// get the next active sensor number
		nextSensorNumber = lastSensorNumber + 1; if (nextSensorNumber > SENSOR_ARRAY_SIZE) nextSensorNumber = 0;
		while (frSkyTeensySensors.SensorActive[nextSensorNumber] == false) {
			nextSensorNumber++; if (nextSensorNumber > SENSOR_ARRAY_SIZE) nextSensorNumber = 0;
		}
		updateValue(nextSensorNumber);
		NewValueSport(val);             //Pass it to the SPort handler
	}
}


void NewValueSport(byte val) {
	byte physicalID;

	if (val == SPORT_START) {
		// next byte is physical ID
		physIDnext = true;
	}
	else if (physIDnext) {
		//Get physical id without CRC bits
		physicalID = val & 0x1F;
		physIDnext = false;

		if (physicalID == SENSOR_PHYSICAL_ID) {
			//Needs my data so send it 
			sendFrame();
		}
	}
}


void sendByte(byte b) {
	if (b == 0x7E)
	{
		sport.write(0x7D);
		sport.write(0x5E);
		sportCRC += b; sportCRC += sportCRC >> 8; sportCRC &= 0x00ff;
	}
	else if (b == 0x7D)
	{
		sport.write(0x7D);
		sport.write(0x5D);
		sportCRC += b; sportCRC += sportCRC >> 8; sportCRC &= 0x00ff;
	}
	else
	{
		sport.write(b);
		sportCRC += b; sportCRC += sportCRC >> 8; sportCRC &= 0x00ff;
	}
}


void sendFrame() {

	longHelper lh;
	byte frame[8];
	sportCRC = 0;

#if defined (__AVR_ATmega328P__)
	pinMode(SPORT, OUTPUT);
	delay(1);  //Wait for output to be available
#elif defined (__MK20DX256__)
	//Teensy Serial2 Tx Mode
	*uartSingleTxRx |= UART_C3_TXDIR;
#elif defined (__IMXRT1062__)
	//Teensy v4.0 Serial2 Tx Mode
	*uartSingleTxRx |= LPUART_CTRL_TXDIR;
#endif


	if (nextSensorNumber == 0) {
		sensorRefreshRate = millis() - lastSensorMillis;
		Serial.print("Sensor Refresh Rate "); Serial.print(sensorRefreshRate); Serial.println("ms");
		lastSensorMillis = millis();
		Serial.println(nextSensorNumber);
	}

	// If data has changed or long time since last data send an update, otherwise signal data not changed
	if (frSkyTeensySensors.SensorDataChanged[nextSensorNumber] || frSkyTeensySensors.SensorDataSkip[nextSensorNumber] > 5) {
		// prepare data frame for an update
		frame[0] = SPORT_HEADER_DATA;
		// Get the next SensorID from the sensor array
		frame[1] = lowByte(frSkyTeensySensors.SensorID[nextSensorNumber]);
		frame[2] = highByte(frSkyTeensySensors.SensorID[nextSensorNumber]);
		// Get the 32Bit value from the sensor array
		lh.longValue = frSkyTeensySensors.SensorValue[nextSensorNumber];
		frame[3] = lh.byteValue[0];
		frame[4] = lh.byteValue[1];
		frame[5] = lh.byteValue[2];
		frame[6] = lh.byteValue[3];
		// Data will be transmitted so reset the data changed flag
		frSkyTeensySensors.SensorDataChanged[nextSensorNumber] = false;
	}
	else {
		// prepare data frame for no data update signal
		// Use with caution !! 
		//		- this can cause "sensor loss" warnings on values that do not update regularily
		//    - this will casue sensors not to be discovered
		frame[0] = SPORT_HEADER_NODATA;
		frame[1] = 0x00;
		frame[2] = 0x00;
		frame[3] = 0x00;
		frame[4] = 0x00;
		frame[5] = 0x00;
		frame[6] = 0x00;
		frSkyTeensySensors.SensorDataSkip[nextSensorNumber]++;
	}

	//Send the frame
	for (short i = 0; i < 8; i++) {
		frame[7] = 0xFF - sportCRC;  // Keep re-calculating CRC as we send each each byte
		sendByte(frame[i]);
	}

	lastSensorNumber = nextSensorNumber;

	//Switch back to input
#if defined (__AVR_ATmega328P__)
	pinMode(SPORT, INPUT);
#elif defined (__MK20DX256__)
	sportFlusher->flush();
	//Teensy v3.2 Serial2 Rx Mode
	*uartSingleTxRx &= ~UART_C3_TXDIR;
#elif defined (__IMXRT1062__)
//Teensy v4.0 Serial2 Rx Mode
	*uartSingleTxRx &= ~LPUART_CTRL_TXDIR;
#endif  
}
