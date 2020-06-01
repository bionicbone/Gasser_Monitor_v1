// 
// 
// 

#include "SdCard.h"
#include "Config.h"
#include "SD.h"
#include "SPI.h"
#include "TimeLib.h"
#include "RPM.h"
#include "RxLinkQuality.h"
#include "Power.h"
#include "Telemetry.h"
#include "Temperature.h"


// Private Variables
bool sdCardLoggingActive = false;

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
		// Choose the Next FileName number
		char myFileName[13];
		char fileNamePrefix[] = "GM_";
		char fileNameSuffix[] = ".csv";
		for (uint16_t i = 0; i < 65535; i++) {
			sprintf(myFileName, "%s%u%s", fileNamePrefix, i, fileNameSuffix);
			if (!SD.exists(myFileName)) break;
		}
		// Open the filename
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
		text.concat(","); text.concat(_sbusFrameLowMicros);
		text.concat(","); text.concat(_sbusFrameHighMicros);
		text.concat(","); text.concat(_wave1);
		text.concat(","); text.concat(_wave2);
		// Power / Charging Data
		text.concat(","); text.concat(_recVoltage);
		text.concat(","); text.concat(_becVoltage);
		text.concat(","); text.concat(_becDischargeLoopAmps);
		text.concat(","); text.concat(_becTemp);
		float cells = cell[0] + cell[1];
		text.concat(","); text.concat(cells);
		text.concat(","); text.concat(_batteryDischargeLoopAmps);
		text.concat(","); text.concat(_batteryDischargeTotalMAH);
		text.concat(","); text.concat(cell[0]);
		text.concat(","); text.concat(cell[1]);
		text.concat(","); text.concat(_teensyVoltage);
		// Temperature Data
		text.concat(","); text.concat(_ambientTemp);
		text.concat(","); text.concat(_canopyTemp);
		text.concat(","); text.concat(_engineTemp);


		Serial.println(text);
		SdFile.println(text);
		// Just flush the data, never close the file:
		SdFile.flush();
	}
	else {
		// if the file didn't open, print an error:
		// TODO - Send Telemetry Error instead
		Serial.println("error writting test.txt");
	}
}


// Private Functions

void sd_WriteLogHeader() {
	if (SdFile) {
		String text = "Date,Time,ERR,ErrD,InFlight,MAIN,CLUT,TFCK,LFP,E2EQ,BFP,MCHM,16FS,SBLO,SBHI,Wav1,Wav2,RecV,BecV,BecA,BecT,CELS,BatA,BmAH,Cel1,Cel2,TeeV,AmbT,CanT,EngT";
		Serial.println(text);
		SdFile.println(text);
		// Just flush the data, never close the file:
		SdFile.flush();
	}
	else {
		// if the file didn't open, print an error:
		// TODO - Send Telemetry Error instead
		Serial.println("error writting test.txt");
	}
}