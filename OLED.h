// OLED.h

#pragma once

#ifndef _OLED_h
#define _OLED_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//Config
const int PIN_OLED_DC = 24;
const int PIN_OLED_RES = 25;
const int PIN_OLED_D1 = 32;
const int PIN_OLED_D0 = 33;
const int PIN_OLED_CS = 4;                            // Not Physically Connected
const float OLED_CRITICAL_VOLTAGE_WARNING = 3.15;
const float OLED_OVERCHARGED_VOLTAGE_WARNING = 3.55;

//Public Variables



// Public Functions
void _oled_Setup();
void _oled_FlightBatteryVoltage();
void _oled_Device_Name();
void _oled_DateAndTime();
void _oled_inFlight();


//Private Functions
int percentcell(float targetVoltage);
void drawBattery(int cellNumber, float cellVoltage, int x, int y);

#endif  //.h #endif

