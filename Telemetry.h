// Telemetry.h

#pragma once

#ifndef _TELEMETRY_h
#define _TELEMETRY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Config
constexpr auto SPORT_START = 0x7E;									// FrSky Start frame indicator
constexpr auto SPORT_HEADER_DATA = 0x10;						// FrSky Data frame indicator
constexpr auto SENSOR_GASSER_ID = 0xF2;							// (0xF2 on SPort Bus)
constexpr auto SPORT_HEADER_NODATA = 0x00;					// Signal no refresh required
constexpr auto SENSOR_FLVSS_ID = 0xA1;							// (0xA1 on SPort Bus)
constexpr auto SENSOR_FLVSS_DECODE_DELAY = 680;			// 680 allows a 708ms decode of the FLVSS data while ensuring the encoding rates of the telemetry data is not impacted
constexpr auto SMOOTH_MULTIPLIER = 0.25;						// used to smooth the cell value, 0.1 maxmum smoothing and 0.1 minimum smoothing
constexpr auto SMOOTH_OVER_RIDE = 0.3;							// Over rides the smooth function and follow new value istantly if the change is more than this


// Public Variables
extern float				 cell[6];												// Final cell voltage store
extern float cellSmoothed[6];												// Final cell voltage after smoothing
extern unsigned long	sensorRefreshRate;						// Sensor refresh rate
extern unsigned long	FLVSSRefreshRate;							// Sensor refresh rate


// Public Functions
void _telemetry_ActivateTelemetry();
void _telemetry_SendTelemetry();


// Private Functions
void updateValue(byte sensorNumber);
void sendValueData(byte val);
void sendByte(byte b);
void sendFrame();
void getFLVSSdata();
void smoothFLVXXdata(byte cellArrayNumber);

#endif  // .h #endif