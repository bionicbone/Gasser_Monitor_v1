/*-------------------------------------------------------------------------------
 * While almost redesigned the concept for this Telemetry code was taken 
 * from the FrameLost Sensor by Tadango.
 * The FrameLost Sensor was redesigned by Reinhard to make the LQBB sensor.
 *-------------------------------------------------------------------------------*/

/*
 The design is such that we only use one SensorID (F2 - Sensor 19), but this sensor has upto 32 different values
 LUA will set the types of the values (RPM, Voltage, Amps, Temperature etc) and rename them on discovery (if script active)
 Each value can be activated or deactivated. Each active value adds 24ms to the refresh rate, thus:
		one value active the refresh rate will be 24ms, with all 32 values active the refresh rate will be 768ms
 If fitted the FrSky FLVSS sensor adds randomly a 0 - 120ms depending on the number of active values to the refresh rate
 The FLVSS will only be decoded every 708ms which is the maximum possible speed otherwise the refresh rate of the values will be significantly impacted
*/

/*
Value | SensorID | Name | Description
-------- | ----| -----| -----------
5100 | 19 | ERR | Error - Info / Error Number(see error page)
5101 | 19 | ErrD | Error - Info / Error Extra(see error page)
5102 | 19 | TFCK | SBUS - Total SBUS Frames Received(*1000)
5103 | 19 | BFP | SBUS - Bad Frames as calculated from SBUS data @bionicbone method in last 100 frames.To add a comparison for those using Reinhard LQBB sensor
5104 | 19 | E2EQ | SBUS - End to End Quality as calculated from SBUS data @bionicbone method in last 100 frames
5105 | 19 | MAIN | RPM - Main Engine RPM
5106 | 19 | CLUT | RPM - Clutch RPM
5107 | 19 | AmbT | Temperature - Ambient
5108 | 19 | CanT | Temperature - Canopy
5109 | 19 | EngT | Temperature - Engine
510A | 19  |      | Not Used
510B | 19  |      | Not Used
510C | 19  |      | Not Used
510D | 19  |      | Not Used
510E | 19  |      | Not Used
510F | 19  |      | Not Used
5110 | 19  |      | Not Used
5111 | 19  |      | Not Used
5112 | 19  |      | Not Used
5113 | 19  |      | Not Used
5114 | 19  |      | Not Used
5115 | 19  |      | Not Used
5116 | 19  |      | Not Used
5117 | 19  |      | Not Used
5118 | 19  |      | Not Used
5119 | 19  |      | TEST DATA
511A | 19  |      | TEST DATA
511B | 19  |      | TEST DATA
511C | 19  |      | TEST DATA
511D | 19  |      | TEST DATA
511E | 19  |      | TEST DATA
511F | 19  |      | TEST DATA
*/

/*
TODO List:-
	- add sensor timing to allow "key sensors" to be sent more often than others
	- calculate "no data" skips timer based on number of sensors rather than hard code max 5 "SensorDataSkip"
	- tidy code, check flow etc.
*/ 


#include "Config.h"
#include "RPM.h"
#include "RxLinkQuality.h"
#include "Telemetry.h"
#include "Temperature.h"
#include "Power.h"

// Public Variables
float					cell[6] = { 0.0000 };								// Final cell voltage store
float cellSmoothed[6] = { 0.0000 };								// Final cell voltage after smoothing
unsigned long	sensorRefreshRate = 0;					// Sensor refresh rate
unsigned long	FLVSSRefreshRate = 0;						// Sensor refresh rate


