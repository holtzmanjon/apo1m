/****************************************************************************/
/*                                                              */
/*      Module:                 tracking.cpp                    */
/*                                                              */
/*      Purpose:                routines to modify the tracking rates   */
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
/*      Author:         M. Donahue                                      */
/*                                                                      */
/****************************************************************************/
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream.h>

#include "tracking.h"
#include "globals.h"
#include "tcs.h"
#include "tcs_math.h"
#include "pcx.h"
#include "systimer.h"
#include "slalib.h"
#include "slamac.h"
#include "io.h"

// update_tracking_rates
//
// modify the tracking rates.  if stopped is TRUE, we are starting from the
// stopped position.

#define MAXPTS 50
double az_error[MAXPTS];
double alt_error[MAXPTS];
double xx[MAXPTS];
double sig[MAXPTS];
void fit(double x[], double y[], int ndata, double sig[], int mwt, double *a,
	double *b, double *siga, double *sigb, double *chi2, double *q);
int xpt, ypt, ipt;

void update_tracking_rates(BOOL stopped)
	{

		// do not update if the telescope is not initialized 
		if (!G->telescope_initialized) return;

		// do not update if we are shutdown
		if (G->shutdown_state) return;

		// if the timer has not expired, exit
		if (!SysTimer[SYSTMR_UPDATE_TRACKING].Expired()) return;

#ifdef OUTPUT
                BOOL output ;
                if (stopped || SysTimer[SYSTMR_TRACKING_CHECK].Expired())
                   output = TRUE;
                else 
                   output = FALSE;
	        output = FALSE;
                //  Reset the timers until next check
		if (output) SysTimer[SYSTMR_TRACKING_CHECK].
                                NewTimerSecs(SYSTMR_TRACKING_CHECK_INC);
		SysTimer[SYSTMR_UPDATE_TRACKING].NewTimer(SYSTMR_TRACKING_INC);
#endif
		// do not update if the tracking is turned off
		if (!G->tracking_on) return;

		// do not update if the telescope is in the middle of a move
		if (G->telescope_is_slewing) return;

                // do not update if the telescope is at home
  		if (G->telescope_at_home) return;

		// if we are starting from stopped, we are on a new star.  Set 
		// the tracking modifier factors back to 1.0;
		if (stopped) {
		  G->x_tracking_factor = G->y_tracking_factor = G->z_tracking_factor = 1.0;
                  xpt = 0;
                  ypt = 0;
#ifdef no_hardware
                  randomize();
#endif
                }

                if (G->tracking_factor_mod && SysTimer[SYSTMR_TF].Expired()) {
                   G->x_tracking_factor = G->x_tracking_factor_new;
                   G->y_tracking_factor = G->y_tracking_factor_new;
                   G->z_tracking_factor = G->z_tracking_factor_new;
                   G->tracking_factor_mod = FALSE;
                }

#ifdef OUTPUT
		if (output) {
			sprintf(outbuf,  
                             "------------------------------\n"
			     "Calculate fresh tracking rates\n"
			     "------------------------------\n");
                        writeline(outbuf,3);
                }
#endif
                // get the current time
                static struct mytime t1, t2;
                struct date d1, d2;
                struct ALLTIMES timeRec1, timeRec2;

                // update the current telescope position from the encoders
                update_encoder_position();
                mygettime(&d1,&t1);
                get_all_times_at(d1, t1, timeRec1);

		double curr_ha, curr_dec;
		double curr_az, curr_alt, curr_rot;
		double curr_desired_az, curr_desired_alt, curr_desired_rot;
		double new_ha, new_dec;
		double new_az, new_alt, new_rot;
		double d_az, d_alt, d_rot;

#ifdef OUTPUT
		if (output) {
				sprintf(outbuf,   
                                        "Current time/date\n"
		               "\t%0.2d:%0.2d:%0.2d.%0.2d  %0.2d.%0.2d.%d\n\n",
					t1.ti_hour, t1.ti_min, t1.ti_sec, t1.ti_hund,
					d1.da_day, d1.da_mon, d1.da_year);
                                writeline(outbuf,3);

				sprintf(outbuf, "Current time record\n"
					"\tmjd_tt         : %.10f\n"
					"\tmjd_utc        : %.10f\n"
					"\tLAST (radians) : %.10f\n"
					"\tLAST (hours)   : %.10f\n"
					"\tequinox        : %.10f\n\n",
					timeRec1.mjd_tt, timeRec1.mjd_utc, timeRec1.last,
					timeRec1.lasth, timeRec1.equinox);
                                writeline(outbuf,3);
		}
#endif

	// add tracking_dtime 1/100th seconds and get the times in the future
		get_new_date_time_at_hund(d1, t1, G->tracking_dtime, d2, t2);
		get_all_times_at(d2, t2, timeRec2);

#ifdef OUTPUT
		if (output) {
				sprintf(outbuf,   
                                        "Future time/date\n"
			        "\t%0.2d:%0.2d:%0.2d.%0.2d  %0.2d.%0.2d.%d\n\n",
					t2.ti_hour, t2.ti_min, t2.ti_sec, t2.ti_hund,
					d2.da_day, d2.da_mon, d2.da_year);
                                writeline(outbuf,3);

				sprintf(outbuf, "Future time record\n"
					"\tmjd_tt         : %.10f\n"
					"\tmjd_utc        : %.10f\n"
					"\tLAST (radians) : %.10f\n"
					"\tLAST (hours)   : %.10f\n"
					"\tequinox        : %.10f\n\n",
					timeRec2.mjd_tt, timeRec2.mjd_utc, timeRec2.last,
					timeRec2.lasth, timeRec2.equinox);
                                writeline(outbuf,3);
		}
#endif
		// get the future position
		if (!sysGlobal->mount_type)
			mean_to_mount_corrected(timeRec2,
					G->current_mean_ra, G->current_mean_dec,
					G->current_mean_epoch,
					G->current_mean_pmra, G->current_mean_pmdec,
					G->current_mean_parallax,
					G->current_mean_radial_velocity,
					new_ha, new_dec, new_rot);
		else
			mean_to_mount_corrected(timeRec2,
					G->current_mean_ra, G->current_mean_dec,
					G->current_mean_epoch,
					G->current_mean_pmra, G->current_mean_pmdec,
					G->current_mean_parallax,
					G->current_mean_radial_velocity,
					new_az, new_alt, new_rot);

// Get the current DESIRED position. With encoder tracking we don't
//   really need this, but we do need it to compute the current error
//   signal which we use to determine the pointing coords and also
//   during moves to see if we've converged. This is a bit of wasted
//   calculation, perhaps.

		// get the current position
		mean_to_mount_corrected(timeRec1,
					G->current_mean_ra, G->current_mean_dec,
					G->current_mean_epoch,
					G->current_mean_pmra, G->current_mean_pmdec,
					G->current_mean_parallax,
					G->current_mean_radial_velocity,
					curr_desired_az, curr_desired_alt, 
                                        curr_desired_rot);

#ifdef OUTPUT
// if we are doing output, convert current desired to steps and compare with
//       current encoder readings
		if (output) {
//  compute where we should be and compare with where we are in encoder units
                     long x_pos, y_pos, z_pos, dx_pos, dy_pos, dz_pos;

                     degrees_to_steps(curr_desired_az*DR2D,
                                      curr_desired_alt*DR2D,
                                      curr_desired_rot*DR2D,x_pos,y_pos,z_pos);
	             sprintf(outbuf, " desired, encoder x position: %ld %ld\n"
                        " desired, encoder y position: %ld %ld\n"
                        " desired, encoder z position: %ld %ld\n" ,
                        x_pos, G->current_enc_az,
                        y_pos, G->current_enc_alt, 
                        z_pos, G->current_enc_rot);
                     writeline(outbuf,3);
	        }
#endif

// if we're going to use the encoder_tracking option, we need to calculate
//     where we are now according to the _encoders_ and use that. In any
//     case, calculate it so we can see an error signal
                steps_to_degrees(G->current_enc_az, G->current_enc_alt,
                                 G->current_enc_rot, curr_az, curr_alt,
                                 curr_rot);
                curr_az *= DD2R;
                curr_alt *= DD2R;
                curr_rot *= DD2R;
#ifdef no_hardware
                G->last_az_error = (0.0006*((float)rand()-RAND_MAX/2))/RAND_MAX;
                G->last_alt_error = (0.0006*((float)rand()-RAND_MAX/2))/RAND_MAX;
                G->last_rot_error = 0.;
#else
                G->last_az_error = slaDrange(curr_az - curr_desired_az)*DR2D;
                G->last_alt_error = slaDrange(curr_alt - curr_desired_alt)*DR2D;
                G->last_rot_error = slaDrange(curr_rot - curr_desired_rot)*DR2D;
#endif

// If encoder_tracking==0, use current desired position, i.e. rate at this point in sky
// If encoder_tracking==1, use current encoder position
// If encoder_tracking>1, fit trend of last number of specified points
// If encoder_tracking<0, use weighted average of encoder rate and position rate

		if (G->x_encoder_tracking>1) {
                  xpt++;
	          ipt = xpt %G->x_encoder_tracking;
                  xx[ipt+1] = timeRec1.mjd_utc;
                  az_error[ipt+1] = G->last_az_error;
	          double az_corr, a, b, siga, sigb, chi2, q;
                  if (xpt >= G->x_encoder_tracking) {
                    // Predict where we are now based on last npts points
	            fit(xx,az_error,G->x_encoder_tracking,sig,0,&a,&b,&siga,&sigb,&chi2,&q);
                    az_corr = a + b*timeRec1.mjd_utc;
                    outbuf[0] = 0;
                    int i;
                    for (i=1;i<=G->x_encoder_tracking;i++) {
                      sprintf(outbuf+strlen(outbuf),"%d %lf %lf\n",i,xx[i],az_error[i]);
                    }
                    sprintf(outbuf+strlen(outbuf),"%lf %lf\n",timeRec1.mjd_utc,az_corr);
                    writeline(outbuf,3);
                    if (fabs(G->last_az_error)<fabs(az_corr)) az_corr = G->last_az_error;
                    sprintf(outbuf, "x: %lf %lf %lf\r\n\n",curr_az, curr_desired_az+az_corr, az_corr);
                    writeline(outbuf,3);
                    az_corr *= DD2R;
                    curr_az = curr_desired_az + az_corr;
                  }
                }
		if (G->y_encoder_tracking>1 ) {
                  ypt++;
	          ipt = ypt %G->y_encoder_tracking;
                  xx[ipt+1] = timeRec1.mjd_utc;
                  alt_error[ipt+1] = G->last_alt_error;
	          double alt_corr, a, b, siga, sigb, chi2, q;
                  // Predict where we are now based on last npts points
                  if (ypt >= G->y_encoder_tracking) {
	            fit(xx,alt_error,G->y_encoder_tracking,sig,0,&a,&b,&siga,&sigb,&chi2,&q);
                    alt_corr = a + b*timeRec1.mjd_utc;
                    outbuf[0] = 0;
                    int i;
                    for (i=1;i<=G->y_encoder_tracking;i++) {
                      sprintf(outbuf+strlen(outbuf),"%d %lf %lf\n",i,xx[i],alt_error[i]);
                    }
                    sprintf(outbuf+strlen(outbuf),"%lf %lf\n",timeRec1.mjd_utc,alt_corr);
                    writeline(outbuf,3);
                    if (fabs(G->last_alt_error)<fabs(alt_corr)) alt_corr = G->last_alt_error;
                    sprintf(outbuf,
                           "y: %lf %lf %lf %lf\r\n\n",curr_alt, curr_desired_alt+alt_corr,
                           alt_corr, G->last_alt_error);
                    writeline(outbuf,3);
                    alt_corr *= DD2R;
                    curr_alt = curr_desired_alt + alt_corr;
                  }
                }

// nominal rate (radians/sec)
                double x_rate0 = slaDrange(new_az - curr_desired_az) / G->tracking_dtime * 100;
                double y_rate0 = (new_alt - curr_desired_alt) / G->tracking_dtime * 100;
                double z_rate0 = slaDrange(new_rot - curr_desired_rot) / G->tracking_dtime * 100;
// calculated rate (radians/sec)
                double x_rate = slaDrange(new_az - curr_az) / G->tracking_dtime * 100 ;
                double y_rate = (new_alt - curr_alt) / G->tracking_dtime * 100 ;
                double z_rate = slaDrange(new_rot - curr_rot) / G->tracking_dtime * 100 ;

		if (G->x_encoder_tracking==0 ) 
                  G->x_tracking_rate = x_rate0 * DR2D * sysGlobal->x_steps_degree;
                else if (G->x_encoder_tracking>0) 
                  G->x_tracking_rate = x_rate * DR2D * sysGlobal->x_steps_degree;
                else {
                  double x_rate_error = G->x_encoder_error * DAS2R / G->tracking_dtime * 100;
                  G->x_tracking_rate = ( fabs(x_rate - x_rate0 ) / x_rate_error * x_rate + x_rate0 ) /
                       ( fabs(x_rate-x_rate0)/x_rate_error + 1 );
                  G->x_tracking_rate *= DR2D * sysGlobal->x_steps_degree;
                }
                G->x_tracking_rate *= G->x_tracking_factor;

		if (G->y_encoder_tracking==0 ) 
                  G->y_tracking_rate = y_rate0 * DR2D * sysGlobal->y_steps_degree;
                else if (G->y_encoder_tracking>0)
                  G->y_tracking_rate = y_rate * DR2D * sysGlobal->y_steps_degree;
                else {
                  double y_rate_error = G->y_encoder_error * DAS2R / G->tracking_dtime * 100;
                  G->y_tracking_rate = ( fabs(y_rate - y_rate0 ) / y_rate_error * y_rate + y_rate0 ) /
                       ( fabs(y_rate-y_rate0)/y_rate_error + 1 );
                  G->y_tracking_rate *= DR2D * sysGlobal->y_steps_degree;
                }
                G->y_tracking_rate *= G->y_tracking_factor;

		if (G->z_encoder_tracking==0 ) 
                  G->z_tracking_rate = z_rate0 * DR2D * sysGlobal->z_steps_degree;
                else if (G->z_encoder_tracking>0)
                  G->z_tracking_rate = z_rate * DR2D * sysGlobal->z_steps_degree;
                else {
                  double z_rate_error = G->z_encoder_error * DAS2R / G->tracking_dtime * 100;
                  G->z_tracking_rate = ( fabs(z_rate - z_rate0 ) / z_rate_error * z_rate + z_rate0 ) /
                       ( fabs(z_rate-z_rate0)/z_rate_error + 1 );
                  G->z_tracking_rate *= DR2D * sysGlobal->z_steps_degree;
                }
                G->z_tracking_rate *= G->z_tracking_factor;

/*
  struct date d;
  struct mytime t;
  mygettime(&d,&t);
  double secs;
  secs = t.ti_sec + t.ti_hund/100.;
  sprintf(outbuf,"%d:%d:%f  %10.7f %10.7f %10.7f %10.7f %10.7f %10.7f %8.2f %8.2f %8.2f",t.ti_hour,t.ti_min,secs,x_rate,y_rate,z_rate,x_rate0,y_rate0,z_rate0,G->x_tracking_rate,G->y_tracking_rate,G->z_tracking_rate);
  writeline(outbuf,1);
*/

#ifdef OUTPUT
		if (output) {
			sprintf(outbuf,  "Future az   : %f\n"
			"Present az  : %f\n"
                        "Delta az    : %f\n"
			"Future alt  : %f\n"
			"Present alt : %f\n"
			"Delta alt   : %f\n"
			"Future rot  : %f\n"
			"Present rot : %f\n"
			"Delta rot   : %f\n\n",
			(new_az * DR2D), (curr_az * DR2D), (d_az * DR2D),
			(new_alt * DR2D), (curr_alt * DR2D), d_alt * DR2D,
			(new_rot * DR2D), (curr_rot * DR2D), d_rot * DR2D);
                        writeline(outbuf,3);
			sprintf(outbuf,  
                          "x tracking rate : %.4f\n"
			  "y tracking rate : %.4f\n"
			  "z tracking rate : %.4f\n\n",
			  G->x_tracking_rate, G->y_tracking_rate, G->z_tracking_rate);
                        writeline(outbuf,3);
			sprintf(outbuf,  
                          "x tracking factor : %.4f\n"
			  "y tracking factor : %.4f\n"
			  "z tracking factor : %.4f\n\n",
			  G->x_tracking_factor, G->y_tracking_factor, G->z_tracking_factor);
                        writeline(outbuf,3);
		}
#endif
		// create the command string
		char command[256];
		x_rate = (sysGlobal->x_geartrain_normal) ? 
                     G->x_tracking_rate : -G->x_tracking_rate;

		y_rate = (sysGlobal->y_geartrain_normal) ? 
                     G->y_tracking_rate : -G->y_tracking_rate;

		z_rate = 0.;

		if (sysGlobal->z_axis_enabled)
		     z_rate = (sysGlobal->z_geartrain_normal) ? 
                               G->z_tracking_rate : -G->z_tracking_rate;

                x_rate = min(max(x_rate,(double)(-0.1*sysGlobal->x_max_velocity)),
                             (double)(0.1*sysGlobal->x_max_velocity));
                y_rate = min(max(y_rate,(double)(-0.1*sysGlobal->y_max_velocity)),
                             (double)(0.1*sysGlobal->y_max_velocity));
                z_rate = min(max(z_rate,(double)(-0.1*sysGlobal->z_max_velocity)),
                             (double)(0.1*sysGlobal->z_max_velocity));

                if (G->verbose) fprintf(G->move_file,"%f %f %f %f %f %f %f\n", 
                   timeRec1.mjd_utc,
                   G->last_az_error*3600, G->last_alt_error*3600, G->last_rot_error*3600,
                   x_rate,y_rate,z_rate);

#ifdef OUTPUT
                sprintf(outbuf,"%f %f %f %f %f %f %f\n", 
                   timeRec1.mjd_utc,
                   G->last_az_error*3600, G->last_alt_error*3600, G->last_rot_error*3600,
                   x_rate,y_rate,z_rate);
                writeline(outbuf,3);
#endif
/*
if ( fabs((x_rate-G->last_x_rate)/x_rate) > G->max_rate_change) {
    if (x_rate > G->last_x_rate) 
          x_rate = G->last_x_rate + (G->max_rate_change * fabs(G->last_x_rate));
    else
          x_rate = G->last_x_rate - (G->max_rate_change * fabs(G->last_x_rate));
}
if ( fabs((y_rate-G->last_y_rate)/y_rate) > G->max_rate_change) {
    if (y_rate > G->last_y_rate) 
          y_rate = G->last_y_rate + (G->max_rate_change * fabs(G->last_y_rate));
    else
          y_rate = G->last_y_rate - (G->max_rate_change * fabs(G->last_y_rate));
}
if ( fabs((z_rate-G->last_z_rate)/z_rate) > G->max_rate_change) {
    if (z_rate > G->last_z_rate) 
          z_rate = G->last_z_rate + (G->max_rate_change * fabs(G->last_z_rate));
    else
          z_rate = G->last_z_rate - (G->max_rate_change * fabs(G->last_z_rate));
}
*/
                char x_stop[4], y_stop[4], z_stop[4];
                if (G->last_x_rate*x_rate >= 0 )
                  x_stop[0] = '\0';
                else
                  sprintf(x_stop,"ST;");
                if (G->last_y_rate*y_rate >= 0 )
                  y_stop[0] = '\0';
                else
                  sprintf(y_stop,"ST;");
                if (G->last_z_rate*z_rate >= 0 )
                  z_stop[0] = '\0';
                else
                  sprintf(z_stop,"ST;");
                G->last_x_rate = x_rate;
                G->last_y_rate = y_rate;
                G->last_z_rate = z_rate;

                // Use a JG or a JF command?  cant use JF for large velocities
                char x_com[32], y_com[32], z_com[32];
                if (stopped) {
#ifdef NOTDEF
                  sprintf(x_com,"ax hf; ac%ld; jf%.4f;",
                            sysGlobal->x_acceleration,x_rate);
                  sprintf(y_com,"ay hf; ac%ld; jf%.4f;",
                            sysGlobal->y_acceleration,y_rate);
		  if (sysGlobal->z_axis_enabled) 
                      sprintf(z_com,"az hf; ac%ld; jf%.4f;",
                            sysGlobal->z_acceleration,z_rate);
#endif
                  if (fabs(x_rate) > 1000 )
                    sprintf(x_com,"ax hf; ac%ld; jg%ld;",
                            sysGlobal->x_acceleration,(long)x_rate);
                  else
                    sprintf(x_com,"ax hf; ac%ld; jf%.4f;",
                            sysGlobal->x_acceleration,x_rate);
                  if (fabs(y_rate) > 1000 )
                    sprintf(y_com,"ay hf; ac%ld; jg%ld;",
                            sysGlobal->y_acceleration,(long)y_rate);
                  else
                    sprintf(y_com,"ay hf; ac%ld; jf%.4f;",
                            sysGlobal->y_acceleration,y_rate);
		  if (sysGlobal->z_axis_enabled) {
                    if (fabs(z_rate) > 1000 )
                      sprintf(z_com,"az hf; ac%ld; jg%ld;",
                            sysGlobal->z_acceleration,(long)z_rate);
                    else
                      sprintf(z_com,"az hf; ac%ld; jf%.4f;",
                            sysGlobal->z_acceleration,z_rate);
                  }
                 } else {
                   if (fabs(x_rate) > 1000 )
                     sprintf(x_com,"ax %s jg%ld;", x_stop,(long)x_rate);
                   else
                     sprintf(x_com,"ax %s jf%.4f;", x_stop,x_rate);
                   if (fabs(y_rate) > 1000 )
                     sprintf(y_com,"ay %s jg%ld;", y_stop,(long)y_rate);
                   else
                     sprintf(y_com,"ay %s jf%.4f;", y_stop,y_rate);
	 	   if (sysGlobal->z_axis_enabled) {
                     if (fabs(z_rate) > 1000 )
                       sprintf(z_com,"az %s jg%ld;", z_stop,(long)z_rate);
                     else
                       sprintf(z_com,"az %s jf%.4f;", z_stop,z_rate);
                   }
                 }

#ifdef OUTPUT
		if (output) {
			sprintf(outbuf, "x tracking rate (adj): %.4f\n"
					"y tracking rate (adj): %.4f\n"
					"z tracking rate (adj): %.4f\n\n",
					x_rate, y_rate, z_rate);
                        writeline(outbuf,3);
                }
#endif

		if (!sysGlobal->z_axis_enabled)
			sprintf(command,"%s %s",x_com,y_com);
		else
			sprintf(command,"%s %s %s",x_com,y_com,z_com);

		// send the rates to the PC-38 card
#ifdef OUTPUT
		if (output) {
			sprintf(outbuf, "Command string: %s\n\n", command);
                        writeline(outbuf,3);
                }
#endif

		#ifndef no_hardware
                if (!G->const_tracking)
		  pc38_send_commands(command);
		#endif

	}

