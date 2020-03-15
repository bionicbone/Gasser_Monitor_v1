// RPM.h

#pragma once

#ifndef _RPM_h
#define _RPM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

extern uint32_t mainRPMSensorDetectedRPM;
extern uint32_t clutchRPMSensorDetectedRPM;
extern bool inFlight;


void calcualte_RPMSensorPulse();
void onMainRPMSensorPulse();
void onClutchRPMSensorPulse();
void rpm_ActivateInterrupts();