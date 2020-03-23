// 
// 
// 

#include "Config.h"
#include "RxLinkQuality.h"


// Config
SBUS sbus(Serial1);
const byte BAD_FRAME_MAX_INCREASE = 10;
const byte BAD_FRAME_NORMAL_INCREASE = 8;
const uint16_t MAX_WAIT_TIME_MS = 200;


// Public Variables
uint16_t lostFramesPercentage100Result = 0;
uint16_t badFramesPercentage100Result = 0;
uint32_t totalFrames = 0;
byte badFramesMonitoringChannel = 0;


// Private Variables
uint16_t channels[16];
uint16_t channelsPrevious[16];
bool lostFrame = false;
bool failSafe = false;

uint16_t badFramesDifference = 0;
uint32_t badFramesPercentage100Counter = 0;
uint32_t badFramesPercentage100Array[100] = { 0 };


uint32_t lostFrameCounter = 0;
bool lostFrameDetected = false;
unsigned long lostFrameStartMillis = 0;
uint32_t lostFrameLongestMillis = 0;
uint32_t lostFramesPercentage100Counter = 0;
uint32_t lostFramesPercentage100Array[100] = { 0 };


//TODO - Add FailSafe Counter
//TODO - Add the Maximum Channels Held over last 100 frames 
//TODO - Add the Min / Max SBUS Refresh Rates over last 100 frames


// Check the Quality of the Rx signal
void rxLinkQuality_Scan() {
	if (sbus.read(&channels[0], &failSafe, &lostFrame)) {
		// Increase total frames received
		totalFrames++;

#if defined(DEBUG_DATA)
		if (failSafe) { Serial.print("failSafe = "); Serial.println(failSafe); }
		if (lostFrame) { Serial.print("lostFrame = "); Serial.println(lostFrame); }
		debug_Data();
#endif

		calculate_LostFrames();
		calculate_BadFrames();
	}
}


// Check the "real" lost frames based on SBUS extra data changes
void calculate_BadFrames() {
	// check we know the correct scan channel before attempting to scan
	if (badFramesMonitoringChannel == 0) { badFramesMonitoringChannel = find_WaveChannel(); }

	// Did SBUS channel increase by more than an expected amount
	badFramesDifference = abs(channels[badFramesMonitoringChannel] - channelsPrevious[badFramesMonitoringChannel]);
	if (badFramesDifference > BAD_FRAME_MAX_INCREASE) {
		
		// calculate how many frames were skipped
		int badFrames = ((float)(abs(channels[badFramesMonitoringChannel] - channelsPrevious[badFramesMonitoringChannel])) / BAD_FRAME_NORMAL_INCREASE) - 1;

#if defined(REPORT_ERRORS)		
		Serial.print(badFrames); Serial.println(" bad frames found");
#endif

		// Add number of bad frames to the array
		badFramesPercentage100Array[badFramesPercentage100Counter] = badFrames;
	}
	else {
		// Good frame so add a Zero to the array
		badFramesPercentage100Array[badFramesPercentage100Counter] = 0;
	}
	badFramesPercentage100Counter++;
	// If we reach the end of the array overwrite from the beginning
	if (badFramesPercentage100Counter >= 100)  badFramesPercentage100Counter = 0;
	
	// Calculate bad frame % based on last 100 frames received.
	badFramesPercentage100Result = 0;
	for (int i = 0; i < 100; i++) {
		badFramesPercentage100Result += badFramesPercentage100Array[i];
	}
	// Due to lost frames it is possible for the result to be more than 100!!
	if (badFramesPercentage100Result > 100) badFramesPercentage100Result = 100;

	// The % calculation
	badFramesPercentage100Result = 100 - badFramesPercentage100Result;

	// Cpature the current channel value for the next loop
	channelsPrevious[badFramesMonitoringChannel] = channels[badFramesMonitoringChannel];
}


