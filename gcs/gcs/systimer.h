/****************************************************************************/
/*								*/
/*	Module:			systimer.h			*/
/*								*/
/*	Purpose:		system timed events		*/
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
#ifndef _SYSTIMER_H
	#define _SYSTIMER_H

#include "evtimer.h"
#include "mytype.h"

#define MAX_SYS_TIMERS		18

// timer ids
#define SYSTMR_WDOG_CHECK	0
#define SYSTMR_RAIN_STOP	1
#define SYSTMR_WEATHER_CHECK  	2
#define SYSTMR_HUMIDITY_CLEAR 	3
#define SYSTMR_WIND_SPEED_CLEAR	4
#define SYSTMR_CAB_TEMP_CLEAR	5
#define SYSTMR_OUT_TEMP_CLEAR	6
#define SYSTMR_WWV_CHECK	7
#define SYSTMR_ENCODER_CHECK	8
#define SYSTMR_UPDATE_TRACKING	9
#define SYSTMR_TRACKING_CHECK   10
#define SYSTMR_UPDATE_DISPLAY   11
#define SYSTMR_REMOTE           12
#define FAKEMOVE                13
#define SYSTMR_NFS              14
#define SYSTMR_TF               15
#define SYSTMR_UPDATE_STATUS    16
#define SYSTMR_CHECK_COMMAND    17

// timer increment values
#define SYSTMR_WDOG_INC		180	// clear watchdog timer every 3 minutes
#define SYSTMR_WWV_INC		900	// check the WWV every 15 minutes
#define SYSTMR_ENCODER_INC        9 	// check encoders every 9 tics
#define SYSTMR_TRACKING_INC       6	// update the tracking every 6 tics
#define SYSTMR_TRACKING_CHECK_INC 10	// check the tracking against encoders
                                        //   every 10 seconds
#define SYSTMR_DISPLAY_INC        7     // update display every 9 tics
#define SYSTMR_REMOTE_INC         5      // remote timeout every 5 secs
#define SYSTMR_NFS_INC            3      // remote timeout every 3 tics

extern EventTimer	SysTimer[MAX_SYS_TIMERS];
extern BOOL SysTimerActive[MAX_SYS_TIMERS];

extern void display_timer_info();
extern void init_system_timers();
#endif
/********************************* EOF **************************************/

