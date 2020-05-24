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
constexpr byte PIN_ERROR_LED = 5;						// Digital - Output Pin for Warning LEDs.


// Public Variables


// Public Functions
void _errorHandling_Setup();
void _errorHandling_checkErrors();


// Private Functions
void errorHandling_errorLEDsOn(bool status);

#endif  // .h #endif

