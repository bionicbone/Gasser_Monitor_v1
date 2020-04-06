// 
// 
// 

#include "Config.h"
#include "RxLinkQuality.h"


// Config
SBUS sbus(Serial1);
const uint16_t MAX_WAIT_TIME_MS = 200;


// Public Variables
uint16_t	lostFramesPercentage100Result = 0;
uint16_t	badFramesPercentage100Result = 0;
uint32_t	totalFrames = 0;
byte			badFramesMonitoringChannel1 = 0;
byte			badFramesMonitoringChannel2 = 0;
uint32_t	failSafeCounter = 0;
uint32_t	failSafeLongestMillis = 0;
uint16_t	wave1 = 0;																			// Used to pass current value to telemetry
uint16_t	wave2 = 0;																			// Used to pass current value to telemetry
uint32_t  channelsMaxHoldMillis100Resul = 0;							// Stores max millis() for every 100 readings, for 16ch 2 waves it is the highest of all readings
float			channel16chFrameSyncSuccessRate = 0;						// Store the SBUS Frame Sync Success Rate when in 16ch mode, should be >98% based on X4R


// Private Variables
uint16_t channels[16];
uint16_t channelsPrevious[16];
bool lostFrame = false;
bool failSafe = false;

uint16_t badFramesDifference = 0;
uint32_t badFramesPercentage100Counter = 0;
uint32_t badFramesPercentage100Array[100] = { 0 };
byte badFramesMonitoringType = 0;

uint32_t		channelsStartHoldMillis = 0 ;						// Stores millis() when hold is first detected 
bool				channelHoldTriggered[3] = { false };		// Tracks current status
uint8_t			channelHoldCounter = 0;									// Tracks reset after 100 readings
uint32_t		channelsMaxHoldMillis[3] = { 0 };			// Stores max millis() for every 100 readings (for some reason this has to be [3] not [2])
bool				channel16chFrameSync = false;						// True if the 1-8ch frame is expected next
bool				channel16chFrameSyncError = false;			// True if sync is incorrect or both frames change or both frames hold
uint32_t		channel16chFrameSyncErrorCounter = 0;		// increaments on each error, channel16chFrameSyncErrorCounter / totalFrames * 100 = %

uint32_t lostFrameCounter = 0;
bool lostFrameDetected = false;
unsigned long lostFrameStartMillis = 0;
uint32_t lostFrameLongestMillis = 0;
uint32_t lostFramesPercentage100Counter = 0;
uint32_t lostFramesPercentage100Array[100] = { 0 };

bool failSafeDetected = false;
unsigned long failSafeStartMillis = 0;


//TODO - Add the Maximum Channels Held over last 100 frames 
//TODO - Fix the wave values that are transmitted on telemetry
//TODO - Add the Min / Max SBUS Refresh Rates over last 100 frames
//TODO - Align Badframes with LQBB4 calculation -- testing !!
//TODO - Allow sending of the Test Wave Forms over Telemetry (possible two channels)
//TODO - Add an overall SBUS quality indicator, i.e. SBUS refresh rate, SBUS channel holds, SBUS Frame Sync Errors etc into one formula in every 100 frames to detect "new Rx" not acting like X4RSB.


// begin the SBUS communication
void rxLinkQuality_ActivateSBUS() {
	uint32_t maxWaitTimeMillis = millis();
	byte counter = 0;

	sbus.begin();

	/*
		Clear the SBUS when first starting by reading many frames.
		It was noted that the X4R can "infrequently" have strange values on ch9-16 when in 8ch mode using v1 LBT or v2.1.0 LBT
			- During start up sometimes Ch15 would have a value of 1187 for several frames
			- During start up many frames or randomly for one frame during flights all channels may contain the following values
					CH9 = 0, CH10 = 40, CH11 = 1496, CH12 = 64 (v1) or 72 (v2.1.0), CH13 102, CH14 = 82, CH15 1250, CH16 = 1920
		Given that this can happen randomly "for one frame" we have to add a coutner when detecting 16ch mode.
		Other Rx were not tested
	*/
	
	uint16_t maxWaitTime = MAX_WAIT_TIME_MS * 2;
	while (counter < 300 && millis() - maxWaitTimeMillis < maxWaitTime) {
		sbus.read(&channels[0], &failSafe, &lostFrame);
		counter++;
	}
	Serial.print("SBUS Startup Cleared with "); Serial.print(counter); Serial.print(" reads");
	Serial.print(" in (ms) "); Serial.println(millis() - maxWaitTimeMillis);
}


