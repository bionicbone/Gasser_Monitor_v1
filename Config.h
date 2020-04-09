#pragma once


// DEVELOPER - Program debug options

//#define REPORT_FS_LF_ERRORS										// Activate to report Rx FailSafe and LostFrame Flags on USB Serial
//#define DEBUG_ALL_CHANNEL_DATA								// Activate to display all SBUS channels on USB Serial
//#define DEBUG_WAVE_CHANNEL_DATA								// Activate to display the wave channel(s) & BFP value on USB Serial
//#define REPORT_BAD_FRAME_ERRORS								// Activate to display the BB_Bits (calculated lost frames) on USB Serial
//#define REPORT_CURRENT_BFP										// Activate constent monitoring of BFP value on USB Serial
//#define REPORT_SBUS_16CH_FRAME_SYNC_DATA			// Activate to display detailed SBUS Frame Sync information
//#define REPROT_CHANNEL_HOLD_DATA							// Activate to display detailed SBUS Channel Hold information
//#define REPROT_SBUS_FRAME_TIME								// Activate to display detailed SBUS Frame Timing information

#if defined (REPORT_FS_LF_ERRORS)
#error DONT FLASH WITH REPORT_FS_LF_ERRORS ACTIVATED
#endif

#if defined (DEBUG_ALL_CHANNEL_DATA)
#error DONT FLASH WITH DEBUG_ALL_CHANNEL_DATA ACTIVATED
#endif

#if defined (DEBUG_WAVE_CHANNEL_DATA)
#error DONT FLASH WITH DEBUG_WAVE_CHANNEL_DATA ACTIVATED
#endif


#if defined (REPORT_BAD_FRAME_ERRORS)
#error DONT FLASH WITH REPORT_BAD_FRAME_ERRORS ACTIVATED
#endif

#if defined (REPORT_CURRENT_BFP)
#error DONT FLASH WITH REPORT_CURRENT_BFP ACTIVATED
#endif

#if defined (REPORT_SBUS_16CH_FRAME_SYNC_DATA)
#error DONT FLASH WITH REPORT_SBUS_16CH_FRAME_SYNC_DATA ACTIVATED
#endif

#if defined (REPROT_CHANNEL_HOLD_DATA)
#error DONT FLASH WITH REPROT_CHANNEL_HOLD_DATA ACTIVATED
#endif

#if defined (REPROT_SBUS_FRAME_TIME)
#error DONT FLASH WITH REPROT_SBUS_FRAME_TIME ACTIVATED
#endif