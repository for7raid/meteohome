#include <lib_bl999.h>

static BL999Info info;

void startbl999() {

	//set digital pin to read info from
	bl999_set_rx_pin(EA2_PIN);

	//start reading data from sensor
	bl999_rx_start();

	Serial.println("Started ea2");
}

void Processbl999() {
	if (bl999_get_message(info)) {
		Serial.println("====== Got message from BL999 sensor: ");
		Serial.println("Channel: " + String(info.channel));
		Serial.println("PowerUUID: " + String(info.powerUUID));
		Serial.println("Battery is " + String(info.battery == 0 ? "Ok" : "Low"));
		Serial.println("Temperature: " + String(info.temperature / 10.0));
		Serial.println("Humidity: " + String(info.humidity) + "%");

		_weatherData.O_T = info.temperature / 10.0;


		//Улица
		tft.loadFont("Fonts/Arial70");
		tft.setTextPadding(98);
		tft.setTextDatum(BR_DATUM);
		tft.drawNumber((int)_weatherData.O_T, 133, 234);
		tft.unloadFont();

		tft.loadFont("Fonts/Arial48");
		tft.setTextPadding(25);
		tft.setTextDatum(BL_DATUM);
		tft.drawNumber((_weatherData.O_T - (int)_weatherData.O_T) * 10, 135, 230);
		tft.setTextPadding(0);
		tft.setTextDatum(TL_DATUM);
		tft.drawString("\xB0", 135, 157);
		tft.unloadFont();

		tft.loadFont("Fonts/Arial10");
		tft.setTextPadding(0);
		tft.setTextDatum(TL_DATUM);
		tft.drawString("УЛИЦА", 5, 167);
		tft.unloadFont();


		if (info.battery != 0) {
			tft.setTextSize(6);
			tft.drawString("!", 10, 150);
		}

	}
	else {
		Serial.println("No");
		tft.loadFont("Fonts/Arial70");
		tft.setTextPadding(98);
		tft.setTextDatum(BR_DATUM);
		tft.drawString("--", 133, 234);
		tft.unloadFont();

		tft.loadFont("Fonts/Arial48");
		tft.setTextPadding(0);
		tft.setTextDatum(BL_DATUM);
		tft.drawString(" ", 135, 230);
		tft.setTextDatum(TL_DATUM);
		tft.drawString("\xB0", 135, 157);
		tft.unloadFont();

		tft.loadFont("Fonts/Arial10");
		tft.setTextPadding(0);
		tft.setTextDatum(TL_DATUM);
		tft.drawString("УЛИЦА", 5, 167);
		tft.unloadFont();
	}
}