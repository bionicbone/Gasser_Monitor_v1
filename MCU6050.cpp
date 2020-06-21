// 
// 
// 

#include "MCU6050.h"
#include <Wire.h>
extern TwoWire Wire1;

// Private Variables
int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;					// MCU-6050 values
int avgAcX, avgAcY, avgAcZ, avgTmp;							// MCU-6050 are calcualted over several readings (50 hard coded)
int avgGyX, avgGyY, avgGyZ;											// MCU-6050 are calcualted over several readings (50 hard coded)
int MCU6050Readings = 0;												// Number of MCU-6050 readings for calculating the average
int MCU6050WakeCounter = 0;											// Number of times one of the MCU-6050 WAKE readings is more then the allowed difference in a row
int MCU6050EngineCounter = 0;										// Number of times one of the MCU-6050 ENGINE readings is more then the allowed difference in a row
bool wakeUp = false;														// true if the OLED screen should be on
bool wakeUpNow = false;													// true if the OLED screen should initilise from a sleep, forces refresh
unsigned long wakeUpStartTimerMS = 0;						// the millis() when the OLED screen wakes up


// Public Functions
void _mcu6050_Setup() {
	Wire1.begin();
	Wire1.beginTransmission(MPU_addr);
	Wire1.write(0x6B);  // PWR_MGMT_1 register
	Wire1.write(0);     // set to zero (wakes up the MPU-6050)
	Wire1.endTransmission(true);
}


void _mcu6050_Read() {

	if (wakeUp == true && millis() > wakeUpStartTimerMS + WAKE_UP_TIME) wakeUp = false;
	Wire1.beginTransmission(MPU_addr);
	Wire1.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
	Wire1.endTransmission(false);

	Wire1.requestFrom(MPU_addr, 14, true);  // request a total of 14 registers
	avgAcX += Wire1.read() << 8 | Wire1.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
	avgAcY += Wire1.read() << 8 | Wire1.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
	avgAcZ += Wire1.read() << 8 | Wire1.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
	avgTmp += Wire1.read() << 8 | Wire1.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
	avgGyX += Wire1.read() << 8 | Wire1.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
	avgGyY += Wire1.read() << 8 | Wire1.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
	avgGyZ += Wire1.read() << 8 | Wire1.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
	MCU6050Readings++;

	if (MCU6050Readings > 50) {
		int AcX_Diff = AcX - (avgAcX / MCU6050Readings);
		int AcY_Diff = AcY - (avgAcY / MCU6050Readings);
		int AcZ_Diff = AcZ - (avgAcZ / MCU6050Readings);

		if (AcX && AcY && AcZ) {  // Safe guards the initial run of the code
			// Work out if the Diff has been Breached at wake up level
			if ((abs(AcX_Diff) >= MCU_WAKE_AcX_ALLOWED_DIFF
				&& abs(AcX_Diff) < MCU_ENGINE_AcX_ALLOWED_DIFF)
				|| (abs(AcY_Diff) >= MCU_WAKE_AcY_ALLOWED_DIFF
					&& abs(AcY_Diff) < MCU_ENGINE_AcY_ALLOWED_DIFF)
				|| (abs(AcZ_Diff) >= MCU_WAKE_AcZ_ALLOWED_DIFF
					&& abs(AcZ_Diff) < MCU_ENGINE_AcZ_ALLOWED_DIFF)) {
				MCU6050WakeCounter++;
			}
			else {
				MCU6050WakeCounter = 0;
			}
			if (abs(AcX_Diff) >= MCU_ENGINE_AcX_ALLOWED_DIFF
				|| abs(AcY_Diff) >= MCU_ENGINE_AcY_ALLOWED_DIFF
				|| abs(AcZ_Diff) >= MCU_ENGINE_AcZ_ALLOWED_DIFF) {
				MCU6050EngineCounter++;
			}
			else {
				MCU6050EngineCounter = 0;
			}
		}
		// Do I need to report?
		if (millis() > MCU_STABILITY_DELAY) { // only perform updates once stable
			// Do we need to wake up?
			if (MCU6050WakeCounter >= 2) {
				wakeUpStartTimerMS = millis();
				wakeUp = true;
				wakeUpNow = true;
				Serial.println("Wake Up...");
				Serial.print("AcX Diff = "); Serial.println(abs(AcX_Diff));
				Serial.print("AcY Diff = "); Serial.println(abs(AcY_Diff));
				Serial.print("AcZ Diff = "); Serial.println(abs(AcZ_Diff));
			}
			else if (MCU6050EngineCounter >= 100) {
				wakeUp = false;
				Serial.println("Engine Running...");
				Serial.print("AcX Diff = "); Serial.println(abs(AcX_Diff));
				Serial.print("AcY Diff = "); Serial.println(abs(AcY_Diff));
				Serial.print("AcZ Diff = "); Serial.println(abs(AcZ_Diff));
			}


			AcX = avgAcX / MCU6050Readings;
			AcY = avgAcY / MCU6050Readings;
			AcZ = avgAcZ / MCU6050Readings;
			Tmp = avgTmp / MCU6050Readings;
			Tmp = Tmp / 340.00 + 36.53;
			GyX = avgGyX / MCU6050Readings;
			GyY = avgGyY / MCU6050Readings;
			GyZ = avgGyZ / MCU6050Readings;


			// TODO - Add Calibration

		}

		MCU6050Readings = 0;
		avgAcX = 0;
		avgAcY = 0;
		avgAcZ = 0;
		avgTmp = 0;
		avgGyX = 0;
		avgGyY = 0;
		avgGyZ = 0;


		//Serial.print("AcX = "); Serial.print(AcX);
		//Serial.print(" | AcY = "); Serial.print(AcY);
		//Serial.print(" | AcZ = "); Serial.print(AcZ);
		//Serial.print(" | Tmp = "); Serial.print(Tmp / 340.00 + 36.53);  //equation for temperature in degrees C from datasheet
		//Serial.print(" | GyX = "); Serial.print(GyX);
		//Serial.print(" | GyY = "); Serial.print(GyY);
		//Serial.print(" | GyZ = "); Serial.println(GyZ);
	}

}




