/****************************************************************************/
/*							*/
/*	Module:		systimer.cpp			*/
/*							*/
/*	Purpose:	system scheduled events		*/
/*							*/
/****************************************************************************/
/*							*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION */
/*                        2637 Midpoint Dr., Suite D    */
/*                         Fort Collins, CO  80525      */
/*							*/
/*                            Copyright 1995            */
/*              Unauthorized duplication or use is prohibited.	*/
/*								*/
/*	Author:		M. Donahue				*/
/*								*/
/****************************************************************************/
#include <iostream.h>

#include "globals.h"
#include "systimer.h"
#include "io.h"

EventTimer SysTimer[MAX_SYS_TIMERS];
BOOL SysTimerActive[MAX_SYS_TIMERS];

void display_timer_info()
	{
		sprintf(outbuf,"System timers status:");
	        writeline(outbuf,1);
		if (SysTimerActive[SYSTMR_WDOG_CHECK])
		  sprintf(outbuf,"\tWatchdog check timer   - %d",
			SysTimer[SYSTMR_WDOG_CHECK].RemainingTimeInSecs());
		else
		  sprintf(outbuf,"\tWatchdog check timer   - inactive\n");
	        writeline(outbuf,1);

		if (SysTimerActive[SYSTMR_RAIN_STOP])
		  sprintf(outbuf,"\tRain clear timer       - %d",
			 SysTimer[SYSTMR_RAIN_STOP].RemainingTimeInSecs()) ;
		else
		  sprintf(outbuf,"\tRain clear timer       - inactive\n");
	        writeline(outbuf,1);

		if (SysTimerActive[SYSTMR_WEATHER_CHECK])
		  sprintf(outbuf,"\tWeather station check  - %d",
			 SysTimer[SYSTMR_WEATHER_CHECK].RemainingTimeInSecs()) ;
		else
		  sprintf(outbuf,"\tWeather station check  - inactive\n");
	        writeline(outbuf,1);

		if (SysTimerActive[SYSTMR_HUMIDITY_CLEAR])
		  sprintf(outbuf,"\tHumidity clear timer   - %d",
			 SysTimer[SYSTMR_HUMIDITY_CLEAR].RemainingTimeInSecs());
		else
		  sprintf(outbuf,"\tHumidity clear timer   - inactive\n");
	        writeline(outbuf,1);

		if (SysTimerActive[SYSTMR_WIND_SPEED_CLEAR])
		  sprintf(outbuf,"\tWind speed clear timer - %d",
		    SysTimer[SYSTMR_WIND_SPEED_CLEAR].RemainingTimeInSecs());
		else
		  sprintf(outbuf,"\tWind speed clear timer - inactive\n");
	        writeline(outbuf,1);

		if (SysTimerActive[SYSTMR_CAB_TEMP_CLEAR])
		  sprintf(outbuf,"\tCab temp clear timer   - %d",
			 SysTimer[SYSTMR_CAB_TEMP_CLEAR].RemainingTimeInSecs());
		else
		  sprintf(outbuf,"\tCab temp clear timer   - inactive\n");
	        writeline(outbuf,1);

		if (SysTimerActive[SYSTMR_OUT_TEMP_CLEAR])
		  sprintf(outbuf,"\tOut temp clear timer   - %d",
			 SysTimer[SYSTMR_OUT_TEMP_CLEAR].RemainingTimeInSecs());
		else
		  sprintf(outbuf,"\tOut temp clear timer   - inactive\n");
	        writeline(outbuf,1);

		if (SysTimerActive[SYSTMR_WWV_CHECK])
		  sprintf(outbuf,"\tWWV check timer        - %d",
			 SysTimer[SYSTMR_WWV_CHECK].RemainingTimeInSecs());
		else
		  sprintf(outbuf,"\tWWV check timer        - inactive\n");
	        writeline(outbuf,1);
	}

void init_system_timers()
	{
		for (int i = 0; i < MAX_SYS_TIMERS; i++)
		{
			// reset all timers
			SysTimer[i].NewTimer(1);  // set to expire immediately
			SysTimerActive[i] = FALSE;
			}

		if (sysGlobal->watchdog_installed)
			SysTimerActive[SYSTMR_WDOG_CHECK] = TRUE;

		if (sysGlobal->weather_installed)
			SysTimerActive[SYSTMR_WEATHER_CHECK] = TRUE;

		if (sysGlobal->wwv_type)
    	SysTimerActive[SYSTMR_WWV_CHECK] = TRUE;

        // Start the network timer
		SysTimer[SYSTMR_NETWORK_LOST].NewTimerSecs(SYSTMR_NETWORK_LOST_INC);
	}

/********************************* EOF **************************************/

