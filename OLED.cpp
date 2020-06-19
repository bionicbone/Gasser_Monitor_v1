// 
// 
// 

#include "OLED.h"
#include <U8g2lib.h>
#include <Wire.h>
#include "Telemetry.h"
#include "Power.h"


// Display U8G2 Constructor using Standard Software SPI
U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, PIN_OLED_D0, PIN_OLED_D1, PIN_OLED_CS, PIN_OLED_DC, PIN_OLED_RES);


// Public Functions
void _oled_Setup() {
	// Set up the OLED Interface if required
	u8g2.begin();
}


void _oled_FlightBatteryVoltage() {
	// Displays a screen that details the current state of the flight battery
	// Includes:
	// graphical display of C1 and C2, with % and current voltage of each
	// total % and voltage of the battery with either "CRITICAL !!" or "OVER CHARGED" if not in the limits

	// Prepare the Total Battery Percentage (tp) and Total Battery Voltage (tv) as char* values
	int perc = percentcell((cell[0] + cell[1]) / 2);
	char tp[5];
	String conString = "";
	conString.concat(perc);
	conString.concat("%");
	conString.toCharArray(tp, 5);
	char tv[6];
	conString = "";
	conString.concat(String(cell[0] + cell[1]));
	conString.concat("v");
	conString.toCharArray(tv, 6);

	// Prepare the content of the OLED
	u8g2.firstPage();
	do {
		u8g2.setFont(u8g2_font_6x10_tf);		// many font available https://github.com/olikraus/u8g2/wiki/fntlistall
		u8g2.drawStr(4, 8, "Flight Batt Voltage");

		// draw the two graphical batteries
		drawBattery(1, cell[0], 18, 16);
		drawBattery(2, cell[1], 18, 33);

		// Display totals or Errors on bottom line
		u8g2.setFont(u8g2_font_tenfatguys_tf);	// many font available https://github.com/olikraus/u8g2/wiki/fntlistall
		if (cell[0] > OLED_OVERCHARGED_VOLTAGE_WARNING || cell[1] > OLED_OVERCHARGED_VOLTAGE_WARNING) {
			u8g2.drawStr(0, 60, "OVER CHARGED");
		}
		else if (cell[0] < OLED_CRITICAL_VOLTAGE_WARNING || cell[1] < OLED_CRITICAL_VOLTAGE_WARNING) {
			u8g2.drawStr(12, 60, "CRITICAL !!");
		}
		else {
			u8g2.drawStr(7, 60, tp);
			u8g2.drawStr(71, 60, tv);
		}
	}
	// Start the OLED display update
	while (u8g2.nextPage());
}


// Private Functions
int percentcell(float targetVoltage) {
	// selects the correct % based on the current "targetVoltage"
	// from the global array "myArrayList"

	int result = 0;
	for (unsigned int i = 0; i < sizeof _myArrayList / sizeof _myArrayList[0]; i++) {
		float v = _myArrayList[i][0];
		if (v >= targetVoltage) {
			result = _myArrayList[i][1];
			break;
		}
	}
	return result;
}


void drawBattery(int cellNumber, float cellVoltage, int x, int y) {
	// Lookup the Cell Voltages & Prepare the Cell Number, Cell Voltage & % output Strings
	char cn[3];
	String conString = "C";
	conString.concat(cellNumber);
	conString.toCharArray(cn, 3);

	char cp[5];
	int perc = percentcell(cellVoltage);
	conString = "";
	conString.concat(perc);
	conString.concat("%");
	conString.toCharArray(cp, 5);

	char cv[6];
	conString = "";
	conString.concat(String(cellVoltage));
	conString.concat("v");
	conString.toCharArray(cv, 6);

	// Draw the graphical battery
	u8g2.drawStr(0, y + 9, cn);
	u8g2.drawLine(x, y, x + 30, y);
	u8g2.drawLine(x + 30, y, x + 30, y + 10);
	u8g2.drawLine(x + 30, y + 10, x, y + 10);
	u8g2.drawLine(x, y + 10, x, y);
	u8g2.drawVLine(x + 31, y + 4, 3);
	u8g2.drawVLine(x + 32, y + 4, 3);
	u8g2.drawStr(x + 44, y + 9, cp);
	u8g2.drawStr(x + 77, y + 9, cv);

	// Fill the graphical battery with the current charge level
	int counter = 0;
	for (float i = 0; i < percentcell(cellVoltage); i = i + 3.5)
	{
		u8g2.drawVLine(x + 1 + counter, y + 1, 9);
		counter++;
	}
}
