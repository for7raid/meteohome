// WeatherDataStruc.h

#ifndef _WEATHERDATASTRUC_h
#define _WEATHERDATASTRUC_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

struct WeatherData
{
	short M_RH;
	float M_T;
	short M_P;
	short M_CO2;
	float O_T;
};

#endif

