/****************************************************************************/
/*								*/
/*	Module:			weather.h			*/
/*								*/
/*	Purpose:		routines for handling the weather station*/
/*								*/
/****************************************************************************/
/*								*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION         */
/*                        2637 Midpoint Dr., Suite D            */
/*                         Fort Collins, CO  80525              */
/*								*/
/*                            Copyright 1995                    */
/*              Unauthorized duplication or use is prohibited.	*/
/*								*/
/*								*/
/*	Author:		M. Donahue				*/
/*								*/
/****************************************************************************/
#ifndef _WEATHER_H
	#define _WEATHER_H
struct WEATHERPACK
	{
		int WindSpeed;	// kph
		int WindDir;		// 00 - 3600
		double AuxTemp;		// cabinet temperature 0C
		double CabTemp;		// cabinet temperature 0C
		double OutTemp;		// outside temperature 0C
		int Humidity;		// %
		int Pressure;		// millibars
	};

extern unsigned read_weather_station(struct WEATHERPACK &weather);
extern unsigned setup_weather();
#endif
/********************************* EOF **************************************/
