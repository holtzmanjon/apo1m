#undef no_weather
/****************************************************************************/
/*                                                              */
/*      Module:         weather.cpp                             */
/*                                                              */
/*      Purpose:        routines to handle the weather station  */
/*                                                              */
/****************************************************************************/
/*                                                              */
/*                    PROPERTY OF AUTOSCOPE CORPORATION         */
/*                        2637 Midpoint Dr., Suite D            */
/*                         Fort Collins, CO  80525              */
/*                                                              */
/*                            Copyright 1995                    */
/*              Unauthorized duplication or use is prohibited.  */
/*                                                              */
/*      Author:         M. Donahue                              */
/*                                                              */
/****************************************************************************/
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <mem.h>
#include <iostream.h>
#include "serial.h"

#include "weather.h"
#include "status.h"
#include "globals.h"
#include "mytype.h"
#include "io.h"

int WSinit = 1;

main()
	{
		#ifndef no_weather
		struct WEATHERPACK weather;
		BOOL english;
		float tempf;
		char *token;
		char temp[6];
		char buffer[80];
		struct date d;
		struct mytime t;

		memset(buffer, 0, 80);

		// open the proper port
		WSinit = setup_weather();
		if (WSinit != 0)
			return TCSERR_COMNA;

	   // read the weather string (wait for up to 2 seconds for each try)
		int io_result;
		int tries = 0;
		outbuf[0] = 0;
		while (tries < 3)
			{
				delay(1000);
				SendString("R\0");
				GetString(buffer);
		 mygettime(&d,&t);
		 sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%f    %s\n",
		    t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund,buffer);

				io_result = strlen(buffer);
				if (io_result < 77) {
		 sprintf(outbuf+strlen(outbuf),
			 "Bad weather read returned %d bytes", io_result);
					tries++;
					delay(1000);
				}
				else
					break;
			}

		// close this serial port
		closeserial();

		if (io_result < 77)
			return TCSERR_COMBADRESP;

		// is the string in English or Metric
		english = (buffer[22] == 'M');

		// parse out the pieces
		// skip the time
		token = strtok(buffer, " ");

		// skip the date
		token = strtok(NULL, " ");

		// get the wind direction
		token = strtok(NULL, " ");

		if (!strcmpi(token, "N"))               weather.WindDir = 0;
		else if (!strcmpi(token, "NNE"))        weather.WindDir = 23;
		else if (!strcmpi(token, "NE"))         weather.WindDir = 45;
		else if (!strcmpi(token, "ENE"))        weather.WindDir = 68;
		else if (!strcmpi(token, "E"))          weather.WindDir = 90;
		else if (!strcmpi(token, "ESE"))        weather.WindDir = 113;
		else if (!strcmpi(token, "SE"))         weather.WindDir = 135;
		else if (!strcmpi(token, "SSE"))        weather.WindDir = 158;
		else if (!strcmpi(token, "S"))          weather.WindDir = 180;
		else if (!strcmpi(token, "SSW"))        weather.WindDir = 203;
		else if (!strcmpi(token, "SW"))         weather.WindDir = 225;
		else if (!strcmpi(token, "WSW"))        weather.WindDir = 248;
		else if (!strcmpi(token, "W"))          weather.WindDir = 270;
		else if (!strcmpi(token, "WNW"))        weather.WindDir = 293;
		else if (!strcmpi(token, "NW"))         weather.WindDir = 315;
		else if (!strcmpi(token, "NNW"))        weather.WindDir = 338;
		else weather.WindDir = 0;

		// get wind speed
		token = strtok(NULL, " ");
		weather.WindSpeed = 
		    (10 * (token[0] - (int)'0')) + (token[1] - (int)'0');
		if (english)
		  weather.WindSpeed = 
		    ((((weather.WindSpeed * 1.609) * 10) + 5) / 10);

		// skip the aux temp
		token = strtok(NULL, " ");

		// get the cabinet temperature
		token = strtok(NULL, " ");
		memset(temp, 0, 6);
		for (int i = 0; i < 3; i++)
			temp[i] = token[i];
		weather.CabTemp = atoi(temp);
		if (english)
			weather.CabTemp = (((((weather.CabTemp - 32.0) *
				(5.0 / 9.0)) * 10) + 5) / 10);

		// get the outside temperature
		token = strtok(NULL, " ");
		memset(temp, 0, 6);
		for (i = 0; i < 3; i++)
			temp[i] = token[i];
		weather.OutTemp = atoi(temp);
		if (english)
			weather.OutTemp = (((((weather.OutTemp - 32.0) *
				(5.0 / 9.0)) * 10) + 5) / 10);

		// get the humidity
		token = strtok(NULL, " ");
		memset(temp, 0, 6);
		for (i = 0; i < 3; i++)
			temp[i] = token[i];
		weather.Humidity = atoi(temp);

		// get the pressure
		token = strtok(NULL, " ");
		memset(temp, 0, 6);
		for (i = 0; i < 5; i++)
			temp[i] = token[i];

		if (english)
		{
			tempf = atof(temp);
			tempf *= 33.86388;
			weather.Pressure = tempf;
		}
		else
			weather.Pressure = atoi(temp);

		sprintf(outbuf+strlen(outbuf),
			"English          -  %d\r\n"
			"Wind direction   -  %d\r\n"
			"Wind speed       -  %d\r\n"
			"Cabinet temp     -  %d\r\n"
			"Outside temp     -  %d\r\n"
			"Humidity         -  %d\r\n"
			"Pressure         -  %d\r\n",
			(int)english,
			weather.WindDir,
			weather.WindSpeed,
			weather.CabTemp,
			weather.OutTemp,
			weather.Humidity,
			weather.Pressure);

		writeline(outbuf,1);
		#else
		weather.WindDir = 0;
		weather.WindSpeed = 0;
		weather.CabTemp = 20;
		weather.OutTemp = 10;
		weather.Humidity = 35;
		weather.Pressure = 950;
		#endif
		return TCSERR_OK;
	}

/********************************* EOF **************************************/


unsigned setup_weather()
{
		WSinit = SetSerial(COM1, 9600, NO_PARITY, 8, 1);    
		if (WSinit != 0)
			return TCSERR_COMNA;
		else {
			initserial();
			ctrlbrk(c_break);
			return(0);
		}
}
/********************************* EOF **************************************/

