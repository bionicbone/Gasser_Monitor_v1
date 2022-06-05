// SdCard.h

#pragma once

#ifndef _SDCARD_h
#define _SDCARD_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Public Functions

void _sd_SetUp();
void _sd_WriteLogDate();


// Private Functions

void sd_WriteLogHeader();

#endif

