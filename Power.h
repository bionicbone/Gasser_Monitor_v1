// Power.h

#pragma once

#ifndef _POWER_h
#define _POWER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Config
const int PIN_BATTERY_AMPS = A7;						// Analog - AMPs measurement to / from battery
const int PIN_REGULATOR_VOLTAGE = A2;				// Analog
const int PIN_BEC_VOLTAGE = A3;							// Analog

// Public Variables
extern float	reg, bec;											// Regulator and BEC voltages
extern float	batteryDischargeTotalMAH;						// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
extern float	batteryDischargeLoopAmps;
extern float	batteryDischargeLoopMAH;					// The last MAH used during the loop

// Public Functions
void power_Setup();
void power_chargeVoltages();
void power_Battery_Amps_ASC714();

// Private Functions


#endif

