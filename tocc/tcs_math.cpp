/****************************************************************************/
/*                                                              */
/*      Module:         tcs_math.cpp                            */
/*                                                              */
/*      Purpose:        common conversion routines, etc.        */
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
#include <mem.h>
#include <conio.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream.h>
#include <stdlib.h>

#include "tcs_math.h"
#include "status.h"
#include "globals.h"
#include "guiding.h"
#include "inst.h"
#include "slalib.h"
#include "slamac.h"
#include "tp.h"
#include "tcs.h"
#include "io.h"

int tcs_math_verbose = 0;

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
				struct ALLTIMES &timeRec)
	{
		int rval;
		int stat = 0;

		// calculate the equinox
		int nday;
		int nyear;
		double fday;

		slaClyd(d.da_year, d.da_mon, d.da_day, &nyear, &nday, &rval);
		stat |= rval;
		timeRec.equinox = (double)nyear + ((double)nday / 365.0);
		fday = (double)t.ti_hour * 3600.0;
		fday += (double)t.ti_min * 60.0;
		fday += (double)t.ti_sec;
		fday += (double)t.ti_hund / 100.0;
		fday /= (86400.0 * 365.25);
		timeRec.equinox += fday;

		// calculate the UTC as mjd
		double fmjd;
		double fsec;

		slaCldj(d.da_year, d.da_mon, d.da_day, &timeRec.mjd_utc, &rval);
		stat |= rval;
		fsec = (double)t.ti_sec + ((double)t.ti_hund / 100.0);
		slaDtf2d(t.ti_hour, t.ti_min, fsec, &fmjd, &rval);
		stat |= rval;
		timeRec.mjd_utc += fmjd;

		// calculate the terrestrial time
		double mjd_ut1 = timeRec.mjd_utc + (G->ut1_minus_utc / 86400.0);
		double delta_tt = slaDtt(mjd_ut1);
		timeRec.mjd_tt = mjd_ut1 + (delta_tt / 86400.0);

		// calculate the apparent LST
		double gmst = slaGmst(mjd_ut1);
		timeRec.last = slaDranrm(gmst + G->longitude + slaEqeqx(mjd_ut1));
		timeRec.lasth = timeRec.last * DR2H;
		return stat;
	}

// get_new_date_time_at
//
// given a date, time, and a number of seconds in the future, return a new
// date and time.
//
unsigned char mon_arr[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31,
															 30, 31 };

void get_new_date_time_at(struct date sd, struct mytime st, unsigned seconds, 
			  struct date &nd, struct mytime &nt)
{
    memcpy(&nt, &st, sizeof(st));
    memcpy(&nd, &sd, sizeof(sd));

// *****************
// **** NOTE!!! ****
// *****************
//
// since the time fields are unsigned chars, and can only accomdate values
// between 0 and 255, we can only add up to 196 seconds (59 + 196 = 255)
// at a time.

    unsigned char inc;
    unsigned char temp;

    while (seconds > 0)
    {
	inc = (seconds > 196) ? 196 : seconds;
	seconds -= inc;

	nt.ti_sec += inc;
	if (nt.ti_sec >= 60)
	{
		temp = nt.ti_sec;
		nt.ti_sec %= 60;
		nt.ti_min += (temp / 60);

		if (nt.ti_min >= 60)
		{
			nt.ti_min -= 60;
			nt.ti_hour++;
  
			if (nt.ti_hour >= 24)
			{
				nt.ti_hour -= 24;
				nd.da_day++;

				if (nd.da_day > mon_arr[nd.da_mon - 1])
				{
					nd.da_day = 1;
					nd.da_mon++;

					if (nd.da_mon == 13)
					{
						nd.da_mon = 1;
						nd.da_year++;
					}
				}
			}
		}
	}
     }
}

void get_new_date_time_at_hund(struct date sd, struct mytime st, unsigned hund, 
			       struct date &nd, struct mytime &nt)
{
    memcpy(&nt, &st, sizeof(st));
    memcpy(&nd, &sd, sizeof(sd));

// *****************
// **** NOTE!!! ****
// *****************
//
// since the time fields are unsigned chars, and can only accomdate values
// between 0 and 255, we can only add up to 156 seconds (99 + 156 = 255)
// at a time.

    unsigned char temp;

    nt.ti_hund += hund;
    while (nt.ti_hund >= 100.) {
	  nt.ti_hund -= 100.;
	  nt.ti_sec ++;
    }

