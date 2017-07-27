/****************************************************************************/
/*								*/
/*	Module:			tcs_math.h			*/
/*								*/
/*	Purpose:		common conversion routines, etc.*/
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
#ifndef _TCS_MATH_H
	#define _TCS_MATH_H

#include <dos.h>
#include <math.h>

extern int tcs_math_verbose;

struct ALLTIMES
	{
		double mjd_tt;	// terrestrial time as modified julian date
		double mjd_utc;	// coordinated universal time as modified julian date
		double last;	// local apparent sidereal time in radians
		double lasth;	// local apparent sidereal time in hours (0 < lasth < 24)
		double equinox;	// fractional equinox
	};

// get_new_date_time_at
//
// given a date, time, and a number of seconds in the future, return a new
// date and time.
//
extern void get_new_date_time_at(struct date, struct mytime, unsigned seconds,
				struct date&, struct mytime&);
extern void get_new_date_time_at_hund(struct date, struct mytime, unsigned hund,
				struct date&, struct mytime&);

// get_all_times_at
//
// return ALLTIMES structure filled in according to the date/time supplied
//
// d - date
// t - time
//
// timeRec - ALLTIMES structure filled in according to d&t
//
extern int get_all_times_at(const struct date d,
				const struct mytime t,
				struct ALLTIMES &timeRec);

// equtohor
//
// convert equatorial position to az/alt
//
extern void equtohor(	double ra,		// radians
			double dec,		// radians
			double epoch,
			double pmra, 	// radians
			double pmdec,	// radians
			double parallax,
			double radial_velocity,
			double &az,		// radians
			double &alt		// radians
			);

// hortoequ
//
// convert az/alt to equatorial
//
extern void hortoequ(double az,		// radians 0 - 2pi
			double alt,	// radians
			double &ra,	// radians
			double &dec	// radians
			);

// mean_to_mount_corrected
//
// convert a mean position at a given time to the fully corrected position
extern void mean_to_mount_corrected(struct ALLTIMES timeRec,
			double mean_ra,			// radians
			double mean_dec,		// radians
			double mean_epoch,
			double mean_pmra,		// radians
			double mean_pmdec,	// radians
			double mean_parallax,
			double mean_radial_velocity,
			double &corrected_parm1,	// rad (ha/az)
			double &corrected_parm2,	// rad (dec/alt)
			double &corrected_parm3		// rad (pa)
			);
extern void mean_to_mount_corrected_sub(struct ALLTIMES timeRec,
			double mean_ra,			// radians
			double mean_dec,		// radians
			double mean_epoch,
			double mean_pmra,		// radians
			double mean_pmdec,	// radians
			double mean_parallax,
			double mean_radial_velocity,
			double &corrected_parm1,	// rad (ha/az)
			double &corrected_parm2,	// rad (dec/alt)
			double &corrected_parm3,		// rad (pa)
			double &parang		// rad (pa)
			);


// calc_move_time
//
// calculate a move time given the axis steps and other information
extern double calc_move_time(long steps, double minimum_slew_rate,
			long maximum_slew_rate, long acceleration);

// calc_equ_move_time_steps
//
// given a delta_ra and delta_dec, return the time necessary to move
// and the stesp in each axis
extern void calc_equ_move_time_steps(double delta_ha, 	// radians
					double delta_dec,	// radians
					double &move_time,
					long &x_steps,
					long &y_steps);

// calc_altaz_move_time_steps
//
// given delta_az, delta_alt, delta_rot, return time and steps for move
extern void calc_altaz_move_time_steps(double delta_az,	// radians
					double delta_alt,	// radians
					double delta_rot,	// radians
					double &move_time,
					long &x_steps,
					long &y_steps,
					long &z_steps);

// calc_alt_az_deltas
//
// calculate the deltas for an alt/az taking into account limits, etc.
// all parameters are in radians
extern void calc_alt_az_deltas(double curr_az, double curr_alt, double curr_pa,
			double new_az, double new_alt, double new_pa,
			double &delta_az, double &delta_alt, double &delta_pa);

// read_mount_correction_model
//
// read the mount correction model
extern unsigned read_mount_correction_model(char *);
extern unsigned set_mount_correction_model();

extern void steps_to_degrees(long steps_x, long steps_y, long steps_z, 
        double &deg_x, double &deg_y, double &deg_z);
extern void degrees_to_steps( double deg_x, double deg_y, double deg_z,
        long &steps_x, long &steps_y, long &steps_z);

#endif
/********************************* EOF **************************************/