/********************************* EOF **************************************/

#include <math.h>
#define NRANSI


/* CAUTION: This is the ANSI C (only) version of the Numerical Recipes
   utility file nrutil.h.  Do not confuse this file with the same-named
   file nrutil.h that is supplied in the 'misc' subdirectory.
   *That* file is the one from the book, and contains both ANSI and
   traditional K&R versions, along with #ifdef macros to select the
   correct version.  *This* file contains only ANSI C.               */

#ifndef _NR_UTILS_H_
#define _NR_UTILS_H_

static double sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

static double dsqrarg;
#define DSQR(a) ((dsqrarg=(a)) == 0.0 ? 0.0 : dsqrarg*dsqrarg)

static double dmaxarg1,dmaxarg2;
#define DMAX(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) > (dmaxarg2) ?\
        (dmaxarg1) : (dmaxarg2))

static double dminarg1,dminarg2;
#define DMIN(a,b) (dminarg1=(a),dminarg2=(b),(dminarg1) < (dminarg2) ?\
        (dminarg1) : (dminarg2))

static double maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))

static double minarg1,minarg2;
#define FMIN(a,b) (minarg1=(a),minarg2=(b),(minarg1) < (minarg2) ?\
        (minarg1) : (minarg2))

static long lmaxarg1,lmaxarg2;
#define LMAX(a,b) (lmaxarg1=(a),lmaxarg2=(b),(lmaxarg1) > (lmaxarg2) ?\
        (lmaxarg1) : (lmaxarg2))