// Main control loop to determine the Quality of the Rx SBUS signal
void rxLinkQuality_Scan() {
	if (sbus.read(&channels[0], &failSafe, &lostFrame)) {
		// Increase total frames received
		totalFrames++;

#if defined(REPORT_FS_LF_ERRORS) || defined(DEBUG_WAVE_CHANNEL_DATA)
		if (failSafe) { Serial.print("millis():"); Serial.print(millis()); Serial.print("   failSafe = "); Serial.println(failSafe); }
		if (lostFrame) { Serial.print("millis():"); Serial.print(millis()); Serial.print("   lostFrame = "); Serial.println(lostFrame); }
#endif
#if defined(DEBUG_ALL_CHANNEL_DATA)
		debug_Data();
#endif
#if defined(DEBUG_WAVE_CHANNEL_DATA)
		debug_Wave_Data();
#endif
		
		check_FailSafe();
		calculate_LostFrames();
		
		// Call Frame Sync if in a 16ch mode
		if (badFramesMonitoringType > 0) { sync_16chFrame(); }
				
		calculate_FrameHolds();
		
		calculate_BB_Bits();		// must run last because previous channels is updated

		// Capture the current channel value for Telemetry
		wave1 = channels[badFramesMonitoringChannel1 - 1];
		wave2 = channels[badFramesMonitoringChannel2 - 1];

		// Capture the current channel value for the next loop
		channelsPrevious[badFramesMonitoringChannel1 - 1] = channels[badFramesMonitoringChannel1 - 1];
		channelsPrevious[badFramesMonitoringChannel2 - 1] = channels[badFramesMonitoringChannel2 - 1];
	}
}


// Internal Calls Only


