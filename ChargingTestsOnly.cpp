// 
// 
// 

#include "ChargingTestsOnly.h"
#include "Telemetry.h"
#include "Power.h"
#include "Temperature.h"
#include "RPM.h"

byte status = 0;  // 0=off, 1=charge, 2=discharge, 3=pluse discharge
unsigned long storedDataTimer = millis();  // millis(), every 30 seconds write data to array.
unsigned long this_dischargeStoreTimeMs = millis();	// Sets to millis() each time the MAH has been calculated
//float this_dischargeLoopMAH = 0.00;					// The last MAH used during the loop
// this_dischargeTotalMAH = 0.00;					// Keeps the total MAH used during the whole cycle, can go up as well as down for charging / discharging
int aCounter = 0;						// Array Counter for the next reading, incremented just before adding new value.
unsigned long startOfTestTimer;			// millis() at the start of the test.
float stabilityVoltageDelta = 0.0000;	// voltage change during stability reporting loop
int stabiltyCount = 0;
constexpr auto REPORT_TIMER = 10000;			// default report time, in  millis(), 30000 = 30 seconds
constexpr auto PULSE_TIMER = 2000;					//	default pulse time, in millis(), 1000 = 1 second
byte tCounter = 0;												// Stability Counter

void chargingTestOnly_Setup() {
	power_Setup();
	
	pinMode(PIN_CHARGE_RELAY, OUTPUT);
	pinMode(PIN_DISCHARGE_RELAY, OUTPUT);
	pinMode(PIN_PULSE_DISCHARGE_MOSFET, OUTPUT);
	startOfTestTimer = millis();
	storedDataTimer = millis();
	this_dischargeStoreTimeMs = millis();
	//for (byte i = 0; i < 20; i++) {
	//	chargeRelay_Status_Status(true);
	//	dischargeRelay_Status_Status(true);
	//	dischargeMosfet_Status(true);
	//	delay(50);
	//	chargeRelay_Status_Status(false);
	//	dischargeRelay_Status_Status(false);
	//	dischargeMosfet_Status(false);
	//	delay(50);
	//}

		// Place this here to allow Serial to start
	Serial.println(""); Serial.println(""); Serial.println("Test Started");
	Serial.print("Headers:,");
	Serial.print("Counter,");
	Serial.print("Minutes,");
	Serial.print("Cycle Type,");
	Serial.print("Cell1 Voltage,");
	Serial.print("Cell2 Voltage,");
	Serial.print("Cell3 Voltage,");
	Serial.print("Amps,");
	Serial.print("Cycle mAh,");
	Serial.print("BEC Ouptut Voltage,");
	Serial.print("BAT Input Voltage,");
	Serial.println("Stability Counter");

}


