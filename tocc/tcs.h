/****************************************************************************/
/*                                                                       */
/*      Module:         tcs.h                                            */
/*                                                                       */
/*      Purpose:        Telescope Control System functions               */
/*                                                                       */
/****************************************************************************/
/*                                                                       */
/*                    PROPERTY OF AUTOSCOPE CORPORATION                  */
/*                        2637 Midpoint Dr., Suite D                     */
/*                         Fort Collins, CO  80525                       */
/*                                                                       */
/*                            Copyright 1995                             */
/*              Unauthorized duplication or use is prohibited.           */
/*                                                                       */
/*                                                                       */
/*      Author:         M. Donahue                                       */
/*                                                                       */
/****************************************************************************/
#ifndef _TCS_H
	#define _TCS_H

#include "mytype.h"
#include "evtimer.h"

/****************************************************************************/
/*      typedefs, etc.                                                   */
/****************************************************************************/
struct HOMESTATUSPACK
	{
		BOOL t_axis;    // TRUE indicates axis is at home
		BOOL u_axis;
		BOOL v_axis;
		BOOL x_axis;
		BOOL y_axis;
		BOOL z_axis;
	};

struct LIMITSTATUSPACK
	{
		char x_axis;    // -1 negative active, 0 none, 1 positive active
		char y_axis;
		char z_axis;
		char t_axis;
		char u_axis;
		char v_axis;
	};

/****************************************************************************/
/*      funciton prototypes                                              */
/****************************************************************************/
extern unsigned tcs_aux_off(int channel);
extern unsigned tcs_aux_on(int channel);
extern unsigned tcs_calibrate_soft_limits(char axis);
extern unsigned tcs_calibrate_steps_degree(char axis);
extern unsigned calibrate_az_steps(double &value);
extern unsigned calibrate_rotator_steps(double &value);
extern void     tcs_disble_limits();
extern void     tcs_emergency_stop();
extern void     tcs_enable_limits();
extern unsigned tcs_home_secondary();
extern unsigned tcs_home_telescope();
extern void     tcs_initialize();
extern void     tcs_set_encoders();
extern unsigned tcs_is400_select(int port);
extern unsigned tcs_mark_coordinates( double ra,    // radians
					double dec,             // radians
					double epoch,
					double pmra,            // radians
					double pmdec,           // radians
					double parallax,
					double radial_velocity,
					double eff_wavelength,
					double tlr,
                                        BOOL sessiononly = TRUE);
extern unsigned old_tcs_move_noupdate(double ra_sec, double dec_arcsec);
extern unsigned tcs_move_noupdate(double ra_sec, double dec_arcsec);
extern unsigned tcs_move_secondary_steps(long t_axis, long u_axis, long v_axis);
extern unsigned tcs_move_telescope_steps(long x_axis, long y_axis, long z_axis,
		EventTimer *timer = NULL);
extern unsigned tcs_move_to_az_alt(double az,   // radians
					double alt     // radians
					);
extern unsigned tcs_move_to_coordinates(double ra,              // radians
					double dec,             // radians
					double epoch,
					double pmra,            // radians
					double pmdec,           // radians
					double parallax,
					double radial_velocity,
					double eff_wavelength,
					double tlr,
                                        double pa);
extern unsigned tcs_dust(int open);
extern BOOL tcs_pos_within_limits(const char axis, const long step_pos);
extern unsigned tcs_reset_home_position(BOOL sessiononly = FALSE);
extern long tcs_return_encoder_position(char axis);
extern long tcs_return_step_position(char axis);
extern void tcs_return_home_status(struct HOMESTATUSPACK &hstatus);
extern void tcs_return_limit_status(struct LIMITSTATUSPACK &lstatus);
extern void tcs_set_deinit_telescope();
extern void tcs_store_z_axis_pos();
extern unsigned tcs_set_tracking_rates(double x_axis, double y_axis,
					double z_axis); // arcsec/sec
extern unsigned tcs_telescope_park(int);
extern unsigned tcs_telescope_fill(double);
extern void set_ccd_speed(int);
extern unsigned tcs_telescope_service();
extern unsigned tcs_quick_move(double, double);
extern void tcs_telescope_stop(const BOOL wait = TRUE);

extern unsigned track_to_position();

extern void update_telescope_position();
extern void update_encoder_position();
#endif
/********************************* EOF **************************************/
