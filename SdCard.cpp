// 
// 
// 

#include "SdCard.h"
#include "Config.h"
#include "SD.h"
#include "SPI.h"
#include "RPM.h"
#include "RxLinkQuality.h"
#include "Power.h"
#include "Telemetry.h"
#include "Temperature.h"
#include <TimeLib.h>
#include "MCU6050.h"


// Private Variables
bool sdCardLoggingActive = false;
bool fileExists = false;

File SdFile;

// Public Functions

void _sd_SetUp() {
	delay(100);
	if (!SD.begin(14)) { // CS Pin 14
		// TODO - Send Telemetry Error instead - persistent
		Serial.println("initialization failed!");
		sdCardLoggingActive = false;
	}
	else {
		sdCardLoggingActive = true;
		char myFileName[13];

		// New Method
		// Format Date for Filename
		char fileNamePrefix[9];
		char fileNameSuffix[5] = ".csv";
		char stryear[5], strmonth[3], strday[3];
		sprintf(stryear, "%04d", year());
		sprintf(strmonth, "%02d", month());
		sprintf(strday, "%02d", day());

		strcpy(fileNamePrefix, stryear);
		strcat(fileNamePrefix, strmonth);
		strcat(fileNamePrefix, strday);

		sprintf(myFileName, "%s%s", fileNamePrefix, fileNameSuffix);

		// Open the filename
		fileExists = SD.exists(myFileName);
		SdFile = SD.open(myFileName, FILE_WRITE);
		delay(1);
		sd_WriteLogHeader();
	}
}


void _sd_WriteLogDate() {
	// if the file opened okay, write to it:
	if (SdFile && sdCardLoggingActive) {
		String text = "";
		// Format data and time 
		char strmonth[3], strday[3], strhour[3], strminute[3], strsecond[3];
		sprintf(strmonth, "%02d", month());
		sprintf(strday, "%02d", day());
		sprintf(strhour, "%02d", hour());
		sprintf(strminute, "%02d", minute());
		sprintf(strsecond, "%02d", second());
		text.concat(year()); text.concat("-"); text.concat(strmonth); text.concat("-"); text.concat(strday);
		text.concat(","); text.concat(strhour); text.concat(":"); text.concat(strminute); text.concat(":"); text.concat(strsecond);
		text.concat("."); text.concat(millis());
		// Error Data
		text.concat(","); text.concat("ERR");
		text.concat(","); text.concat("ErrD");
		// RPM Data
		text.concat(","); text.concat(_inFlight);
		text.concat(","); text.concat(_mainRPMSensorDetectedRPM);
		text.concat(","); text.concat(_clutchRPMSensorDetectedRPM);
		// SBUS Quality Data
		text.concat(","); text.concat(_totalFrames);
		text.concat(","); text.concat(_lostFramesPercentage100Result);
		text.concat(","); text.concat(_overallE2EQuality);
		text.concat(","); text.concat(_badFramesPercentage100Result);
		text.concat(","); text.concat(_channelsMaxHoldMillis100Resul);
		text.concat(","); text.concat(_channel16chFrameSyncSuccessRate);
		text.concat(","); text.concat(_wave1);
		text.concat(","); text.concat(_wave2);
		// Power / Charging Data
		text.concat(","); text.concat(_recVoltage);
		text.concat(","); text.concat(_becVoltage);
		text.concat(","); text.concat(_becDischargeLoopAmps);
		text.concat(","); text.concat(_becTemp);
		text.concat(","); text.concat(_batteryVoltage);
		text.concat(","); text.concat(_batteryDischargeLoopAmps);
		text.concat(","); text.concat(_batteryDischargeTotalMAH);
		float cells = cell[0] + cell[1];
		text.concat(","); text.concat(cells);
		text.concat(","); text.concat(cell[0]);
		text.concat(","); text.concat(cell[1]);
		text.concat(","); text.concat(_teensyVoltage);
		// Temperature Data
		text.concat(","); text.concat(_ambientTemp);
		text.concat(","); text.concat(_canopyTemp);
		text.concat(","); text.concat(_engineTemp);
		// Vibration Data
		text.concat(","); text.concat(_AccX);
		text.concat(","); text.concat(_AccY);
		text.concat(","); text.concat(_AccZ);
		text.concat(","); text.concat(_AccTmp);
		text.concat(","); text.concat(_GyrX);
		text.concat(","); text.concat(_GyrY);
		text.concat(","); text.concat(_GyrZ);
		text.concat(","); text.concat(_vibrationStatus);

		Serial.println(text);
		SdFile.println(text);
		// Just flush the data, never close the file:
		SdFile.flush();
	}
	else {
		// if the file didn't open, print an error:
		// TODO - Send Telemetry Error instead
		Serial.println("error writting to SD Card");
	}
}


// Private Functions

void sd_WriteLogHeader() {
	if (SdFile) {
		String text = "Date,Time,ERR,ErrD,InFlight,MAIN,CLUT,TFCK,LFP,E2EQ,BFP,MCHM,16FS,Wav1,Wav2,RecV,BecV,BecA,BecT,BatV,BatA,BmAH,CELS,Cel1,Cel2,TeeV,AmbT,CanT,EngT,AccX,AccY,AccZ,AccTmp,GyrX,GyrY,GyrZ,VibS";
		Serial.println(text);
		if (!fileExists) {
			SdFile.println(text);
			Serial.println("Header written to SD Card");
		}
		// Just flush the data, never close the file:
		SdFile.flush();
	}
	else {
		// if the file didn't open, print an error:
		// TODO - Send Telemetry Error instead
		Serial.println("error writting to SD Card");
	}
}