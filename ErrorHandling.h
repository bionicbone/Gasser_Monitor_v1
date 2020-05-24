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
const byte PIN_ERROR_LED = 5;						// Digital - Output Pin for Warning LEDs.


// Public Variables


// Public Functions
void errorHandling_Setup();
void errorHandling_checkErrors();
void errorHandling_errorLEDsOn(bool status);


// Private Functions


#endif  // .h #endif

