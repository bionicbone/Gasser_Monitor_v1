// 
// 
// 

#include "Power.h"

// Config
const float		REG_CALIBRATION = -0.70;
const float		BEC_CALIBRATION = -0.10; 
const float		AMPS_CALIBRATION = 0.095;										// Amps calibration for Current discharging the battery
const float		AMPS_CALIBRATION_NEG = -0.115;							// Amps calibration for Current charging the battery (if lagging behind actual make this figure more negative)
const int			adcRaw_Precision = 4096;										// DO NOT ALTER
const float		VrefCalculationVoltage = 4.128496405;				// 
const float		amps_offfset_ASC714 = -2.50;								// ASC714 offset to 0 Amps as per datasheet
const float		amps_offfset_ASC714_Calibration = 0.006862;	// ASC714 offset to 0 Amps as required


// Public Variables
float					reg, bec;													// Regulator and BEC voltages
float					dischargeTotalMAH = 0;						// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging


// Private Variables
float					avgReg, avgBec;										// Regulator and BEC voltages are calcualted over several readings (3 hard coded)
int						chargeReadings = 0;								// Number of Regulator and BEC voltage readings for calculating the average
unsigned long dischargeLoopTimeMs = 0;					// The last discharge timer value in MS
unsigned long dischargeStoreTimeMs = millis();	// Sets to millis() each time the MAH has been calculated
float					dischargeLoopMAH = 0.00;					// The last MAH used during the loop
float					dischargeLoopAmps = 0.00;


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
	// Calculate the voltage from the ASC714
	result = (result * (VrefCalculationVoltage / (float)adcRaw_Precision));  // reading as actual voltage
	// Compensate for the voltage divider
	result = result * 1.55625;
	// Remove the ASC714 offest and Calculate the AMPs
	result = result + amps_offfset_ASC714 + amps_offfset_ASC714_Calibration;			// remove the offset to 0v = zero current
	//result += amps_offfset_ASC714_Calibration;
	if (result >= -0.01 && result <= 0.01) result = 0.00;
	result = result / 0.066;							// 30A version / 0.066 & 20A version / 0.100
	// Calibrate final AMPs
	if (result != 0) {
		if (result > 0) {
			result += AMPS_CALIBRATION;
		}
		else {
			result += AMPS_CALIBRATION_NEG;

		}
	}

	//Calculate the AMPs and MAH used since last call
	//Serial.print("myAMPs "); Serial.println(result);
	dischargeLoopTimeMs = (millis() - dischargeStoreTimeMs);
	dischargeStoreTimeMs = millis();
	float myFloat = (float)dischargeLoopTimeMs;
	dischargeLoopMAH = ((result / 1000)  * myFloat) / 3600000;
	dischargeLoopMAH = dischargeLoopMAH * 1000000;
	dischargeTotalMAH += dischargeLoopMAH;
	dischargeLoopAmps = result;
	//Serial.print("dischargeTotalMAH "); Serial.println(int(dischargeTotalMAH));
}



void read_chargeVoltages() {
	// takes the average of 3 readings and populates reg and bec global variables.

	// Flash built in LED so we know Arduino is still responding
	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

	avgReg += (analogRead(PIN_REGULATOR_VOLTAGE) * (40.5 / 4096.0) + REG_CALIBRATION);
	avgBec += (analogRead(PIN_BEC_VOLTAGE) * (7.5 / 4096.0) + BEC_CALIBRATION);
	chargeReadings++;
	if (chargeReadings > 3) {
		reg = avgReg / chargeReadings;
		bec = avgBec / chargeReadings;
		chargeReadings = 0;
		if (reg == REG_CALIBRATION) reg = 0;
		if (bec == BEC_CALIBRATION) bec = 0;
		avgReg = 0;
		avgBec = 0;

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


