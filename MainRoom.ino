



void StartRoom() {
	attachInterrupt(CO2_PIN, ProcessCO2Interupt, CHANGE);
	
}

void UpdateMainRoomData() {


	_weatherData.M_T = SI7021.getTemp();// bme.readTemperature() * .97;
	_weatherData.M_RH = SI7021.getRH(); // bme.readHumidity();
	_weatherData.M_P = bme.readPressure() / 100.0F * 0.75006375541921;

	//Зал

	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(98);
	tft.setTextDatum(BR_DATUM);
	tft.drawNumber((int)_weatherData.M_T, 133, 77);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial48");
	tft.setTextPadding(25);
	tft.setTextDatum(BL_DATUM);
	tft.drawNumber((_weatherData.M_T - (int)_weatherData.M_T) * 10, 135, 73);
	tft.setTextPadding(0);
	tft.setTextDatum(TL_DATUM);
	tft.drawString("\xB0", 135, 0);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial10");
	tft.setTextPadding(0);
	tft.setTextDatum(TL_DATUM);
	tft.drawString("ЗАЛ", 5, 10);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(75);
	tft.setTextDatum(BL_DATUM);
	tft.drawNumber(_weatherData.M_RH, 180, 77);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial24");
	tft.setTextPadding(0);
	tft.setTextDatum(BL_DATUM);
	tft.drawString("%", 265, 67);
	tft.drawString("ВЛ", 265, 41);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(100);
	tft.setTextDatum(BL_DATUM);
	tft.drawNumber(_weatherData.M_P, 180, 234);
	tft.unloadFont();

	short co2 = constrain(_weatherData.M_CO2, 500, 2000);
	uint32_t color = co2 < 1000 ? TFT_GREEN : co2 < 1500 ? TFT_YELLOW : TFT_RED;
	co2 = map(co2, 500, 2000, 0, 480);

	tft.fillRect(306, 0, 9, 480 - co2, TFT_BLACK);

	tft.fillRect(306, 480 - co2, 9, co2, color);

}

void ProcessCO2Interupt() {
	static unsigned long last;
	static short lowDuration;
	unsigned long current = millis();
	short duration = current - last;
	last = current;

	if (digitalRead(CO2_PIN) == LOW) {
		_weatherData.M_CO2 = constrain(5000 * (duration - 2) / (duration + lowDuration - 4), 500, 5000);
	}
	else {
		lowDuration = duration;
	}

}