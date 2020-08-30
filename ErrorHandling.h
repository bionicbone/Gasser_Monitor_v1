// ErrorHandling.h

#pragma once

#ifndef _ERRORHANDLING_h
#define _ERRORHANDLING_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


// Config
constexpr byte PIN_ERROR_LED = 5;													// Digital - Output Pin for Warning LEDs.
constexpr int ERROR_MIN_NEXT_ERROR_REPORT_TIME = 1000;		// in 1000's of a second, i.e. 500 = 0.5 seconds.
constexpr int ERROR_MIN_ALL_ERRORS_LOOP_TIME = 5000;			// in 1000's of a second, i.e. 5000 = 5 seconds.
constexpr auto ERROR_MIN_CELL_VOLTAGE = 3.30;
constexpr auto ERROR_MAX_CELL_VOLTAGE = 3.45;
constexpr auto ERROR_MAX_CELL_DIFFERENCE = 0.05;
constexpr auto ERROR_MIN_BATTERY_VOLTAGE = 6.6;
constexpr auto ERROR_MAX_BATTERY_VOLTAGE = 6.9;
constexpr auto ERROR_CHARGING_MIN_RPM = 9000;
constexpr auto ERROR_CHARGING_MIN_RECTIFIER_VOLTAGE = 9.0;
constexpr auto ERROR_CHARGING_MAX_RECTIFIER_VOLTAGE = 11.5;
constexpr auto ERROR_CHARGING_MIN_BEC_VOLTAGE = 6.6;
constexpr auto ERROR_CHARGING_MAX_BEC_VOLTAGE = 7.0;
constexpr auto ERROR_MIN_BEC_TEMPERATURE = 4.0;
constexpr auto ERROR_MAX_BEC_TEMPERATURE = 50.0;
constexpr auto ERROR_MIN_CANOPY_TEMPERATURE = 4.0;
constexpr auto ERROR_MAX_CANOPY_TEMPERATURE = 50.0;
constexpr auto ERROR_MIN_ENGINE_TEMPERATURE = 4.0;
constexpr auto ERROR_MAX_ENGINE_TEMPERATURE = 170.0;

// Public Variables
extern uint32_t _error;
extern uint32_t _error1;


// Public Functions
void _errorHandling_Setup();
void _errorHandling_checkErrors();


// Private Functions
void errorHandling_errorLEDsOn(bool status);

#endif  // .h #endif