// Private variables
bool					physIDnext = false;							// flag
uint16_t			sportCRC = 0;										// S.PORT CRC calculation
byte					lastSensorNumber = -1;					// Increaments after each sensor transmits, steps through sensor array
byte					nextSensorNumber = 0;						// Used to calculate the next sesnor number at the begining of send sensor process
unsigned long	lastSensorMillis = millis();		// Used to calcualte the sensor refresh rate
unsigned long	lastFLVSSMillis = millis();			// Used to calcualte the FLVSS refresh rate


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
// *** Maintain void updateValue(byte sensorNumber) when new sensors are added ***
struct frSkyTeensySensorArray {
	bool			SensorActive[32]	= { true,   true,   true,   true,   true,   true,   true,   true,   true,   true,   false,   false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  false,  true,   true,   true,   true,   true,   true,   true };
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
		if ((uint32_t)badFramesPercentage100Result != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = badFramesPercentage100Result;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 4:														// 5104 - SBUS - End to End Quality as calculated from SBUS data @bionicbone method in last 100 frames
		if ((uint32_t)overallE2EQuality != frSkyTeensySensors.SensorValue[sensorNumber]) {
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
		if ((uint32_t)ambientTemp != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = (uint32_t)ambientTemp;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 8:														// 5108 - Temperature - Canopy
		if ((uint32_t)canopyTemp != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = (uint32_t)canopyTemp;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 9:														// 5109 - Temperature - Engine
		if ((uint32_t)engineTemp != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = (uint32_t)engineTemp;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 10:													// 510A - Use Next
		if (0 != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = 999;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;



	// 25 onwards used for temporary testing values
	case 25:														// 5119 - TEST DATA - Teensy Voltage (Ratio 3.0)
		if (teensyVoltage != frSkyTeensySensors.SensorValue[sensorNumber]) {
			float calc = teensyVoltage * 100;
			uint32_t Tx = calc;
			frSkyTeensySensors.SensorValue[sensorNumber] = Tx;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 26:														// 511A - TEST DATA - Rectifier Voltage (Ratio 3.0)
		if (recVoltage != frSkyTeensySensors.SensorValue[sensorNumber]) {
			float calc = recVoltage * 100;
			uint32_t Tx = calc;
			frSkyTeensySensors.SensorValue[sensorNumber] = Tx;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 27:														// 511B - TEST DATA - BEC voltage (Ratio 3.0)
		if (becVoltage != frSkyTeensySensors.SensorValue[sensorNumber]) {
			float calc = becVoltage * 100;
			uint32_t Tx = calc;
			frSkyTeensySensors.SensorValue[sensorNumber] = Tx;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 28:														// 511C - TEST DATA - BEC AMPS (Ratio 3.0)
		if (becDischargeLoopAmps != frSkyTeensySensors.SensorValue[sensorNumber]) {
			float calc = becDischargeLoopAmps * 100;
			uint32_t Tx = calc;
			frSkyTeensySensors.SensorValue[sensorNumber] = Tx;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 29:														// 511D - TEST DATA - Battery AMPS (Ratio 3.0)
		if (batteryDischargeLoopAmps != frSkyTeensySensors.SensorValue[sensorNumber]) {
			float calc = batteryDischargeLoopAmps * 100;
			int32_t Tx = calc;
			frSkyTeensySensors.SensorValue[sensorNumber] = Tx;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 30:														// 511E - TEST DATA - Battery mAH
		if (batteryDischargeTotalMAH != frSkyTeensySensors.SensorValue[sensorNumber]) {
			int32_t Tx = batteryDischargeTotalMAH;
			frSkyTeensySensors.SensorValue[sensorNumber] = Tx;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	case 31:														// 511F - TEST DATA - BEC Temperature
		if ((float)becTemp != frSkyTeensySensors.SensorValue[sensorNumber]) {
			frSkyTeensySensors.SensorValue[sensorNumber] = (uint32_t)becTemp;
			frSkyTeensySensors.SensorDataChanged[sensorNumber] = true;
		}
		break;
	}
}


void _telemetry_ActivateTelemetry() {
	//Sport UART 8N1 57K6
#if defined (__AVR_ATmega328P__)
	sport.begin(57600);
#elif defined (__MK20DX256__)
#define sport Serial2
	sportFlusher = &Serial2;
	sport.begin(57600, SERIAL_8N1_RXINV_TXINV);   // 58824  // 56376
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


void _telemetry_SendTelemetry() {
	// Read SPort bytes
	while (sport.available()) {
		byte val = 0;
		val = sport.read();
		sendValueData(val);             //Pass it to the SPort handler
	}
}


void sendValueData(byte val) {
	byte physicalID;

	if (val == SPORT_START) {
		// next byte is physical ID
		physIDnext = true;
	}
	else if (physIDnext) {
		// Get physical id 
		physicalID = val;
		physIDnext = false;

		switch (physicalID)
		{
		case SENSOR_GASSER_ID:					// Send a value
				// get the next active value number for the array
				nextSensorNumber = lastSensorNumber + 1; if (nextSensorNumber > SENSOR_ARRAY_SIZE) nextSensorNumber = 0;
				while (frSkyTeensySensors.SensorActive[nextSensorNumber] == false) {
					nextSensorNumber++; if (nextSensorNumber > SENSOR_ARRAY_SIZE) nextSensorNumber = 0;
				}
				updateValue(nextSensorNumber);
				sendFrame();
			break;

		case SENSOR_FLVSS_ID:						// Get a value from an attachec FrSky FLVSS module
			// only allow FLVSS decoding once every 708ms, otherwise sending of values will be significantly effected
			if (millis() - lastFLVSSMillis > SENSOR_FLVSS_DECODE_DELAY) getFLVSSdata(); 
			break;
		
		default:
			break;
		}
	}
}


void sendFrame() {
	longHelper lh;
	byte frame[8];
	sportCRC = 0;

	// Activate Tx Mode
#if defined (__AVR_ATmega328P__)
	// Nano type processor
	pinMode(SPORT, OUTPUT);
	delay(1);  //Wait for output to be available
#elif defined (__MK20DX256__)
	//Teensy Serial2 Tx Mode
	*uartSingleTxRx |= UART_C3_TXDIR;
#elif defined (__IMXRT1062__)
	//Teensy v4.0 Serial2 Tx Mode
	*uartSingleTxRx |= LPUART_CTRL_TXDIR;
#endif
	
	// Keep track of the Refresh Rate to send all sensors
	if (nextSensorNumber == 0) {
		sensorRefreshRate = millis() - lastSensorMillis;
		lastSensorMillis = millis();
#if defined (DEBUG_TELEMETRY_REFRESH_RATES)
		Serial.print("All Values Refresh Rate "); Serial.print(sensorRefreshRate); Serial.println("ms");
#endif
	}
		
	// If data has changed or long time since last data send an update, otherwise send "data not changed" info
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

	// Send the frame byte by byte
	for (short i = 0; i < 8; i++) {
		frame[7] = 0xFF - sportCRC;  // Keep re-calculating CRC as we send each each byte
		sendByte(frame[i]);
	}
	// Keep track of the value in the array we have sent
	lastSensorNumber = nextSensorNumber;

	//Switch back to Rx mode
#if defined (__AVR_ATmega328P__)
	// Nano type processors
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


void getFLVSSdata() {
	/*
	Vaild Data format for FLVSS:
	0x7E Sensor Start
	0x01 FLVSS Data
	0x10 Header Data Flag (abort the read if this is not correct, No Data to update is 0x00 and not sure why but sometimes the data is not what we think)
	0x00 & 0x03 Cell 0-1 0x0300
	0x?? Start and End Cell Data, 4 bits each (0x30 = Cells 0-1, 0x32 = cells 2-3 etc) use & 0x0F to calculate first, second is always first + 1
	0x?? & 0x?? & 0X?? Data for two cells, 12 bits each and needs / 500 to be a correct float
	0x?? CRC bit
	*/

	uint8_t		data[8] = { 0 };			// Holds the 8 bytes of valid data that arrive after the Header
	byte			counter = 0;					// Counter for data[] Array
	uint32_t	value = 0;						// Used to hold the value of the two cells (12 bits each, 1.5 bytes)
	byte			frameStartCell = 0;		// Used to hold the value of the starting cell in the S.PORT frame (0,2,4)

#if defined (DEBUG_FLVSS_CALCULATION)
	Serial.println("FLVSS DATA");
	Serial.print("Already received "); Serial.println(SPORT_START, HEX);
	Serial.print("Already received "); Serial.println(SENSOR_FLVSS_ID, HEX);
#endif

	// Each sensor frame is 10 bytes so there should be 8 bytes left
	// Store them in data[]
	// It is possible here that the data is not what we want, in this case we have to abort!!
	while (counter < 8) {
		if (sport.available()) {
			data[counter] = sport.read();
			if (data[0] != SPORT_HEADER_DATA) {
				// Bad data or No Data Changed frame received
#if defined (DEBUG_FLVSS_CALCULATION) 
				Serial.println("Bad Data Abort!! ");
#endif
				break;
			}
#if defined (DEBUG_FLVSS_CALCULATION) 
			Serial.print("New Data "); Serial.println(data[counter], HEX);
#endif
			counter++;
		}
	}
	// check [0-2] is a valid header for FLVSS
	if (data[0] == SPORT_HEADER_DATA) {
		if (data[1] == 0x00) {
			if (data[2] == 0x03) {
				// Header looks good so work out the starting cell that the frame is for (0,2,4 ?)
				frameStartCell = data[3] & 0x0F;
				//format the cell data because each cells value is 12 bits
				((uint8_t*)&value)[0] = data[4];
				((uint8_t*)&value)[1] = data[5];
				((uint8_t*)&value)[2] = data[6];
				
				//  TODO - must check CRC at this point data[7] before accepting data into cell[] or cellSmoothed[]!! 
				
				// Work out each cell value using 12 bits.
				// This got me the value appears to be multiplied by 500
				cell[frameStartCell] = (value & 0x0FFF) / 500.0;
				smoothFLVXXdata(frameStartCell);
				value >>= 12; // loose 12 bits
				cell[frameStartCell + 1] = (value & 0x0FFF) / 500.0;
				smoothFLVXXdata(frameStartCell + 1);
				
				FLVSSRefreshRate = millis() - lastFLVSSMillis; lastFLVSSMillis = millis();

				//if (frameStartCell == 0) {
				//	Serial.print(cell[frameStartCell]); Serial.print(", ");
				//	Serial.print(cell[frameStartCell + 1]); Serial.print(", ");
				//	Serial.print(cell[frameStartCell + 2]); Serial.print(", ");
				//	Serial.print(cellSmoothed[frameStartCell]); Serial.print(", ");
				//	Serial.print(cellSmoothed[frameStartCell + 1]); Serial.print(", ");
				//	Serial.println(cellSmoothed[frameStartCell + 2]);
				//}

#if defined (DEBUG_FLVSS_CALCULATION) 
				Serial.print("frameStartCell "); Serial.println(frameStartCell, HEX);
				Serial.print("cell "); Serial.print(frameStartCell); Serial.print(" = "); Serial.print(cell[frameStartCell]); Serial.print(" & smoothed = "); Serial.println(cellSmoothed[frameStartCell]);
				Serial.print("cell "); Serial.print(frameStartCell + 1); Serial.print(" = "); Serial.print(cell[frameStartCell + 1]); Serial.print(" & smoothed = "); Serial.println(cellSmoothed[frameStartCell + 1]);
				Serial.print("FLVSS Refresh Rate "); Serial.print(FLVSSRefreshRate); Serial.println("ms");
#endif
#if defined (DEBUG_TELEMETRY_REFRESH_RATES)
				Serial.print("FLVSS Refresh Rate "); Serial.print(FLVSSRefreshRate); Serial.println("ms");
#endif
			}
		}
	}
}


void smoothFLVXXdata(byte cellArrayNumber) {
	// Initialize the smoothing on first run or if the voltage change is high then just follow it.
	// After bench testing 0.3 seems a fair compromise, but it may need to be higher once flight testing starts. 
	if (abs(cell[cellArrayNumber] - cellSmoothed[cellArrayNumber]) > SMOOTH_OVER_RIDE)
	{
		cellSmoothed[cellArrayNumber] = cell[cellArrayNumber];
	}
	else {
		cellSmoothed[cellArrayNumber] += (cell[cellArrayNumber] - cellSmoothed[cellArrayNumber]) * SMOOTH_MULTIPLIER;
	}
}