void chargingTestOnly_Control() {
	
/*
		I need to test a 5 main routines next in this order
		stabiliseVoltages(100, 0.0040, true);			// good stabilty (needs to be 100 @ 0.0040 with FLVSS data smoothed at 0.25)
		stabiliseVoltages(40, 0.0040, true);			// reasonable stability
		pulseDischargeReading(5, PULSE_TIMER, 1);		// 0=PD with Good Stab, 1=PD with reasonable Stab, 2=PULSE_TIMER*5, 3=1 Sec 
		discharge_mAh(105);
		stabiliseVoltages(40, 0.0012, true);			// reasonable stability (with old cell read reoutines)
		chargeBattery();
*/
	//discharge_mAh(105);
	chargingTestOnly_Setup();

		// Place this here to allow Serial to start
	Serial.println(""); Serial.println(""); Serial.println("Test Started");
	Serial.print("Headers:,");
	Serial.print("Counter,");
	Serial.print("ms,");
	Serial.print("TeensyV,");
	Serial.print("RPM,");
	Serial.print("RecT,");
	Serial.print("RecV,");
	Serial.print("BecT,");
	Serial.print("BecV,");
	Serial.print("BecA,");
	Serial.print("BatA,");
	Serial.print("BatmAH,");
	Serial.print("Cell1,");
	Serial.println("Cell2");
	startOfTestTimer = millis();
	uint32_t counter = 0;

	//digitalWrite(PIN_DISCHARGE_RELAY, HIGH); 
	//digitalWrite(PIN_CHARGE_RELAY, HIGH);

	while (true) {
		calcualte_RPMSensorPulse();
		for(int i=0;i<100;i++) read_temperatures();
		power_Battery_Amps_ASC712();
		power_BEC_Amps_ASC712();
		power_chargeVoltages(); 
		telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
		//Serial.print("Data:,"); Serial.print(counter); Serial.print(", "); Serial.print(millis() - startOfTestTimer); Serial.print(", ");
		//Serial.print(teensy); Serial.print(", "); Serial.print(mainRPMSensorDetectedRPM); Serial.print(", ");
		//Serial.print(engineTemp); Serial.print(", "); Serial.print(reg); Serial.print(", "); 
		//Serial.print(ambientTemp); Serial.print(", "); Serial.print(bec); Serial.print(", "); 
		//Serial.print(becDischargeLoopAmps); Serial.print(", "); 
		//Serial.print(batteryDischargeLoopAmps); Serial.print(", "); Serial.print(0 - batteryDischargeTotalMAH); Serial.print(", ");
		//Serial.print(cellSmoothed[0]); Serial.print(", "); Serial.println(cellSmoothed[1]);
		counter++;
		if (engineTemp >= 35) { digitalWrite(PIN_DISCHARGE_RELAY, LOW); }
		if (ambientTemp >= 55) { digitalWrite(PIN_DISCHARGE_RELAY, LOW); }
		if (cellSmoothed[0] <= 3.0) { digitalWrite(PIN_DISCHARGE_RELAY, LOW); }
		if (cellSmoothed[1] <= 3.0) { digitalWrite(PIN_DISCHARGE_RELAY, LOW); }
		if (cellSmoothed[0] >= 3.43) { digitalWrite(PIN_CHARGE_RELAY, LOW); }
		if (cellSmoothed[1] >= 3.43) { digitalWrite(PIN_CHARGE_RELAY, LOW); }
		delay(300);
	}
}


void chargeBattery() {
	digitalWrite(PIN_CHARGE_RELAY, HIGH);
	while (true) { //(reg < 19.2) {
		storedDataTimer = millis();

		// If we think we have charged more than 1900mAH then stop the charge
		if (batteryDischargeTotalMAH > 1900) {
			storeDataInArrays(10);
			break;
		}

		// If any cell > 3.5 then stop the charge
		// We use the actual result of th FLVSS sensor for this, otherwise the smoothing lowers the value too much
		if ((cell[0] > 3.6) || (cell[1] > 3.6)) {
			storeDataInArrays(9);
			break;
		}

		// pause an acurate 1 second while maintaining telemetry that also updates cell[] and cellSmoothed[] values
		while (millis() - storedDataTimer < 1000) {
			telemetry_SendTelemetry();
		}
		storedDataTimer = millis();
		
		// Read the current AMPS going into the battery
		power_Battery_Amps_ASC714();
		storeDataInArrays(8);
	}
	digitalWrite(PIN_CHARGE_RELAY, LOW);
}


