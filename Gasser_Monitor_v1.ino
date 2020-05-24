		/*
    Name:       Gasser_Monitor_v1.ino
    Created:		15/03/2020 16:19:35
    Author:     (c) Kevin Guest - AKA The Bionicbone on RCGroups.
		Decription:	Migrating code from the Gasser Charger Module and placing into cpp & h files.
		Use:				No Commercial Activities

		Licensing GPL & note the following:
		My licencing requirements do not allow commercial use of this software without permission in writing from myself.
			By means of clarity this includes anyone using this software or specific hardware sensor design for any activity
			where there will be a financial or other reward whether that results in a financial profit or not.
		My licencing does allow people to make a "small profit" by helping other RC hobbyists, this includes pre-flashing a Teensy,
		building and suppling sensors and even fully fitting to an RC model.
			By means of clarity "small profit" is deemed to be not more than $35 overall profit for services provided to any individual RC Hobbyist.
		Where misalignments occure between the above and the GPL attached then you should seek further clarification for myself

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
		*/


#include "ChargingTestsOnly.h"
#include "ErrorHandling.h"
#include "Power.h"
#include "Temperature.h"
#include "Config.h"
#include "Telemetry.h"
#include <SBUS.h>
#include "RxLinkQuality.h"
#include "RPM.h"
#include "SdCard.h"
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>


unsigned long		timeLoopMicros = 0;								// Used to remember micros() at the start of the main loop.
unsigned long		lastLoopMicros = 0;								// actual loop time in micros(), time between FrSky Telemetry Sends must be less than 3600us
uint16_t				firstRunCounter = 0;							// counts the first loops
bool						firstRun = true;									// Resets to false when firstRunCounter hits its target


unsigned long		testMillis = 0;

void setup() {
	// Start the USB serial for debugging
	Serial.begin(115200);
	
	// Set analogue reference voltage to 3.3v with 12 bit resolution
	// ** DO NOT ALTER **
	analogReference(DEFAULT);
	analogReadResolution(12);
	
	delay(500);

	// Start the SD Card Logging
	_sd_SetUp();

	// Start RPM for Engine and Clutch sensor monitoring
	_rpm_ActivateInterrupts();

	// Start the RX Link Quality monitoring
	_rxLinkQuality_ActivateSBUS();

	// Start the Telemetry Sensors
	_telemetry_ActivateTelemetry();

	// Start the Temperature Readings
	_temperature_Setup();

	// Start the Power (Volts / Current) readings
	_power_Setup();

	// Start the Error Handling Processes
	_errorHandling_Setup();

	//*******************************
	//*** START - TESTING ONLY !! ***
	//*******************************

	//chargingTestOnly_Setup();

	//*******************************
	//*** END -  TESTING ONLY !!  ***
	//*******************************

	Serial.println("Setup Complete");
	Serial.print("System Started millis() "); Serial.println(millis());

	testMillis = millis();
}



void loop() {

	// Create a loop timer
	// Must be at the start of the loop
	timeLoopMicros = micros();

	// Calculate the Current RPM data
	// updates variables mainRPMSensorDetectedRPM & clutchRPMSensorDetectedRPM
	// also updates variable inFlight
	_rpm_calculate_SensorPulse();

	// Calculate the voltages and current sensors
	// updates variables 
	_power_ReadSensors();

	// Calculate the Rx Quality Indicators
	// updates variables lostFramesPercentage100Result & badFramesPercentage100Result
	// also updates variable totalFrames
	_rxLinkQuality_Scan(firstRun);

	// read the temperatures (ambient/canopy/engine)
	_temperatures_Read();

	// check for errors and report
	_errorHandling_checkErrors();

	// write SD log
	_sd_WriteLogDate();

	//*******************************
	//*** START - TESTING ONLY !! ***
	//*******************************

	

	//*******************************
	//*** END -  TESTING ONLY !!  ***
	//*******************************

	// calculate the time it took to run the loop.
	// it counts everything other than the time to send the Telemetry data
	lastLoopMicros = micros() - timeLoopMicros;

	if (firstRun == MIN_MAIN_LOOP_BEFORE_REPORTING_ERRORS && lastLoopMicros > MAX_MAIN_LOOP_TIME_BEFORE_ERROR) {
		Serial.print(millis()); Serial.print(": Long Loop @ "); Serial.print(lastLoopMicros); Serial.println("us");
	}

	// Format and Send the telemetry data using the FrSky S.Port solution
	_telemetry_SendTelemetry();
	// Deal with the first run that inhibits errors etc.
	if (firstRunCounter < MIN_MAIN_LOOP_BEFORE_REPORTING_ERRORS) { 
		firstRunCounter++; 
		if (firstRunCounter == MIN_MAIN_LOOP_BEFORE_REPORTING_ERRORS) { firstRun = false; }
	}
	// *** !!! Place nothing else here !!! ***

}


// Used for debugging to get the program to stop
void stop() {
	while (1);
}
