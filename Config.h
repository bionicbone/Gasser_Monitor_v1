// config.h

#pragma once


#ifndef _CONFIG_h
#define _CONFIG_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


const uint16_t	MIN_MAIN_LOOP_BEFORE_REPORTING_ERRORS = 5000;		// Must do x loops before triggering any loop errors
const uint16_t	MAX_MAIN_LOOP_TIME_BEFORE_ERROR = 100;					// Value triggers the Long Loop Error - May need to increase


// DEVELOPER - Program debug options

//#define DEBUG_FS_LF_ERRORS																	// Activate to report Rx FailSafe and LostFrame Flags on USB Serial
//#define DEBUG_ALL_CHANNEL_DATA															// Activate to display all SBUS channels on USB Serial
//#define DEBUG_WAVE_CHANNEL_DATA															// Activate to display the wave channel(s) & BFP value on USB Serial
//#define DEBUG_BAD_FRAME_ERRORS															// Activate to display the BB_Bits (calculated lost frames) on USB Serial
//#define DEBUG_CURRENT_BFP																		// Activate constent monitoring of BFP value on USB Serial
//#define DEBUG_SBUS_16CH_FRAME_SYNC_DATA											// Activate to display detailed SBUS Frame Sync information
//#define DEBUG_CHANNEL_HOLD_DATA															// Activate to display detailed SBUS Channel Hold information
//#define DEBUG_SBUS_FRAME_TIME																// Activate to display detailed SBUS Frame Timing information
//#define DEBUG_E2E_OVERALL_QUALITY														// Activate to display the individual results that make up the End to End QI  Value
//#define DEBUG_LM35_TEMPERATURE_READINGS											// Activate to display LM35 Temperature information
//#define DEBUG_TELEMETRY_REFRESH_RATES												// Activate to display the refresh rates of the Values and FLVSS decoding
//#define DEBUG_FLVSS_CALCULATION															// Activate to display how the Cell voltages are being calculated from the FLVSS sensor
//#define DEBUG_ASC714_BATTERY_AMPS_CALCULATION								// Activate to display how the AMPS are being calculated from the ASC714 hall effect sensor
//#define DEBUG_ASC713_BEC_AMPS_CALCULATION										// Activate to display how the AMPS are being calculated from the ASC713 hall effect sensor
//#define DEBUG_ASC712_BEC_AMPS_CALCULATION										// Activate to display how the AMPS are being calculated from the ASC712 hall effect sensor
//#define DEBUG_ASC712_BATTERY_AMPS_CALCULATION								// Activate to display how the AMPS are being calculated from the ASC712 hall effect sensor


#if defined (DEBUG_FS_LF_ERRORS)
#error DONT FLASH WITH DEBUG_FS_LF_ERRORS ACTIVATED
#endif

#if defined (DEBUG_ALL_CHANNEL_DATA)
#error DONT FLASH WITH DEBUG_ALL_CHANNEL_DATA ACTIVATED
#endif

#if defined (DEBUG_WAVE_CHANNEL_DATA)
#error DONT FLASH WITH DEBUG_WAVE_CHANNEL_DATA ACTIVATED
#endif


#if defined (DEBUG_BAD_FRAME_ERRORS)
#error DONT FLASH WITH DEBUG_BAD_FRAME_ERRORS ACTIVATED
#endif

#if defined (DEBUG_CURRENT_BFP)
#error DONT FLASH WITH DEBUG_CURRENT_BFP ACTIVATED
#endif

#if defined (DEBUG_SBUS_16CH_FRAME_SYNC_DATA)
#error DONT FLASH WITH DEBUG_SBUS_16CH_FRAME_SYNC_DATA ACTIVATED
#endif

#if defined (DEBUG_CHANNEL_HOLD_DATA)
#error DONT FLASH WITH DEBUG_CHANNEL_HOLD_DATA ACTIVATED
#endif

#if defined (DEBUG_SBUS_FRAME_TIME)
#error DONT FLASH WITH DEBUG_SBUS_FRAME_TIME ACTIVATED
#endif

#if defined (DEBUG_E2E_OVERALL_QUALITY)
#error DONT FLASH WITH REPORT_SBUS_OVERALL_QUALITY ACTIVATED
#endif

#if defined (DEBUG_LM35_TEMPERATURE_READINGS)
#error DONT FLASH WITH DEBUG_LM35_TEMPERATURE_READINGS ACTIVATED
#endif

#if defined (DEBUG_TELEMETRY_REFRESH_RATES)
#error DONT FLASH WITH DEBUG_TELEMETRY_REFRESH_RATES ACTIVATED
#endif

#if defined (DEBUG_FLVSS_CALCULATION)
#error DONT FLASH WITH DEBUG_FLVSS_CALCULATION ACTIVATED
#endif

#if defined (DEBUG_ASC714_BATTERY_AMPS_CALCULATION)
#error DONT FLASH WITH DEBUG_ASC714_BATTERY_AMPS_CALCULATION ACTIVATED
#endif

#if defined (DEBUG_ASC713_BEC_AMPS_CALCULATION)
#error DONT FLASH WITH DEBUG_ASC713_BEC_AMPS_CALCULATION ACTIVATED
#endif

#if defined (DEBUG_ASC712_BEC_AMPS_CALCULATION)
#error DONT FLASH WITH DEBUG_ASC713_BEC_AMPS_CALCULATION ACTIVATED
#endif

#if defined (DEBUG_ASC712_BATTERY_AMPS_CALCULATION)
#error DONT FLASH WITH DEBUG_ASC713_BEC_AMPS_CALCULATION ACTIVATED
#endif

#endif  // .h #endif