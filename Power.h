// Power.h

#ifndef _POWER_h
#define _POWER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Config
const int PIN_AMPS = A7;										// Analog - AMPs measurement to / from battery
const int PIN_REGULATOR_VOLTAGE = A2;				// Analog
const int PIN_BEC_VOLTAGE = A3;							// Analog

// Public Variables
extern float	reg, bec;											// Regulator and BEC voltages
extern float	dischargeTotalMAH;						// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging


// Public Functions
void power_Setup();


// Private Functions
void read_chargeVoltages();
void read_Amps_ASC714();

#endif

