
#include <TimeLib.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <WiFiUdp.h>
#include "Solarlib.h"



#define mylat 55.755826
#define mylon 37.617300 

String monthStrings[12] = { "янв", "фев","мар", "апр", "май", "июн", "июл", "авг", "сен", "окт" , "ноя", "дек" };
String dayStrings[7] = { "вс","пн","вт","ср","чт","пт","сб" };

int timeZone = 3;
unsigned int localPort = 2390;      // local port to listen for UDP packets

									/* Don't hardwire the IP address or we won't get the benefits of the pool.
									*  Lookup the IP address for the host name instead */
									//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

									// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

String weekdayStr(time_t t) {
	int i = weekday(t);
	return dayStrings[i - 1];
}

void setupClock() {

	setTime(23, 50, 55, 28, 2, 18);


	udp.begin(localPort);
	Serial.print("Local port: ");
	Serial.println(udp.localPort());

	setSyncProvider(getNtpTime, getTimeCallback);
	setSyncInterval(SECS_PER_HOUR);
	//setSyncInterval(15);

	initSolarCalc(timeZone, mylat, mylon);

}

void getTimeCallback() {
	Serial.print("now: ");
	Serial.println(now());
	DisplayHours();
	DisplayDate();
	DisplaySun();
}

void DisplaySeconds() {

	int s = second();
	if (s == 0)
		DisplayHours();
	tft.setTextDatum(MC_DATUM);
	tft.loadFont("Fonts/Arial24");
	tft.setTextPadding(30);
	tft.drawString(getDigitsString(s), 80, 432);
	tft.unloadFont();
}
void DisplayHours() {
	String h = getDigitsString(hour()) + "   " + getDigitsString(minute());

	tft.loadFont("Fonts/Arial48");
	tft.setTextPadding(142);
	tft.setTextDatum(MC_DATUM);
	tft.drawString(h, 80, 432);
	tft.unloadFont();

}

void DisplayTime() {
	DisplaySeconds();

	if (minute() == 0) {
		DisplayDate();
		DisplaySun();
	}

}


void DisplayDate() {
	time_t n = now();
	String datestr = String(day(n));
	datestr += " ";
	int i = month(n);
	datestr += monthStrings[i - 1];
	datestr += ", ";
	datestr += weekdayStr(n);
	datestr += " ";

	tft.loadFont("Fonts/Arial20");
	tft.setTextPadding(0);
	tft.setTextDatum(MC_DATUM);
	tft.drawString(datestr, 80, 467);
	tft.unloadFont();
}

void DisplaySun() {
	time_t n = now();
	String datestr;

	tft.loadFont("Fonts/Arial20");
	tft.setTextPadding(106);
	tft.setTextDatum(MC_DATUM);

	time_t tmp = getSunriseTime(n);
	datestr = getDigitsString(hour(tmp)) + ":" + getDigitsString(minute(tmp)) + "-";
	tmp = getSunsetTime(n);
	datestr += getDigitsString(hour(tmp)) + ":" + getDigitsString(minute(tmp));
	tft.drawString(datestr, 234, 417);

	tmp = getSolarNoonTime(n);
	datestr = getDigitsString(hour(tmp)) + ":" + getDigitsString(minute(tmp)) + ",";
	tmp = getSunsetTime(n) - getSunriseTime(n);
	datestr += getDigitsString(hour(tmp)) + ":" + getDigitsString(minute(tmp));
	tft.drawString(datestr, 234, 441);

	tft.setTextSize(2);
	tft.drawString(moon_phase(), 234, 465);

	tft.unloadFont();
}

String getDigitsString(int digits) {
	String result = "";

	if (digits < 10)
		result += '0';
	result.concat(digits);
	return result;

}



time_t getNtpTime()
{
	while (udp.parsePacket() > 0); // discard any previously received packets
	Serial.println("Transmit NTP Request");
	WiFi.hostByName(ntpServerName, timeServerIP);
	sendNTPpacket(timeServerIP);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.println("Receive NTP Response");
			udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];

			return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
		}
	}
	Serial.println("No NTP Response :-(");
	return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
	Serial.println("sending NTP packet...");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
							 // 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}



String moon_phase() {

	/*
	calculates the moon phase (0-7), accurate to 1 segment.
	0 = > new moon.
	4 => full moon.
	*/

	time_t n = now();
	int y = year(n),
		m = month(n),
		d = day(n);

	int c, e;
	double jd;
	int b;

	if (m < 3) {
		y--;
		m += 12;
	}
	++m;
	c = 365.25*y;
	e = 30.6*m;
	jd = c + e + d - 694039.09;  /* jd is total days elapsed */
	jd /= 29.53;           /* divide by the moon cycle (29.53 days) */
	b = jd;		   /* int(jd) -> b, take integer part of jd */
	jd -= b;		   /* subtract integer part to leave fractional part of original jd */
	b = jd * 8 + 0.5;	   /* scale fraction from 0-8 and round by adding 0.5 */
	b = b & 7;		   /* 0 and 8 are the same so turn 8 into 0 */

	switch (b)
	{
	case 0:
	case 8:
		return " Новолуние ";
	case 1:
	case 2:
	case 3:
		return "Раст.  луна";
	case 4:
		return "Полнолуние ";
	case 5:
	case 6:
	case 7:
		return "Убыв.  луна";
	default:
		break;
	}
	if (b > 7) return "Полнолуние ";
	if (b >= 0 && b < 4) return "Убыв.  луна";
	if (b == 4) return " Новолуние ";
	if (b > 4) return "Раст.  луна";

}
