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

#include "systimer.h"

EventTimer SysTimer[MAX_SYS_TIMERS];
BOOL SysTimerActive[MAX_SYS_TIMERS];

void display_timer_info()
	{
		cout << endl << "System timers status:\n";
		if (SysTimerActive[SYSTMR_WDOG_CHECK])
			cout	<< "\tWatchdog check timer   - "
						<< SysTimer[SYSTMR_WDOG_CHECK].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tWatchdog check timer   - inactive\n";

		if (SysTimerActive[SYSTMR_RAIN_STOP])
			cout	<< "\tRain clear timer       - "
						<< SysTimer[SYSTMR_RAIN_STOP].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tRain clear timer       - inactive\n";

		if (SysTimerActive[SYSTMR_WEATHER_CHECK])
			cout	<< "\tWeather station check  - "
						<< SysTimer[SYSTMR_WEATHER_CHECK].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tWeather station check  - inactive\n";

		if (SysTimerActive[SYSTMR_HUMIDITY_CLEAR])
			cout	<< "\tHumidity clear timer   - "
						<< SysTimer[SYSTMR_HUMIDITY_CLEAR].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tHumidity clear timer   - inactive\n";

		if (SysTimerActive[SYSTMR_WIND_SPEED_CLEAR])
			cout	<< "\tWind speed clear timer - "
						<< SysTimer[SYSTMR_WIND_SPEED_CLEAR].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tWind speed clear timer - inactive\n";

		if (SysTimerActive[SYSTMR_CAB_TEMP_CLEAR])
			cout	<< "\tCab temp clear timer   - "
						<< SysTimer[SYSTMR_CAB_TEMP_CLEAR].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tCab temp clear timer   - inactive\n";

		if (SysTimerActive[SYSTMR_OUT_TEMP_CLEAR])
			cout	<< "\tOut temp clear timer   - "
						<< SysTimer[SYSTMR_OUT_TEMP_CLEAR].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tOut temp clear timer   - inactive\n";

		if (SysTimerActive[SYSTMR_WWV_CHECK])
			cout	<< "\tWWV check timer        - "
						<< SysTimer[SYSTMR_WWV_CHECK].RemainingTimeInSecs() << endl;
		else
			cout	<< "\tWWV check timer        - inactive\n";
	}

void init_system_timers()
	{
		for (int i = 0; i < MAX_SYS_TIMERS; i++)
			{
				// reset all timers
				SysTimer[i].NewTimer(1);	// set to expire immediately
				SysTimerActive[i] = FALSE;
			}

	}

/********************************* EOF **************************************/

