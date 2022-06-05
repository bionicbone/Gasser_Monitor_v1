// 
// 
// 

#include "MCU6050.h"
#include <Wire.h>
extern TwoWire Wire1;

// Public Variables
int _AccX, _AccY, _AccZ, _AccTmp, _GyrX, _GyrY, _GyrZ;	// MCU-6050 values
byte _vibrationStatus = 0;															// MCU-6050 status based on values, 0 = Low to zero vibration, 1 = Wake Up OLED, 2 = Engine Running, 3 = High Vibration, 4 = Extreme Vibration


// Private Variables
int avgAccX, avgAccY, avgAccZ, avgTmp;							// MCU-6050 are calcualted over several readings (50 hard coded)
int avgGyrX, avgGyrY, avgGyrZ;											// MCU-6050 are calcualted over several readings (50 hard coded)
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
	avgAccX += Wire1.read() << 8 | Wire1.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
	avgAccY += Wire1.read() << 8 | Wire1.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
	avgAccZ += Wire1.read() << 8 | Wire1.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
	avgTmp += Wire1.read() << 8 | Wire1.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
	avgGyrX += Wire1.read() << 8 | Wire1.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
	avgGyrY += Wire1.read() << 8 | Wire1.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
	avgGyrZ += Wire1.read() << 8 | Wire1.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
	MCU6050Readings++;

	if (MCU6050Readings > 50) {
		int AccX_Diff = _AccX - (avgAccX / MCU6050Readings);
		int AccY_Diff = _AccY - (avgAccY / MCU6050Readings);
		int AccZ_Diff = _AccZ - (avgAccZ / MCU6050Readings);
		_vibrationStatus = 0;

		if (_AccX && _AccY && _AccZ) {  // Safe guards the initial run of the code
			// Work out if the Diff has been Breached at wake up level
			if ((abs(AccX_Diff) >= MCU_WAKE_AccX_ALLOWED_DIFF
				&& abs(AccX_Diff) < MCU_ENGINE_AccX_ALLOWED_DIFF)
				|| (abs(AccY_Diff) >= MCU_WAKE_AccY_ALLOWED_DIFF
					&& abs(AccY_Diff) < MCU_ENGINE_AccY_ALLOWED_DIFF)
				|| (abs(AccZ_Diff) >= MCU_WAKE_AccZ_ALLOWED_DIFF
					&& abs(AccZ_Diff) < MCU_ENGINE_AccZ_ALLOWED_DIFF)) {
				MCU6050WakeCounter++;
			}
			else {
				MCU6050WakeCounter = 0;
			}
			if (abs(AccX_Diff) >= MCU_ENGINE_AccX_ALLOWED_DIFF
				|| abs(AccY_Diff) >= MCU_ENGINE_AccY_ALLOWED_DIFF
				|| abs(AccZ_Diff) >= MCU_ENGINE_AccZ_ALLOWED_DIFF) {
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
				_vibrationStatus = 1;
				Serial.println("Wake Up...");
				Serial.print("AcX Diff = "); Serial.println(abs(AccX_Diff));
				Serial.print("AcY Diff = "); Serial.println(abs(AccY_Diff));
				Serial.print("AcZ Diff = "); Serial.println(abs(AccZ_Diff));
			}
			else if (MCU6050EngineCounter >= 100) {
				wakeUp = false;
				_vibrationStatus = 2;
				Serial.println("Engine Running...");
				Serial.print("AcX Diff = "); Serial.println(abs(AccX_Diff));
				Serial.print("AcY Diff = "); Serial.println(abs(AccY_Diff));
				Serial.print("AcZ Diff = "); Serial.println(abs(AccZ_Diff));
			}


			_AccX = avgAccX / MCU6050Readings;
			_AccY = avgAccY / MCU6050Readings;
			_AccZ = avgAccZ / MCU6050Readings;
			_AccTmp = avgTmp / MCU6050Readings;
			_AccTmp = _AccTmp / 340.00 + 36.53;
			_GyrX = avgGyrX / MCU6050Readings;
			_GyrY = avgGyrY / MCU6050Readings;
			_GyrZ = avgGyrZ / MCU6050Readings;


			// TODO - Add Calibration

		}

		MCU6050Readings = 0;
		avgAccX = 0;
		avgAccY = 0;
		avgAccZ = 0;
		avgTmp = 0;
		avgGyrX = 0;
		avgGyrY = 0;
		avgGyrZ = 0;


		//Serial.print("AccX = "); Serial.print(_AccX);
		//Serial.print(" | AccY = "); Serial.print(_AccY);
		//Serial.print(" | AccZ = "); Serial.print(_AccZ);
		//Serial.print(" | AccTmp = "); Serial.print(_Tmp); 
		//Serial.print(" | GyrX = "); Serial.print(_GyrX);
		//Serial.print(" | GyrY = "); Serial.print(_GyrY);
		//Serial.print(" | GyrZ = "); Serial.println(_GyrZ);
	}

}