// Check the "real" lost frames by monitoring a wave form and chacking the data increases on each SBUS update
// Also determines the correct Tx / Rx mode (8ch or 16ch)
void calculate_BB_Bits() {
#if defined(REPORT_CURRENT_BFP)	
	uint16_t lastbadFramesPercentage100Result = badFramesPercentage100Result;
#endif

	uint16_t MaxTriangleDiff = 0;
	// check we know the correct scan channel before attempting to scan
	if (badFramesMonitoringType == 0) { find_WaveChannel_New(badFramesMonitoringChannel1, badFramesMonitoringChannel2, badFramesMonitoringType); }

	/*
		badFramesMonitoringType == 1 - 8ch mode, wave on Ch1 - Ch8
		badFramesMonitoringType == 2 - 16ch mode, wave on Ch1 - Ch8
		badFramesMonitoringType == 3 - 16ch mode, wave on Ch9 - Ch16
		badFramesMonitoringType == 4 - 16ch mode, wave on Ch1 - Ch16
	*/

	bool goodFrame = true;
	// Did SBUS channel increase by more than an expected amount for 8 Channels
	badFramesDifference = abs(channels[badFramesMonitoringChannel1 - 1] - channelsPrevious[badFramesMonitoringChannel1 - 1]);

	// get channels 9-16 if applicable for the mode type and overwrite badFramesDifference
	if (badFramesMonitoringType >= 3) {
		badFramesDifference = abs(channels[badFramesMonitoringChannel2 - 1] - channelsPrevious[badFramesMonitoringChannel2 - 1]); 
	}

	// determine BB_Bit threshold based on BB Link Quality	
	if (badFramesMonitoringType == 1) {														// 8ch mode.
		//Serial.println("8ch Mode");
		MaxTriangleDiff = MAX_TRIANGLE_DIFF_8CH_1;												//  11
		if (badFramesPercentage100Result < TRSHLD_8CH_1_CHNG) {			// <75			
			MaxTriangleDiff = MAX_TRIANGLE_DIFF_8CH_2;											// 9
			if (badFramesPercentage100Result < TRSHLD_8CH_2_CHNG) {		// <50
				MaxTriangleDiff = MAX_TRIANGLE_DIFF_8CH_3;										// 9
			}
		}
	}
	else {																													// 16ch mode.
		// determine BB_Bit threshold based on BB Link Quality						
		//Serial.println("16ch Mode");
		MaxTriangleDiff = MAX_TRIANGLE_DIFF_16CH_1;												//  11
		if (badFramesPercentage100Result < TRSHLD_16CH_1_CHNG) {			// <75			
			MaxTriangleDiff = MAX_TRIANGLE_DIFF_16CH_2;											// 9
			if (badFramesPercentage100Result < TRSHLD_16CH_2_CHNG) {		// <50
				MaxTriangleDiff = MAX_TRIANGLE_DIFF_16CH_3;										// 9
			}
		}
	}


#if defined(REPORT_BAD_FRAME_ERRORS)	
	Serial.println("Monitor Bad Frames");
	debug_Wave_Data();
	Serial.print("badFramesDifference   "); Serial.println(badFramesDifference);
	Serial.print("MaxTriangleDiff   "); Serial.println(MaxTriangleDiff);
#endif


	/*
		MaxDiff has to be > 11 if badFramesPercentage100Result is >=75
		MaxDiff has to be > 9 if badFramesPercentage100Result is <75
		MaxDiff has to be > 9 if badFramesPercentage100Result is <50
	*/
	
	if (badFramesMonitoringType == 1) {															// 8ch mode
		if (badFramesDifference > MaxTriangleDiff) {
			// calculate how many frames were skipped
			uint16_t BB_Bits = badFramesDifference / MaxTriangleDiff;
#if defined(REPORT_BAD_FRAME_ERRORS)		
			Serial.print(BB_Bits); Serial.println(" BB_Bits found (8ch mode)");
#endif
			// Add number of bad frames to the array
			badFramesPercentage100Array[badFramesPercentage100Counter] = BB_Bits;
			goodFrame = false;
		}
	}
	else {																													// 16ch mode
		if (badFramesDifference > MaxTriangleDiff) {
			// calculate how many frames were skipped
			uint16_t BB_Bits = badFramesDifference / MaxTriangleDiff;
			// Add number of bad frames to the array
			if (badFramesMonitoringType == 4) {
				badFramesPercentage100Array[badFramesPercentage100Counter] = BB_Bits;				// Exact
#if defined(REPORT_BAD_FRAME_ERRORS)		
				Serial.print(BB_Bits); Serial.println(" BB_Bits found (16ch mode - Exact)");
#endif
			}
			else {
				// stop major LQ jumps due to estimation of 2nd wave by limiting BB_Bits and adding to the next frame
				if (BB_Bits >= MAX_16CH_ESTIMATED_BB_Bits) {
					badFramesPercentage100Array[badFramesPercentage100Counter] = BB_Bits;				// Limit Estimate	
				}
				else {
					badFramesPercentage100Array[badFramesPercentage100Counter] = BB_Bits * 2;		// Estimate
				}
#if defined(REPORT_BAD_FRAME_ERRORS)		
				Serial.print(badFramesPercentage100Array[badFramesPercentage100Counter]); Serial.println(" BB_Bits found (16ch mode - *2 Estimated)");
#endif
			}
			goodFrame = false;
		}
	}

	// Good frame so add a Zero to the array
	if (goodFrame) { badFramesPercentage100Array[badFramesPercentage100Counter] = 0; }

	// Add a bad frame if FailSafe is triggered, otherwise we stay on current % until is recovers
	if (failSafe) { badFramesPercentage100Array[badFramesPercentage100Counter] = 1; }

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

	/*
	badFramesMonitoringType == 1 - 8ch mode, wave on Ch1 - Ch8
	badFramesMonitoringType == 2 - 16ch mode, wave on Ch1 - Ch8
	badFramesMonitoringType == 3 - 16ch mode, wave on Ch9 - Ch16
	badFramesMonitoringType == 4 - 16ch mode, wave on Ch1 - Ch16
*/

// Handle Telemetry Reporting of the Monitoring Type and Channels
	if (totalFrames < 1000 && badFramesMonitoringType == 1) { badFramesPercentage100Result = 8; }
	if (totalFrames < 1000 && badFramesMonitoringType > 1) { badFramesPercentage100Result = 16; }
	if (totalFrames >= 1000 && totalFrames < 2000 && badFramesMonitoringType != 3) { badFramesPercentage100Result = badFramesMonitoringChannel1; }
	if (totalFrames >= 1500 && totalFrames < 2000 && badFramesMonitoringType == 4) { badFramesPercentage100Result = badFramesMonitoringChannel2; }

#if defined(REPORT_CURRENT_BFP)		
	if (badFramesPercentage100Result != lastbadFramesPercentage100Result) {
		Serial.print("millis(): "); Serial.print(millis()); Serial.print("   ");
		Serial.print("BFP: "); Serial.println(badFramesPercentage100Result);
	}
#endif
}