static long lminarg1,lminarg2;
#define LMIN(a,b) (lminarg1=(a),lminarg2=(b),(lminarg1) < (lminarg2) ?\
        (lminarg1) : (lminarg2))

static int imaxarg1,imaxarg2;
#define IMAX(a,b) (imaxarg1=(a),imaxarg2=(b),(imaxarg1) > (imaxarg2) ?\
        (imaxarg1) : (imaxarg2))

static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

void nrerror(char error_text[]);
double *vector(long nl, long nh);
int *ivector(long nl, long nh);
unsigned char *cvector(long nl, long nh);
unsigned long *lvector(long nl, long nh);
double *dvector(long nl, long nh);
double **matrix(long nrl, long nrh, long ncl, long nch);
double **dmatrix(long nrl, long nrh, long ncl, long nch);
int **imatrix(long nrl, long nrh, long ncl, long nch);
double **submatrix(double **a, long oldrl, long oldrh, long oldcl, long oldch,
	long newrl, long newcl);
double **convert_matrix(double *a, long nrl, long nrh, long ncl, long nch);
double ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh);
void free_vector(double *v, long nl, long nh);
void free_ivector(int *v, long nl, long nh);
void free_cvector(unsigned char *v, long nl, long nh);
void free_lvector(unsigned long *v, long nl, long nh);
void free_dvector(double *v, long nl, long nh);
void free_matrix(double **m, long nrl, long nrh, long ncl, long nch);
void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);
void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch);
void free_submatrix(double **b, long nrl, long nrh, long ncl, long nch);
void free_convert_matrix(double **b, long nrl, long nrh, long ncl, long nch);
void free_f3tensor(double ***t, long nrl, long nrh, long ncl, long nch,
	long ndl, long ndh);

