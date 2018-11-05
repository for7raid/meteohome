#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


#include <JPEGDecoder.h>



void UpdateInternetWeather() {
	HTTPClient http;


	http.begin(F("http://api.apixu.com/v1/forecast.json?key=fc699afbfc3a47309b9191054161205&q=moscow&days=4&lang=ru")); //HTTP

	Serial.println(F("[HTTP] GET api.apixu.com...\n"));
	// start connection and send HTTP header
	int httpCode = http.GET();

	// httpCode will be negative on error
	if (httpCode > 0) {
		// HTTP header has been send and Server response header has been handled
		Serial.printf("[HTTP] GET... code: %d\n", httpCode);
		updateInternetWeather.Set(updateInternetWeatherInterval);

		// file found at server
		if (httpCode == HTTP_CODE_OK) {
			String payload = http.getString();
			ShowInternetWeather(payload);

		}
	}
	else {
		updateInternetWeather.Set(SECS_PER_MIN * 1000);
		Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();
}

void ShowInternetWeather(String data) {
	const size_t capacity = 2166;
	DynamicJsonBuffer jsonBuffer(capacity);

	JsonObject& root = jsonBuffer.parseObject(data);
	if (!root.success()) {
		Serial.println(F("Parsing failed!"));
		return;
	}

	int temp, wind, pres; int code = 0;
	time_t epoch;
	String d, fileName;
	temp = root["current"]["temp_c"];
	wind = (int)(root["current"]["wind_kph"].as<int>() * 0.2777777);
	code = root["current"]["condition"]["code"].as<int>() - 887;

	tft.loadFont("Fonts/Arial20");
	tft.setTextDatum(BC_DATUM);
	tft.setTextPadding(75);

	tft.drawString("Сейчас", 37, 260);
	tft.setTextPadding(50);
	tft.drawString(String(temp) + "\xB0С", 37, 364);
	tft.drawString(String(wind) + "м/с", 37, 393);

	fileName = "/apixu/" + String(code) + ".jpg";
	drawFileJpeg(fileName, 8, 265);

	JsonArray& forecast_forecastday = root["forecast"]["forecastday"];
	int start = hour(now()) > 20 ? 1 : 0;
	for (int j = 0, i = start; i < start + 3; j++, i++)
	{
		int shift = (j + 1) * 76;

		epoch = forecast_forecastday[i]["date_epoch"];
		temp = forecast_forecastday[i]["day"]["avgtemp_c"];
		wind = forecast_forecastday[i]["day"]["maxwind_kph"];
		code = forecast_forecastday[i]["day"]["condition"]["code"].as<int>() - 887;

		tft.setTextPadding(75);
		if (i == 0) {
			d = "Сегодня";
		}
		else if (i == start) {
			d = "Завтра";
		}
		else {
			d = String(day(epoch)) + ", " + weekdayStr(epoch);
		}

		tft.drawString(d, 37 + shift, 260);
		tft.setTextPadding(50);
		tft.drawString(String(temp) + "\xB0С", 37 + shift, 364);
		tft.drawString(String(wind) + "м/с", 37 + shift, 393);

		fileName = "/apixu/" + String(code) + ".jpg";
		drawFileJpeg(fileName, 8 + shift, 265);


	}

	tft.unloadFont();

}

void drawFileJpeg(const String& filename, int xpos, int ypos) {

	auto jpegFile = SPIFFS.open(filename, "r");

	if (!jpegFile) {
		Serial.println("no file " + String(filename));
		tft.setTextSize(2);
		tft.drawString(filename, xpos, ypos);
		return;
	}

	boolean decoded = JpegDec.decodeFsFile(filename);

	if (decoded) {
		//jpegInfo();
		renderJPEG(xpos, ypos);
	}
	else {
		Serial.println("Jpeg file format not supported!");
	}

}
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))
void renderJPEG(int xpos, int ypos) {

	// retrieve infomration about the image
	uint16_t *pImg;
	uint16_t mcu_w = JpegDec.MCUWidth;
	uint16_t mcu_h = JpegDec.MCUHeight;
	uint32_t max_x = JpegDec.width;
	uint32_t max_y = JpegDec.height;

	// Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
	// Typically these MCUs are 16x16 pixel blocks
	// Determine the width and height of the right and bottom edge image blocks
	uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
	uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

	// save the current image block size
	uint32_t win_w = mcu_w;
	uint32_t win_h = mcu_h;

	// record the current time so we can measure how long it takes to draw an image
	uint32_t drawTime = millis();

	// save the coordinate of the right and bottom edges to assist image cropping
	// to the screen size
	max_x += xpos;
	max_y += ypos;

	// read each MCU block until there are no more
	while (JpegDec.read()) {

		// save a pointer to the image block
		pImg = JpegDec.pImage;

		// calculate where the image block should be drawn on the screen
		int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
		int mcu_y = JpegDec.MCUy * mcu_h + ypos;

		// check if the image block size needs to be changed for the right edge
		if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
		else win_w = min_w;

		// check if the image block size needs to be changed for the bottom edge
		if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
		else win_h = min_h;

		// copy pixels into a contiguous block
		if (win_w != mcu_w)
		{
			uint16_t *cImg;
			int p = 0;
			cImg = pImg + win_w;
			for (int h = 1; h < win_h; h++)
			{
				p += mcu_w;
				for (int w = 0; w < win_w; w++)
				{
					*cImg = *(pImg + w + p);
					cImg++;
				}
			}
		}

		// calculate how many pixels must be drawn
		uint32_t mcu_pixels = win_w * win_h;

		// draw image MCU block only if it will fit on the screen
		if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
		{
			// Now set a MCU bounding window on the TFT to push pixels into (x, y, x + width - 1, y + height - 1)
			tft.setWindow(mcu_x, mcu_y, mcu_x + win_w - 1, mcu_y + win_h - 1);

			// Write all MCU pixels to the TFT window
			while (mcu_pixels--) {
				// Push each pixel to the TFT MCU area
				tft.pushColor(*pImg++);
			}

		}
		else if ((mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding
	}


}

//void jpegInfo() {
//	Serial.println(F("==============="));
//	Serial.println(F("JPEG image info"));
//	Serial.println(F("==============="));
//	Serial.print(F("Width      :")); Serial.println(JpegDec.width);
//	Serial.print(F("Height     :")); Serial.println(JpegDec.height);
//	Serial.print(F("Components :")); Serial.println(JpegDec.comps);
//	Serial.print(F("MCU / row  :")); Serial.println(JpegDec.MCUSPerRow);
//	Serial.print(F("MCU / col  :")); Serial.println(JpegDec.MCUSPerCol);
//	Serial.print(F("Scan type  :")); Serial.println(JpegDec.scanType);
//	Serial.print(F("MCU width  :")); Serial.println(JpegDec.MCUWidth);
//	Serial.print(F("MCU height :")); Serial.println(JpegDec.MCUHeight);
//	Serial.println(F("==============="));
//}