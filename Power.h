// Power.h

#pragma once

#ifndef _POWER_h
#define _POWER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Calibration
constexpr float BAT_VOLTAGE_MULTIPLIER = 7.792053295;				// Follow Calibration Process
constexpr float TEENSY_VOLTAGE_MULTIPLIER = 1.7464;					// Follow Calibration Process
constexpr float REC_VOLTAGE_MULTIPLIER = 16.18122977;				// Follow Calibration Process
constexpr float BEC_VOLTAGE_MULTIPLIER = 7.853345148;				// Follow Calibration Process  


// Config
constexpr int		PIN_BATTERY_VOLTAGE = A6;										// Analog - FLight Battery Voltage
constexpr int		PIN_BATTERY_AMPS = A7;											// Analog - AMPs measurement to / from battery
constexpr int		PIN_BEC_AMPS = A8;													// Analog - AMPs measurement from BEC
constexpr int		PIN_TEENSY_VOLTAGE = A9;										// Analog - TEENSY Voltage 
constexpr int		PIN_RECTIFIER_VOLTAGE = A2;									// Analog - Rectifier Voltage going into BEC
constexpr int		PIN_BEC_VOLTAGE = A3;												// Analog - BEC output voltage
constexpr int		ADCRAW_PRECISION = 4096;										// DO NOT ALTER, Teensy 12 bit
constexpr float	VREF_CALCULATION_VOLTAGE = 3.3;							// 3.3v for  Teensy, measured and perfect with 5v to Teensy
constexpr float	BATTERY_ASC712_0_AMPS_OFFSET = -2.53;				// Battery ASC712 offset to 0 Amps as per datasheet
constexpr float	BATTERY_ASC712_0_AMPS_POS_DIVIDER = 0.165;	// Battery ASC712 mV/A divider, as measured under load
constexpr float	BATTERY_ASC712_0_AMPS_NEG_DIVIDER = 0.175;	// Battery ASC712 mV/A divider, as measured under load
constexpr float	BEC_ASC712_0_AMPS_OFFSET = -2.5284;					// BEC ASC712 offset to 0 Amps as per datasheet
constexpr float	BEC_ASC712_0_AMPS_POS_DIVIDER = 0.150;			// BEC ASC712 mV/A divider, as measured under load
constexpr float	BEC_ASC712_0_AMPS_NEG_DIVIDER = 0.185;			// BEC ASC712 mV/A divider, as measured under load

constexpr float	BATTERY_VOLTAGE_DIVIDER_MULTIPLIER = 1.00;  // Teensy is only 3.3v, if a divider circuit is used we ...
constexpr float	BEC_VOLTAGE_DIVIDER_MULTIPLIER = 1.00;			// Teensy is only 3.3v, if a divider circuit is used we, 
																														// need to recaluate the voltage back to 2.5v @ 0 AMPS
																														// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier

// Public Variables
extern float	_teensyVoltage, _recVoltage, _becVoltage, _batteryVoltage;			// Teensy 5v line, Regulator and BEC voltages
extern float	_batteryDischargeTotalMAH;										// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
extern float	_batteryDischargeLoopAmps;										// The current AMPS measured when function called
extern float	_batteryDischargeLoopMAH;											// The last MAH used during the loop
extern float	_becDischargeTotalMAH;												// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
extern float	_becDischargeLoopAmps;												// The current AMPS measured when function called
extern float	_becDischargeLoopMAH;													// The last MAH used during the loop
extern float	_myArrayList[102][2];																// Battery Cell values

// Public Functions
void _power_Setup();
void _power_ReadSensors();

// Private Functions
void power_chargeVoltages();
void power_BEC_Amps_ASC712();
void power_Battery_Amps_ASC712();

#endif  // .h #endif