#endif /* _NR_UTILS_H_ */

void fit(double x[], double y[], int ndata, double sig[], int mwt, double *a,
	double *b, double *siga, double *sigb, double *chi2, double *q)
{
	//double gammq(double a, double x);
	int i;
	double wt,t,sxoss,sx=0.0,sy=0.0,st2=0.0,ss,sigdat;

	*b=0.0;
	if (mwt) {
		ss=0.0;
		for (i=1;i<=ndata;i++) {
			wt=1.0/SQR(sig[i]);
			ss += wt;
			sx += x[i]*wt;
			sy += y[i]*wt;
		}
	} else {
		for (i=1;i<=ndata;i++) {
			sx += x[i];
			sy += y[i];
		}
		ss=ndata;
	}
	sxoss=sx/ss;
	if (mwt) {
		for (i=1;i<=ndata;i++) {
			t=(x[i]-sxoss)/sig[i];
			st2 += t*t;
			*b += t*y[i]/sig[i];
		}
	} else {
		for (i=1;i<=ndata;i++) {
			t=x[i]-sxoss;
			st2 += t*t;
			*b += t*y[i];
		}
	}
	*b /= st2;
	*a=(sy-sx*(*b))/ss;
	*siga=sqrt((1.0+sx*sx/(ss*st2))/ss);
	*sigb=sqrt(1.0/st2);
	*chi2=0.0;
	if (mwt == 0) {
		for (i=1;i<=ndata;i++)
			*chi2 += SQR(y[i]-(*a)-(*b)*x[i]);
		*q=1.0;
		sigdat=sqrt((*chi2)/(ndata-2));
		*siga *= sigdat;
		*sigb *= sigdat;
	} else {
		for (i=1;i<=ndata;i++)
			*chi2 += SQR((y[i]-(*a)-(*b)*x[i])/sig[i]);
		//*q=gammq(0.5*(ndata-2),0.5*(*chi2));
	}
}
#undef NRANSI
