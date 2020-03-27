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
extern uint16_t lostFramesPercentage100Result;
extern uint16_t badFramesPercentage100Result;
extern uint32_t totalFrames;
extern byte badFramesMonitoringChannel1;
extern byte badFramesMonitoringChannel2;
extern uint32_t failSafeCounter;
extern uint32_t failSafeLongestMillis;

// Public Functions
void rxLinkQuality_ActivateSBUS();
void rxLinkQuality_Scan();


// Private Functions
void calculate_BadFrames();
void calculate_LostFrames();
void check_FailSafe();
void find_WaveChannel_New(byte &badFramesMonitoringChannel1, byte &badFramesMonitoringChannel2, byte &badFramesMonitoringType);
void debug_Data();
void debug_Wave_Data();