// Discharges in step of x mAH from the battery with Pulse Discharging in between to monitor battery recovery
// with flat battery cut off.
// Use discharge_mAh(105) for final code
void discharge_mAh(float mAh) {
	// Discharges a fix amount of mAh
	// Then after battery recovery completes a Pulse Discharge 
	// Cuts out if battery Cell voltage falls < 3.0v
	// Validated as working after Voltage and AMPs calibration

	byte lowVoltage = false;
	float thisDischargeMAH = 0.000;
	float thismAh = 0.000; 
	batteryDischargeLoopAmps = 0.000;

	// pulse discharge so we get a first reading, ultimately this is the data needed
	// what is the voltage drop and recovery when the battery has x % remaining
	pulseDischargeReading(5, PULSE_TIMER, 0);

	while (!lowVoltage) {
		// Start the discharge.
		this_dischargeStoreTimeMs = millis();
		thisDischargeMAH = 0.000;
		digitalWrite(PIN_DISCHARGE_RELAY, HIGH);

		// calculate want mAh we need to remove to ensure
		// at each pulse discharge we are at the correct
		// battery capacity.
		// this takes into account that the pulse discharge
		// effects the remaining capacity.
		thismAh = mAh - (((batteryDischargeTotalMAH / mAh) - int(batteryDischargeTotalMAH / mAh)) * mAh);
		//Serial.println(thismAh);
		int delayLowVoltageCounterOnStartUp = 0;
		while (thisDischargeMAH <= thismAh) {
			read_temperatures();
			power_Battery_Amps_ASC714();
			telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
			thisDischargeMAH += batteryDischargeLoopMAH;
			//Serial.println(thisDischargeMAH);
			// delayLowVoltageCounterOnStartUp allows battery to stabilise at start of a discharge
			// otherwise it can be detected as a flat battery way before its actaully flat
			if ((cellSmoothed[0] <= 3.0 || cellSmoothed[1] <= 3.0) && delayLowVoltageCounterOnStartUp >= 10) {
				lowVoltage = true;
				break;
			}
			if (delayLowVoltageCounterOnStartUp < 1000) delayLowVoltageCounterOnStartUp++;
			//storeDataInArrays(2);
		}
		// Stop the discharge
		digitalWrite(PIN_DISCHARGE_RELAY, LOW);

		// Always write the data, the constant is battery capacity not time
		if (!lowVoltage) {
			storeDataInArrays(2);
			storedDataTimer = millis();
		}
		else {
			storeDataInArrays(4);
		}
		
		// pulse discharge so we get a first reading, ultimately this is the data needed
		// what is the voltage drop and recovery when the battery has x % remaining
		pulseDischargeReading(5, PULSE_TIMER, 0);
	}
}


