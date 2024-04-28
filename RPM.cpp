// 
// 
// 

#include "Config.h"
#include "RPM.h"


// Public Variables
uint32_t				_mainRPMSensorDetectedRPM = 0;			// Holds the main current RPM value (based on an average over several readings)
uint32_t				_clutchRPMSensorDetectedRPM = 0;		// Holds the clutch current RPM value (based on an average over several readings)
uint32_t				_headRPMCalculatedRPM = 0;					// Holds the blade current RPM value (based on clutch / gearing ratio)
bool						_inFlight = false;									// true if in flight, detected by RPM > 600


// Private Variables
unsigned long		mainRPMSensorStartTime;							// New version that disables interrupts
float						mainRPMSensorAvgRPM = 0.0;					// RPM is calcualted over several readings (RPM_AVERAGE_DIVIDER) 
int							mainRPMSensorAvgCounter = 0;				// Used as a counter for the average
volatile int		mainRPMSensorReadings = 0;					// Number of RPM readings for calculating the average
float						clutchRPMSensorAvgRPM = 0.0;				// RPM is calcualted over several readings (RPM_AVERAGE_DIVIDER) 
int							clutchRPMSensorAvgCounter = 0;			// Used as a counter for the average
volatile int		clutchRPMSensorReadings = 0;				// Number of RPM readings for calculating the average

// Public Functions

void _rpm_calculate_SensorPulse() {
	// interrupts complete mainRPMSensorReadings++ and clutchRPMSensorReadings++ ONLY

	// Update if 60 or more interrupt signals or it is more than MAX_UPDATE_TIME_MS since the last update
	// the MAX_UPDATE_TIME_MS is added to cover slow RPM or Zero RPM
	// RPM checks are based on main (engine) RPM data 
	if (mainRPMSensorReadings > 60 || micros() > mainRPMSensorStartTime + MAX_UPDATE_TIME_MS) {
		
		// keep track of the number of interrupts signals for main RPM
		mainRPMSensorAvgRPM += mainRPMSensorReadings * (60000000 / (micros() - mainRPMSensorStartTime));
		mainRPMSensorAvgCounter++;
		mainRPMSensorReadings = 0;

		// keep track of the number of interrupts signals for the clutch RPM
		clutchRPMSensorAvgRPM += clutchRPMSensorReadings * (60000000 / (micros() - mainRPMSensorStartTime));
		clutchRPMSensorAvgCounter++;
		clutchRPMSensorReadings = 0;

		// calculate the average over all the readings taken this period
		if (mainRPMSensorAvgCounter >= RPM_AVERAGE_DIVIDER) {
			
			// average the main RPM
			_mainRPMSensorDetectedRPM = (mainRPMSensorAvgRPM / mainRPMSensorAvgCounter) / 2; 
			mainRPMSensorAvgRPM = 0;
			mainRPMSensorAvgCounter = 0;

			// average the clutch RPM
			_clutchRPMSensorDetectedRPM = (clutchRPMSensorAvgRPM / clutchRPMSensorAvgCounter);
			clutchRPMSensorAvgRPM = 0;
			clutchRPMSensorAvgCounter = 0;

			// Uncomment the next line to sync clutch RPM with main (engine RPM) during testing
			//_clutchRPMSensorDetectedRPM = _mainRPMSensorDetectedRPM;
			
			// Calculate Blade RPM
			_headRPMCalculatedRPM = _clutchRPMSensorDetectedRPM / HEAD_GEAR_RATIO;

			#ifdef DEBUG_RPM
			Serial.print("Engine RPM "); Serial.println(_mainRPMSensorDetectedRPM);
			Serial.print("Clutch RPM "); Serial.println(_clutchRPMSensorDetectedRPM);
			Serial.print("Head RPM "); Serial.println(_headRPMCalculatedRPM);
			#endif // DEBUG_RPM
		
		}
		
		mainRPMSensorStartTime = micros();
	}


	// update the inFlight flag
	// inFlight flag used mainly to stop OLED updates which can take time and effect the RPM accuracy
	if (_mainRPMSensorDetectedRPM < 3500 || _mainRPMSensorDetectedRPM > 30000) {
		_mainRPMSensorDetectedRPM = 0;
		_inFlight = false;
	}
	else {
		_inFlight = true;
	}
}


// Setup the interrupt pins for the attached RPM sensors
void _rpm_ActivateInterrupts() {

#if defined(__MK20DX256__) || defined(__MK64FX512__)
	// Pull up the PIN and activate interrupt IRQ 0 (NANO pin2, Teensy pin1) for the RPM sensor
	pinMode(PIN_MAIN_RPM_SENSOR_INTERRUPT_0, INPUT_PULLUP);
	attachInterrupt(PIN_MAIN_RPM_SENSOR_INTERRUPT_0, onMainRPMSensorPulse, FALLING);
	// Pull up the PIN and activate interrupt IRQ 1 (NANO pin3, Teensy pin1) for the RPM sensor
	pinMode(PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1, INPUT_PULLUP);
	attachInterrupt(PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1, onClutchRPMSensorPulse, FALLING);
	Serial.println("Setup Teensy Interrupts for RPM");
#else
	// Pull up the PIN and activate interrupt IRQ 0 (NANO pin2, Teensy pin1) for the RPM sensor
	pinMode(PIN_MAIN_RPM_SENSOR_INTERRUPT_0, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(2), onMainRPMSensorPulse, CHANGE);
	// Pull up the PIN and activate interrupt IRQ 1 (NANO pin3, Teensy pin1) for the RPM sensor
	pinMode(PIN_CLUTCH_RPM_SENSOR_INTERRUPT_1, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(3), onClutchRPMSensorPulse, CHANGE);
	Serial.println("Setup Nano Interrupts for RPM");
#endif
}


// Private Functions

// The interrupt routine
void onMainRPMSensorPulse() {
	mainRPMSensorReadings++;
}


// The interrupt routine
void onClutchRPMSensorPulse() {
	clutchRPMSensorReadings++;
}