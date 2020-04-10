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
#define MAX_TRIANGLE_DIFF_8CH_1  11      // for 75 >= badFramesPercentage100Result
#define MAX_TRIANGLE_DIFF_8CH_2  9       // for 75 <  badFramesPercentage100Result
#define MAX_TRIANGLE_DIFF_8CH_3  9       // for 50 <  badFramesPercentage100Result
#define TRSHLD_8CH_1_CHNG        75
#define TRSHLD_8CH_2_CHNG        50  
#define MAX_TRIANGLE_DIFF_16CH_1  19			// for 75 >= badFramesPercentage100Result
#define MAX_TRIANGLE_DIFF_16CH_2  18      // for 75 <  badFramesPercentage100Result
#define MAX_TRIANGLE_DIFF_16CH_3  17      // for 50 <  badFramesPercentage100Result
#define TRSHLD_16CH_1_CHNG        75
#define TRSHLD_16CH_2_CHNG        50  
#define MAX_16CH_ESTIMATED_BB_Bits 12			// over this the remainder will be added to the next frame in 16ch 1 wave mode.

// these parameters are mainly to stop weird values on first starting up which effect charting the data
#define SBUS_MIN_FRAME_RATE				4000		// If the SBUS frame is determined under this value it will be set to this
#define SBUS_MAX_FRAME_RATE				24000		// If the SBUS frame is determined under this value it will be set to this
#define SBUS_DEFAULT_FRAME_RATE		10000		// If below min or above max it will be set to this

// Public Variables
extern uint16_t lostFramesPercentage100Result;
extern uint16_t badFramesPercentage100Result;
extern uint32_t totalFrames;
extern uint16_t wave1;																		// Used to pass current value to telemetry
extern uint16_t wave2;																		// Used to pass current value to telemetry
extern uint32_t channelsMaxHoldMillis100Resul;						// Stores max millis() for every 100 readings
extern float		channel16chFrameSyncSuccessRate;					// Store the SBUS Frame Sync Success Rate when in 16ch mode, should be >98% based on X4R
extern uint16_t	sbusFrameLowMicros;											// Stores the SBUS Lowest time before next refresh over the last 100 frames
extern uint16_t	sbusFrameHighMicros;										// Stores the SBUS highest time before next refresh over the last 100 frames

// Public Functions
void rxLinkQuality_ActivateSBUS();
void rxLinkQuality_Scan(bool firstRun);


// Private Functions
void calculate_BB_Bits();
void calculate_FrameHolds();
void sync_16chFrame(bool firstRun);
void calculate_LostFrames();
void check_FailSafe();
void find_WaveChannel_New(byte &badFramesMonitoringChannel1, byte &badFramesMonitoringChannel2, byte &badFramesMonitoringType);
void debug_Data();
void debug_Wave_Data();
void sbus_FrameRate();