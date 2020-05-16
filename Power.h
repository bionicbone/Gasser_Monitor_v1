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
const int PIN_BEC_AMPS = A8;								// Analog - AMPs measurement from BEC
const int PIN_TEENSY_VOLTAGE = A9;					// Analog - Checks the TEENSY Voltage is a good 5v otherwise calculations are off
const int PIN_RECTIFIER_VOLTAGE = A2;				// Analog
const int PIN_BEC_VOLTAGE = A3;							// Analog

// Public Variables
extern float	teensyVoltage, recVoltage, becVoltage;	// Teensy 5v line, Regulator and BEC voltages
extern float	batteryDischargeTotalMAH;								// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
extern float	batteryDischargeLoopAmps;								// The current AMPS measured when function called
extern float	batteryDischargeLoopMAH;								// The last MAH used during the loop
extern float	becDischargeTotalMAH;										// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
extern float	becDischargeLoopAmps;										// The current AMPS measured when function called
extern float	becDischargeLoopMAH;										// The last MAH used during the loop

// Public Functions
void power_Setup();
void power_ReadSensors();

// Private Functions
void power_chargeVoltages();
void power_Battery_Amps_ASC714();
void power_BEC_Amps_ASC713();
void power_BEC_Amps_ASC712();
void power_Battery_Amps_ASC712();

#endif

