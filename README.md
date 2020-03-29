# Gasser Monitor

This project aims to create a bolt on device that has huge potential to monitor RC equipment for a full range of problems
or just provide a standard data set that can be reviewed and understood across all models.

It will be initially designed for Frsky users (due to superior Telemetry functions), however eventually it may be rolled out for other protocols with the ability to download data post flight.

Over the last 18 months many proof of concept bench tests have been completed and it is now time to migrate code into a
project so some more real life testing and data comparisons can take place.

All sensors are at different levels of development but include:-

* RPM sensors
  * Up to 2 RPM sensors (potentially more) can be connected with data sent back via telemetry
  * Ideal for gasser engine RC models to review clutch bite RPM and checking for clutch slip during flights 
* Battery Cell Voltage
  * Initially up to 3 battery cells will be monitored with data sent beck via telemetry
  * Battery status will be visible on OLED display
* Temperature Sensors
  * Gasser Engine / ESC Temperature
  * Inside Canopy Temperature
  * Ambient Temperature
* SMART Power Charing / Monitoring
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
* Rx Link Quality
  * Using data from the SBUS to determine
    * Lock outs (i.e. SBUS not updating servos) and length of lock out in ms.
    * Quality of the link from Tx to Rx as a percentage of last 100 frames
    * Actual SBUS refresh rates being achieved
    * Lost Frames indicated by Rx
    * Fail Safes indicated by Rx
 
