/****************************************************************************/
/*                                                              */
/*      Module:         ocs.h                                   */
/*                                                              */
/*      Purpose:        Observatory Control System functions    */
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
#ifndef _OCS_H
	#define _OCS_H

#include <dos.h>

#include "mytype.h"

/****************************************************************************/
/*      typedefs, etc.                                                                                                                                                                                                                                  */
/****************************************************************************/
#define OCS_DOME_HOME_BIT 0x01
#define OCS_AUX1_BIT                    0x02
#define OCS_AUX2_BIT                    0x04
#define OCS_RAIN_BIT                    0x08
#define OCS_BATT_LOW_BIT        0x10
#define OCS_LINE_FAIL_BIT       0x20
#define OCS_WATCHDOG_BIT        0x40
#define OCS_AUX3_BIT                    0x80

struct UPSPACK
	{
		BOOL    line_fail;
		BOOL    battery_low;
	};

/****************************************************************************/
/*      funciton prototypes                                                                                                                                                                                                                     */
/****************************************************************************/
extern unsigned ocs_aux_on(int channel);
extern unsigned ocs_aux_off(int channel);
extern unsigned ocs_aux_status(unsigned &status);
extern void                     ocs_calc_dome_counts(double new_azimuth);
extern unsigned ocs_calibrate_dome_steps(double &newValue);
extern unsigned ocs_close_shutter();
extern void                     ocs_correct_last_dome_move();
extern unsigned ocs_dome_finished(BOOL &finished);
extern void                     ocs_emergency_stop();
extern unsigned ocs_get_date_time(struct date &dateRec, struct mytime &timeRec);
extern unsigned ocs_get_set_date_time();
extern unsigned ocs_get_last_wwv_sync(unsigned &last_sync);
extern unsigned ocs_home_dome(BOOL force_move, BOOL reset_pos, BOOL &backwards);
extern unsigned ocs_home_dome_force(double);
extern unsigned ocs_home_dome(BOOL force_move, BOOL reset_pos);
extern void                     ocs_initialize();
extern void                     ocs_initialize_lookup_table();
extern BOOL             ocs_installed();
extern BOOL                     ocs_is_dome_home();
extern void                     ocs_kill_ups();
extern unsigned ocs_open_shutter(int);
extern unsigned ocs_open_shutters();
extern unsigned ocs_close_shutters();
extern BOOL                     ocs_rain_detector_active();
extern unsigned ocs_read_inputs();
extern void                     ocs_reset_watchdog();
extern double           ocs_return_dome_azimuth();
extern long                     ocs_return_dome_encoder();
extern unsigned ocs_rotate_dome(double new_azimuth);
extern unsigned ocs_slave_dome();
extern void                     ocs_start_dome_move();
extern unsigned ocs_unslave_dome();
extern void                     ocs_ups_status(struct UPSPACK &upsStatus);
extern BOOL                     ocs_watchdog_active();
extern long calculate_lookup_move_counts(double naz, double delta);
extern void     ccd_fill_status();
extern void     check_fill_status();
extern void     check_ccd_temp(int);
extern BOOL     ocs_ccd_fill_open();
extern void     check_network();
extern void     ocs_heater(int);
extern unsigned ocs_open_lower_shutter(int);
extern unsigned ocs_close_lower_shutter();
#endif
/********************************* EOF **************************************/

