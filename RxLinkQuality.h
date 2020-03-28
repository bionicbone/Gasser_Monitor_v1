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

// These setting control the "badFrames" calculation and were defined by Reinhard (RC Groups) to make BB_Bits
// Calculate to be very similar to the LF flag under FrSky Firmware v1.
// dynamic threshold for LostFrameCtr_V2 increment (depends on slope rate of triangle (here for 2s))
#define MAX_TRIANGLE_DIFF_8CH_1 11       // for 75 >= badFramesPercentage100Result
#define MAX_TRIANGLE_DIFF_8CH_2  9       // for 75 <  badFramesPercentage100Result
#define MAX_TRIANGLE_DIFF_8CH_3  9       // for 50 <  badFramesPercentage100Result
#define TRSHLD_8CH_1_CHNG        75
#define TRSHLD_8CH_2_CHNG        50  


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