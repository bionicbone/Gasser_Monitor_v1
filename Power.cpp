// 
// 
// 

#include "Power.h"
#include "Config.h"


// Public Variables
float				_teensyVoltage, _recVoltage, _becVoltage, _batteryVoltage;			// Teensy, Rectifier and BEC voltages
float				_batteryDischargeTotalMAH = 0;								// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
float				_batteryDischargeLoopAmps = 0.00;							// The current AMPS measured when function called
float				_batteryDischargeLoopMAH = 0.00;							// The last MAH used during the loop
float				_becDischargeTotalMAH = 0;;										// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
float				_becDischargeLoopAmps = 0.00;									// The current AMPS measured when function called
float				_becDischargeLoopMAH = 0.00;									// The last MAH used during the loop

																													// Life Voltages & accosiated percentages
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK66FX1M0__) || defined(__MK64FX512__)
// Full List
float _myArrayList[][2] = { {2.45, 0}, { 2.543, 1 }, { 2.646, 2 }, { 2.751, 3 }, { 2.851, 4 }, { 2.927, 5 }, { 2.994, 6 }, { 3.051, 7 }, { 3.087, 8 }, { 3.114, 9 }, { 3.129, 10 }, { 3.133, 11 }, { 3.139, 12 }, { 3.142, 13 }, { 3.155, 14 }, { 3.16, 15 }, { 3.163, 16 }, { 3.165, 17 }, { 3.17, 18 }, { 3.181, 19 }, { 3.185, 20 }, { 3.194, 21 }, { 3.203, 22 }, { 3.206, 23 }, { 3.208, 24 }, { 3.212, 25 }, { 3.217, 26 }, { 3.224, 27 }, { 3.23, 28 }, { 3.233, 29 }, { 3.236, 30 }, { 3.239, 31 }, { 3.244, 32 }, { 3.247, 33 }, { 3.25, 34 }, { 3.252, 35 }, { 3.255, 36 }, { 3.258, 37 }, { 3.261, 38 }, { 3.265, 39 }, { 3.268, 40 }, { 3.272, 41 }, { 3.275, 42 }, { 3.279, 43 }, { 3.283, 44 }, { 3.286, 45 }, { 3.29, 46 }, { 3.293, 47 }, { 3.297, 48 }, { 3.3, 49 }, { 3.304, 50 }, { 3.307, 51 }, { 3.31, 52 }, { 3.313, 53 }, { 3.316, 54 }, { 3.32, 55 }, { 3.324, 56 }, { 3.329, 57 }, { 3.338, 58 }, { 3.343, 59 }, { 3.347, 60 }, { 3.352, 61 }, { 3.356, 62 }, { 3.361, 63 }, { 3.368, 64 }, { 3.373, 65 }, { 3.378, 66 }, { 3.389, 67 }, { 3.393, 68 }, { 3.399, 69 }, { 3.405, 70 }, { 3.411, 71 }, { 3.418, 72 }, { 3.424, 73 }, { 3.431, 74 }, { 3.437, 75 }, { 3.444, 76 }, { 3.451, 77 }, { 3.457, 78 }, { 3.464, 79 }, { 3.471, 80 }, { 3.479, 81 }, { 3.486, 82 }, { 3.494, 83 }, { 3.502, 84 }, { 3.512, 85 }, { 3.524, 86 }, { 3.535, 87 }, { 3.545, 88 }, { 3.555, 89 }, { 3.561, 90 }, { 3.566, 91 }, { 3.57, 92 }, { 3.575, 93 }, { 3.579, 94 }, { 3.585, 95 }, { 3.595, 96 }, { 3.626, 97 }, { 3.629, 98 }, { 3.643, 99 }, { 3.65, 100 }, { 9.99, 100 } };
#else
// Limited list due to memory
// TODO - We can have more resolution if there is Variable Memory available
float _myArrayList[][2] = { {2.45, 0}, { 2.927, 5 }, { 3.129, 10 }, { 3.16, 15 }, { 3.185, 20 }, { 3.212, 25 }, { 3.236, 30 }, { 3.252, 35 }, { 3.268, 40 }, { 3.286, 45 }, { 3.304, 50 }, { 3.32, 55 }, { 3.347, 60 }, { 3.373, 65 }, { 3.405, 70 }, { 3.437, 75 }, { 3.471, 80 }, { 3.512, 85 }, { 3.561, 90 }, { 3.585, 95 }, { 3.65, 100 }, { 9.99, 100 } };
#endif


