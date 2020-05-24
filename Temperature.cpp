// 
// 
// 

#include "Config.h"
#include "Temperature.h"


// Public Variables
float ambientTemp = 0.00;			// NOTE: These are float values on purpose so we can detect disconnected sensors
float canopyTemp = 0.00;			// NOTE: These are float values on purpose so we can detect disconnected sensors
float engineTemp = 0.00;			// NOTE: These are float values on purpose so we can detect disconnected sensors
float becTemp = 0.00;					// NOTE: These are float values on purpose so we can detect disconnected sensors


// Private Variables
float			avgAmbientTemp = 0.00;
float			avgCanopyTemp = 0.00;
float			avgEngineTemp = 0.00;
float			avgBecTemp = 0.00;
uint8_t		tempReadings = 0;
bool			eepromUpdateRequired = false;


// Public Functions
void _temperature_Setup() {
	pinMode(PIN_AMBIENT_TEMPERATURE_LM35, INPUT);
	pinMode(PIN_CANOPY_TEMPERATURE_LM35, INPUT);
	pinMode(PIN_ENGINE_TEMPERATURE_LM35, INPUT);
	pinMode(PIN_BEC_TEMPERATURE_LM35, INPUT);
}


void _temperatures_Read() {
	// takes the average of 50 readings and populates ambientTemp, canopyTemp &  engineTemp global variables

	avgAmbientTemp += (analogRead(PIN_AMBIENT_TEMPERATURE_LM35) * 3.3 / 4096) * 100 + AMBIENT_TEMP_CALIBRATION;
	avgCanopyTemp += (analogRead(PIN_CANOPY_TEMPERATURE_LM35) * 3.3 / 4096) * 100 + CANOPY_TEMP_CALIBRATION;
	avgEngineTemp += (analogRead(PIN_ENGINE_TEMPERATURE_LM35) * 3.3 / 4096) * 100 + ENGINE_TEMP_CALIBRATION;
	avgBecTemp += (analogRead(PIN_BEC_TEMPERATURE_LM35) * 3.3 / 4096) * 100 + BEC_TEMP_CALIBRATION;
	tempReadings++;
	if (tempReadings > TEMPERATURE_READINGS_FOR_AVERAGE) {
		ambientTemp = avgAmbientTemp / tempReadings;
		canopyTemp = avgCanopyTemp / tempReadings;
		engineTemp = avgEngineTemp / tempReadings;
		becTemp = avgBecTemp / tempReadings;
		tempReadings = 0;
		if (ambientTemp == AMBIENT_TEMP_CALIBRATION) ambientTemp = 0;
		if (canopyTemp == CANOPY_TEMP_CALIBRATION) canopyTemp = 0;
		if (engineTemp == ENGINE_TEMP_CALIBRATION) engineTemp = 0;
		if (becTemp == BEC_TEMP_CALIBRATION) becTemp = 0;
		avgAmbientTemp = 0;
		avgCanopyTemp = 0;
		avgEngineTemp = 0;
		avgBecTemp = 0;

		// ********************************************************
		// TODO - Activate code to write min / max values to EEPROM
		// ********************************************************

		//if (millis() > PROG_DELAY_BEFORE_FLIGHT_STAT_UPDATES) { // only perform updates once stable
		//// update ambient temperature flight stats
		//	if (ambientTemp < customFlightSatistics.MinAmbientTemperature) {
		//		customFlightSatistics.MinAmbientTemperature = ambientTemp; eepromUpdateRequired = true;
		//	}
		//	else if (ambientTemp > customFlightSatistics.MaxAmbientTemperature) {
		//		customFlightSatistics.MaxAmbientTemperature = ambientTemp; eepromUpdateRequired = true;
		//	}

		//	// update engine temperature flight stats
		//	if (engineTemp < customFlightSatistics.MinEngineTemperature) {
		//		customFlightSatistics.MinEngineTemperature = engineTemp; eepromUpdateRequired = true;
		//	}
		//	else if (engineTemp > customFlightSatistics.MaxEngineTemperature) {
		//		customFlightSatistics.MaxEngineTemperature = engineTemp; eepromUpdateRequired = true;
		//	}

		//	// update canopy temperature flight stats
		//	if (canopyTemp < customFlightSatistics.MinCanopyTemperature) {
		//		customFlightSatistics.MinCanopyTemperature = canopyTemp; eepromUpdateRequired = true;
		//	}
		//	else if (canopyTemp > customFlightSatistics.MaxCanopyTemperature) {
		//		customFlightSatistics.MaxCanopyTemperature = canopyTemp; eepromUpdateRequired = true;
		//	}

		//	// check if we need to update the EEPROM
		//	if (eepromUpdateRequired == true) eeprom_WriteFilghtDataStats();
		//}

		// Sanity check the values and and set to zero if LM35 sensor is not attached
		if (ambientTemp > 200) ambientTemp = 0;
		if (canopyTemp > 200) canopyTemp = 0;
		if (engineTemp > 200) engineTemp = 0;
		if (becTemp > 200) engineTemp = 0;

#if defined (DEBUG_LM35_TEMPERATURE_READINGS)
		Serial.print("Ambient Temperature = "); Serial.print(ambientTemp); Serial.println("c");
		Serial.print(" Canopy Temperature = "); Serial.print(canopyTemp); Serial.println("c");
		Serial.print(" Engine Temperature = "); Serial.print(engineTemp); Serial.println("c");
		Serial.print("    BEC Temperature = "); Serial.print(becTemp); Serial.println("c");
#endif
	}
}
