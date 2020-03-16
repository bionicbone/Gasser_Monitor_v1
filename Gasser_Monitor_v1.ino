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

#include <FrSkySportDecoder.h>
#include "Telemetry.h"
#include <SBUS.h>
#include "RxLinkQuality.h"
#include "RPM.h"

//#define REPORT_ERRORS


void setup() {
	Serial.begin(9600);
	rpm_ActivateInterrupts(); 
	
	rxLinkQuality_ActivateSBUS();


	void telemetry_ActivateTelemetry();
	Serial.println("Setup Complete");
}


void loop() {
	calcualte_RPMSensorPulse();
	rxLinkQuality_Scan();


	telemetry_SendTelemetry();



}


void stop() {
	while (1);
}