    if (nt.ti_sec >= 60)
    {
	temp = nt.ti_sec;
	nt.ti_sec %= 60;
	nt.ti_min += (temp / 60);

	if (nt.ti_min >= 60)
	{
		nt.ti_min -= 60;
		nt.ti_hour++;
  
		if (nt.ti_hour >= 24)
		{
			nt.ti_hour -= 24;
			nd.da_day++;

			if (nd.da_day > mon_arr[nd.da_mon - 1])
			{
				nd.da_day = 1;
				nd.da_mon++;

				if (nd.da_mon == 13)
				{
					nd.da_mon = 1;
					nd.da_year++;
				}
			}
		}
	}
    }
}

// equtohor
//
// convert equatorial position to az/alt
//
extern void equtohor(   double ra,              // radians
			double dec,             // radians
			double epoch,
			double pmra,    // radians
			double pmdec,   // radians
			double parallax,
			double radial_velocity,
			double &az,             // radians
			double &alt             // radians
			)

	{
		struct ALLTIMES timeRec;
		struct mytime t;
		struct date d;
		double p1, p2, p3;

		// get the mjd and lst
		mygettime(&d,&t);
		//getdate(&d);
		get_all_times_at(d, t, timeRec);

		mean_to_mount_corrected(timeRec, ra, dec, epoch, 
		   pmra, pmdec, parallax, radial_velocity, p1, p2, p3);

		if (sysGlobal->mount_type)
			{
				az = p1;
				alt = p2;
			}
		else
			slaDe2h(p1, p2, G->latitude, &az, &alt);
	}

// hortoequ
//
// convert az/alt to equatorial
//
extern void hortoequ(double az,         // radians
			double alt,     // radians
			double &ra,     // radians
			double &dec     // radians
			)

	{
		double ha;
		struct mytime t;
		struct date d;
		struct ALLTIMES timeRec;

		mygettime(&d,&t);
		//getdate(&d);
		get_all_times_at(d, t, timeRec);

		slaDh2e(az, alt, G->latitude, &ha, &dec);
		ra = slaDranrm(timeRec.last - ha);
	}

// mean_to_mount_corrected
//
// convert a mean position at a given time to the fully corrected position
void mean_to_mount_corrected(struct ALLTIMES timeRec,
		double mean_ra,                 // radians
		double mean_dec,                // radians
		double mean_epoch,
		double mean_pmra,               // radians
		double mean_pmdec,      // radians
		double mean_parallax,
		double mean_radial_velocity,
		double &corrected_parm1,        // radians (ha/az)
		double &corrected_parm2,        // radians (dec/alt)
		double &corrected_parm3         // radians (dec/alt)
		)

{
	double az1, el1, rot1, parang1, ha1, dec1;
	double az2, el2, rot2, parang2, ha2, dec2;
	double rot;

	mean_to_mount_corrected_sub(timeRec, mean_ra, mean_dec, mean_epoch,
		   mean_pmra, mean_pmdec, mean_parallax, mean_radial_velocity,
		   az1, el1, rot1, parang1);
 // OK, convert this back to an artificial RA/DEC assuming a perfect system
	slaDh2e(az1, el1, G->latitude, &ha1, &dec1);
 // Now convert this back to an artificial RA/DEC assuming a perfect system
 //   for an offset in elevation (y)
	slaDh2e(az1, el1+5.e-5, G->latitude, &ha2, &dec2);
	rot = -el1 - atan2(dec2-dec1,(ha2-ha1)*cos(dec1)) + G->current_pa;

	double new_ra, new_dec;

	new_dec = mean_dec + 5.e-5;
	mean_to_mount_corrected_sub(timeRec, mean_ra, new_dec, mean_epoch,
		   mean_pmra, mean_pmdec, mean_parallax, mean_radial_velocity,
		   az2, el2, rot2, parang2);
        if (autoGlobal->tertiary_port==1)
	  rot = -el1 - atan2((az2-az1)*cos(el1),el2-el1) + G->current_pa;
        else if (autoGlobal->tertiary_port==2)
	  rot = el1 - atan2((az2-az1)*cos(el1),el2-el1) + G->current_pa;
	if (tcs_math_verbose) {
	  sprintf(outbuf,
		 "rot: %lf %lf %lf %lf\r\n  %lf %lf %lf %lf\n  %lf %lf %lf",
		  rot1*DR2D,rot2*DR2D,rot*DR2D,(rot-rot1)*DR2D,
		  az1*DR2D,az2*DR2D,el1*DR2D,el2*DR2D,
		  parang1, atan2((az2-az1)*cos(el1),el2-el1),
		  atan2(dec2-dec1,(ha2-ha1)*cos(dec1)));
	  writeline(outbuf,1);
	}

	corrected_parm1 = az1;
	corrected_parm2 = el1;
	corrected_parm3 = rot;
}

