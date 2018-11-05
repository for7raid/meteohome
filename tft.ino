/*
Test the tft.print() viz the libraries embedded write() function

This sketch used font 2, 4, 7

Make sure all the required fonts are loaded by editting the
User_Setup.h file in the TFT_eSPI library folder.

#########################################################################
###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
#########################################################################
*/


/*
Высота шрифта от размера
Шрифт	Высота
70		77
24		26
48		55
20		23
*/

#include "WeatherDataStruc.h"
#include <SmartDelay.h>
#include <pgmspace.h>

#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define FS_NO_GLOBALS
#include "FS.h"

#define updateInternetWeatherInterval SECS_PER_MIN * 15 * 1000 * 1000

#define CO2_PIN D2
#define EA2_PIN D1

char ssid[] = "Raids";  //  your network SSID (name)
char pass[] = "sr65v1986";       // your network password


#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();

#define TFT_GRAY 0x5AEB // New colour

#define CENTRE 240
#define BACKGROUND TFT_BLACK//TFT_GRAY //Серый фон
#define FONTCOLOR TFT_GRAY

SmartDelay updateInternetWeather(updateInternetWeatherInterval);
SmartDelay updateTime(1000 * 1000);
//SmartDelay updateEa2(60 * 1000 * 1000);
SmartDelay updateMainRoom(30000 * 1000);
//SmartDelay sendToServer(SECS_PER_MIN * 5 * 1000 * 1000);

WeatherData _weatherData;

volatile word pulse;
void ext_int_1(void) {

	static word last;
	// determine the pulse length in microseconds, for either polarity
	pulse = micros() - last;
	last += pulse;
	Serial.println(pulse);
}