// Calculate the longest FrameHold in every 100 frames
// at least one channel should transmit the wave, check all 16 channels for movement.
// TODO - Add 16ch modes
void calculate_FrameHolds() {
	/*
	badFramesMonitoringType == 1 - 8ch mode, wave on Ch1 - Ch8
	badFramesMonitoringType == 2 - 16ch mode, 1 wave on Ch1 - Ch8
	badFramesMonitoringType == 3 - 16ch mode, 1 wave on Ch9 - Ch16
	badFramesMonitoringType == 4 - 16ch mode, 2 waves, 1 on Ch1 - Ch8 and 1 on ch9 - Ch16
	*/

#if defined(REPROT_CHANNEL_HOLD_DATA)
	Serial.println("Frame Holds Monitoring");
	debug_Wave_Data();
	Serial.print("FrameSyncError = "); Serial.println(channel16chFrameSyncError);
	Serial.print("FrameSyncValue = "); Serial.println(channel16chFrameSync);
	Serial.print("Current channelHoldTriggered = "); Serial.println(channelHoldTriggered[channel16chFrameSync]);
	Serial.print("Checking Hold on  1-8? = "); Serial.println(!channel16chFrameSync);
	Serial.print("Checking Hold on 9-16? = "); Serial.println(channel16chFrameSync);
#endif


	// only run if the frames are in sync.
	if (channel16chFrameSyncError == false) {
		channelHoldCounter++;
		
		Serial.print("channelHoldCounter = "); Serial.println(channelHoldCounter);
		Serial.print("channelHoldTriggered = "); Serial.println(channelHoldTriggered[channel16chFrameSync]);
		
		
		// If a hold is not in progress and we've captured 100 readings reset the max millis()
		// It is better to use the [0] & [1] method here to ensure syncing is correct every 100 frames
		if (channelHoldTriggered[0] == false && channelHoldTriggered[1] == false && channelHoldCounter >= 100) {
			channelHoldCounter = 0;
			channelsMaxHoldMillis[0] = 0;
			channelsMaxHoldMillis[1] = 0;
		}

		// check when in 16ch 1 wave mode we are scanning the correct channel with the wave
		if (badFramesMonitoringType == 2 && channel16chFrameSync == true) goto RETURN_EARLY;
		if (badFramesMonitoringType == 3 && channel16chFrameSync == false) goto RETURN_EARLY;

		// Scan all channels to see if anything has changed
		// TODO - Use badFramesMonitoringType and channel16chFrameSync to just scan the correct channel !!
		bool held = true;
		for (int ch = 0; ch < 16; ch++) {
			if (channels[ch] != channelsPrevious[ch]) { held = false; }
		}

		// Detect a new hold
		if (held == true && channelHoldTriggered[channel16chFrameSync] == false) {
#if defined(REPROT_CHANNEL_HOLD_DATA)
			Serial.print("_____HoldTrigger = "); Serial.println(channelHoldTriggered[channel16chFrameSync]);
#endif
			channelHoldTriggered[channel16chFrameSync] = true;
			channelsStartHoldMillis = millis();
		}

		// Detect when a hold ends
		if (held == false && channelHoldTriggered[channel16chFrameSync] == true) {
			uint16_t diff = millis() - channelsStartHoldMillis;
			
			if (badFramesMonitoringType == 1) { diff += 9; } else { diff += 18; } // add time before first missing frame detected.

			if (diff > 20 && diff > channelsMaxHoldMillis[channel16chFrameSync]) {			// Skip 10th Frame (OpenTx timing issue)
				channelsMaxHoldMillis[channel16chFrameSync] = diff;
			}
#if defined(REPROT_CHANNEL_HOLD_DATA)
			Serial.print("_____Channel Hold Recovered "); Serial.print(diff); Serial.println("ms"); 
#endif
			channelHoldTriggered[channel16chFrameSync] = false;
		}
	}
RETURN_EARLY:
	// uodate the Telemetry value with the highest value of both frames
	if (channelsMaxHoldMillis[0] > channelsMaxHoldMillis[1]) {
		channelsMaxHoldMillis100Resul = channelsMaxHoldMillis[0];
	}
	else {
		channelsMaxHoldMillis100Resul = channelsMaxHoldMillis[1];
	}
	
#if defined(REPROT_CHANNEL_HOLD_DATA)
	Serial.print("MFH1 = "); Serial.print(channelsMaxHoldMillis[0]); Serial.println("ms");
	Serial.print("MFH2 = "); Serial.print(channelsMaxHoldMillis[1]); Serial.println("ms");
	Serial.print("MFH  = "); Serial.print(channelsMaxHoldMillis100Resul); Serial.println("ms");
#endif
}


