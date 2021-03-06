// RxLinkQuality.h

#pragma once

#ifndef _RXLINKQUALITY_h
#define _RXLINKQUALITY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include "SBUS.h"

// Config
constexpr auto MAX_WAIT_TIME_MS = 200;										// Maximum wait time for a valid SBUS signal when activating SBUS or looking for a valid wave

// These setting control the "badFrames" calculation and were defined by Reinhard (RC Groups) to make BB_Bits
// Calculate to be very similar to the LF flag under FrSky Firmware v1.
// dynamic threshold for LostFrameCtr_V2 increment (depends on slope rate of triangle (here for 2s))
constexpr auto MAX_TRIANGLE_DIFF_8CH_1 = 11;			// for 75 >= badFramesPercentage100Result
constexpr auto MAX_TRIANGLE_DIFF_8CH_2 = 9;				// for 75 <  badFramesPercentage100Result
constexpr auto MAX_TRIANGLE_DIFF_8CH_3 = 9;				// for 50 <  badFramesPercentage100Result
constexpr auto TRSHLD_8CH_1_CHNG = 75;
constexpr auto TRSHLD_8CH_2_CHNG = 50;
constexpr auto MAX_TRIANGLE_DIFF_16CH_1 = 19;			// for 75 >= badFramesPercentage100Result
constexpr auto MAX_TRIANGLE_DIFF_16CH_2 = 18;			// for 75 <  badFramesPercentage100Result
constexpr auto MAX_TRIANGLE_DIFF_16CH_3 = 17;			// for 50 <  badFramesPercentage100Result
constexpr auto TRSHLD_16CH_1_CHNG = 75;
constexpr auto TRSHLD_16CH_2_CHNG = 50;
constexpr auto MAX_16CH_ESTIMATED_BB_Bits = 12;		// over this the remainder will be added to the next frame in 16ch 1 wave mode.

// these parameters are mainly to stop weird values on first starting up which effect charting the data
//constexpr auto SBUS_MIN_FRAME_RATE = 4000;				// If the SBUS frame is determined under this value it will be set to this
//constexpr auto SBUS_MAX_FRAME_RATE = 24000;				// If the SBUS frame is determined under this value it will be set to this
//constexpr auto SBUS_DEFAULT_FRAME_RATE = 10000;		// If below min or above max it will be set to this

// these values are for controling the overall End to End quality indicator result
//constexpr auto E2E_QI_RATE_ALLOWED_INCREASE = 250;	// SBUS Frame can the late by this value in us before it effects the QI value
//constexpr auto E2E_QI_RATE_DIVIDOR = 40;						// Result divisor 
constexpr auto E2E_QI_LOSTFRAME_ALLOWED_MIN = 98;		// LostFrame% can drop to this level before it effects the QI value
constexpr auto E2E_QI_LOSTFRAME_MULTIPLIER = 20;		// Result multiplier
constexpr auto E2E_QI_BADFRAME_ALLOWED_MIN = 75;		// BadFrame% can drop to this level before it effects the QI value
constexpr auto E2E_QI_BADFRAME_MULTIPLIER = 3;			// Result multiplier
constexpr auto E2E_QI_16CHSYNC_ALLOWED_MIN = 97;		// BadFrame% can drop to this level before it effects the QI value
constexpr auto E2E_QI_16CHSYNC_MULTIPLIER = 20;			// Result multiplier
constexpr auto E2E_QI_FRAMEHOLD_ALLOWED_MAX = 48;		// Frame Holds can increase to this level in ms before it effects the QI value


// Public Variables
extern uint16_t _lostFramesPercentage100Result;			// The final % value for lost frames calculated using the BB_Bits Method
extern uint16_t _badFramesPercentage100Result;			// The final % value for bad frames calculated using the BB_Bits Method
extern uint32_t _totalFrames;												// Total number of SBUS frames processed
extern uint16_t _wave1;															// Used to pass current value to telemetry
extern uint16_t _wave2;															// Used to pass current value to telemetry
extern uint32_t _channelsMaxHoldMillis100Resul;			// Stores max millis() for every 100 readings
extern float		_channel16chFrameSyncSuccessRate;		// Store the SBUS Frame Sync Success Rate when in 16ch mode, should be >98% based on X4R
extern int8_t		_overallE2EQuality;									// A calculation that includes lostFrames%, BadFrames%, Ch16%, SbusFrameRate, ChMaxHold, failSafe to give 0-100 quality indicator


// Public Functions
void _rxLinkQuality_ActivateSBUS();
void _rxLinkQuality_Scan(bool firstRun);


// Private Functions
void calculate_BB_Bits();
void calculate_FrameHolds();
void sync_16chFrame(bool firstRun);
void calculate_LostFrames();
void check_FailSafe();
void find_WaveChannel_New(byte &badFramesMonitoringChannel1, byte &badFramesMonitoringChannel2, byte &badFramesMonitoringType);
void debug_Data();
void debug_Wave_Data();
void calculate_Overall_EndToEnd_Quality();


#endif  // .h #endif