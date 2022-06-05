// Temperature.h

#pragma once

#ifndef _TEMPERATURE_h
#define _TEMPERATURE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


// Config
constexpr auto TEMPERATURE_READINGS_FOR_AVERAGE = 50;						// take x temperature reading and calculate the average
constexpr float AMBIENT_TEMP_CALIBRATION = -0.48;								// LM35's are never aligned, tweaking to get similar in ambient conditions is a good idea
constexpr float CANOPY_TEMP_CALIBRATION = -0.13;								// LM35's are never aligned, tweaking to get similar in ambient conditions is a good idea
constexpr float ENGINE_TEMP_CALIBRATION = 0.41;									// LM35's are never aligned, tweaking to get similar in ambient conditions is a good idea
constexpr float BEC_TEMP_CALIBRATION = 0.00;										// LM35's are never aligned, tweaking to get similar in ambient conditions is a good idea

// Pins for the LM35 Temperature Probes
constexpr auto  PIN_CANOPY_TEMPERATURE_LM35 = 26;								// Pin number for the canopy LM35 sensor
constexpr auto PIN_AMBIENT_TEMPERATURE_LM35 = 27;								// Pin number for the ambient LM35 sensor
constexpr auto PIN_ENGINE_TEMPERATURE_LM35 = 28;								// Pin number for the engine / ESC LM35 sensor
constexpr auto PIN_BEC_TEMPERATURE_LM35 = 31;										// Pin number for the BEC sensor 

// Public Variables
extern float _ambientTemp;			// NOTE: These are float values on purpose so we can detect disconnected sensors
extern float _canopyTemp;				// NOTE: These are float values on purpose so we can detect disconnected sensors
extern float _engineTemp;				// NOTE: These are float values on purpose so we can detect disconnected sensors
extern float _becTemp;					// NOTE: These are float values on purpose so we can detect disconnected sensors

// Public Functions
void _temperatures_Read();
void _temperature_Setup();

// Private Functions



#endif  // .h #endif