// 16ch frame sync
void sync_16chFrame() {
	/*
	badFramesMonitoringType == 1 - 8ch mode, wave on Ch1 - Ch8
	badFramesMonitoringType == 2 - 16ch mode, 1 wave on Ch1 - Ch8
	badFramesMonitoringType == 3 - 16ch mode, 1 wave on Ch9 - Ch16
	badFramesMonitoringType == 4 - 16ch mode, 2 waves, 1 on Ch1 - Ch8 and 1 on ch9 - Ch16
	*/

#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
	Serial.println("Start Frame Sync");
	debug_Wave_Data();
#endif

	channel16chFrameSyncError = false;

	if (badFramesMonitoringType == 1) {
		// 8 channel mode
		// just set the sync like 16ch mode with no sync errors and 
		channel16chFrameSync = false;
	}
	//else if (badFramesMonitoringType == 3) { 
	//	channel16chFrameSync = true; 
	//}
	else {
		// 16 channel mode

		// Check to see if both frames have updated together
		// Testing on the X4RSB_LBT_v2.1.0 detected this occurs every x SBUS frames
		if ((channelsPrevious[badFramesMonitoringChannel1 - 1] != channels[badFramesMonitoringChannel1 - 1]
			&& channelsPrevious[badFramesMonitoringChannel2 - 1] != channels[badFramesMonitoringChannel2 - 1])) {
			channel16chFrameSyncError = true;
			channel16chFrameSyncErrorCounter++;
			// Set false because when both frames change more often than not the next update is on ch9-16 on X4R_LBT_2.1.0
			channel16chFrameSync = false;

#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
			Serial.println("Sync Error");
#endif
		}

		// Check to see if both frames are held
		// In this case we dont want to attempt to sync the frame but dont set the channel16chFrameSyncError because otherwise the FrameHold will not measure the hold
		if ((channelsPrevious[badFramesMonitoringChannel1 - 1] == channels[badFramesMonitoringChannel1 - 1]
			&& channelsPrevious[badFramesMonitoringChannel2 - 1] == channels[badFramesMonitoringChannel2 - 1])) {
			// When both frames are held then the next updated frame can be either on X4R_LBT_v2.1.0 so just leave current sequencing.
			// However, to ensure frameHolds timing is calculated correctly then must keep the frames in sync.
			channel16chFrameSync = !channel16chFrameSync;
#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
			Serial.println("Sync Hold");
#endif
		}
		else {
			// Only run if we are in sync and at least one frame set has updated
			if (channel16chFrameSyncError == false && (channelsPrevious[badFramesMonitoringChannel1 - 1] != channels[badFramesMonitoringChannel1 - 1]
				|| channelsPrevious[badFramesMonitoringChannel2 - 1] != channels[badFramesMonitoringChannel2 - 1])) {

				// channel16chFrameSync == true  for 1-8ch expected
				// channel16chFrameSync == false for 9-16ch expected
				if (channel16chFrameSync == true) {
					if (channelsPrevious[badFramesMonitoringChannel1 - 1] != channels[badFramesMonitoringChannel1 - 1]) {
						channel16chFrameSync = false;  // expect 9-16ch next
#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
						Serial.println("1-8ch OK");
#endif
					}
					else {
#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
						Serial.println("1-8ch Out of Sync");
#endif
						channel16chFrameSyncError = true;
						channel16chFrameSyncErrorCounter++;
						// If we go out of sync it is better not to change the expected next channel, this makes it get back in sync quicker.
					}
				}
				else {
					if (channelsPrevious[badFramesMonitoringChannel2 - 1] != channels[badFramesMonitoringChannel2 - 1]) {
						channel16chFrameSync = true;  // expect 1-8ch next
#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
						Serial.println("9-16ch OK");
#endif
					}
					else {
#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
						Serial.println("9-16ch Out of Sync");
#endif
						channel16chFrameSyncError = true;
						channel16chFrameSyncErrorCounter++;
						// If we go out of sync it is better not to change the expected next channel, this makes it get back in sync quicker.
					}
				}
			}
		}
		channel16chFrameSyncSuccessRate = 100 - (float)channel16chFrameSyncErrorCounter / totalFrames * 100;

		// TEMP - Serial Print if the success rate is low
		if (channel16chFrameSyncSuccessRate < 97.5) {
			// TODO - Add this as a ERROR on telemetry inside sync_16chFrame() - once done #if this out with   REPORT_SBUS_16CH_FRAME_SYNC_DATA
			Serial.print("Low Sync Frame Success Rate @ "); Serial.print(channel16chFrameSyncSuccessRate); Serial.println("%");
		}
	}

#if defined(REPORT_SBUS_16CH_FRAME_SYNC_DATA)
	Serial.print("FrameSyncError? = "); Serial.println(channel16chFrameSyncError);
	Serial.print("FrameSyncValue = "); Serial.println(channel16chFrameSync);
	Serial.print("Next Frame expected on  1-8? = "); Serial.println(channel16chFrameSync);
	Serial.print("Next Frame expected on 9-16? = "); Serial.println(!channel16chFrameSync);
	Serial.print("Sync Frame Success Rate = "); Serial.print(channel16chFrameSyncSuccessRate); Serial.println("%");
#endif
}