// Private Variables
float					avgTeensy, avgRec, avgBec, avgBat;					// Regulator and BEC voltages are calcualted over several readings (3 hard coded)
int						chargeReadings = 0;													// Number of Rectifier and BEC voltage readings for calculating the average
unsigned long batteryDischargeLoopTimeMs = 0;							// The last discharge timer value in MS
unsigned long batteryDischargeStoreTimeMs = millis();			// Sets to millis() each time the MAH has been calculated
unsigned long becDischargeLoopTimeMs = 0;									// The last discharge timer value in MS
unsigned long becDischargeStoreTimeMs = millis();					// Sets to millis() each time the MAH has been calculated
unsigned long dischargeStoreTimeMs = 0;										// The last discharge timer value in MS
float					calibrationTeeV, calibrationRecV, calibrationBecV, calibrationBatV;
int						calibrationReadings;


// Set up the Power Reading Pins on the Teensy
void _power_Setup() {
	pinMode(PIN_BATTERY_AMPS, INPUT);
	pinMode(PIN_BEC_AMPS, INPUT);
	pinMode(PIN_TEENSY_VOLTAGE, INPUT);
	pinMode(PIN_RECTIFIER_VOLTAGE, INPUT);
	pinMode(PIN_BEC_VOLTAGE, INPUT);
	batteryDischargeStoreTimeMs = millis();
	becDischargeStoreTimeMs = millis();
}


// Read all the attached sensors
void _power_ReadSensors() {
	power_chargeVoltages();
	power_chargeAmps();
}