void mean_to_mount_corrected_sub(struct ALLTIMES timeRec,
		double mean_ra,                 // radians
		double mean_dec,                // radians
		double mean_epoch,
		double mean_pmra,               // radians
		double mean_pmdec,      // radians
		double mean_parallax,
		double mean_radial_velocity,
		double &corrected_parm1,        // radians (ha/az)
		double &corrected_parm2,        // radians (dec/alt)
		double &corrected_parm3,                // radians (dec/alt)
		double &parang          // radians (dec/alt)
		)

	{
		double corrected_ra, corrected_dec;
		double inst_ra, inst_dec;

		// Correct for position of instrument relative to rotator center
		apply_instrument_correction(mean_ra, mean_dec, 
					    inst_ra, inst_dec);

		// Guiding correction
		apply_guide_correction_radec(&inst_ra, &inst_dec);

                if (G->apply_rates) {
                  inst_ra += G->dra*(timeRec.mjd_utc-G->epoch0);
                  inst_dec += G->ddec*(timeRec.mjd_utc-G->epoch0);
                }

		// proper motion IS DONE IN MAPQK ALREADY
		//slaPm(mean_ra, mean_dec, mean_pmra, mean_pmdec, mean_parallax,
		//      mean_radial_velocity, mean_epoch, timeRec.equinox,
		//      &corrected_ra, &corrected_dec);

		// quick mean to apparent
		slaMapqk(inst_ra, inst_dec, mean_pmra, mean_pmdec,
		    mean_parallax, mean_radial_velocity, G->mean_to_app_parms,
		    &corrected_ra, &corrected_dec);

		// recompute siderial time in app_to_obs_parms
		slaAoppat(timeRec.mjd_utc, G->app_to_obs_parms);

		// apparent to observed
		double obs_az, obs_zen_dx, obs_ha, obs_dec, obs_ra;
		slaAopqk(corrected_ra, corrected_dec, G->app_to_obs_parms,
			&obs_az, &obs_zen_dx, &obs_ha, &obs_dec, &obs_ra);

	// slaAopqk will return an azimuth between -pi and +pi. Set to 0 to 2pi
		obs_az = slaDranrm(obs_az);
		double obs_el = DPIBY2 - obs_zen_dx;

		// parallactic angle (for rotator angle)
		double hour_angle = G->app_to_obs_parms[13] - corrected_ra;
		double parallactic_angle =
		  slaPa(slaDrange(hour_angle), corrected_dec, G->latitude);
                parang = parallactic_angle;

		// rotator angle is combination of elevation, parallactic angle
		//    and desired instrument position angle
                if (autoGlobal->tertiary_port==1)
		  corrected_parm3 = -obs_el - parallactic_angle + G->current_pa ;
                else if (autoGlobal->tertiary_port=2)
		  corrected_parm3 = obs_el - parallactic_angle + G->current_pa ;

		// apply alt-az offset
		apply_guide_correction_altaz(obs_az,obs_el);

		// run through mount corrections
		if (!G->mc_enabled) {
		  if (!sysGlobal->mount_type) {
			corrected_parm1 = obs_ha;
			corrected_parm2 = obs_dec;
		  } else {
			corrected_parm1 = obs_az;
			corrected_parm2 = obs_el;
		  }
		  return;
		}

		switch (sysGlobal->mount_type) {
			case 0: // equatorial
			  equat_f(obs_az, obs_el, G->latitude, 
				G->tpoint_me,
				G->tpoint_ma, G->tpoint_ch, 
				G->tpoint_np, G->tpoint_fo,
				G->tpoint_tf, G->tpoint_ih, 
				G->tpoint_id,
				&corrected_parm1, &corrected_parm2);
			  break;

			case 1: // alt-az cassegrain
			  altaz_c(obs_az, obs_el, 
				G->tpoint_an, G->tpoint_aw,
				G->tpoint_ca, G->tpoint_npae, 
				G->tpoint_tf,
				G->tpoint_ia, G->tpoint_ie,
				&corrected_parm1, &corrected_parm2);
				break;

			case 2: // alt-az naysmith
		          altaz_n(obs_az, obs_el, 
				G->tpoint_an, G->tpoint_aw,
				G->tpoint_nrx, G->tpoint_nry, 
				G->tpoint_ca,
				G->tpoint_npae, G->tpoint_tf, 
				G->tpoint_ia, G->tpoint_ie,
				&corrected_parm1, &corrected_parm2);
	if (tcs_math_verbose) { 
          sprintf(outbuf,
            "%12.6f %12.6f %12.6f %12.6f %12.6f %12.6f %12.6f %12.6f %12.6f\n",
	    G->tpoint_an, G->tpoint_aw, G->tpoint_nrx, G->tpoint_nry, 
	    G->tpoint_ca, G->tpoint_npae, G->tpoint_tf, 
	    G->tpoint_ia, G->tpoint_ie);
	  writeline(outbuf,1);
	  sprintf(outbuf,"mount correction: %12.4f %12.4f %12.4f %12.4f\n",
		  corrected_parm1*DR2D, corrected_parm2*DR2D, 
                  (corrected_parm1-obs_az)*DR2AS*cos(corrected_parm2),
		  (corrected_parm2-obs_el)*DR2AS);
	  sprintf(outbuf+strlen(outbuf),"mount+guide correction: %12.3f %12.3f\n",
		  (corrected_parm1-obs_az)*DR2AS*cos(corrected_parm2),
		  (corrected_parm2-obs_el)*DR2AS);
	  writeline(outbuf,1);
	}
			  break;
			}

	}