// Stabilises the battery voltage and performs a discharge of around 3A for x ms and repeats x times
// Use pulseDischargeReading(1, 1000, 0); for final code
// attempts = number of times to repeat the whole process (inc stabilise)
// milliseconds - length of the discharge pulse
// stabilise = 0 to perform good stabilise, 1 to perform average stabilise, 2 to delay for 5  x milliseconds between pulses, > 2 back to back
void pulseDischargeReading(byte attempts, int milliseconds, char stabilise) {
	float sCell1 = 0.0000;
	float sCell2 = 0.0000;
	//float sCell3 = 0.0000;
	float dCell1 = 0.0000;
	float dCell2 = 0.0000;
	//float dCell3 = 0.0000;
	float fCell1 = 0.0000;
	float fCell2 = 0.0000;
	//float fCell3 = 0.0000;

	for (int i = 0; i < attempts; i++) {
		stabiltyCount = PULSE_TIMER;
		stabilityVoltageDelta = 0.0000;
		if (stabilise == 0) {
			stabiliseVoltages(100, 0.0040, true);
		}
		else if (stabilise == 1) {
			stabiliseVoltages(40, 0.0040, true);
		}
		else if (stabilise == 2) {
			delay(milliseconds * 5);
		}
		else {
			delay(1000);
		}
		// Prime everything and write data
		read_temperatures();
		power_Battery_Amps_ASC714();
		telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
		storedDataTimer = millis();
		sCell1 = cellSmoothed[0];
		sCell2 = cellSmoothed[1];
		stabilityVoltageDelta = (cellSmoothed[0] + cellSmoothed[1]) - (sCell1 + sCell2);
		stabiltyCount = PULSE_TIMER;
		storeDataInArrays(5);
		//Serial.print("Cell1: start "); Serial.println(sCell1);
		//Serial.print("Cell2: start "); Serial.println(sCell2);
		//Serial.print("Amps: "); Serial.println(this_dischargeTotalMAH);
		// Start the discharge.
		this_dischargeStoreTimeMs = millis();
		digitalWrite(PIN_PULSE_DISCHARGE_MOSFET, HIGH);
		// Keep the discharge for x ms
		while (millis() - this_dischargeStoreTimeMs < (unsigned long)milliseconds) {
		// We need to keep updating Telemetry
			telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
		//	Serial.print("Stabilise:"); Serial.print(" - Posistion: "); Serial.print(millis()); Serial.print(" - Cell1: "); Serial.print(cell1, 4); Serial.print(" v  -- Cell2: "); Serial.print(cell2, 4); Serial.println(" v");
		}
		telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
		power_Battery_Amps_ASC714();
		read_temperatures();
		storedDataTimer = millis();
		dCell1 = cellSmoothed[0];
		dCell2 = cellSmoothed[1];
		stabilityVoltageDelta = (dCell1 + dCell2) - (sCell1 + sCell2);
		stabiltyCount = PULSE_TIMER;
		storeDataInArrays(6);
		// Stop the discharge
		digitalWrite(PIN_PULSE_DISCHARGE_MOSFET, LOW);
		//Serial.print("Cell1: discharge "); Serial.println(dCell1);
		//Serial.print("Cell2: discharge "); Serial.println(dCell2);
		//Serial.print("Amps: "); Serial.println(this_dischargeTotalMAH);
		// Allow battery to recover for x ms
		this_dischargeStoreTimeMs = millis();
		while (millis() - this_dischargeStoreTimeMs < ((unsigned long)milliseconds) * 5) {
			// We need to keep updating Telemetry
			telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
		//	Serial.print("Stabilise:"); Serial.print(" - Posistion: "); Serial.print(millis()); Serial.print(" - Cell1: "); Serial.print(cell1, 4); Serial.print(" v  -- Cell2: "); Serial.print(cell2, 4); Serial.println(" v");
		}
		// Take recovery readings
		telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
		power_Battery_Amps_ASC714();
		read_temperatures();
		storedDataTimer = millis();
		fCell1 = cellSmoothed[0];
		fCell2 = cellSmoothed[1];
		stabilityVoltageDelta = (fCell1 + fCell2) - (dCell1 + dCell2);
		stabiltyCount = PULSE_TIMER;
		storeDataInArrays(7);
	}

	//Serial.print("Cell1: finish "); Serial.println(fCell1);
	//Serial.print("Cell2: finish "); Serial.println(fCell2);
	//Serial.print("Amps: "); Serial.println(this_dischargeTotalMAH);

	//Serial.println("");
	//Serial.print("Cell1: start "); Serial.print(sCell1); Serial.print(" discharge "); Serial.print(dCell1); Serial.print(" finish "); Serial.println(fCell1);
	//Serial.print("Cell1: d-s "); Serial.print(dCell1 - sCell1,4); Serial.print(" f-d "); Serial.print(fCell1 - dCell1); Serial.print(" s-f "); Serial.println(sCell1 - fCell1,4);
	//Serial.print("Cell2: start "); Serial.print(sCell2); Serial.print(" discharge "); Serial.print(dCell2); Serial.print(" finish "); Serial.println(fCell2);
	//Serial.print("Cell2: d-s "); Serial.print(dCell2 - sCell2,4); Serial.print(" f-d "); Serial.print(fCell2 - dCell2); Serial.print(" s-f "); Serial.println(sCell2 - fCell2,4);

	stabilityVoltageDelta = 0.0000;
}


// Waits until battery voltage is stable (not at cell level)
// Use stabiliseVoltages(100, 0.0009, true); for good stabilty
// Use stabiliseVoltages(40, 0.0012, true); for reasonable stabilty
// howStable = Number of consecutive voltages required (i.e. 100)
// accuracy = Maximum difference between each reading allowed before triggering counter restart (i.e. 0.0008)
// reportEvery30Seconds = true to report status by Serial.print
void stabiliseVoltages(byte howStable, float accuracy, bool reportEvery30Seconds) {
	unsigned long startMillis = 0;
	tCounter = 0;
	// Wait for voltage to stabilise before the test is started.
	float tCell1 = 0.0000;
	float tCell2 = 0.0000;
	//float tCell3 = 0.0000;

	// check every second until we have "howStable" readings all the same.
	while (tCounter < howStable) {
		startMillis = millis();
		power_chargeVoltages();
		telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
		// if not the same as the previous reading start counter again.
		float answer = (cellSmoothed[0] + cellSmoothed[1]) - (tCell1 + tCell2);
		if (abs(stabilityVoltageDelta) < abs(answer) && tCell1 != 0) stabilityVoltageDelta = answer;
		if (answer <= -accuracy || answer >= accuracy) tCounter = 0;
		tCounter++;
		tCell1 = cellSmoothed[0];
		tCell2 = cellSmoothed[1];
		//Serial.print("Stabilise:"); Serial.print(" - Posistion: "); Serial.print(tCounter); Serial.print(" - Cell1: "); Serial.print(cell1, 4); Serial.print(" v  -- Cell2: "); Serial.print(cell2, 4); Serial.println(" v");
		// Create a delay for 1 second, triggering the update if need be
		while (millis() - startMillis < 1000) {
			telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
			if (millis() - storedDataTimer > REPORT_TIMER) {
				storedDataTimer = millis();
				stabiltyCount = tCounter;
				if (reportEvery30Seconds) storeDataInArrays(1);
				stabilityVoltageDelta = 0;
			}
		}
	}
	// dont leave until we have just written the latest data
	while (millis() - storedDataTimer < REPORT_TIMER) {
		// Keep telemetry live
		telemetry_SendTelemetry();  // Updates cell[] and cellSmoothed[] values
	}
	storedDataTimer = millis();
	stabiltyCount = tCounter;
	storeDataInArrays(1);
	stabilityVoltageDelta = 0;
}


