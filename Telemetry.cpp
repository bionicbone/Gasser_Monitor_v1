// 
// 
// 

#include "Config.h"
#include "Telemetry.h"

#include <FrSkySportDecoder.h>
#include "FrSkySportSensor.h"
#include "FrSkySportSensorFlvss.h"
#include "FrSkySportSensorRpm.h"
#include "FrSkySportSensorSp2uart.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportTelemetry.h"
#if !defined(__MK20DX128__) && !defined(__MK20DX256__) && !defined(__MKL26Z64__) && !defined(__MK66FX1M0__) && !defined(__MK64FX512__)
#include "SoftwareSerial.h"
#endif
//#include <FrSkySportPolling.h>
//#include "FrSkySportSensorAss.h"
//#include "FrSkySportSensorFcs.h"
//#include "FrSkySportSensorGps.h"
//#include "FrSkySportSensorVario.h"


// Config
FrSkySportSensorFlvss flvss1;														// Create FLVSS sensor with default ID
FrSkySportSensorRpm rpm;																// Create RPM sensor with default ID of 5
FrSkySportSensorRpm rpm2(FrSkySportSensor::ID15);       // Create RPM sensor with given ID
FrSkySportSensorRpm rpm3(FrSkySportSensor::ID16);       // Create RPM sensor with given ID
FrSkySportSensorRpm rpm4(FrSkySportSensor::ID17);       // Create RPM sensor with given ID
FrSkySportSensorSp2uart sp2uart;												// Create SP2UART Type B sensor with default ID
FrSkySportTelemetry telemetry;													// Create telemetry object without polling
//FrSkySportSensorAss ass;                              // Create ASS sensor with default ID
//FrSkySportSensorFcs fcs;                              // Create FCS-40A sensor with default ID (use ID8 for FCS-150A)
//FrSkySportSensorFlvss flvss2(FrSkySportSensor::ID15); // Create FLVSS sensor with given ID
//FrSkySportSensorGps gps;                              // Create GPS sensor with default ID
//FrSkySportSensorVario vario;                          // Create Variometer sensor with default ID


// Public Variables



// Private Variables


// TODO - Look at the LQBB4 way of transmitting data
// TODO - Review the RxLinkQuality variables to see if anything useful could be sent via telemetry
// TODO - Activate cell1 and cell2 and remove temporary variable declarations below
// TODO - Activate enginTemp and remove temporary variable declaration below
// TODO - Activate error and error1 and remove temporary variable declarations below
// TODO - Activate reg and bec and remove temporary variable declarations below
// TODO - Tidy code and check comments


void telemetry_SendTelemetry() {
	// Procedure will populate the telemetry control and then call the send data routine
	// Several controls are not used and are commented out to save memory / timing.

	//Send the telemetry data, note that the data will only be sent for sensors
	//that are being polled at given moment
	float cell1 = 3.01;
	float cell2 = 3.02;
	//ambientTemp = 20;
	//canopyTemp = 35;
	int engineTemp = 999;
	//mainRPMSensorDetectedRPM = 14500;
	//clutchRPMSensorDetectedRPM = 14400;
	//clutchFullyEngagedRPM = 8995;
	//clutchFullyEngaged = true;
	int error = 99;
	int error1 = 998;
	float reg = 28.1;
	float bec = 6.9;

	// Set LiPo voltage sensor (FLVSS) data (we use two sensors to simulate 8S battery 
	// (set Voltage source to Cells in menu to use this data for battery voltage)
	// (each cell must be above 0.5v to be counted)
	// (set any cell to 0.01 to stop transmission)
	flvss1.setData(cell1, cell2, 0.00, 0.00, 0.00, 0.00);  // Cell voltages in volts (cells 1-6)
	//flvss2.setData(4.13, 4.14);                          // Cell voltages in volts (cells 7-8) - Not Used

	// Set RPM/temperature sensor data
	// (set number of blades to 2 in telemetry menu to get correct rpm value)
	uint32_t totalFrames1K = totalFrames / 1000;
	rpm.setData(mainRPMSensorDetectedRPM,				// ID5 - Rotations per minute
		totalFrames1K,														// Total SBUS Frames / 1000
		badFramesPercentage100Result);						// Total SBUS Lost Frames

	rpm2.setData(clutchRPMSensorDetectedRPM,	// ID 15 - Rotations per minute
		engineTemp,								// Temperature #1 in degrees Celsuis (can be negative, will be rounded)
		lostFramesPercentage100Result);						// Bad Frames Detected
	
	rpm3.setData(error,							// ID 16 - Error Number, 0 = OK
		error1,									// Will contain the error data 1
		999);					// Bad frames detected as Percentage against Total frames

	rpm4.setData(999,							// ID 17 - Spare
		wave1,											// Wave Form 1
		wave2);											// Wave Form 2

	// Set SP2UART sensor data
	// (values from 0.0 to 3.3 are accepted)
	sp2uart.setData(reg,	// ADC3 voltage in volts
		bec);	// ADC4 voltage in volts

// Set variometer data
// (set Variometer source to VSpd in menu to use the vertical speed data from this sensor for variometer).
//vario.setData(250.5,  // Altitude in meters (can be negative)
//              -1.5);  // Vertical speed in m/s (positive - up, negative - down)

// Set airspeed sensor (ASS) data
//ass.setData(76.5);	// Airspeed in km/h

// Set current/voltage sensor (FCS) data
// (set Voltage source to FAS in menu to use this data for battery voltage,
//  set Current source to FAS in menu to use this data for current readins)
//fcs.setData(25.3,		// Current consumption in amps
//            12.6);	// Battery voltage in volts

// Set GPS data
//gps.setData(48.858289, 2.294502,  // Latitude and longitude in degrees decimal (positive for N/E, negative for S/W)
//            245.5,                // Altitude in m (can be negative)
//            100.0,                // Speed in m/s
//            90.23,                // Course over ground in degrees (0-359, 0 = north)
//            14, 9, 14,            // Date (year - 2000, month, day)
//            12, 00, 00);          // Time (hour, minute, second) - will be affected by timezone setings in your radio
	telemetry.send();
}


void telemetry_ActivateTelemetry() {
	// The full range of possible FrSky telemetry is not required so a new line has been added, uncommented original for reference.
	// Configure the telemetry serial port and sensors (remember to use & to specify a pointer to sensor)
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK66FX1M0__) || defined(__MK64FX512__)
	//telemetry.begin(FrSkySportSingleWireSerial::SERIAL_2, &ass, &fcs, &flvss1, &flvss2, &gps, &rpm, &sp2uart, &vario);
	//telemetry.begin(FrSkySportSingleWireSerial::SERIAL_2, &flvss1, &rpm, &rpm2, &rpm3, &sp2uart);
	telemetry.begin(FrSkySportSingleWireSerial::SERIAL_2, &flvss1, &rpm, &rpm2, &rpm3, &rpm4, &sp2uart);
#else
	//telemetry.begin(FrSkySportSingleWireSerial::SOFT_SERIAL_PIN_12, &ass, &fcs, &flvss1, &flvss2, &gps, &rpm, &sp2uart, &vario);
	telemetry.begin(FrSkySportSingleWireSerial::SOFT_SERIAL_PIN_8, &flvss1, &rpm, &rpm2, &rpm3, &sp2uart);
#endif
}