void setup(void) {
	Serial.begin(115200);



	SPIFFS.begin();

	tft.init();

	

	tft.setRotation(2);
	//attachInterrupt(digitalPinToInterrupt(EA2_PIN), ext_int_1, CHANGE);
	//UpdateInternetWeather();
	//return;

	/*unsigned char i1, i2, c3;
	tft.fillScreen(BACKGROUND);
	tft.setTextColor(TFT_WHITE);
	tft.setTextSize(2);
	tft.setCursor(0, 0);

	for (i1 = 0; i1<8; i1++) {
		for (i2 = 0; i2<16; i2++) {
			c3 = i1 * 16 + i2;
			if (c3 == 0x0A || c3 == 0x0D) tft.print(" ");
			else tft.write(c3);
		}
		tft.println("");
	}

	tft.println("");
	for (i1 = 8; i1<16; i1++) {
		for (i2 = 0; i2<16; i2++)
			tft.write(i1 * 16 + i2);
		tft.println("");
	}
	tft.println(""); tft.println("\xB0");
	return;*/

	tft.setTextColor(FONTCOLOR, BACKGROUND);
	tft.fillScreen(BACKGROUND);

	setupWifi();

	tft.fillScreen(BACKGROUND);
	
	tft.drawFastVLine(305, 0, 480, FONTCOLOR);

	tft.drawFastHLine(0, 78, 305, FONTCOLOR);
	tft.drawFastHLine(0, 156, 305, FONTCOLOR);
	tft.drawFastHLine(0, 234, 305, FONTCOLOR);

	tft.drawFastVLine(168, 0, 235, FONTCOLOR);
	tft.drawFastHLine(0, 402, 305, FONTCOLOR);
	tft.drawFastVLine(163, 402, 78, FONTCOLOR);


	tft.fillRect(315, 0, 5, 160, TFT_RED);
	tft.fillRect(315, 160, 5, 160, TFT_YELLOW);
	tft.fillRect(315, 320, 5, 160, TFT_GREEN);
	
	
	//Зал

	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(98);
	tft.setTextDatum(BR_DATUM);
	tft.drawString("--", 133, 77);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial48");
	tft.setTextPadding(25);
	tft.setTextDatum(BL_DATUM);
	tft.drawString(" ", 135, 73);
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
	tft.drawString("--", 180, 77);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial24");
	tft.setTextPadding(0);
	tft.setTextDatum(BL_DATUM);
	tft.drawString("%", 265, 67);
	tft.drawString("ВЛ", 265, 41);
	tft.unloadFont();


	//Спальня
	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(98);
	tft.setTextDatum(BR_DATUM);
	tft.drawString("--", 133, 156);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial48");
	tft.setTextPadding(25);
	tft.setTextDatum(BL_DATUM);
	tft.drawString(" ", 135, 152);
	tft.setTextPadding(0);
	tft.setTextDatum(TL_DATUM);
	tft.drawString("\xB0", 135, 79);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial10");
	tft.setTextPadding(0);
	tft.setTextDatum(TL_DATUM);
	tft.drawString("СПАЛЬНЯ", 5, 89);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(75);
	tft.setTextDatum(BL_DATUM);
	tft.drawString("--", 180, 156);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial24");
	tft.setTextPadding(0);
	tft.setTextDatum(BL_DATUM);
	tft.drawString("%", 265, 146);
	tft.drawString("ВЛ", 265, 120);
	tft.unloadFont();

	//Улица
	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(98);
	tft.setTextDatum(BR_DATUM);
	tft.drawString("--", 133, 234);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial48");
	tft.setTextPadding(25);
	tft.setTextDatum(BL_DATUM);
	tft.drawString(" ", 135, 230);
	tft.setTextPadding(0);
	tft.setTextDatum(TL_DATUM);
	tft.drawString("\xB0", 135, 157);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial10");
	tft.setTextPadding(0);
	tft.setTextDatum(TL_DATUM);
	tft.drawString("УЛИЦА", 5, 167);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial70");
	tft.setTextPadding(98);
	tft.setTextDatum(BL_DATUM);
	tft.drawString("---", 180, 234);
	tft.unloadFont();

	/*/часы
	tft.loadFont("Fonts/Arial48");
	tft.setTextPadding(142);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("29   45", 80, 434);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial24");
	tft.setTextPadding(0);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("59", 80, 434);
	tft.unloadFont();

	tft.loadFont("Fonts/Arial20");
	tft.setTextPadding(0);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("21 янв, пн", 80, 465);
	tft.unloadFont();

	//Солнце и луна
	tft.loadFont("Fonts/Arial20");
	tft.setTextPadding(106);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("05:12-21:45", 234, 417);
	tft.drawString("12:12,20:39", 234, 441);
	tft.drawString("Убыв.  луна", 234, 465);
	tft.unloadFont();

	//Погода
	String fileName;

	tft.loadFont("Fonts/Arial20");
	tft.setTextDatum(BC_DATUM);
	tft.setTextPadding(75);
	tft.drawString("Сейчас", 37, 260);
	tft.drawString("-10\xB0С", 37, 365);
	tft.drawString("9м/с", 37, 395);
	tft.unloadFont();

	fileName = "/apixu/" + String(113) + ".jpg";
	drawFileJpeg(fileName, 8, 265);

	*/
	
	
	
	StartRoom();
	setupClock();
	UpdateInternetWeather();
	UpdateMainRoomData();
	//SendToServer();
	//startbl999();

}

void setupWifi() {
	Serial.print("Connecting to ");
	Serial.println(ssid);
	if (WiFi.status() == WL_CONNECTED) return;

	WiFi.disconnect();
	WiFi.begin(ssid, pass);
	wl_status_t s;
	while ((s= WiFi.status()) != WL_CONNECTED) {
		delay(500);
		Serial.print(s);
		tft.print(".");
	}
	Serial.println("");

	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void loop() {

	if (updateTime.Now()) DisplayTime();
	if (updateInternetWeather.Now()) UpdateInternetWeather();
	if (updateMainRoom.Now()) UpdateMainRoomData();
	//if (sendToServer.Now()) SendToServer();
	//if (updateEa2.Now()) Processbl999();

	//Processbl999();
	
}



