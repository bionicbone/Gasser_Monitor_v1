// Telemetry.h

#pragma once

#ifndef _TELEMETRY_h
#define _TELEMETRY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


// Links to "extern" variables


// Public Variables
extern float				 cell[6];											// Final cell voltage store
extern float cellSmoothed[6];											// Final cell voltage after smoothing
extern unsigned long	sensorRefreshRate;					// Sensor refresh rate
extern unsigned long	FLVSSRefreshRate;						// Sensor refresh rate


// Public Functions
void telemetry_ActivateTelemetry();
void telemetry_SendTelemetry();


// Private Functions
void updateValue(byte sensorNumber);
void sendValueData(byte val);
void sendByte(byte b);
void sendFrame();
void getFLVSSdata();
void smoothFLVXXdata(byte cellArrayNumber);

#endif  // .h #endif