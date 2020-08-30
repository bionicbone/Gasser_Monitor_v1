// 
// 
// 

#include "Config.h"
#include "ErrorHandling.h"
#include "Telemetry.h"
#include "Power.h"	
#include "Temperature.h"
#include "RPM.h"

// TODO - Review and activate each error
// TODO - Ensure we consider the scrolling errors on the Taranis screen etc.

// Public Variables
uint32_t _error = 99;
uint32_t _error1 = 0;
bool _errorArray[101] = { false };


// Private Variables
unsigned long lastErrorMillis;  // delays between errors 
unsigned long loopErrorMillis;  // delays between starting error loop again
uint8_t errorArrayCounter = 0;


// Public Functions

void _errorHandling_Setup() {
	pinMode(PIN_ERROR_LED, OUTPUT);
	for (byte i = 0; i < 10; i++) {
		errorHandling_errorLEDsOn(true);
		delay(50);
		errorHandling_errorLEDsOn(false);
		delay(50);
	}
}


void _errorHandling_checkErrors() {
	// Where there is more than one error they are reported in sequnece, i.e. error 1, then 2, then 15 etc.
	// Variable "error" is set to 99 when all errors have been reported so that there is a delay before the errors are reported again
	// If an error is cleared it is no longer reported.
	// If an error still exists then it is reported again.

	// Ensure we are not reporting errors on every microprocessor loop.
	if (_error == 99 && millis() > loopErrorMillis + ERROR_MIN_ALL_ERRORS_LOOP_TIME) {
		_error = 0;
		digitalWrite(PIN_ERROR_LED, LOW);
	}
	else {
		// Ensure we are not reporting a 2nd error before the taranis has
		// has a chance  to respond to the 1st one over the telemetry signal.
		if (_error != 99 && millis() > lastErrorMillis + ERROR_MIN_NEXT_ERROR_REPORT_TIME) {

			 // review to see  if there is an error / another error to report
			
			// Battery Voltage Errors
			if (cell[0] <= ERROR_MIN_CELL_VOLTAGE || cell[0] >= ERROR_MAX_CELL_VOLTAGE) {
				_errorArray[1] = true;
			}
			if (cell[1] <= ERROR_MIN_CELL_VOLTAGE || cell[1] >= ERROR_MAX_CELL_VOLTAGE) {
				_errorArray[2] = true;
			}
			if (abs(cell[0] - cell[1]) >= ERROR_MAX_CELL_DIFFERENCE) {
				_errorArray[3] = true;
			}
			if (_batteryVoltage <= ERROR_MIN_BATTERY_VOLTAGE || _batteryVoltage >= ERROR_MAX_BATTERY_VOLTAGE) {
				_errorArray[4] = true;
			}
			
			// Charging Errors
			if (_mainRPMSensorDetectedRPM >= ERROR_CHARGING_MIN_RPM) {
				if (_recVoltage <= ERROR_CHARGING_MIN_RECTIFIER_VOLTAGE || _recVoltage >= ERROR_CHARGING_MAX_RECTIFIER_VOLTAGE) {
					_errorArray[10] = true;
				}
				if (_becVoltage <= ERROR_CHARGING_MIN_BEC_VOLTAGE || _becVoltage >= ERROR_CHARGING_MAX_BEC_VOLTAGE) {
					_errorArray[11] = true;
				}
			}
			
			// Temperature Errors
			if (_becTemp <= ERROR_MIN_BEC_TEMPERATURE || _becTemp>= ERROR_MAX_BEC_TEMPERATURE) {
				_errorArray[20] = true;
			}
			if (_canopyTemp <= ERROR_MIN_CANOPY_TEMPERATURE || _canopyTemp >= ERROR_MAX_CANOPY_TEMPERATURE) {
				_errorArray[21] = true;
			}
			if (_engineTemp <= ERROR_MIN_ENGINE_TEMPERATURE || _engineTemp >= ERROR_MAX_ENGINE_TEMPERATURE) {
				_errorArray[22] = true;
			}


			//// Clutch Slipping Error
			//if (mainRPMSensorDetectedRPM - clutchRPMSensorDetectedRPM >= 10 && clutchFullyEngaged == true && _error < 25) {
			//	_error = 25;
			//	_error1 = mainRPMSensorDetectedRPM - clutchRPMSensorDetectedRPM;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//// Clutch Engaged Information
			//if (mainRPMSensorDetectedRPM == clutchRPMSensorDetectedRPM && clutchFullyEngaged == false && clutchRPMSensorDetectedRPM != 0 && _error < 26) {
			//	_error = 26;
			//	lastErrorMillis = millis();
			//	// digitalWrite(PIN_ERROR_LED, HIGH);  // Information Only
			//	clutchFullyEngaged = true;
			//	clutchFullyEngagedRPM = mainRPMSensorDetectedRPM;
			//	return;
			//}
			//// Clutch Disengaged - resets variables only
			//if (mainRPMSensorDetectedRPM <= clutchFullyEngagedRPM - (clutchFullyEngagedRPM / 10)) {
			//	// error = 26;							// Reset Variables Onlys
			//	// lastErrorMillis = millis();			// Reset Variables Onlys
			//	// digitalWrite(PIN_ERROR_LED, HIGH);	// Reset Variables Onlys
			//	clutchFullyEngaged = false;
			//	clutchFullyEngagedRPM = 0;
			//	//return;								// Reset Variables Onlys
			//}


			// detect slow running main loop and signal an error
			if (lastLoopMicros >= 3600) {
				_errorArray[97] = true;
				_error1 = lastLoopMicros;
			}

			if (_error == 98) {
				for (uint8_t counter = errorArrayCounter; counter < 102; counter++) {
					if (_errorArray[counter] == true) {

						_errorArray[counter] = false;
						lastErrorMillis = millis();
						digitalWrite(PIN_ERROR_LED, HIGH);
						_error = counter;
						errorArrayCounter = counter + 1;
						return;
					}
				}
				errorArrayCounter = 0;
			}


			//// Set error to 99 to signal that we need to wait at least 
			//// ERROR_MIN_ALL_ERRORS_LOOP_TIME before reporting another error.
			//if (errorArrayCounter = 0 && _error != 99) {
			//	Serial.println("pause");
			//	_error = 99;
			//	_error1 = 0;
			//	loopErrorMillis = millis();
			//}
		}
	}
}



// Private Functions

void errorHandling_errorLEDsOn(bool status) {
	if (status) digitalWrite(PIN_ERROR_LED, HIGH); else digitalWrite(PIN_ERROR_LED, LOW);
}