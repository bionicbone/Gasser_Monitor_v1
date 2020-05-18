// 
// 
// 

#include "Power.h"
#include "Config.h"

// Config
const float		REG_CALIBRATION = 0.00;
const float		BEC_CALIBRATION = 0.00; 
const float		TEENSY_CALIBRATION = -0.06;
const int			ADCRAW_PRECISION = 4096;										// DO NOT ALTER, Teensy 12 bit
const float		VREF_CALCULATION_VOLTAGE = 3.3;							// 3.3v for  Teensy, measured and perfect with 5v to Teensy
const float		ASC714_0_AMPS_OFFSET = -2.4790;							// ASC714 offset to 0 Amps as per datasheet
const float		ASC714_0_AMPS_OFFSET_CALIBRATION = -0.07;		// ASC714 offset to 0 Amps as required, in case of slight variation
const float		ASC713_0_AMPS_OFFSET = -0.4955;							// ASC713 offset to 0 Amps as per datasheet
const float		ASC713_0_AMPS_OFFSET_CALIBRATION = -0.040;	// ASC713 offset to 0 Amps as required, in case of slight variation
const float		BATTERY_ASC712_0_AMPS_OFFSET = -2.4948;			// Battery ASC712 offset to 0 Amps as per datasheet
const float		BATTERY_ASC712_0_AMPS_POS_DIVIDER = 0.175;	// Battery ASC712 mV/A divider, as measured under load
const float		BATTERY_ASC712_0_AMPS_NEG_DIVIDER = 0.1311;	// Battery ASC712 mV/A divider, as measured under load
const float		BEC_ASC712_0_AMPS_OFFSET = -2.5100;					// BEC ASC712 offset to 0 Amps as per datasheet
const float		BEC_ASC712_0_AMPS_POS_DIVIDER = 0.215;			// BEC ASC712 mV/A divider, as measured under load
const float		BEC_ASC712_0_AMPS_NEG_DIVIDER = 0.1311;			// BEC ASC712 mV/A divider, as measured under load
const float		TEENSY_VOLTAGE_DIVIDER_MULTIPLIER = 1.00;		// Teensy is only 3.3v, if a divider circuit is used we ...
const float		BATTERY_VOLTAGE_DIVIDER_MULTIPLIER = 1.00;  // Teensy is only 3.3v, if a divider circuit is used we ...
const float		BEC_VOLTAGE_DIVIDER_MULTIPLIER = 1.00;			// Teensy is only 3.3v, if a divider circuit is used we, 
																													// need to recaluate the voltage back to 2.5v @ 0 AMPS
																													// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier

// Public Variables
float					teensyVoltage, recVoltage, becVoltage;			// Teensy, Rectifier and BEC voltages
float					batteryDischargeTotalMAH = 0;								// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
float					batteryDischargeLoopAmps = 0.00;						// The current AMPS measured when function called
float					batteryDischargeLoopMAH = 0.00;							// The last MAH used during the loop
float					becDischargeTotalMAH = 0;;									// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
float					becDischargeLoopAmps = 0.00;								// The current AMPS measured when function called
float					becDischargeLoopMAH = 0.00;									// The last MAH used during the loop

// Private Variables
float					avgTeensy, avgRec, avgBec;									// Regulator and BEC voltages are calcualted over several readings (3 hard coded)
int						chargeReadings = 0;													// Number of Rectifier and BEC voltage readings for calculating the average
unsigned long batteryDischargeLoopTimeMs = 0;							// The last discharge timer value in MS
unsigned long batteryDischargeStoreTimeMs = millis();			// Sets to millis() each time the MAH has been calculated
unsigned long becDischargeLoopTimeMs = 0;									// The last discharge timer value in MS
unsigned long becDischargeStoreTimeMs = millis();					// Sets to millis() each time the MAH has been calculated


// Set up the Power Reading Pins on the Teensy
void power_Setup() {
	pinMode(PIN_BATTERY_AMPS, INPUT);
	pinMode(PIN_BEC_AMPS, INPUT);
	pinMode(PIN_TEENSY_VOLTAGE, INPUT);
	pinMode(PIN_RECTIFIER_VOLTAGE, INPUT);
	pinMode(PIN_BEC_VOLTAGE, INPUT);
	batteryDischargeStoreTimeMs = millis();
	becDischargeStoreTimeMs = millis();
}


// Read all the attached sensors
void power_ReadSensors() {
	power_chargeVoltages();
	power_Battery_Amps_ASC712();
	power_BEC_Amps_ASC712();
}


