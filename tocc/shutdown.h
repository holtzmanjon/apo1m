/****************************************************************************/
/*                                                                      */
/*      Module:                 shutdown.h                              */
/*                                                                      */
/*      Purpose:                monitor weather conditions, etc. for shutdown*/
/*                                                                      */
/****************************************************************************/
/*                                                                      */
/*                    PROPERTY OF AUTOSCOPE CORPORATION                 */
/*                        2637 Midpoint Dr., Suite D                    */
/*                         Fort Collins, CO  80525                      */
/*                                                                      */
/*                            Copyright 1995                            */
/*              Unauthorized duplication or use is prohibited.          */
/*                                                                      */
/*                                                                      */
/*      Author:         M. Donahue                                      */
/*                                                                      */
/****************************************************************************/
#ifndef _SHUTDOWN_H
	#define _SHUTDOWN_H

#include "mytype.h"

// these bits determine why we are in shutdown mode
#define SDC_RAIN        0x0001U
#define SDC_WIND        0x0002U
#define SDC_HUMIDITY    0x0004U
#define SDC_OUTTEMP     0x0008U
#define SDC_CABTEMP     0x0010U
#define SDC_WDOG        0x0020U
#define SDC_UPS_BATT    0x0040U
#define SDC_UPS_LINE    0x0080U
#define SDC_35M_CLOSED  0x0100U
#define SDC_35M_NOT_OPEN 0x0200U
#define SDC_WSFAIL      0x0400U
#define SDC_NO_NETWORK  0x0800U
#define SDC_SHUTDOWN    0x8000U  // signals if shutdown has been performed

extern BOOL check_priority(unsigned& status);
	// returns FALSE if we need to perform a shutdown

extern void check_sensors();
	// check watchdog/rain detector and shutdown if necessary

extern void check_weather();
	// check the weather station

extern void do_shutdown();
	// shutdown the observatory

extern BOOL watchdog_ok(unsigned& status);
	// return TRUE if the watchdog timer is ok

extern void check_35m();
extern void check_ups();
extern void reset_shutdown_timers();
#endif
/********************************* EOF **************************************/

