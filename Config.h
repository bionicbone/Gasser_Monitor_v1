#pragma once


// DEVELOPER - Program debug options

//#define REPORT_FS_LF_ERRORS										// Activate to report Rx FailSafe and LostFrame Flags on USB Serial
//#define DEBUG_ALL_CHANNEL_DATA								// Activate to display all SBUS channels on USB Serial
//#define DEBUG_WAVE_CHANNEL_DATA								// Activate to display the wave channel(s) & BFP value on USB Serial
#define REPORT_BAD_FRAME_ERRORS								// Activate to display the BB_Bits (calculated lost frames) on USB Serial
#define REPORT_CURRENT_BFP										// Activate constent monitoring of BFP value on USB Serial


#if defined (REPORT_FS_LF_ERRORS)
#warning DONT FLASH WITH REPORT_FS_LF_ERRORS ACTIVATED
#endif

#if defined (DEBUG_ALL_CHANNEL_DATA)
#warning DONT FLASH WITH DEBUG_ALL_CHANNEL_DATA ACTIVATED
#endif

#if defined (DEBUG_WAVE_CHANNEL_DATA)
#warning DONT FLASH WITH DEBUG_WAVE_CHANNEL_DATA ACTIVATED
#endif


#if defined (REPORT_BAD_FRAME_ERRORS)
#warning DONT FLASH WITH REPORT_BAD_FRAME_ERRORS ACTIVATED
#endif

#if defined (REPORT_CURRENT_BFP)
#warning DONT FLASH WITH REPORT_CURRENT_BFP ACTIVATED
#endif