// Private Functions

// Read the Battery Current Flow with a bidirectional Hall Effect Sensor
// Review ASC712 code before using this
//void power_Battery_Amps_ASC714() {
//	int adcRaw = 0;
//	float result = 0.000;
//
//	// Take 100 readings
//	for (int i = 0; i < 100; i++) {
//		adcRaw += analogRead(PIN_BATTERY_AMPS);			// Raw ADC Reading
//	}
//	// Make the average
//	result = adcRaw / 100;
//	// Calculate the voltage from the ASC714, this is the actual voltage on the pin
//	result = (result * (VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
//#if defined(DEBUG_ASC714_BATTERY_AMPS_CALCULATION)
//	Serial.print("Pin Volts "); Serial.print(result,4);
//#endif
//	// Compensate for 5v line supply deviation
//	result = result + ((5.0 - teensyVoltage) / 1.8);
//	// Compensate for the voltage divider circuit, make back to 2.5v @ 0 AMPS
//	// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier
//	result = result * BATTERY_VOLTAGE_DIVIDER_MULTIPLIER;
//#if defined(DEBUG_ASC714_BATTERY_AMPS_CALCULATION)
//	Serial.print("   calc. Volts "); Serial.print(result,4);
//#endif
//	// Remove the ASC714 offset to make 0v = zero current and Calculate the AMPs
//	result = result + ASC714_0_AMPS_OFFSET;
//#if defined(DEBUG_ASC714_BATTERY_AMPS_CALCULATION)
//	Serial.print("   OffSet Volts "); Serial.print(result);
//#endif							
//	// Stop the drifting when there is very little usage 
//	if (result >= -0.01 && result <= 0.01) result = 0.00;
//
//	// 30A version / 0.066 & 20A version / 0.100
//	result = result / (0.066 / BATTERY_VOLTAGE_DIVIDER_MULTIPLIER);
//
//	// Calibrate  ASC714  Sensor
//	if (result) result += ASC714_0_AMPS_OFFSET_CALIBRATION;
//
//	//Calculate the AMPs and MAH used since last call
//	batteryDischargeLoopTimeMs = (millis() - batteryDischargeStoreTimeMs);
//	batteryDischargeStoreTimeMs = millis();
//	float myFloat = (float)batteryDischargeLoopTimeMs;
//	batteryDischargeLoopMAH = ((result / 1000)  * myFloat) / 3600000;
//	batteryDischargeLoopMAH = batteryDischargeLoopMAH * 1000000;
//	batteryDischargeTotalMAH += batteryDischargeLoopMAH;
//	batteryDischargeLoopAmps = result;
//#if defined(DEBUG_ASC714_BATTERY_AMPS_CALCULATION)
//	Serial.print("   myAMPs "); Serial.print(result);
//	Serial.print("   batteryDischargeTotalMAH "); Serial.println(int(batteryDischargeTotalMAH));
//#endif
//}


