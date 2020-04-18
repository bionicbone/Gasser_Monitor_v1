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


// Public Functions
void power_Setup();

// Private Functions
void read_chargeVoltages();
void read_Amps_ASC714();

#endif

