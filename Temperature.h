// Temperature.h

#pragma once

#ifndef _TEMPERATURE_h
#define _TEMPERATURE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif


// Config
constexpr auto TEMPERATURE_READINGS_FOR_AVERAGE					= 50;
constexpr auto PROG_DELAY_BEFORE_FLIGHT_STAT_UPDATES		= 7500;		// if too low min/max values may not be stable. DEFAULT 7500;
constexpr auto ERROR_MIN_CANOPY_TEMPERATURE = 2;
constexpr auto ERROR_MAX_CANOPY_TEMPERATURE = 35;
constexpr auto ERROR_MIN_ENGINE_TEMPERATURE = 2;
constexpr auto ERROR_MAX_ENGINE_TEMPERATURE = 149;
constexpr auto ERROR_MIN_AMBIENT_TEMPERATURE = 2;
constexpr auto ERROR_MAX_AMBIENT_TEMPERATURE = 28;
// TODO - Add Min / Max for BEC temperature

// Pins for the LM35 Temperature Probes
#define PIN_CANOPY_TEMPERATURE_LM35				26						// Pin number for the canopy LM35 sensor
#define PIN_AMBIENT_TEMPERATURE_LM35			27						// Pin number for the ambient LM35 sensor
#define PIN_ENGINE_TEMPERATURE_LM35				28						// Pin number for the engine / ESC LM35 sensor
#define PIN_BEC_TEMPERATURE_LM35					31						// Pin number for the BEC sensor 

// Public Variables
extern float ambientTemp;			// NOTE: These are float values on purpose so we can detect disconnected sensors
extern float canopyTemp;			// NOTE: These are float values on purpose so we can detect disconnected sensors
extern float engineTemp;			// NOTE: These are float values on purpose so we can detect disconnected sensors
extern float becTemp;					// NOTE: These are float values on purpose so we can detect disconnected sensors

// Public Functions
void read_temperatures();
void temperature_Setup();

// Private Functions