// Read the Battery Current Flow with a bidirectional Hall Effect Sensor
void power_Battery_Amps_ASC712() {
	int adcRaw = 0;
	float result = 0.000;

	// Take 100 readings
	for (int i = 0; i < 100; i++) {
		adcRaw += analogRead(PIN_BATTERY_AMPS);			// Raw ADC Reading
	}
	// Make the average
	result = adcRaw / 100;
	// Calculate the voltage from the ASC712, this is the actual voltage on the pin
	result = (result * (VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
#if defined(DEBUG_ASC712_BATTERY_AMPS_CALCULATION)
	Serial.print("Teensy Volts "); Serial.print(teensy, 2);
	Serial.print("   Pin Volts "); Serial.print(result, 4);
#endif
	// Compensate for the voltage divider circuit, make back to 2.5v @ 0 AMPS
	// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier
	result = result * BATTERY_VOLTAGE_DIVIDER_MULTIPLIER;
#if defined(DEBUG_ASC712_BATTERY_AMPS_CALCULATION)
	Serial.print("   calc. Volts "); Serial.print(result, 4);
#endif
	// Remove the ASC714 offset to make 0v = zero current and Calculate the AMPs
	result = result + BATTERY_ASC712_0_AMPS_OFFSET;
#if defined(DEBUG_ASC712_BATTERY_AMPS_CALCULATION)
	Serial.print("   OffSet Volts "); Serial.print(result, 4);
#endif							
	// Stop the drifting when there is very little usage 
	//if (result >= -0.01 && result <= 0.01) result = 0.00;

	// 30A version / 0.133 & 20A version / 0.185
	// in bench tests the values could be significantly different for POS and NEG readings
	if (result > 0) {
		result = result / (BATTERY_ASC712_0_AMPS_POS_DIVIDER / BATTERY_VOLTAGE_DIVIDER_MULTIPLIER);
	}
	else {
		result = result / (BATTERY_ASC712_0_AMPS_NEG_DIVIDER / BATTERY_VOLTAGE_DIVIDER_MULTIPLIER);
	}

	//Calculate the AMPs and MAH used since last call
	batteryDischargeLoopTimeMs = (millis() - batteryDischargeStoreTimeMs);
	batteryDischargeStoreTimeMs = millis();
	float myFloat = (float)batteryDischargeLoopTimeMs;
	batteryDischargeLoopMAH = ((result / 1000)  * myFloat) / 3600000;
	batteryDischargeLoopMAH = batteryDischargeLoopMAH * 1000000;
	batteryDischargeTotalMAH += batteryDischargeLoopMAH;
	batteryDischargeLoopAmps = result;
#if defined(DEBUG_ASC712_BATTERY_AMPS_CALCULATION)
	Serial.print("   myAMPs "); Serial.print(result);
	Serial.print("   batteryDischargeTotalMAH "); Serial.println(int(batteryDischargeTotalMAH));
#endif
}


// Read the BEC Current Flow with a bidirectional Hall Effect Sensor
// Review ASC712 code before using this
//void power_BEC_Amps_ASC713() {
//	int adcRaw = 0;
//	float result = 0.000;
//
//	// Take 100 readings
//	for (int i = 0; i < 100; i++) {
//		adcRaw += analogRead(PIN_BEC_AMPS);			// Raw ADC Reading
//	}
//	// Make the average
//	result = adcRaw / 100;
//	// Calculate the voltage from the ASC714, this is the actual voltage on the pin
//	result = (result * (VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
//#if defined(DEBUG_ASC713_BEC_AMPS_CALCULATION)
//	Serial.print("Pin Volts "); Serial.print(result,4);
//#endif
//	// Compensate for 5v line supply deviation
//	 result = result + ((5.0 - teensyVoltage) / 8);
//	// Compensate for the voltage divider circuit, make back to 2.5v @ 0 AMPS
//	// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier
//	result = result * BEC_VOLTAGE_DIVIDER_MULTIPLIER;
//#if defined(DEBUG_ASC713_BEC_AMPS_CALCULATION)
//	Serial.print("   calc. Volts "); Serial.print(result,4);
//#endif
//	// Remove the ASC714 offset to make 0v = zero current and Calculate the AMPs
//	result = result + ASC713_0_AMPS_OFFSET;
//#if defined(DEBUG_ASC713_BEC_AMPS_CALCULATION)
//	Serial.print("   OffSet Volts "); Serial.print(result);
//#endif							
//	// Stop the drifting when there is very little usage 
//	if (result >= -0.01 && result <= 0.01) result = 0.00; 
//
//	// 30A version / 0.133 & 20A version / 0.185
//	result = result / (0.133 / BEC_VOLTAGE_DIVIDER_MULTIPLIER);
//
//	// Calibrate
//	if(result) result += ASC713_0_AMPS_OFFSET_CALIBRATION;
//
//	//Calculate the AMPs and MAH used since last call
//	becDischargeLoopTimeMs = (millis() - becDischargeStoreTimeMs);
//	becDischargeStoreTimeMs = millis();
//	float myFloat = (float)becDischargeLoopTimeMs;
//	becDischargeLoopMAH = ((result / 1000)  * myFloat) / 3600000;
//	becDischargeLoopMAH = becDischargeLoopMAH * 1000000;
//	becDischargeTotalMAH += becDischargeLoopMAH;
//	becDischargeLoopAmps = result;
//#if defined(DEBUG_ASC713_BEC_AMPS_CALCULATION)
//	Serial.print("   myAMPs "); Serial.print(result);
//	Serial.print("   becDischargeTotalMAH "); Serial.println(int(becDischargeTotalMAH));
//#endif
//}


// Read the BEC Current Flow with a bidirectional Hall Effect Sensor
void power_BEC_Amps_ASC712() {
	int adcRaw = 0;
	float result = 0.000;

	// Take 100 readings
	for (int i = 0; i < 100; i++) {
		adcRaw += analogRead(PIN_BEC_AMPS);			// Raw ADC Reading
	}
	// Make the average
	result = adcRaw / 100;
	// Calculate the voltage from the ASC714, this is the actual voltage on the pin
	result = (result * (VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
#if defined(DEBUG_ASC712_BEC_AMPS_CALCULATION)
	Serial.print("Teensy Volts "); Serial.print(teensy, 2);
	Serial.print("   Pin Volts "); Serial.print(result, 4);
#endif
	// Compensate for 5v line supply deviation
	//result = result + ((5.0 - teensy) / 8);
	// Compensate for the voltage divider circuit, make back to 2.5v @ 0 AMPS
	// i.e. 1.59v on pin @ 0 AMPS then 2.5 / 1.59 = 1.5723 as a Mulitplier
	result = result * BEC_VOLTAGE_DIVIDER_MULTIPLIER;
#if defined(DEBUG_ASC712_BEC_AMPS_CALCULATION)
	Serial.print("   calc. Volts "); Serial.print(result, 4);
#endif
	// Remove the ASC714 offset to make 0v = zero current and Calculate the AMPs
	result = result + BEC_ASC712_0_AMPS_OFFSET;
#if defined(DEBUG_ASC712_BEC_AMPS_CALCULATION)
	Serial.print("   OffSet Volts "); Serial.print(result, 4);
#endif							
	// Stop the drifting when there is very little usage 
	//if (result >= -0.01 && result <= 0.01) result = 0.00;

	// 30A version / 0.133 & 20A version / 0.185
	// in bench tests the values could be significantly different for POS and NEG readings
	if (result > 0) {
		result = result / (BEC_ASC712_0_AMPS_POS_DIVIDER / BEC_VOLTAGE_DIVIDER_MULTIPLIER);
	}
	else {
		result = result / (BEC_ASC712_0_AMPS_NEG_DIVIDER / BEC_VOLTAGE_DIVIDER_MULTIPLIER);
	}

	// Calibrate
	//if (result) result += BEC_ASC712_0_AMPS_CALIBRATION;

	//Calculate the AMPs and MAH used since last call
	becDischargeLoopTimeMs = (millis() - becDischargeStoreTimeMs);
	becDischargeStoreTimeMs = millis();
	float myFloat = (float)becDischargeLoopTimeMs;
	becDischargeLoopMAH = ((result / 1000)  * myFloat) / 3600000;
	becDischargeLoopMAH = becDischargeLoopMAH * 1000000;
	becDischargeTotalMAH += becDischargeLoopMAH;
	becDischargeLoopAmps = result;
#if defined(DEBUG_ASC712_BEC_AMPS_CALCULATION)
	Serial.print("   myAMPs "); Serial.print(result);
	Serial.print("   becDischargeTotalMAH "); Serial.println(int(becDischargeTotalMAH));
#endif
}


// Read the Regulator and BEC Voltages
void power_chargeVoltages() {
	// takes the average of 3 readings and populates reg and bec global variables.
	int adcRaw = 0;

	// Flash built in LED so we know Arduino is still responding
	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

	adcRaw = analogRead(PIN_RECTIFIER_VOLTAGE);
	avgRec += (adcRaw *(VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
	adcRaw = analogRead(PIN_BEC_VOLTAGE);
	avgBec += (adcRaw *(VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
	adcRaw = analogRead(PIN_TEENSY_VOLTAGE);
	avgTeensy += (adcRaw *(VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));

	chargeReadings++;
	if (chargeReadings > 3) {
		recVoltage = (avgRec / chargeReadings) * 16.41920578;
		becVoltage = (avgBec / chargeReadings) * 7.739580311;
		teensyVoltage = (avgTeensy / chargeReadings) * 2;
		chargeReadings = 0;
		
		// Calibrate
		recVoltage += REG_CALIBRATION;
		becVoltage += BEC_CALIBRATION;
		teensyVoltage += TEENSY_CALIBRATION;
		
		if (recVoltage == REG_CALIBRATION) recVoltage = 0;
		if (becVoltage == BEC_CALIBRATION) becVoltage = 0;
		if (teensyVoltage == TEENSY_CALIBRATION) teensyVoltage = 0;

		avgRec = 0;
		avgBec = 0;
		avgTeensy = 0;


		// Serial.print("Reg "); Serial.print(reg, 6); Serial.print("    Bec "); Serial.print(bec, 6); Serial.print("    Teensy "); Serial.println(teensy, 6);

		// WARNING if 5v line is < 4.97  or > 5.03
		//if (teensyVoltage < 4.95 || teensyVoltage > 5.08) Serial.println("TEENSY 5v DRIFTING - NEED TO FIX !!!");

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

