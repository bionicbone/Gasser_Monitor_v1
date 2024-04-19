// RPM.h

#pragma once

#ifndef _RPM_h
#define _RPM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Config
constexpr byte	RPM_AVERAGE_DIVIDER = 8;
const uint32_t	MAX_UPDATE_TIME_MS = 250000;
const uint8_t		PIN_MAIN_RPM_SENSOR_INTERRUPT_0 = 2;			// Digital Input Pin for RPM Sensor (A3144 Hall Effect Sensor)
const uint8_t		PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1 = 3;		// Digital Input Pin for RPM Sensor (A3144 Hall Effect Sensor)
const float			HEAD_GEAR_RATIO = 6.765;									// Used to calculate the Blade RPM and is the gear ratio on the heli

// Public Variables
extern uint32_t _mainRPMSensorDetectedRPM;
extern uint32_t _clutchRPMSensorDetectedRPM;
extern bool			_inFlight;
extern uint32_t _headRPMCalculatedRPM;

// Public Functions
void _rpm_calculate_SensorPulse();
void _rpm_ActivateInterrupts();

// Private Functions
void onMainRPMSensorPulse();
void onClutchRPMSensorPulse();

#endif  // .h #endif