// Private Functions

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
	adcRaw = analogRead(PIN_BATTERY_VOLTAGE);
	avgBat += (adcRaw *(VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));

	chargeReadings++;
	if (chargeReadings >= 3) {
		_recVoltage = (avgRec / chargeReadings) * REC_VOLTAGE_MULTIPLIER;
		_becVoltage = (avgBec / chargeReadings) * BEC_VOLTAGE_MULTIPLIER;
		_teensyVoltage = (avgTeensy / chargeReadings) * TEENSY_VOLTAGE_MULTIPLIER;
		_batteryVoltage = (avgBat / chargeReadings) * BAT_VOLTAGE_MULTIPLIER;
		chargeReadings = 0;

		// Reset the Averages
		avgRec = 0; avgBec = 0; avgTeensy = 0; avgBat = 0;


#if !defined(CALIBRATION_POWER)
		// Stop irrelevent data (disable if calibrating
		if (_recVoltage < 1) _recVoltage = 0;
		if (_becVoltage < 1) _becVoltage = 0;
		if (_teensyVoltage < 1) _teensyVoltage = 0;
		if (_batteryVoltage < 1) _batteryVoltage = 0;
#else
		calibrationReadings++; calibrationRecV += _recVoltage; calibrationBecV += _becVoltage; calibrationBatV += _batteryVoltage; calibrationTeeV += _teensyVoltage;
		if (calibrationReadings > 50) {
			calibrationRecV = calibrationRecV / calibrationReadings; calibrationBecV = calibrationBecV / calibrationReadings; calibrationBatV = calibrationBatV / calibrationReadings; calibrationTeeV = calibrationTeeV / calibrationReadings;
			Serial.print("RecV "); Serial.print(calibrationRecV, 3); Serial.print("    BecV "); Serial.print(calibrationBecV, 3); Serial.print("   BatV "); Serial.print(calibrationBatV, 3); Serial.print("    TeeV "); Serial.println(calibrationTeeV, 3);
			calibrationReadings = 0; calibrationRecV = 0.00; calibrationBecV = 0.00; calibrationBatV = 0.00; calibrationTeeV = 0.00;
		}
#endif

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


// Read the Battery and BEC AMPs with a bidirectional Hall Effect Sensor
void power_chargeAmps() {
	// if its been less than 500ms since last reading then skip
	if (millis() - dischargeStoreTimeMs < 500) return;

	// initilise the reading variables
	int adcRawBat = 0;
	int adcRawBec = 0;
	float resultBat = 0.000;
	float resultBec = 0.000;
	float pinVoltageBat = 0.000;
	float pinVoltageBec = 0.000;


	// Take 50 readings
	for (int i = 0; i < 50; i++) {
		adcRawBat += analogRead(PIN_BATTERY_AMPS);	// Raw ADC Reading
		adcRawBec += analogRead(PIN_BEC_AMPS);			// Raw ADC Reading
	}
	// Calculate the average
	resultBat = adcRawBat / 50;
	resultBec = adcRawBec / 50;
	// Calculate the voltage from the ASC714, this is the actual voltage on the pin
	resultBat = (resultBat * (VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
	resultBec = (resultBec * (VREF_CALCULATION_VOLTAGE / (float)ADCRAW_PRECISION));
	pinVoltageBat = resultBat;  resultBat = pinVoltageBat;  // Looks strange but stpo annoying unused variable error when not calibrating
	pinVoltageBec = resultBec;  resultBec = pinVoltageBec;	// Looks strange but stpo annoying unused variable error when not calibrating
	// Remove the ASC714 offset to make 0v = zero current and Calculate the AMPs
	resultBat = resultBat + BATTERY_ASC712_0_AMPS_OFFSET;
	resultBec = resultBec + BEC_ASC712_0_AMPS_OFFSET;
	// Multiply to the actual voltage
	resultBat = resultBat * BATTERY_AMPS_MULTIPLIER;
	resultBec = resultBec * BEC_AMPS_MULTIPLIER;

	// Remove small readings
#if !defined(CALIBRATION_POWER)
	// Stop irrelevent data (disable if calibrating)
	if (resultBat < 0.2) resultBat = 0;
	if (resultBec < 0.2) resultBec = 0;
#endif

	//Calculate the AMPs and MAH used since last call
	batteryDischargeLoopTimeMs = (millis() - batteryDischargeStoreTimeMs);
	float myFloat = (float)batteryDischargeLoopTimeMs;
	_batteryDischargeLoopMAH = ((resultBat / 1000)  * myFloat) / 3600000;
	_batteryDischargeLoopMAH = _batteryDischargeLoopMAH * 1000000;
	_batteryDischargeTotalMAH += _batteryDischargeLoopMAH;
	_batteryDischargeLoopAmps = resultBat;

	becDischargeLoopTimeMs = (millis() - becDischargeStoreTimeMs);
	myFloat = (float)becDischargeLoopTimeMs;
	_becDischargeLoopMAH = ((resultBec / 1000)  * myFloat) / 3600000;
	_becDischargeLoopMAH = _becDischargeLoopMAH * 1000000;
	_becDischargeTotalMAH += _becDischargeLoopMAH;
	_becDischargeLoopAmps = resultBec;


	// Reset the next reading timer
	dischargeStoreTimeMs = millis();

#if defined(CALIBRATION_POWER)
	Serial.print("BatPin "); Serial.print(pinVoltageBat, 3); Serial.print("    BatA "); Serial.print(resultBat, 3); Serial.print("    BatmAH "); Serial.print(_batteryDischargeTotalMAH, 3); Serial.print("    BecPin "); Serial.print(pinVoltageBec, 3); Serial.print("    BecA "); Serial.print(_becDischargeLoopAmps, 3); Serial.print("   BecmAH "); Serial.println(_becDischargeTotalMAH, 3);
#endif
}