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
const int MCU_WAKE_AccX_ALLOWED_DIFF = 10000;		// If value greater for x times in a row then signal wake up
const int MCU_WAKE_AccY_ALLOWED_DIFF = 10000;		// If value greater for x times in a row then signal wake up
const int MCU_WAKE_AccZ_ALLOWED_DIFF = 500;			// If value greater for x times in a row then signal wake up
const int MCU_ENGINE_AccX_ALLOWED_DIFF = 18000;	// If value greater for x times in a row then signal engine running
const int MCU_ENGINE_AccY_ALLOWED_DIFF = 21700;	// If value greater for x times in a row then signal engine running
const int MCU_ENGINE_AccZ_ALLOWED_DIFF = 4300;	// If value greater for x times in a row then signal engine running
const int WAKE_UP_TIME = 10000;									// Amount of time to wake up for, sleeps after this.


// Public Variables
extern int _AccX, _AccY, _AccZ, _AccTmp, _GyrX, _GyrY, _GyrZ;	// MCU-6050 values
extern byte _vibrationStatus;																	// MCU-6050 status based on values, 0 = Low to zero vibration, 1 = Wake Up OLED, 2 = Engine Running, 3 = High Vibration, 4 = Extreme Vibration


// Public Functions
void _mcu6050_Setup();
void _mcu6050_Read();


// Private Functions



#endif

