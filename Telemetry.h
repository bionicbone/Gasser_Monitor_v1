// Telemetry.h

#pragma once

#ifndef _TELEMETRY_h
#define _TELEMETRY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#endif


// Links to "extern" variables
#include "RPM.h"
#include "RxLinkQuality.h"



// Public Variables
// none


// Public Functions
void telemetry_ActivateTelemetry();
void telemetry_SendTelemetry();


// Private Functions
void NewValueSport(byte val);
void sendByte(byte b);
void sendFrame();
void updateValue(byte sensorNumber);

