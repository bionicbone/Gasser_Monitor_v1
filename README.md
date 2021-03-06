# Gasser Monitor

Initially this project was a personal development for my Gasser RC Helicopters but given its usefulness I have moved to a public GitHub repository. 

The project aims to create a bolt on device that has huge potential to monitor RC equipment for a full range of problems or just provide a standard data set that can be reviewed and understood across all models (including electric)

It will be initially designed for Frsky users (due to superior telemetry functions), however eventually it may be rolled out for other protocols with the ability to download data post flight.

A Teensy v3.2 (I may change this to a Teensy v4.0 as the project progresses but for now a v3.2 is recommended because most sensors are 5v) will be used as the main controller due to its size, built in USB for easy updating, and built in Hardware / Software serials that support the FrSky inverted SBUS without additional hardware for easy of building.

The intention is to build a modular system whereby the required sensors can be simply added to the Teensy, where possible automation in code will detect the connection of the sensor and configure automatically to reduce the need for the end user to "configure" the Teensy in code to any particular setup.

Over the last 18 months many proof of concept bench and flight tests have been completed and it is now time to migrate code into a project so some more real life testing and data comparisons can take place. This migration may take a number of months given all the sensors have to be compatible with each other and code future proofed for further developments.

All sensors are at different levels of development:-

* RPM sensors **(see wiki)**
  * Up to 2 RPM sensors (potentially more) can be connected with data sent back via telemetry
  * Ideal for gasser engine RC models to review clutch bite RPM and checking for clutch slip during flights
* Battery Cell Voltage **(see wiki)**
  * Initially up to 3 battery cells will be monitored with data sent back via telemetry
  * Battery status will be visible on OLED display
* Temperature Sensors **(see wiki)**
  * Gasser Engine / ESC Temperature
  * Inside Canopy Temperature
  * Ambient Temperature
* SMART Power Charging / Monitoring **(see wiki)**
  * Control correct charging of the flight battery on Gasser models (if fitted).
  * Current provided by a Gasser charging system (if fitted).
  * Current flow from or to flight battery (charging or discharging state).
  * Current consumed by the model.
  * SMART monitoring of the charging system, voltage from the charging motor, voltage from the AC converter, voltage from the BEC etc. (for quick diagnosis of charging issues).
* SMART Vibration Analysis
  * Standard vibration data / analysis across all models
  * SMART functions to allow "normal" vibrations to be "stored" and errors triggered if new vibrations develop (for detecting new problems before they cause a crash - ideal for scale models with less aggressive flying)
* Static Sensor
  * Ability to detect levels of static caused by the HT system of a gasser engine, HT static can cause significant issues with flight controllers which simply changing the HT lead for better quality can resolve.
* Warning LEDs
  * Can be fitted to model to give visual indication that something is wrong and landing should be considered.
* Small Display Screen (1" OLED) **(see wiki)**
  * Useful for pre-flight checks of flight battery cell voltage etc.
  * Useful for checking if any serious failures were reported on the last flight
* Rx Link Quality **(see wiki)**
  * Using data from the SBUS to determine (requires Tx to transmit a wave pattern that can be analysed by the system)
    * Lock outs (i.e. SBUS not updating servos) and length of lock out in ms.
    * Quality of the link from Tx to Rx as a percentage of last 100 frames
    * Actual SBUS refresh rates being achieved
    * Lost Frames indicated by Rx
    * Fail Safes indicated by Rx
    * 8 channel or 16 channel monitoring

 For more information see the WiKi    https://github.com/bionicbone/Gasser_Monitor_v1/wiki

**Acknowledgements:**
* Bolder Flights Systems SBUS by Brian Taylor for the SBUS library (arduino library)
* Oliver for the U8g2 for the OLED library and GFX library (arduino library)
* @ReinhardZ1 for the testing and parameters to align the Rx Link Quality to the popular Tadango LQ Sensor

**Licencing:** 
* My licencing requirements do not allow commercial use of this software without permission in writing from myself. By means of clarity this includes anyone using this software or specific hardware sensor design for any activity where there will be a financial or other reward whether that results in a financial profit or not.
* My licencing does allow people to make a "small profit" by helping other RC hobbyists, this includes pre-flashing a Teensy, building and suppling sensors and even fully fitting to an RC model. By means of clarity "small profit" is deemed to be not more than $35 overall profit for services provided to any individual RC Hobbyist.

**Disclaimer:** 

This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This software is used at your own risk. I am under no obligation to maintain, support or leave this software or documentation in the public domain, likewise I am under no obligation to release designs for all the sensors or other electronics that I have developed.
