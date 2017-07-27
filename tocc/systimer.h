/****************************************************************************/
/*                                                              */
/*      Module:                 systimer.h                      */
/*                                                              */
/*      Purpose:                system timed events             */
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
/*                                                              */
/*      Author:         M. Donahue                              */
/*                                                              */
/****************************************************************************/
#ifndef _SYSTIMER_H
	#define _SYSTIMER_H

#include "evtimer.h"
#include "mytype.h"

#define MAX_SYS_TIMERS          27

// timer ids
#define SYSTMR_WDOG_CHECK       0
#define SYSTMR_RAIN_STOP        1
#define SYSTMR_WEATHER_CHECK    2
#define SYSTMR_HUMIDITY_CLEAR   3
#define SYSTMR_WIND_SPEED_CLEAR 4
#define SYSTMR_CAB_TEMP_CLEAR   5
#define SYSTMR_OUT_TEMP_CLEAR   6
#define SYSTMR_WWV_CHECK        7
#define SYSTMR_ENCODER_CHECK    8
#define SYSTMR_UPDATE_TRACKING  9
#define SYSTMR_TRACKING_CHECK   10
#define SYSTMR_UPDATE_DISPLAY   11
#define SYSTMR_REMOTE           12
#define FAKEMOVE                13
#define SYSTMR_NFS              14
#define SYSTMR_TF               15
#define SYSTMR_UPDATE_STATUS    16
#define SYSTMR_CHECK_COMMAND    17
#define SYSTMR_CCD_FILL_STATUS  18
#define SYSTMR_CCD_TEMP_STATUS  19
#define SYSTMR_CHECK_NETWORK    20 
#define SYSTMR_NETWORK_LOST     21
#define SYSTMR_35M_CHECK        22
#define SYSTMR_35M_OPEN         23
#define SYSTMR_POWER_LOST       24
#define SYSTMR_WRITEPOS_CHECK   25
#define SYSTMR_UPDATE_POSITION  26

// timer increment values
#define SYSTMR_WDOG_INC         180     // clear watchdog timer every 3 minutes
#define SYSTMR_WWV_INC          900     // check the WWV every 15 minutes
#define SYSTMR_ENCODER_INC        9     // check encoders every 9 tics
#define SYSTMR_TRACKING_INC       1     // update the tracking every tic
#define SYSTMR_TRACKING_CHECK_INC 10    // check the tracking against encoders
					//   every 10 seconds
#define SYSTMR_DISPLAY_INC        7     // update display every 9 tics
#define SYSTMR_STATUS_INC         48  // update status (to remote) every 10 tics
#define SYSTMR_REMOTE_INC         12     // remote timeout every 5 secs
#define SYSTMR_NFS_INC            3     // remote timeout every 3 tics
#define SYSTMR_CHECK_COMMAND_INC  12     // look for remote commands every 6 tics
#define SYSTMR_CCD_FILL_INC     900   // Check CCD fill status every 15 mins
#define SYSTMR_CCD_TEMP_INC     300   // Check CCD fill status every 5 mins
#define SYSTMR_CHECK_NETWORK_INC 30 
#define SYSTMR_NETWORK_LOST_INC 600
#define SYSTMR_POWER_LOST_INC    60
#define SYSTMR_WRITEPOS_INC     600

extern EventTimer       SysTimer[MAX_SYS_TIMERS];
extern BOOL SysTimerActive[MAX_SYS_TIMERS];

extern void display_timer_info();
extern void init_system_timers();
#endif
/********************************* EOF **************************************/

