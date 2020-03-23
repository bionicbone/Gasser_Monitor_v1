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
extern byte badFramesMonitoringChannel;


// Public Functions
void rxLinkQuality_ActivateSBUS();
void rxLinkQuality_Scan();


// Private Functions
void calculate_BadFrames();
void calculate_LostFrames();
byte find_WaveChannel();
void debug_Data();