// Checks the Rx Lost Frmae flag in the SBUS
void calculate_LostFrames() {
	
	// Detect new Rx reported Lost Frame
	if (lostFrame == true && lostFrameDetected == false) {
		lostFrameStartMillis = millis();
		lostFrameCounter++;

#if defined(REPORT_ERRORS)
		Serial.println("");
		Serial.print("lostFrames Reported = "); Serial.println(lostFrameCounter);
		Serial.println("");
#endif

		lostFrameDetected = true;

		// Add a lost frame to the array if required
		lostFramesPercentage100Array[lostFramesPercentage100Counter] = 1;
	}
	else {
		lostFramesPercentage100Array[lostFramesPercentage100Counter] = 0;
	}

	// Detect Recovery of Rx reported Lost Frame
	if (lostFrame == false && lostFrameDetected == true) {
		uint32_t recoveryTime = millis() - lostFrameStartMillis;
		if (recoveryTime > lostFrameLongestMillis)  lostFrameLongestMillis = recoveryTime;

#if defined(REPORT_ERRORS)
		Serial.println("");
		Serial.print("lostFrame Recovered = "); Serial.print(recoveryTime); Serial.println("ms");
		Serial.println("");
#endif

		lostFrameDetected = false;
	}
	lostFramesPercentage100Counter++;
	// If we reach the end of the array overwrite from the beginning
	if (lostFramesPercentage100Counter >= 100)  lostFramesPercentage100Counter = 0;

	// Calculate bad frame % based on last 100 frames received.
	lostFramesPercentage100Result = 0;
	for (int i = 0; i < 100; i++) {
		lostFramesPercentage100Result += lostFramesPercentage100Array[i];
	}
	// It should not be possible to have an answer over 100 but just in case
	if (lostFramesPercentage100Result > 100) lostFramesPercentage100Result = 100;
	lostFramesPercentage100Result = 100 - lostFramesPercentage100Result;
}


// begin the SBUS communication
void rxLinkQuality_ActivateSBUS() {
	sbus.begin();
	delay(10);
	badFramesMonitoringChannel = find_WaveChannel();
}


// Search channels 1-16 for the channel that transmits the wave
byte find_WaveChannel() {
	uint32_t maxWaitTimeMillis = millis();
	byte waveChannel = 0;
	byte counter = 0;
	byte channelsAvg[16]{ 0 };
	byte maxHit = 0;
	// Scan all channels 20 times to see which has increases between 6 to 10.
	while (counter < 20 && millis() - maxWaitTimeMillis < MAX_WAIT_TIME_MS) {
		delay(5);
		if (sbus.read(&channels[0], &failSafe, &lostFrame)) {
			for (int ch = 0; ch < 16; ch++) {
				if (abs(channels[ch] - channelsPrevious[ch]) >= 6 && abs(channels[ch] - channelsPrevious[ch]) <= 10) {
					channelsAvg[ch]++;
					if (channelsAvg[ch] > maxHit) {
						maxHit = channelsAvg[ch];
						waveChannel = ch;
					}
				}
				channelsPrevious[ch] = channels[ch];
			}
			counter++;
		}
	}
	if (waveChannel != 0) {
		Serial.print("Wave Channel Found: "); Serial.println(waveChannel + 1);
		Serial.print("Scan Time (ms): "); Serial.println(millis() - maxWaitTimeMillis);
	}
	else {
		Serial.println("Wave Channel not Found");
	}
	return waveChannel;
}


//TODO - Changes will not work with only 8 Channels
// Dumps Previous vs Current Channel information to USB serial.
void debug_Data() {
#if defined(DEBUG_DATA)
	for (int ch = badFramesMonitoringChannel; ch < badFramesMonitoringChannel + 1; ch++) {
		Serial.print(micros()); Serial.print(":");
		Serial.print("CH"); Serial.print(ch + 1); Serial.print(" = ");
		Serial.print(channelsPrevious[ch]); Serial.print(" vs "); Serial.print(channels[ch]);
		Serial.print(" = "); Serial.print(channelsPrevious[ch] - channels[ch]);
		Serial.print("  BFP ="); Serial.println(badFramesPercentage100Result);
	}
#endif
}