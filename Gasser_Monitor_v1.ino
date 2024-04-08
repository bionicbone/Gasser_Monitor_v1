		/*
    Name:       Gasser_Monitor_v1.ino
    Created:		15/03/2020 16:19:35
    Author:     (c) Kevin Guest - AKA The "Bionicbone" on RCGroups / GitHub and "TheBionicbone" on YouTube.
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


#include "MCU6050.h"
#include "U8g2lib.h"
#include "Wire.h"
#include "OLED.h"
#include "ErrorHandling.h"
#include "Power.h"
#include "Temperature.h"
#include "Config.h"
#include "Telemetry.h"
#include "sbus.h"
#include "RxLinkQuality.h"
#include "RPM.h"
#include "SdCard.h"
#include "SD.h"
#include "SPI.h"
#include "TimeLib.h"


unsigned long		timeLoopMicros = 0;								// Used to remember micros() at the start of the main loop.
unsigned long		lastLoopMicros = 0;								// actual loop time in micros(), time between FrSky Telemetry Sends must be less than 3600us
uint16_t				firstRunCounter = 0;							// counts the first loops
bool						firstRun = true;									// Resets to false when firstRunCounter hits its target


unsigned long		sdCardLogMillis = 0;							// Used to log data on SD Card every x milliseconds
unsigned long		oledUpdateMillis = 0;							// Used to update OLED every x milliseconds

bool						oledUpdateInFlight = false;				// True if OLED has been updated to show in Flight

void setup() {
	// Start the USB serial for debugging
	Serial.begin(115200);

	// Set analogue reference voltage to 3.3v with 12 bit resolution
	// ** DO NOT ALTER **
	analogReference(DEFAULT);
	analogReadResolution(12);

	delay(500);

	_oled_Setup();

	_mcu6050_Setup();

	// TODO - Add function to set time
	// Get the RTC time
	setSyncProvider(getTeensy3Time);
	// Set the Time
	if (year() == 1970) {
		setSyncProvider(getTeensy3Time);
		setTime(05, 47, 00, 19, 06, 2020);
		Teensy3Clock.set(now());
	}

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
}



void loop() {

	// Create a loop timer
	// Must be at the start of the loop
	timeLoopMicros = micros();

	// Calculate the Current RPM data (<5us)
	// updates variables mainRPMSensorDetectedRPM & clutchRPMSensorDetectedRPM
	// also updates variable inFlight
	_rpm_calculate_SensorPulse();

	// Calculate the voltages and current sensors (Normally <80us, occational at 2600us)
	// updates variables 
	_power_ReadSensors();

	// Calculate the Rx Quality Indicators (from 10us to occational 60us)
	// updates variables lostFramesPercentage100Result & badFramesPercentage100Result
	// also updates variable totalFrames
	_rxLinkQuality_Scan(firstRun);

	// read the temperatures (ambient/canopy/engine) (<80us)
	_temperatures_Read();

	// read MCU vobration sensor data ( Significant at 1600us )
	// NOTE: Time to execute could cause "Sensor Loss" issues for FrSky Telemetry or SD Card recording
	_mcu6050_Read();

	// check for errors and report
	_errorHandling_checkErrors();

	// display the Date and Time while initialising
	if (firstRun == true) _oled_DateAndTime();

	// write SD log
	if (firstRun == false && millis() - sdCardLogMillis > 200 - 1) {
		sdCardLogMillis = millis();
		_sd_WriteLogDate();
	}

	// update OLED
	if (firstRun == false && millis() - oledUpdateMillis > 210 - 1) {
		oledUpdateMillis = millis();
		if (_inFlight == false) {
			_oled_FlightBatteryVoltage();						// Normally 8us, occasionally massive at 78000-120000us)
			oledUpdateInFlight = false;
		}
		else if (oledUpdateInFlight == false) {		// Only update the display once when in flight
			_oled_inFlight();												// Normally 8us, occasionally massive at 78000us)
			oledUpdateInFlight = true;
		}
	}

	//*******************************
	//*** START - TESTING ONLY !! ***
	//*******************************



	//*******************************
	//*** END -  TESTING ONLY !!  ***
	//*******************************

	// calculate the time it took to run the loop.
	// it counts everything other than the time to send the Telemetry data
	lastLoopMicros = micros() - timeLoopMicros;

	if (firstRun == false && lastLoopMicros > MAX_MAIN_LOOP_TIME_BEFORE_ERROR) {
		Serial.print(millis()); Serial.print(": Long Loop @ "); Serial.print(lastLoopMicros); Serial.println("us");
	}

	// Format and Send the telemetry data using the FrSky S.Port solution
	_telemetry_SendTelemetry();
	
	// Deal with the first run that inhibits errors etc.
	if (firstRunCounter < MIN_MAIN_LOOP_FIRST_RUN_LOOPS) {
		firstRunCounter++;
		if (firstRunCounter == MIN_MAIN_LOOP_FIRST_RUN_LOOPS) { firstRun = false; }
	}
	// *** !!! Place nothing else here !!! ***

}

time_t getTeensy3Time()
{
	return Teensy3Clock.get();
}


// Used for debugging to get the program to stop
void stop() {
	while (1);
}
