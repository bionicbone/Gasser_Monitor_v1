// 
// 
// 

#include "RPM.h"

// Config
const byte			RPM_AVERAGE_DIVIDER = 8;
const uint32_t	MAX_UPDATE_TIME_MS = 250000;
const uint8_t		PIN_MAIN_RPM_SENSOR_INTERRUPT_0 = 2;			// Digital Input Pin for RPM Sensor (A3144 Hall Effect Sensor)
const uint8_t		PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1 = 3;		// Digital Input Pin for RPM Sensor (A3144 Hall Effect Sensor)

// Public Variables
uint32_t				mainRPMSensorDetectedRPM = 0;				// Holds the main current RPM value (based on an average over several readings)
uint32_t				clutchRPMSensorDetectedRPM = 0;			// Holds the main current RPM value (based on an average over several readings)
bool						inFlight = false;										// true if in flight, detected by RPM > 600


// Private Variables
unsigned long		mainRPMSensorStartTime;							// New version that disables interrupts
float						mainRPMSensorAvgRPM = 0.0;					// RPM is calcualted over several readings (RPM_AVERAGE_DIVIDER) 
int							mainRPMSensorAvgCounter = 0;				// Used as a counter for the average
int							mainRPMSensorReadings = 0;					// Number of RPM readings for calculating the average
unsigned long		clutchRPMSensorStartTime;						// New version that disables interrupts
float						clutchRPMSensorAvgRPM = 0.0;				// RPM is calcualted over several readings (RPM_AVERAGE_DIVIDER) 
int							clutchRPMSensorAvgCounter = 0;			// Used as a counter for the average
int							clutchRPMSensorReadings = 0;				// Number of RPM readings for calculating the average


// TODO - Ideal code to turn into a class

void calcualte_RPMSensorPulse() {
	// interrupts complete mainRPMSensorReadings++ and clutchRPMSensorReadings++ ONLY

	// Update if 60 or more interrupt signals or it more than MAX_UPDATE_TIME_MS since the last update
	// the MAX_UPDATE_TIME_MS is added to cover slow RPM or Zero RPM
	if (mainRPMSensorReadings > 60 || micros() > mainRPMSensorStartTime + MAX_UPDATE_TIME_MS) {
		// keep track of the number of interrupts signals
		mainRPMSensorAvgRPM += mainRPMSensorReadings * (60000000 / (micros() - mainRPMSensorStartTime));
		mainRPMSensorAvgCounter++;
		mainRPMSensorReadings = 0;
		
		// do we need to calculate the average
		if (mainRPMSensorAvgCounter >= RPM_AVERAGE_DIVIDER) {
			mainRPMSensorDetectedRPM = (mainRPMSensorAvgRPM / mainRPMSensorAvgCounter) / 2;  // For some unknown reason my flywheel counts double the pluses
			mainRPMSensorAvgRPM = 0;
			mainRPMSensorAvgCounter = 0;
		}
		
		mainRPMSensorStartTime = micros();
	}

	// Update if 60 or more interrupt signals or it more than MAX_UPDATE_TIME_MS since the last update
	// the MAX_UPDATE_TIME_MS is added to cover slow RPM or Zero RPM
	if (clutchRPMSensorReadings > 60 || micros() > clutchRPMSensorStartTime + MAX_UPDATE_TIME_MS) {
		// keep track of the number of interrupts signals
		clutchRPMSensorAvgRPM += clutchRPMSensorReadings * (60000000 / (micros() - clutchRPMSensorStartTime));
		clutchRPMSensorAvgCounter++;
		clutchRPMSensorReadings = 0;
		
		// do we need to calculate the average
		if (clutchRPMSensorAvgCounter >= RPM_AVERAGE_DIVIDER) {
			clutchRPMSensorDetectedRPM = clutchRPMSensorAvgRPM / clutchRPMSensorAvgCounter;
			clutchRPMSensorAvgRPM = 0;
			clutchRPMSensorAvgCounter = 0;
		}
		
		clutchRPMSensorStartTime = micros();
	}

	// update the inFlight flag
	// inFlight flag used mainly to stop OLED updates which can take time and effect the RPM accuracy
	if (mainRPMSensorDetectedRPM < 600 || mainRPMSensorDetectedRPM > 30000) {
		mainRPMSensorDetectedRPM = 0;
		inFlight = false;
	}
	else {
		inFlight = true;
#if defined(OLED_OUTPUT)
		// sleep the OLED screen for safety and not much use in flight.
		oLED_Sleep();
#endif
	}
}


// The interrupt routine
void onMainRPMSensorPulse() {
	mainRPMSensorReadings++;
}


// The interrupt routine
void onClutchRPMSensorPulse() {
	clutchRPMSensorReadings++;
}


// Setup the interrupt pins for the attached RPM sensors
void rpm_ActivateInterrupts() {

#if defined(__MK20DX256__)
	// Pull up the PIN and activate interrupt IRQ 0 (NANO pin2, Teensy pin1) for the RPM sensor
	pinMode(PIN_MAIN_RPM_SENSOR_INTERRUPT_0, INPUT_PULLUP);
	attachInterrupt(PIN_MAIN_RPM_SENSOR_INTERRUPT_0, onMainRPMSensorPulse, FALLING);
	// Pull up the PIN and activate interrupt IRQ 1 (NANO pin3, Teensy pin1) for the RPM sensor
	pinMode(PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1, INPUT_PULLUP);
	attachInterrupt(PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1, onClutchRPMSensorPulse, FALLING);
	Serial.println("Teensy Interrupts Setup");
#else
	// Pull up the PIN and activate interrupt IRQ 0 (NANO pin2, Teensy pin1) for the RPM sensor
	pinMode(PIN_MAIN_RPM_SENSOR_INTERRUPT_0, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(2), onMainRPMSensorPulse, CHANGE);
	// Pull up the PIN and activate interrupt IRQ 1 (NANO pin3, Teensy pin1) for the RPM sensor
	pinMode(PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(3), onClutchRPMSensorPulse, CHANGE);
	Serial.println("Nano Interrupts Setup");
#endif
}