// calc_move_time
//
// calculate a move time given the axis steps and other information
double calc_move_time(long steps, double minimum_slew_rate,
		      long maximum_slew_rate, long acceleration)
	{
		double move_time, accel, x, vo, vmax, xmax;

		if (!steps)
			return 0.0;

		accel = acceleration;
		x = labs(steps);
		vo = abs(minimum_slew_rate);
		vmax = maximum_slew_rate;

		// First, see how far the move must be to achieve full velocity
		xmax = (pow(vmax,2) - pow(vo,2)) / (2.0 * accel);

// If the distance is at least half the total distance, we will achieve 
// maximum velocity during the travel.  So, the time will consist of   
// ramp-up, constant velocity, and ramp-down.                         

		if (xmax <= (x / 2.0))
		{
			// Calculate the ramp-up/ramp-down time
			move_time = (4.0 * xmax) / (vo + vmax);

			// Add the constant velocity time
			move_time = move_time + ((x - (2.0 * xmax)) / vmax);
		}
		else
		{
	// We will not achieve maximum velocity.  First, determine the maximum 
	// velocity that will be reached.  NOTE: We use half the distance in   
	// this equation since we have to ramp back down.                     
			vmax = sqrt(pow(vo,2) + (2.0 * accel * (x / 2.0)));

			// Now, calculate the time using this maximum velocity
			move_time = (4.0 * (x / 2.0)) / (vo + vmax);
		}
		return move_time;
	}

// calc_equ_move_time_steps
//
// given a delta_ra and delta_dec, return the time necessary to move
// and the stesp in each axis
void calc_equ_move_time_steps(double delta_ha,  // radians
				double delta_dec,       // radians
				double &move_time,
				long &x_steps,
				long &y_steps)
{
		double timex, timey;

		// Convert into steps
		x_steps = (long)(delta_ha * DR2D * sysGlobal->x_steps_degree);
		y_steps = (long)(delta_dec * DR2D * sysGlobal->y_steps_degree);

		// Calculate the move times
		timex = calc_move_time(x_steps,0, sysGlobal->x_max_velocity,
						sysGlobal->x_acceleration);
		timey = calc_move_time(y_steps,0, sysGlobal->y_max_velocity,
						sysGlobal->y_acceleration);

		move_time = (timex >= timey) ? timex : timey;

		sprintf(outbuf, "\nCalculate EQU move time & steps\n"
														"-------------------------------\n"
				"Delta hour-angle  : %.10f\n"
				"Delta declination : %.10f\n"
				"x steps           : %ld\n"
				"y steps           : %ld\n"
				"move time         : %.10f\n",
				(delta_ha * DR2H),
				(delta_dec * DR2D),
				x_steps,
				y_steps,
				move_time);
		writeline(outbuf,2);
	}

