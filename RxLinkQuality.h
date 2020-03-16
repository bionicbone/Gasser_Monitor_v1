// RxLinkQuality.h

#pragma once

#ifndef _RXLINKQUALITY_h
#define _RXLINKQUALITY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

// Links to "extern" variables
#include "SBUS.h"


// Public Variables
extern float lostFramesPercentage100Result;
extern float badFramesPercentage100Result;
extern uint32_t totalFrames;


// Public Functions
void rxLinkQuality_ActivateSBUS();
void report_WaveChannelsError(byte badFramesMonitoringChannel);


// Private Functions
void calculate_BadFrames();
void calculate_LostFrames();
byte find_WaveChannel();
