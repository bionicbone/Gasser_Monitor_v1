// RxLinkQuality.h

#pragma once

#ifndef _RXLINKQUALITY_h
#define _RXLINKQUALITY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
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

// these values are for controling the overall End to End quality indicator result
#define E2E_QI_RATE_ALLOWED_INCREASE	250	// SBUS Frame can the late by this value in us before it effects the QI value
#define E2E_QI_RATE_DIVIDOR						40	// Result divisor 
#define E2E_QI_LOSTFRAME_ALLOWED_MIN	98	// LostFrame% can drop to this level before it effects the QI value
#define E2E_QI_LOSTFRAME_MULTIPLIER		20	// Result multiplier
#define E2E_QI_BADFRAME_ALLOWED_MIN		75	// BadFrame% can drop to this level before it effects the QI value
#define E2E_QI_BADFRAME_MULTIPLIER		3		// Result multiplier
#define E2E_QI_16CHSYNC_ALLOWED_MIN		97	// BadFrame% can drop to this level before it effects the QI value
#define E2E_QI_16CHSYNC_MULTIPLIER		20	// Result multiplier
#define E2E_QI_FRAMEHOLD_ALLOWED_MAX	48	// Frame Holds can increase to this level in ms before it effects the QI value


// Public Variables
extern uint16_t lostFramesPercentage100Result;
extern uint16_t badFramesPercentage100Result;
extern uint32_t totalFrames;
extern uint16_t wave1;																		// Used to pass current value to telemetry
extern uint16_t wave2;																		// Used to pass current value to telemetry
extern uint32_t channelsMaxHoldMillis100Resul;						// Stores max millis() for every 100 readings
extern float		channel16chFrameSyncSuccessRate;					// Store the SBUS Frame Sync Success Rate when in 16ch mode, should be >98% based on X4R
extern uint16_t	sbusFrameLowMicros;												// Stores the SBUS Lowest time before next refresh over the last 100 frames
extern uint16_t	sbusFrameHighMicros;											// Stores the SBUS highest time before next refresh over the last 100 frames
extern int8_t		overallE2EQuality;												// A calculation that includes lostFrames%, BadFrames%, Ch16%, SbusFrameRate, ChMaxHold, failSafe to give 0-100 quality indicator


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
void calculate_Overall_EndToEnd_Quality();


#endif  // .h #endif