// 
// 
// 

#include "ErrorHandling.h"

// TODO - Review and activate each error
// TODO - Ensure we consider the scrolling errors on the Taranis screen etc.

void errorHandling_Setup() {
	pinMode(PIN_ERROR_LED, OUTPUT);
	for (byte i = 0; i < 10; i++) {
		errorHandling_errorLEDsOn(true);
		delay(50);
		errorHandling_errorLEDsOn(false);
		delay(50);
	}
}


void errorHandling_checkErrors() {
	//// Where there is more than one error they are reported in sequnece, i.e. error 1, then 2, then 15 etc.
	//// Variable "error" is set to 99 when all errors have been reported so that there is a delay before the errors are reported again
	//// If an error is cleared it is no longer reported.
	//// If an error still exists then it is reported again.

	//// Ensure we are not reporting errors on every microprocessor loop.
	//if (error == 99 && millis() > loopErrorMillis + ERROR_MIN_ALL_ERRORS_LOOP_TIME) {
	//	error = 0;
	//	digitalWrite(PIN_ERROR_LED, LOW);
	//}
	//else {
	//	// Ensure we are not reporting a 2nd error before the taranis has
	//	// has a chance  to respond to the 1st one over the telemetry signal.
	//	if (error != 99 && millis() > lastErrorMillis + ERROR_MIN_NEXT_ERROR_REPORT_TIME) {

			// review to see  if there is an error / another error to report
			//if (cell1 <= customControlParameters.ERROR_MIN_CELL_VOLTAGE && error < 1) {
			//	error = 1;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (cell2 <= customControlParameters.ERROR_MIN_CELL_VOLTAGE && error < 2) {
			//	error = 2;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (cell1 >= customControlParameters.ERROR_MAX_CELL_VOLTAGE && error < 3) {
			//	error = 3;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (cell2 >= customControlParameters.ERROR_MAX_CELL_VOLTAGE && error < 4) {
			//	error = 4;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (cell1 + cell2 <= customControlParameters.ERROR_MIN_PACK_VOLTAGE && error < 5) {
			//	error = 5;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (cell1 + cell2 >= customControlParameters.ERROR_MAX_PACK_VOLTAGE && error < 6) {
			//	error = 6;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (mainRPMSensorDetectedRPM >= customControlParameters.CHARGING_MIN_RPM) {
			//	if (reg <= customControlParameters.ERROR_MIN_REG_VOLTAGE && error < 7) {
			//		error = 7;
			//		lastErrorMillis = millis();
			//		return;
			//	}
			//	if (reg >= customControlParameters.ERROR_MAX_REG_VOLTAGE && error < 8) {
			//		error = 8;
			//		lastErrorMillis = millis();
			//		return;
			//	}
			//	if (bec <= customControlParameters.ERROR_MIN_BEC_VOLTAGE && error < 9) {
			//		error = 9;
			//		lastErrorMillis = millis();
			//		return;
			//	}
			//	if (bec >= customControlParameters.ERROR_MAX_BEC_VOLTAGE && error < 10) {
			//		error = 10;
			//		lastErrorMillis = millis();
			//		return;
			//	}
			//}
			//if (ambientTemp <= customControlParameters.ERROR_MIN_AMBIENT_TEMPERATURE && error < 11) {
			//	error = 11;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (ambientTemp >= customControlParameters.ERROR_MAX_AMBIENT_TEMPERATURE && error < 12) {
			//	error = 12;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (canopyTemp <= customControlParameters.ERROR_MIN_CANOPY_TEMPERATURE && error < 13) {
			//	error = 13;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (canopyTemp >= customControlParameters.ERROR_MAX_CANOPY_TEMPERATURE && error < 14) {
			//	error = 14;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (engineTemp <= customControlParameters.ERROR_MIN_ENGINE_TEMPERATURE && error < 15) {
			//	error = 15;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}
			//if (engineTemp >= customControlParameters.ERROR_MAX_ENGINE_TEMPERATURE && error < 16) {
			//	error = 16;
			//	lastErrorMillis = millis();
			//	digitalWrite(PIN_ERROR_LED, HIGH);
			//	return;
			//}

	//		// Clutch Slipping Error
	//		if (mainRPMSensorDetectedRPM - clutchRPMSensorDetectedRPM >= 10 && clutchFullyEngaged == true && error < 25) {
	//			error = 25;
	//			error1 = mainRPMSensorDetectedRPM - clutchRPMSensorDetectedRPM;
	//			lastErrorMillis = millis();
	//			digitalWrite(PIN_ERROR_LED, HIGH);
	//			return;
	//		}
	//		// Clutch Engaged Information
	//		if (mainRPMSensorDetectedRPM == clutchRPMSensorDetectedRPM && clutchFullyEngaged == false && clutchRPMSensorDetectedRPM != 0 && error < 26) {
	//			error = 26;
	//			lastErrorMillis = millis();
	//			// digitalWrite(PIN_ERROR_LED, HIGH);  // Information Only
	//			clutchFullyEngaged = true;
	//			clutchFullyEngagedRPM = mainRPMSensorDetectedRPM;
	//			return;
	//		}
	//		// Clutch Disengaged - resets variables only
	//		if (mainRPMSensorDetectedRPM <= clutchFullyEngagedRPM - (clutchFullyEngagedRPM / 10)) {
	//			// error = 26;							// Reset Variables Onlys
	//			// lastErrorMillis = millis();			// Reset Variables Onlys
	//			// digitalWrite(PIN_ERROR_LED, HIGH);	// Reset Variables Onlys
	//			clutchFullyEngaged = false;
	//			clutchFullyEngagedRPM = 0;
	//			//return;								// Reset Variables Onlys
	//		}


	//		// detect slow running main loop and signal an error
	//		if (lastLoopMicros >= 3600 && error < 98) {
	//			error = 98;
	//			error1 = lastLoopMicros;
	//			lastErrorMillis = millis();
	//			digitalWrite(PIN_ERROR_LED, HIGH);
	//			return;
	//		}

	//		// Set error to 99 to signal that we need to wait at least 
	//		// ERROR_MIN_ALL_ERRORS_LOOP_TIME before reporting another error.
	//		if (error > 0) {
	//			error = 99;
	//			error1 = 0;
	//			loopErrorMillis = millis();
	//		}
	//	}
	//}
}


void errorHandling_errorLEDsOn(bool status) {
	if (status) digitalWrite(PIN_ERROR_LED, HIGH); else digitalWrite(PIN_ERROR_LED, LOW);
}