// calc_altaz_move_time_steps
//
// given delta_az, delta_alt, delta_rot, return time and steps for move
void calc_altaz_move_time_steps(double delta_az,        // radians
				double delta_alt,       // radians
				double delta_rot,       // radians
				double &move_time,
				long &x_steps,
				long &y_steps,
				long &z_steps)

	{
		double timex, timey, timez;

		// Convert into steps
		x_steps = (long)(delta_az * DR2D * sysGlobal->x_steps_degree);
		y_steps = (long)(delta_alt * DR2D * sysGlobal->y_steps_degree);
		z_steps = (long)(delta_rot * DR2D * sysGlobal->z_steps_degree);

		// Calculate the move times.
		timex = calc_move_time(x_steps,0, sysGlobal->x_max_velocity,
						sysGlobal->x_acceleration);
		timey = calc_move_time(y_steps,0, sysGlobal->y_max_velocity,
						sysGlobal->y_acceleration);
		timez = calc_move_time(z_steps,0, sysGlobal->z_max_velocity,
						sysGlobal->z_acceleration);

		// Find the longest move time.
		move_time = timex;
		if (timey > move_time) move_time = timey;
		if (timez > move_time) move_time = timez;

//              sprintf(outbuf, "\nCalculate Alt-Az move time & steps\n"
//                              "----------------------------------\n"
//                              "Delta az  : %.10f\n"
//                              "Delta alt : %.10f\n"
//                              "x steps   : %ld\n"
//                              "y steps   : %ld\n"
//                              "z steps   : %ld\n"
//                              "move time : %.10f\n",
//                              (delta_az * DR2D),
//                              (delta_alt * DR2D),
//                              x_steps,
//                              y_steps,
//                              z_steps,
//                              move_time);
//              writeline(outbuf,2);
	}

// calc_alt_az_deltas
//
// calculate the deltas for an alt/az taking into account limits, etc

struct TestCase
	{
		BOOL good;
		double delta;
		long newPos;
		long limdx;
	};

