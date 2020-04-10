/*
    Name:       Gasser_Monitor_v1.ino
    Created:		15/03/2020 16:19:35
    Author:     (c) Kevin Guest - AKA The Bionicbone on RCGroups.
		Decription:	Migrating code from the Gasser Charger Module and placing into cpp & h files.
*/

/*
		FrSky S-Port Telemetry library example
		(c) Pawelsky 20180402
		Not for commercial use
*/

#include "Config.h"
#include <FrSkySportDecoder.h>
#include "Telemetry.h"
#include <SBUS.h>
#include "RxLinkQuality.h"
#include "RPM.h"

unsigned long		timeLoopMicros = 0;								// Used to remember micros() at the start of the main loop.
unsigned long		lastLoopMicros = 0;								// actual loop time in micros(), time between FrSky Telemetry Sends must be less than 3600us
uint16_t				firstRunCounter = 0;							// counts the first loops
bool						firstRun = true;									// Resets to false when firstRunCounter hits its target


void setup() {
	// Start the USB serial for debugging
	Serial.begin(115200);
	delay(500);

	// Start RPM for Engine and Clutch sensor monitoring
	rpm_ActivateInterrupts();

	// Start the RX Link Quality monitoring
	rxLinkQuality_ActivateSBUS();

	// Start the Telemetry Sensors
	telemetry_ActivateTelemetry();

	Serial.println("Setup Complete");
	Serial.print("System Started millis() "); Serial.println(millis());
}



void loop() {

	// Create a loop timer
	// Must be at the start of the loop
	timeLoopMicros = micros();

	// Calculate the Current RPM data
	// updates variables mainRPMSensorDetectedRPM & clutchRPMSensorDetectedRPM
	// also updates variable inFlight
	calcualte_RPMSensorPulse();

	// Calculate the Rx Quality Indicators
	// updates variables lostFramesPercentage100Result & badFramesPercentage100Result
	// also updates variable totalFrames
	rxLinkQuality_Scan(firstRun);

	// calculate the time it took to run the loop.
	// it counts everything other than the time to send the Telemetry data
	lastLoopMicros = micros() - timeLoopMicros;

	if (firstRun == MIN_MAIN_LOOP_BEFORE_REPORTING_ERRORS && lastLoopMicros > MAX_MAIN_LOOP_TIME_BEFORE_ERROR) {
		Serial.print(millis()); Serial.print(": Long Loop @ "); Serial.print(lastLoopMicros); Serial.println("us");
	}

	// Format and Send the telemetry data using the FrSky S.Port solution
	telemetry_SendTelemetry();
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