void storeDataInArrays(byte cycleType) {
	//display.clearDisplay();


	//char  t[7];
	//display.setCursor(0, 0); display.print("T: ");
	//display.setCursor(20, 0); display.print((millis() - startOfTestTimer) / 1000 / 60);
	//display.setCursor(0, 10); display.print("C: ");
	//dtostrf(cell1, 6, 3, t);
	//display.setCursor(20, 10); display.print(t);
	//display.setCursor(58, 10); display.print(" : ");
	//dtostrf(cell2, 6, 3, t);
	//display.setCursor(78, 10); display.print(t);

	//dtostrf(cell1 + cell2, 6, 3, t);
	//display.setTextSize(2);
	//display.setCursor(0, 20); display.print("V:");
	//display.setCursor(30, 20); display.print(t);
	//display.setTextSize(1);

	//display.setCursor(0, 40); display.print("A: ");
	//dtostrf(this_dischargeTotalMAH, 6, 2, t);
	//display.setCursor(20, 40); display.print(t);
	//display.setCursor(58, 40); display.print(" : ");
	//dtostrf(this_dischargeTotalMAH, 6, 0, t);
	//display.setCursor(78, 40); display.print(t);

	//display.setCursor(0, 50); display.print("B: ");
	//dtostrf(bec, 6, 3, t);
	//display.setCursor(20, 50); display.print(t);
	//display.setCursor(58, 50); display.print(" : ");
	//dtostrf(reg, 6, 2, t);
	//display.setCursor(78, 50); display.print(t);

	//display.display();

	Serial.print("StoreData:,");
	Serial.print(aCounter); Serial.print(",");
	Serial.print((millis() - startOfTestTimer) / 1000 / 60); Serial.print(",");
	Serial.print(cycleType); Serial.print(",");
	Serial.print(cellSmoothed[0], 4); Serial.print(",");
	Serial.print(cellSmoothed[1], 4); Serial.print(",");
	Serial.print(cellSmoothed[2], 4); Serial.print(",");
	Serial.print(batteryDischargeLoopAmps); Serial.print(",");
	Serial.print(batteryDischargeTotalMAH); Serial.print(",");
	Serial.print(bec); Serial.print(",");
	Serial.print(reg); Serial.print(",");
	Serial.println(tCounter);
	aCounter++;
}


void chargeRelay_Status(bool status) {
	if (status) digitalWrite(PIN_CHARGE_RELAY, HIGH); else digitalWrite(PIN_CHARGE_RELAY, LOW);
}

void dischargeRelay_Status(bool status) {
	if (status) digitalWrite(PIN_DISCHARGE_RELAY, HIGH); else digitalWrite(PIN_DISCHARGE_RELAY, LOW);
}

void dischargeMosfet_Status(bool status) {
	if (status) digitalWrite(PIN_PULSE_DISCHARGE_MOSFET, HIGH); else digitalWrite(PIN_PULSE_DISCHARGE_MOSFET, LOW);
}