void calc_alt_az_deltas(double curr_az, double curr_alt, double curr_pa,
			double new_az, double new_alt, double new_pa,
			double &delta_az, double &delta_alt, double &delta_pa)
	{
		struct TestCase cases[4];

		// calculate the delta alt
		delta_alt = new_alt - curr_alt;

		// We'll need to know which way the azimuth and rotator
		//   will be increasing when tracking. Brute force it
		//   with wasteful slalib routines
		double ha, dec;
		slaDh2e(new_az,new_alt,G->latitude,&ha,&dec);
		double az, azd, azdd, el, eld, eldd, pa, pad, padd;
		slaAltaz(ha,dec,G->latitude,&az,&azd,&azdd,&el,&eld,&eldd,
			 &pa,&pad,&padd);
	    

		// Because of the way in which the azimuth drive works, there 
		// are 4 ways to move to the same new position in azimuth.
		//
		//    + delta                           - delta
		// 1. Move delta_az                     Move delta_az
		// 2. Move delta_az + 3600              Move delta_az - 3600
		// 3. Move delta_az - 3600              Move 3600 + delta_az
		// 4. Move delta_az - 7200              Move 7200 + delta_az
		//
		// get the current step position
		long pos_x = tcs_return_step_position('x');

		// calculate the raw delta az
		delta_az = new_az - curr_az;

		// normalize delta_az between -pi and pi
		delta_az = slaDrange(delta_az);

		if (delta_az == 0)
			return;

		cases[0].delta = delta_az;
		cases[1].delta = 
		     (delta_az > 0) ? delta_az + D2PI : delta_az - D2PI;
		cases[2].delta = 
		     (delta_az > 0) ? delta_az - D2PI : delta_az + D2PI;
		cases[3].delta = 
		     (delta_az > 0) ? delta_az - D4PI : delta_az + D4PI;

		// figure out which moves are within the limit switch range
		for (int i = 0; i < 4; i++)
			{
				cases[i].newPos = cases[i].delta * DR2D * 
					sysGlobal->x_steps_degree;
				if (!sysGlobal->x_geartrain_normal)
					cases[i].newPos = -cases[i].newPos;
				cases[i].newPos += pos_x;
				cases[i].good = 
				   tcs_pos_within_limits('x', cases[i].newPos);
			}

		// of those that are still good, which one gives us the most 
		// tracking distance after the move?
		//
		// at the "to" position, is the azimuth increasing or decreasing
		// while tracking?
		BOOL increasing = (azd > 0);
       // old autoscope way is incorrect
//              BOOL increasing = 
//                   ((new_az >= DPIBY2) && (new_az <= (3.0 * DPIBY2)));

   // put this in for high dec where az increases a bit before decreasing
                if (dec>G->latitude) increasing = 0;

		for (i = 0; i < 4; i++)
			{
				if (!cases[i].good)
				// If out of limits, reject
					cases[i].limdx = 0;
				else
				{
					if ((increasing && 
					  sysGlobal->x_geartrain_normal) ||
					  (!increasing && 
					   !sysGlobal->x_geartrain_normal))
				// tracking will go toward the positive limit
					  cases[i].limdx = 
					       sysGlobal->x_pos_soft_limit - 
					       cases[i].newPos;
					else
				// tracking will go toward the negative limit
					  cases[i].limdx = cases[i].newPos - 
					       sysGlobal->x_neg_soft_limit;
				}
sprintf(outbuf,"azimuth %d: %d %ld %ld",i,increasing,cases[i].newPos,cases[i].limdx);
writeline(outbuf,1);
			}

		int best = 0;
		for (i = 0; i < 3; i++)
			if (cases[i].limdx > cases[best].limdx)
				best = i;

		// now we have the best move.  
		// Set delta_az to the best move's delta
		delta_az = cases[best].delta;

		// now do the same for the rotator
		// are 4 ways to move to the same new position in rotator.
		//
		//    + delta                           - delta
		// 1. Move delta_pa             Move delta_pa
		// 2. Move delta_pa + 3600      Move delta_pa - 3600
		// 3. Move delta_pa - 3600      Move 3600 + delta_pa
		// 4. Move delta_pa - 7200      Move 7200 + delta_pa
		//
		// get the current step position
		long pos_z = tcs_return_step_position('z');

		// calculate the raw delta az
		delta_pa = new_pa - curr_pa;

		// normalize delta_az between -pi and pi
		delta_pa = slaDrange(delta_pa);

		if (delta_pa == 0)
			return;

		cases[0].delta = delta_pa;
		cases[1].delta = 
		     (delta_pa > 0) ? delta_pa + D2PI : delta_pa - D2PI;
		cases[2].delta = 
		     (delta_pa > 0) ? delta_pa - D2PI : delta_pa + D2PI;
		cases[3].delta = 
		     (delta_pa > 0) ? delta_pa - D4PI : delta_pa + D4PI;

		// figure out which moves are within the limit switch range
		for (i = 0; i < 4; i++)
			{
				cases[i].newPos = cases[i].delta * DR2D * 
					sysGlobal->z_steps_degree;
				if (!sysGlobal->z_geartrain_normal)
					cases[i].newPos = -cases[i].newPos;
				cases[i].newPos += pos_z;
				cases[i].good = 
				   tcs_pos_within_limits('z', cases[i].newPos);

		    // For rotator, the current algorithm may fail if the
		    //   rotator changes direction after some time, as can
		    //   happen near the zenith. As a result, don't allow
		    //   the rotator to get set _near_ a limit, even if the
		    //   rate at the _current_ location has it moving away
		    //   from the limit; it might turn around!

			 double zpad = 10 * sysGlobal->z_steps_degree;
			 long step_pos = cases[i].newPos;
			 cases[i].good = 
			   ((step_pos > sysGlobal->z_neg_soft_limit+zpad) &&
			   (step_pos < sysGlobal->z_pos_soft_limit-zpad));

			}

	// of those that are still good, which one gives us the most 
	// tracking distance after the move?
	//
	// at the "to" position, is the rotator increasing or decreasing
		// while tracking?
		increasing = ((-eld - pad) > 0);
   // put this in for high dec where az increases a bit before decreasing
                if (dec>G->latitude) increasing = 0;

           if (tcs_math_verbose) {
             sprintf(outbuf,"pa: %lf el: %lf -el-pa: %lf\n"
                      "pad: %lf   eld: %lf -eld-pad: %lf increasing: %d\n",
                      pa, el, -el-pa, pad, eld, -eld-pad, increasing);
             writeline(outbuf,1);
           }

		for (i = 0; i < 4; i++)
			{
				if (!cases[i].good)
					cases[i].limdx = 0;
				else
				{
					if ((increasing && 
					  sysGlobal->z_geartrain_normal) ||
					  (!increasing && 
					   !sysGlobal->z_geartrain_normal))
				// tracking will go toward the positive limit
					  cases[i].limdx = 
					       sysGlobal->z_pos_soft_limit - 
					       cases[i].newPos;
					else
				// tracking will go toward the negative limit
					  cases[i].limdx = cases[i].newPos - 
					       sysGlobal->z_neg_soft_limit;
				}
sprintf(outbuf,"rotator %d: %d %ld %ld",i,increasing,cases[i].newPos,cases[i].limdx);
writeline(outbuf,1);
			}

		best = 0;
		for (i = 0; i < 3; i++)
			if (cases[i].limdx > cases[best].limdx)
				best = i;

		// now we have the best move.  
		// Set delta_pa to the best move's delta
		delta_pa = cases[best].delta;

	}

