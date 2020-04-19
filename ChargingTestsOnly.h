// ChargingTestsOnly.h

#pragma once

#ifndef _CHARGINGTESTSONLY_h
#define _CHARGINGTESTSONLY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//Config
constexpr auto PIN_CHARGE_RELAY = 8;
constexpr auto PIN_DISCHARGE_RELAY = 6;
constexpr auto PIN_PULSE_DISCHARGE_MOSFET = 7;

//Public Variables


//Private Variables


// Public Processes
void chargingTestOnly_Setup();
void chargingTestOnly_Control();


//Private Processes
void chargeBattery();
void discharge_mAh(float mAh);
void pulseDischargeReading(byte attempts, int milliseconds, char stabilise);
void stabiliseVoltages(byte howStable, float accuracy, bool reportEvery30Seconds);
void storeDataInArrays(byte cycleType);
void chargeRelay_Status(bool status);
void dischargeRelay_Status(bool status);
void dischargeMosfet_Status(bool status);





#endif

