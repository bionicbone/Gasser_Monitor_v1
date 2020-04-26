// 
// 
// 

#include "Power.h"

// Config
const float		REG_CALIBRATION = 0.00;
const float		BEC_CALIBRATION = 0.00; 
const int			ADCRAW_PRECISION = 4096;										// DO NOT ALTER, Teensy 12 bit
const float		VREF_CALCULATION_VOLTAGE = 3.3;							// 3.3v for  Teensy, measured and perfect with 5v to Teensy
const float		ASC714_0_AMPS_OFFSET = -2.50;								// ASC714 offset to 0 Amps as per datasheet
const float		ASC714_0_AMPS_OFFSET_CALIBRATION = 0.00;		// ASC714 offset to 0 Amps as required, in case of slight variation
const float		VOLTAGE_DIVIDER_MULTIPLIER = 1.00;					// Teensy is only 3.3v, if a divider circuit is used we 
																													// need to recaluate the voltage back to 2.5v @ 0 AMPS
																													// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier

// Public Variables
float					reg, bec;													// Regulator and BEC voltages
float					dischargeTotalMAH = 0;						// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
float					dischargeLoopAmps = 0.00;
float					dischargeLoopMAH = 0.00;					// The last MAH used during the loop

// Private Variables
float					avgReg, avgBec;										// Regulator and BEC voltages are calcualted over several readings (3 hard coded)
int						chargeReadings = 0;								// Number of Regulator and BEC voltage readings for calculating the average
unsigned long dischargeLoopTimeMs = 0;					// The last discharge timer value in MS
unsigned long dischargeStoreTimeMs = millis();	// Sets to millis() each time the MAH has been calculated




void power_Setup() {
	pinMode(PIN_AMPS, INPUT);
	pinMode(PIN_REGULATOR_VOLTAGE, INPUT);
	pinMode(PIN_BEC_VOLTAGE, INPUT);
}


// this was bench tested on a Teensy and found to be very close
void read_Amps_ASC714() {
	int adcRaw = 0;
	float result = 0.000;

	// Take 100 readings
	for (int i = 0; i < 100; i++) {
		adcRaw += analogRead(PIN_AMPS);			// Raw ADC Reading
	}
	// Make the average
	result = adcRaw / 100;
	// Calculate the voltage from the ASC714, this is the actual voltage on the pin
	result = (result * (VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));  		
#if defined(DEBUG_ASC714_AMPS_CALCULATION)
	Serial.print("Pin Volts "); Serial.print(result);
#endif
	// Compensate for the voltage divider circuit, make back to 2.5v @ 0 AMPS
	// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier
	result = result * VOLTAGE_DIVIDER_MULTIPLIER;
#if defined(DEBUG_ASC714_AMPS_CALCULATION)
	Serial.print("   Multi. Volts "); Serial.print(result);
#endif
	// Remove the ASC714 offest and Calculate the AMPs
	result = result + ASC714_0_AMPS_OFFSET + ASC714_0_AMPS_OFFSET_CALIBRATION;			// remove the offset to 0v = zero current
#if defined(DEBUG_ASC714_AMPS_CALCULATION)
	Serial.print("   OffSet Volts "); Serial.print(result);
#endif																																								// Stop the drifting when there is very little usage 
	if (result >= -0.01 && result <= 0.01) result = 0.00;
	
	result = result / (0.066 / VOLTAGE_DIVIDER_MULTIPLIER);							// 30A version / 0.066 & 20A version / 0.100
	
	//Calculate the AMPs and MAH used since last call
	dischargeLoopTimeMs = (millis() - dischargeStoreTimeMs);
	dischargeStoreTimeMs = millis();
	float myFloat = (float)dischargeLoopTimeMs;
	dischargeLoopMAH = ((result / 1000)  * myFloat) / 3600000;
	dischargeLoopMAH = dischargeLoopMAH * 1000000;
	dischargeTotalMAH += dischargeLoopMAH;
	dischargeLoopAmps = result;
#if defined(DEBUG_ASC714_AMPS_CALCULATION)
	Serial.print("   myAMPs "); Serial.print(result);
	Serial.print("   dischargeTotalMAH "); Serial.println(int(dischargeTotalMAH));
#endif
}



void read_chargeVoltages() {
	// takes the average of 3 readings and populates reg and bec global variables.
	int adcRaw = 0;

	// Flash built in LED so we know Arduino is still responding
	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

	adcRaw = analogRead(PIN_REGULATOR_VOLTAGE);
	avgReg += (adcRaw *(VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
	adcRaw = analogRead(PIN_BEC_VOLTAGE);
	avgBec += (adcRaw *(VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));

	//avgReg += (analogRead(PIN_REGULATOR_VOLTAGE) * (40.5 / 4096.0) + REG_CALIBRATION);
	//avgBec += (analogRead(PIN_BEC_VOLTAGE) * (7.5 / 4096.0) + BEC_CALIBRATION);
	chargeReadings++;
	if (chargeReadings > 3) {
		reg = (avgReg / chargeReadings) * 16.41920578;
		bec = (avgBec / chargeReadings) * 7.739580311;
		chargeReadings = 0;
		if (reg == REG_CALIBRATION) reg = 0;
		if (bec == BEC_CALIBRATION) bec = 0;
		avgReg = 0;
		avgBec = 0;

	//	Serial.print("Reg "); Serial.print(reg, 6); Serial.print("    Bec "); Serial.println(bec, 6);

		// ********************************************************
		// TODO - Activate code to write min / max values to EEPROM
		// ********************************************************


			//if (millis() > PROG_DELAY_BEFORE_FLIGHT_STAT_UPDATES) { // only perform updates once stable
			//	// update voltage regulator flight stats
			//	if (reg < customFlightSatistics.MinRegulatorVoltage) {
			//		customFlightSatistics.MinRegulatorVoltage = reg; eepromUpdateRequired = true;
			//	}
			//	else if (reg > customFlightSatistics.MaxRegulatorVoltage) {
			//		customFlightSatistics.MaxRegulatorVoltage = reg; eepromUpdateRequired = true;
			//	}

			//	// update BEC voltage flight stats
			//	if (bec < customFlightSatistics.MinBECVoltage) {
			//		customFlightSatistics.MinBECVoltage = bec; eepromUpdateRequired = true;
			//	}
			//	else if (bec > customFlightSatistics.MaxBECVoltage) {
			//		customFlightSatistics.MaxBECVoltage = bec; eepromUpdateRequired = true;
			//	}

			//	// check if we need to update the EEPROM
			//	if (eepromUpdateRequired == true) eeprom_WriteFilghtDataStats();
			//}
	}
}