// read_mount_correction_model
//
// read the mount correction model
unsigned set_mount_correction_model()
{
		G->tpoint_npae = autoGlobal->tpoint_npae*DAS2R;
		G->tpoint_nrx = autoGlobal->tpoint_nrx*DAS2R;
		G->tpoint_nry = autoGlobal->tpoint_nry*DAS2R;
		G->tpoint_tf = 0;
		G->tpoint_ih = 0;
		G->tpoint_id = 0;
		G->tpoint_np = 0;
		G->tpoint_ch = 0;
		G->tpoint_me = 0;
		G->tpoint_ma = 0;
		G->tpoint_fo = 0;
		G->tpoint_ie = autoGlobal->tpoint_ie*DAS2R;
		G->tpoint_ia = autoGlobal->tpoint_ia*DAS2R;
		G->tpoint_ca = autoGlobal->tpoint_ca*DAS2R;
		G->tpoint_an = autoGlobal->tpoint_an*DAS2R;
		G->tpoint_aw = autoGlobal->tpoint_aw*DAS2R;

	sprintf(outbuf,"ia: %12.2f %12.4f\n"
		       "ie: %12.2f %12.4f\n"
		       "npae: %12.2f %12.4f\n"
		       "ca: %12.2f %12.4f\n"
		       "an: %12.2f %12.4f\n"
		       "aw: %12.2f %12.4f\n"
		       "nrx: %12.2f %12.4f\n"
		       "nry: %12.2f %12.4f\n",
		       autoGlobal->tpoint_ia,G->tpoint_ia,
		       autoGlobal->tpoint_ie,G->tpoint_ie,
		       autoGlobal->tpoint_npae,G->tpoint_npae,
		       autoGlobal->tpoint_ca,G->tpoint_ca,
		       autoGlobal->tpoint_an,G->tpoint_an,
		       autoGlobal->tpoint_aw,G->tpoint_aw,
		       autoGlobal->tpoint_nrx,G->tpoint_nrx,
		       autoGlobal->tpoint_nry,G->tpoint_nry);
	writeline(outbuf,0);
        return(0);
}

