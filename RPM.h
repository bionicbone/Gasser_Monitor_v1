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

// Public Variables
extern uint32_t mainRPMSensorDetectedRPM;
extern uint32_t clutchRPMSensorDetectedRPM;
extern bool inFlight;

// Public Functions
void calcualte_RPMSensorPulse();
void onMainRPMSensorPulse();
void onClutchRPMSensorPulse();
void rpm_ActivateInterrupts();

// Private Functions


#endif  // .h #endif