// Checks the Rx Lost Frame flag in the SBUS
// Increments the counter, times it, and calculates result based on last 100 frames.
void calculate_LostFrames() {
	
	// Detect new Rx reported Lost Frame
	if (lostFrame == true && lostFrameDetected == false) {
		lostFrameStartMillis = millis();
		lostFrameCounter++;

#if defined(REPORT_FS_LF_ERRORS)
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

#if defined(REPORT_FS_LF_ERRORS)
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


// Checks the fail safe flag in the SBUS
// Increments the counter and times it.
void check_FailSafe() {

	// check for new fail safe
	if (failSafe == true && failSafeDetected == false) {
		failSafeStartMillis = millis();
		failSafeCounter++;
#if defined(REPORT_FS_LF_ERRORS)
		Serial.print("failSafes Detected = "); Serial.println(failSafeCounter);
#endif
		failSafeDetected = true;
	}

	// check for fail safe recovery and record the timing
	if (failSafe == false && failSafeDetected == true) {
		uint32_t recoveryTime = millis() - failSafeStartMillis;
		if (recoveryTime > failSafeLongestMillis)  failSafeLongestMillis = recoveryTime;
#if defined(REPORT_FS_LF_ERRORS)
		Serial.print("FailSafe Recovered = "); Serial.print(recoveryTime); Serial.println("ms");
#endif
		failSafeDetected = false;
	}
}


// Search channels 1-16 for the channels that transmit the wave(s)
// Determine 8 or 16 channel mode and set badFramesMonitoringType
void find_WaveChannel_New(byte &badFramesMonitoringChannel1, byte &badFramesMonitoringChannel2, byte &badFramesMonitoringType) {
	uint32_t startTimeMillis = millis();
	uint32_t maxWaitTimeMillis = millis();
	byte counter = 0;

	// Scan channels 9 to 16 for 20 times to see which any increases between 16 to 20.
	// activity_16ch_Counter ensures we see at least 2 valid frames due to issue describe above
	// Timer ensures we dont get stuck if Tx or Rx is not powered.
	maxWaitTimeMillis = millis();					// The timer
	counter = 0;													// Counts the number of SBUS frames read
	bool activity_16ch = false;					  // True once two valid frames found
	byte activity_16ch_Counter = 0;				// Counts the valid frames
	byte waveHitCounter[16] = { 0 };			// Counts the number of possible wave hits for each channel
	badFramesMonitoringType = 0;					// 1=8ch mode, wave on Ch1-Ch8, 2=16ch mode, wave on Ch1-Ch8, 3=16ch mode, wave on Ch9-Ch16, 4=16ch mode, wave on Ch1-Ch16
	while (counter < 20 && millis() - maxWaitTimeMillis < MAX_WAIT_TIME_MS) {
		delay(5);
		if (sbus.read(&channels[0], &failSafe, &lostFrame)) {
			for (int ch = 8; ch < 16; ch++) { 
				if (channels[ch] > 0 && activity_16ch == false) { activity_16ch_Counter++; }		// First find increase the counter only
				if (channels[ch] > 0 && activity_16ch_Counter > 10) { activity_16ch = true; }	// Second find consider true 16 channel mode is active
				if (abs(channels[ch] - channelsPrevious[ch]) >= 12 && abs(channels[ch] - channelsPrevious[ch]) <= 20) {
					waveHitCounter[ch]++;
					if (waveHitCounter[ch] > badFramesMonitoringChannel2) { badFramesMonitoringChannel2 = ch + 1; }
					channel16chFrameSync = true;
				}
				channelsPrevious[ch] = channels[ch];
			}
			counter++;
		}
	}

	Serial.print("activity = "); Serial.println(activity_16ch);
	Serial.print("Wave Monitoring Type: "); Serial.println(badFramesMonitoringType);
	Serial.print("badFramesMonitoringChannel_FrameSet_1 = "); Serial.println(badFramesMonitoringChannel1);
	Serial.print("badFramesMonitoringChannel_FrameSet_2 = "); Serial.println(badFramesMonitoringChannel2);

	// Scan channels 1 to 8 for 20 times to see which any increases between
	// either 6 to 10 for 8ch mode or 16 to 20 for 16ch mode
	// Timer ensures we dont get stuck if Tx or Rx is not powered.
	maxWaitTimeMillis = millis();				// The timer
	counter = 0;												// Counts the number of SBUS frames read
	while (counter < 20 && millis() - maxWaitTimeMillis < MAX_WAIT_TIME_MS) {
		delay(5);
		if (sbus.read(&channels[0], &failSafe, &lostFrame)) {
			for (int ch = 0; ch < 8; ch++) {
				if ((badFramesMonitoringType == 3 && abs(channels[ch] - channelsPrevious[ch]) >= 12 && abs(channels[ch] - channelsPrevious[ch]) <= 20)
					|| (badFramesMonitoringType == 0 && abs(channels[ch] - channelsPrevious[ch]) >= 6 && abs(channels[ch] - channelsPrevious[ch]) <= 10)) {
					waveHitCounter[ch]++;
					if (waveHitCounter[ch] > badFramesMonitoringChannel1) { badFramesMonitoringChannel1 = ch + 1; }
					channel16chFrameSync = false;
				}
				channelsPrevious[ch] = channels[ch];
			}
			counter++;
		}
	}

	// Work out the badFramesMonitoringType
	if (activity_16ch == false && badFramesMonitoringChannel1 != 0 && badFramesMonitoringChannel2 == 0) { badFramesMonitoringType = 1; }
	if (activity_16ch == true && badFramesMonitoringChannel1 != 0 && badFramesMonitoringChannel2 == 0) { badFramesMonitoringType = 2; }
	if (activity_16ch == true && badFramesMonitoringChannel1 == 0 && badFramesMonitoringChannel2 != 0) { badFramesMonitoringType = 3; }
	if (activity_16ch == true && badFramesMonitoringChannel1 != 0 && badFramesMonitoringChannel2 != 0) { badFramesMonitoringType = 4; }

	/*
	At this stage:
		badFramesMonitoringChannel1 == 0 - no wave on Ch1 - Ch8
		badFramesMonitoringChannel1 > 0 - wave number on Ch1 - Ch8
		badFramesMonitoringChannel2 == 0 - no wave on Ch9 - Ch16
		badFramesMonitoringChannel2 > 0 - wave number on Ch9 - Ch16

		badFramesMonitoringType == 1 - 8ch mode, wave on Ch1 - Ch8
		badFramesMonitoringType == 2 - 16ch mode, 1 wave on Ch1 - Ch8
		badFramesMonitoringType == 3 - 16ch mode, 1 wave on Ch9 - Ch16
		badFramesMonitoringType == 4 - 16ch mode, 2 waves, 1 on Ch1 - Ch8 and 1 on ch9 - Ch16
	*/

	Serial.print("activity = "); Serial.println(activity_16ch);
	Serial.print("badFramesMonitoringChannel_FrameSet_1 = "); Serial.println(badFramesMonitoringChannel1);
	Serial.print("badFramesMonitoringChannel_FrameSet_2 = "); Serial.println(badFramesMonitoringChannel2);
	Serial.print("Wave Monitoring Type: "); Serial.println(badFramesMonitoringType);
	Serial.print("Scan Time (ms): "); Serial.println(millis() - startTimeMillis);
	if (badFramesMonitoringChannel1 != 0) { Serial.print("Wave Channel Found in Ch1-8: "); Serial.println(badFramesMonitoringChannel1); }
	if (badFramesMonitoringChannel2 != 0) { Serial.print("Wave Channel Found in Ch9-16: "); Serial.println(badFramesMonitoringChannel2); }
	if (badFramesMonitoringChannel1 == 0 && badFramesMonitoringChannel2 == 0) { Serial.println("Wave Channel not Found"); }

}


// Dumps Previous vs Current Channel of the Wave Channel(s) to USB serial.
void debug_Wave_Data() {
	// Alway compile this, it is used in many routines
	Serial.print(micros()); Serial.print(":");		Serial.print("  BFP ="); Serial.println(badFramesPercentage100Result);

	if (badFramesMonitoringChannel1 != 0) {
		Serial.print("CH"); Serial.print(badFramesMonitoringChannel1); Serial.print(" = ");
		Serial.print(channelsPrevious[badFramesMonitoringChannel1 - 1]); Serial.print(" vs "); Serial.print(channels[badFramesMonitoringChannel1 - 1]);
		Serial.print(" = "); Serial.println(channelsPrevious[badFramesMonitoringChannel1 - 1] - channels[badFramesMonitoringChannel1 - 1]);
	}
	if (badFramesMonitoringChannel2 != 0) {
		Serial.print("CH"); Serial.print(badFramesMonitoringChannel2); Serial.print(" = ");
		Serial.print(channelsPrevious[badFramesMonitoringChannel2 - 1]); Serial.print(" vs "); Serial.print(channels[badFramesMonitoringChannel2 - 1]);
		Serial.print(" = "); Serial.println(channelsPrevious[badFramesMonitoringChannel2 - 1] - channels[badFramesMonitoringChannel2 - 1]);
	}
}


// Dumps Previous vs Current Channel of all Channels to USB serial.
void debug_Data() {
#if defined(DEBUG_ALL_CHANNEL_DATA)
	for (int ch = 0; ch < 16; ch++) {
		Serial.print("millis():"); Serial.print(millis());
		Serial.print("   CH"); Serial.print(ch +1); Serial.print(" = ");
		Serial.print(channelsPrevious[ch]); Serial.print(" vs "); Serial.print(channels[ch]);
		Serial.print(" = "); Serial.println(channelsPrevious[ch] - channels[ch]);
	}
#endif
}