unsigned read_mount_correction_model(char *filename)
	{

/*
		char *scfdir = getenv(ENV_SCF_DIR);
		char *filename = (char*)malloc(80);
		memset(filename, 0, 80);

		if (scfdir)
			{
				strcpy(filename, scfdir);
				if ((scfdir[strlen(scfdir)-1]) != '\\')
					strcat(filename, "\\");
			}
		strcat(filename, "mntcorr.mod");
*/

		FILE *mcfile = fopen(filename, "r");
		if (!mcfile)
			return TCSERR_NOMCFILE;

//                sprintf(autoGlobal->mc_file,"%s",filename);

		char buffer[256];
		char *label;
		char *term;
		int terms;
/*
		terms = 
	((!sysGlobal->mount_type) ? 8 : ((sysGlobal->mount_type == 1) ? 7 : 9));

		cout << "Reading mount correction model.\n\n";
*/
		G->tpoint_npae = 0;
		G->tpoint_nrx = 0;
		G->tpoint_nry = 0;
		G->tpoint_tf = 0;
		G->tpoint_ih = 0;
		G->tpoint_id = 0;
		G->tpoint_np = 0;
		G->tpoint_ch = 0;
		G->tpoint_me = 0;
		G->tpoint_ma = 0;
		G->tpoint_fo = 0;
		G->tpoint_ie = 0;
		G->tpoint_ia = 0;
		G->tpoint_ca = 0;
		G->tpoint_an = 0;
		G->tpoint_aw = 0;

		while (fgets(buffer, 255, mcfile))
			{
				sprintf(outbuf,"%s\r\n",buffer);
				writeline(outbuf,0);
				label = strtok(buffer, " ");
				term = strtok(NULL, " ");

		// compare for terms whose names are more than 2 characters
				if (strcmpi(label, "npae") == 0)
				{
					G->tpoint_npae = atof(term)*DAS2R;
					terms--;
				}
				else if (strcmpi(label, "nrx") == 0)
				{
					G->tpoint_nrx = atof(term)*DAS2R;
					terms--;
				}
				else if (strcmpi(label, "nry") == 0)
				{
					G->tpoint_nry = atof(term)*DAS2R;
					terms--;
				}
				else
				{
					unsigned i = (label[1] << 8) + label[0];
					terms--;
					switch (i)
					{
				case 'TF': G->tpoint_tf = atof(term)*DAS2R; break;
				case 'IH': G->tpoint_ih = atof(term)*DAS2R; break;
				case 'ID': G->tpoint_id = atof(term)*DAS2R; break;
				case 'NP': G->tpoint_np = atof(term)*DAS2R; break;
				case 'CH': G->tpoint_ch = atof(term)*DAS2R; break;
				case 'ME': G->tpoint_me = atof(term)*DAS2R; break;
				case 'MA': G->tpoint_ma = atof(term)*DAS2R; break;
				case 'FO': G->tpoint_fo = atof(term)*DAS2R; break;
				case 'IE': G->tpoint_ie = atof(term)*DAS2R; break;
				case 'IA': G->tpoint_ia = atof(term)*DAS2R; break;
				case 'CA': G->tpoint_ca = atof(term)*DAS2R; break;
				case 'AN': G->tpoint_an = atof(term)*DAS2R; break;
				case 'AW': G->tpoint_aw = atof(term)*DAS2R; break;
				default: terms++;
					}
				}
			}

		// close the file
		fclose(mcfile);

	sprintf(outbuf,"ia: %12.4f\n"
		       "ie: %12.4f\n"
		       "npae: %12.4f\n"
		       "ca: %12.4f\n"
		       "an: %12.4f\n"
		       "aw: %12.4f\n"
		       "nrx: %12.4f\n"
		       "nry: %12.4f\n"
		       "tf: %12.4f\n",
		       G->tpoint_ia,
		       G->tpoint_ie,
		       G->tpoint_npae,
		       G->tpoint_ca,
		       G->tpoint_an,
		       G->tpoint_aw,
		       G->tpoint_nrx,
		       G->tpoint_nry,
		       G->tpoint_tf);
	writeline(outbuf,0);


		// return the error code
		unsigned status = (terms > 0) ? TCSERR_BADMCFILE : TCSERR_OK;
		return status;
	}

void steps_to_degrees(long steps_x,long steps_y,long steps_z,
		 double &deg_x,double &deg_y,double &deg_z)
{
	if (sysGlobal->x_encoder_installed)
	     deg_x = steps_x /
		  sysGlobal->x_encoder_encoder_steps_deg +  G->ref_az;
	else
	     deg_x = steps_x / sysGlobal->x_steps_degree +  G->ref_az;

	if (sysGlobal->y_encoder_installed)
	     deg_y = steps_y /
		  sysGlobal->y_encoder_encoder_steps_deg +  G->ref_alt;
	else
	     deg_y = steps_y / sysGlobal->y_steps_degree +  G->ref_alt;

	if (sysGlobal->z_encoder_installed)
	     deg_z = steps_z /
		  sysGlobal->z_encoder_encoder_steps_deg +  G->ref_rot;
	else
	     deg_z = steps_z / sysGlobal->z_steps_degree +  G->ref_rot;
}

void degrees_to_steps(double deg_x,double deg_y,double deg_z,
		 long &steps_x,long &steps_y,long &steps_z)
{
	double ref_x, ref_y, ref_z;
 
	if (sysGlobal->x_encoder_installed)
	     steps_x = (long)((deg_x - G->ref_az) *
		  sysGlobal->x_encoder_encoder_steps_deg);
	else
	     steps_x = (long)((deg_x - G->ref_az) * sysGlobal->x_steps_degree);

	if (sysGlobal->y_encoder_installed)
	     steps_y = (long)((deg_y - G->ref_alt) * 
		  sysGlobal->y_encoder_encoder_steps_deg);
	else
	     steps_y = (long)((deg_y - G->ref_alt) * sysGlobal->y_steps_degree);

	if (sysGlobal->z_encoder_installed)
	     steps_z = (long)((deg_z - G->ref_rot) *
		  sysGlobal->z_encoder_encoder_steps_deg);
	else
	     steps_z = (long)((deg_z - G->ref_rot) * sysGlobal->z_steps_degree);
}

/********************************* EOF **************************************/

