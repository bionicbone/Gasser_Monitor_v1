// MCU6050.h

#pragma once

#ifndef _MCU6050_h
#define _MCU6050_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Config
const int MPU_addr = 0x68;											// I2C address of the MPU-6050
const int MCU_STABILITY_DELAY = 10000;					// if too low min/max values may not be stable. DEFAULT 10000
const int MCU_WAKE_AcX_ALLOWED_DIFF = 10000;		// If value greater for x times in a row then signal wake up
const int MCU_WAKE_AcY_ALLOWED_DIFF = 10000;		// If value greater for x times in a row then signal wake up
const int MCU_WAKE_AcZ_ALLOWED_DIFF = 500;			// If value greater for x times in a row then signal wake up
const int MCU_ENGINE_AcX_ALLOWED_DIFF = 18000;	// If value greater for x times in a row then signal engine running
const int MCU_ENGINE_AcY_ALLOWED_DIFF = 21700;	// If value greater for x times in a row then signal engine running
const int MCU_ENGINE_AcZ_ALLOWED_DIFF = 4300;		// If value greater for x times in a row then signal engine running
const int WAKE_UP_TIME = 10000;									// Amount of time to wake up for, sleeps after this.


// Public Variables


// Public Functions
void _mcu6050_Setup();
void _mcu6050_Read();


// Private Functions



#endif

