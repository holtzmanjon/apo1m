#include <conio.h>
#include <stdio.h>
#include "io.h"
#include "pcx.h"
/****************************************************************************/
/*                                                                      */
/*      Module:         ocs.cpp                                         */
/*                                                                      */
/*      Purpose:        Observatory Control System functions            */
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
/*
  The azimuth is measured eastward from the north.  So, North is 00, East is
  900, South is 1800 and West is 2700.

  The DCS-110 and DCS-220 are wired such that "counter-clockwise"
  is increasing azimuth and "clockwise" is decreasing azimuth.

  The software will assume that increasing steps on the encoder is
  increasing azimuth and decreasing move steps on the encoder is
  decreasing azimuth.

  There are 2 switches in the setup program to toggle dome geartrain and
  encoder count direction.

  PC34 bits are as follows:
		bl4 - shutter power
		bl5 - shutter close (normal)

		bl6 - dome power
		bl7 - dome CCW (normal)
*/

#include <stdio.h>
#include <dos.h>
#include <math.h>
#include <iostream.h>

#include "ocs.h"
#include "globals.h"
#include "systimer.h"

#ifndef no_hardware
#include "pcx.h"
#endif

#include "tech80.h"
#include "status.h"
#include "shutdown.h"
#include "evtimer.h"
#include "sounds.h"
#include "systimer.h"
#include "uts10.h"
#include "tracking.h"
#include "slamac.h"
#include "slalib.h"
#include "io.h"
#include "tcs.h"

int dome_wrap;
long old_enc;

//------------------------------------------------------------------
//      Name.......:    ocs_calc_dome_counts
//
//  Purpose....:        calculate the encoder steps to move the dome
//
//      Input......:    new_azimuth     - new target azimuth
//
//      Output.....:    none (modifies module global variables)
//
//------------------------------------------------------------------
void ocs_calc_dome_counts(double new_azimuth)
{
  if ((!G->dome_initialized) || (sysGlobal->enclosure_type != 2)) {
    og_move_counts = 0L;
    og_dome_done = TRUE;
    return;
  }

  double curr_az = ocs_return_dome_azimuth();
  double delta_degs = (new_azimuth - curr_az) * DD2R;
  delta_degs = DR2D * slaDrange(delta_degs);

  sprintf(outbuf,"Current dome azimuth = %.4f\r\n"
		"New azimuth          = %.4f\r\n"
		"Delta degrees        = %.4f\r\n",
		curr_az,new_azimuth,delta_degs);
  writeline(outbuf,1);

  if (delta_degs == 0) {
    og_move_counts = 0L;
    return;
  }

  // calculate the number of counts to move
  if (sysGlobal->dome_use_lookup_table)
    og_move_counts = calculate_lookup_move_counts(new_azimuth, delta_degs);
  else
    og_move_counts = (long)(delta_degs * sysGlobal->dome_counts_degree);
    
  /*
  The equations to calculate the move counts depend on the counts
  being positive in the increasing azimuth direction.  Use the
  encoder count geartrain to reverse the values if necessary.
  */
  og_dome_inc_az = (og_move_counts >= 0L);
    
  if (!sysGlobal->dome_encoder_normal) og_move_counts = -og_move_counts;

  // set encoder direction flag
  og_enc_pos_dir = (og_move_counts >= 0L);

  // get the current dome encoder position
  og_starting_counts = ocs_return_dome_encoder();

  // calculate the ending encoder position
  og_new_lookup_count = og_starting_counts + og_move_counts;

//  if (og_new_lookup_count < 0)
//    og_new_lookup_count += max_tech80_encoder_reading;
//  else if (og_new_lookup_count > max_tech80_encoder_reading)
//    og_new_lookup_count -= max_tech80_encoder_reading;

  sprintf(outbuf,"Move counts (adj)     = %ld\r\n"
		"Starting encoder value =%ld\r\n"
		"Ending encoder value (adj) = %ld\r\n",
		og_move_counts,og_starting_counts,og_new_lookup_count);
  writeline(outbuf,1);
  writeline(outbuf,3);

}

//------------------------------------------------------------------
//      Name.......:    ocs_calibrate_dome_steps
//
//  Purpose....:        calibrate the dome steps-per-degree
//
//      Input......:    none
//
//      Output.....:    error code
//                                                              newValue - new steps-per-degree
//
//------------------------------------------------------------------
unsigned ocs_calibrate_dome_steps(double &newValue)
{
#ifdef no_hardware
  newValue = sysGlobal->dome_counts_degree;
  return TCSERR_OK;
#else
  long count;
  BOOL is_backwards;
  unsigned status;

  if (sysGlobal->enclosure_type != 2)
  return TCSERR_NA;
    
  // first, home the dome and clear the encoder
  status = ocs_home_dome(TRUE, TRUE);
  if (status) {
    G->dome_initialized = FALSE;
    return status;
  }
  delay(2000);
    
  // now, home the dome without clearing the encoder
  status = ocs_home_dome(TRUE, FALSE, is_backwards);
  if (status) {
    G->dome_initialized = FALSE;
    return status;
  }
    
  // take the encoder reading and calculate the steps/degree
  count = ocs_return_dome_encoder();
#ifdef tech80
  if (is_backwards) count = max_tech80_encoder_reading - count;
#endif
    
  sprintf(outbuf,"Count             = %ld\n", count);
  writeline(outbuf,0);
  sprintf(outbuf,"Old counts/degree = %10.4f\n", 
  sysGlobal->dome_counts_degree);
  writeline(outbuf,0);
  newValue = (double)count / 360.0;
  sprintf(outbuf,"New counts/degree = %10.4f\n", newValue);
  writeline(outbuf,0);
  sprintf(outbuf, "Do you want to change to the new value (Y or N)? ");
  writeline(outbuf,0);
  char line[80];
  char ans;
  getline(line,sizeof(line));
  sscanf(line,"%c",&ans);
    
  if (ans == 'Y' || ans == 'y') {
    // Reset the global variables and update the configuration file
    sysGlobal->dome_counts_degree = newValue;
    writesysscf(sysGlobal);
  }
    
  init_dome_encoder();
  dome_wrap = 0;
  old_enc = 0;
  return TCSERR_OK;
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_dome_finished
//
//  Purpose....:        stop the dome when we reach the new azimuth
//
//      Input......:    none
//
//      Output.....:    TRUE - we are at the new azimuth
//
//------------------------------------------------------------------
unsigned ocs_dome_finished(BOOL &finished)
{
#ifdef no_hardware
  finished = TRUE;
  return TCSERR_OK;
#else
  long d;

  if (og_dome_done) {
    finished = TRUE;
    return TCSERR_OK;
  }

  if (sysGlobal->enclosure_type != 2) {
    finished = TRUE;
    return TCSERR_NA;
  }

  if (!G->dome_initialized) {
    finished = TRUE;
    return TCSERR_DOMENOTINIT;
  }

  // update the tracking rates
  update_tracking_rates();

  // if the timer expired, the encoder is probably broken or the dome is
  // not moving.  Return TRUE and de-initialize the dome
  if (og_dome_timer.Expired()) {
    error_sound;
    sprintf(outbuf,
      "\r\nThe dome move time has exceeded the maximum time allowed\r\n"
      "for the move.  Check the dome encoder and/or the dome motor "
      "itself.\r\nAll dome moves are now disabled.\r\n");
    writeline(outbuf,0);
    G->dome_initialized = FALSE;
    finished = TRUE;
    pc34_send_commands("bh6; bh7;");
    return TCSERR_DOMEMOVE;
  }

  // get our current encoder position - function
  //  sets global variable og_current_counts
  G->dome_azimuth = ocs_return_dome_azimuth();
  if (og_enc_pos_dir)
    d = og_current_counts - og_new_lookup_count;
  else
    d = og_new_lookup_count - og_current_counts;
  
//sprintf(outbuf,"curr_counts: %ld  new_lookup_count: %ld pos_dir %d %ld",og_current_counts,og_new_lookup_count,og_enc_pos_dir,d);
//writeline(outbuf,1);

  if (labs(d) < og_max_move_counts) og_dome_done = (d >= 0);
    
  // turn off the power?
  if (og_dome_done) pc34_send_commands("bh6; bh7;");
    
  finished = og_dome_done;
    
  return TCSERR_OK;
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_home_dome
//
//  Purpose....:        put the dome in its home position
//
//      Input......:    force_move      
//                     - TRUE: force the move even if we are at home
//                      reset_pos 
//                     - TRUE: reset the encoder position to 0
//
//      Output.....:    backwards is TRUE if encoder is moving backwards
//                                                              error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned ocs_home_dome(BOOL force_move, BOOL reset_pos, BOOL &backwards)
#else
unsigned ocs_home_dome(BOOL, BOOL, BOOL&)
#endif
{
#ifdef no_hardware
   G->dome_initialized = TRUE;
   return TCSERR_OK;
#else
   if (sysGlobal->enclosure_type != 2)
     return TCSERR_NA;

     // calculate the maximum move counts
     og_max_move_counts = (long)(181.0 * sysGlobal->dome_counts_degree);
     sprintf(outbuf,"Max move counts = %d",og_max_move_counts);
     writeline(outbuf,1);

     sprintf(outbuf,"Beginning dome home sequnce...");
     writeline(outbuf,1);
     G->dome_initialized = FALSE;
     if (force_move)
       sprintf(outbuf,"Force move.");
     else
       sprintf(outbuf,"No force move.");
     writeline(outbuf,1);

     BOOL dome_home = ocs_is_dome_home();
     sprintf(outbuf,"Dome home = %d",(int)dome_home);
     writeline(outbuf,1);
     if ((!force_move) && (dome_home)) {
       sprintf(outbuf,"Dome is already home.");
       writeline(outbuf,0);
       backwards = FALSE;
       if (reset_pos) {
          init_dome_encoder();
          dome_wrap = 0;
          old_enc = 0;
       }
       autoGlobal->dome_azimuth = sysGlobal->dome_home_azimuth;
       sprintf(outbuf,"Dome azimuth = %8.4f", autoGlobal->dome_azimuth);
       writeline(outbuf,0);
       writetoccscf(autoGlobal);
       G->dome_initialized = TRUE;
       return TCSERR_OK;
     }

     // set the direction
     if (sysGlobal->dome_geartrain_normal)
       pc34_send_commands("bl7;");
     else
       pc34_send_commands("bh7;");
     delay(500);

     // apply power
     pc34_send_commands("bl6;");

     // start the dome timer
     og_dome_timer.NewTimerSecs(sysGlobal->dome_max_init_time);

     // wait a bit to check encoder
     if (force_move)
       delay(5000);    // wait until off home sensor
     else
       delay(10);
     backwards = (ocs_return_dome_encoder() > 1000000L);

     unsigned status = TCSERR_OK;
     int state = 0;
     BOOL cont = TRUE;
     EventTimer home_timer;
     static int nfail = 0;

     if (nfail>10) {
       sprintf(outbuf,"Initialization has failed too many times. Giving up until tocc is restarted.");
       writeline(outbuf,0);
       return TCSERR_DOMEHOME;
     }

     sprintf(outbuf,"Initializing dome. May take up to 90 seconds");
     writeline(outbuf,0);
     while (cont)
     {
	// update tracking rates
//      update_tracking_rates();
	update_status(0);
	update_display();

	// check for init timeout
	if (og_dome_timer.Expired()) {
	  pc34_send_commands("bh6; bh7;");
	  error_sound;
	  nfail++;
	  sprintf(outbuf,"Dome initialization time exceeded.");
	  writeline(outbuf,0);
	  return TCSERR_DOMEHOME;
	}

	switch (state) {
		case 0: 
                if (ocs_is_dome_home()) {
		  state++;
		  home_timer.NewTimer(1); // 3 clock tics or 165ms
		} else {
		  // sprintf(outbuf,"Dome init time remaining... %d",
		  //  og_dome_timer.RemainingTimeInSecs());
		  // writeline(outbuf,1);
		  // delay(300);
/*
		  if (!check_priority(status)) {
		    return status;
		  }
                  // Read the encoder so we can keep track of encoder wraps
                  long enc;
                  enc = ocs_return_dome_encoder();
*/
		}
		break;

		case 1: 
		if (!ocs_is_dome_home()) {
	   sprintf(outbuf,"home detriggered too fast");
	   writeline(outbuf,1);
		  state--;
		} else
		  if (home_timer.Expired()) {
		    pc34_send_commands("bh6; bh7;");
		    cont = FALSE;
	    	    }
		    break;
		  }
        }

	if (reset_pos) {
          init_dome_encoder();
          dome_wrap = 0;
          old_enc = 0;
        }
	autoGlobal->dome_azimuth = sysGlobal->dome_home_azimuth;
        sprintf(outbuf, "\nDome azimuth = %.4f\n", autoGlobal->dome_azimuth);
        writeline(outbuf,0);
        writetoccscf(autoGlobal);
        G->dome_initialized = TRUE;
        nfail = 0;
        sprintf(outbuf,"Dome initialized");
        writelog(outbuf,4);

        return status;
#endif
}

unsigned ocs_home_dome_force(double az)
{
	sysGlobal->dome_home_azimuth = az;
        og_max_move_counts = (long)(181.0 * sysGlobal->dome_counts_degree);
        init_dome_encoder();
        dome_wrap = 0;
        old_enc = 0;

	autoGlobal->dome_azimuth = sysGlobal->dome_home_azimuth;
        sprintf(outbuf, "\nDome azimuth = %.4f\n", autoGlobal->dome_azimuth);
        writeline(outbuf,0);
        writetoccscf(autoGlobal);
        G->dome_initialized = TRUE;
        return 0;
}
unsigned ocs_home_dome(BOOL force_move, BOOL reset_pos)
{
  BOOL b;
  return ocs_home_dome(force_move, reset_pos, b);
}

//------------------------------------------------------------------
//      Name.......:    ocs_return_dome_azimuth
//
//  Purpose....:        return the current dome position
//
//      Input......:    none
//
//      Output.....:    dome azimuth
//
//------------------------------------------------------------------
double ocs_return_dome_azimuth()
{
  double pos;

  og_current_counts = ocs_return_dome_encoder();
  pos = (og_current_counts/sysGlobal->dome_counts_degree)
	       + sysGlobal->dome_home_azimuth;
  // take care of 2*pi ambiguity
  pos = slaDranrm(pos*DD2R) * DR2D;
sprintf(outbuf," dome encoder: %ld,  wrap: %d azimuth: %lf old: %ld",og_current_counts,dome_wrap,pos,old_enc);
//writeline(outbuf,1);
return(pos);
}

//------------------------------------------------------------------
//      Name.......:    ocs_return_dome_encoder
//
//  Purpose....:        return the current dome encoder position
//
//      Input......:    none
//
//      Output.....:    encoder value
//
//------------------------------------------------------------------
long ocs_return_dome_encoder()
{
  long enc;

  if (sysGlobal->enclosure_type == 2) {
    enc = read_dome_encoder();
    if ( enc - old_enc < -30000 ) dome_wrap++;
    else if (enc - old_enc  > 30000 ) dome_wrap--;
    old_enc = enc;
    enc = enc + (long)dome_wrap*65536;
    return (enc);
  } else
    return (0L);
}

//------------------------------------------------------------------
//      Name.......:    ocs_rotate_dome
//
//  Purpose....:        move the dome to a new azimuth
//
//      Input......:    new_azimuth - new azimuth to move to
//
//      Output.....:    error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned ocs_rotate_dome(double new_azimuth)
#else
unsigned ocs_rotate_dome(double)
#endif
{
#ifdef no_hardware
  return TCSERR_OK;
#else
  unsigned status = 0;
  BOOL finished = FALSE;

  if (!G->dome_initialized) return TCSERR_DOMENOTINIT;

  if (sysGlobal->enclosure_type != 2) return TCSERR_NA;

  // calculate the desired final encoder position
  sprintf(outbuf,"calc dome counts");
  writeline(outbuf,3);
  ocs_calc_dome_counts(new_azimuth);
  sprintf(outbuf,"done calc dome counts");
  writeline(outbuf,3);

  if (!og_move_counts) return TCSERR_OK;

  // start moving the dome in the right direction
  sprintf(outbuf,"start dome move");
  writeline(outbuf,3);
  ocs_start_dome_move();
  sprintf(outbuf,"done start dome move");
  writeline(outbuf,3);

  // wait for the move to end
  do {
    if (!check_priority(status)) {
      G->dome_initialized = FALSE;
      return status;
    }
    update_status(0);
    update_display();
    status = ocs_dome_finished(finished);
    if (status) return status;
  } while (!finished);

  // save the azimuth
  autoGlobal->dome_azimuth = new_azimuth;
  writetoccscf(autoGlobal);

  return TCSERR_OK;
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_slave_dome
//
//  Purpose....:        slave the dome to the telescope
//
//      Input......:    none
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_slave_dome()
{
  unsigned status;

  if (sysGlobal->enclosure_type != 2) return TCSERR_NA;

  if (!G->dome_initialized) {
	status = ocs_home_dome(FALSE, TRUE);
	if (status) return status;
  }

  autoGlobal->dome_slaved = TRUE;
  writetoccscf(autoGlobal);
  return TCSERR_OK;
}

//------------------------------------------------------------------
//      Name.......:    ocs_start_dome_move
//
//  Purpose....:        starts the dome moving to the new azimuth
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void ocs_start_dome_move()
{
#ifdef no_hardware
  return;
#else
// execute the move
		// BL7 = CCW (inc az), BH7 = CW (dec az)
  if ((!og_move_counts) || (!G->dome_initialized) ||
	(sysGlobal->enclosure_type != 2)) {
    og_dome_done = TRUE;
    return;
  }

  if (og_dome_inc_az) {
	if (sysGlobal->dome_geartrain_normal)
		pc34_send_commands("bl7;");
	else
		pc34_send_commands("bh7;");
  } else {
	if (sysGlobal->dome_geartrain_normal)
		pc34_send_commands("bh7;");
	else
		pc34_send_commands("bl7;");
  }
  Delay(500);

  // apply power
  pc34_send_commands("bl6;");
  og_dome_done = FALSE;

  // start the dome timer
  og_dome_timer.NewTimerSecs(sysGlobal->dome_max_init_time * 0.8);
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_unslave_dome
//
//  Purpose....:        unslave the dome from the telescope
//
//      Input......:    none
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_unslave_dome()
{
  if (sysGlobal->enclosure_type != 2) return TCSERR_NA;

  autoGlobal->dome_slaved = FALSE;
  writetoccscf(autoGlobal);
  return TCSERR_OK;
}

//------------------------------------------------------------------
//      Name.......:    ocs_emergency_stop
//
//  Purpose....:        stop all axes, etc.
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void ocs_emergency_stop()
{
  if (ocs_installed()) {
    //error_sound();
    sprintf(outbuf,"OCS emergency stop!\n");
    writeline(outbuf,0);

#ifndef no_hardware
    pc34_send_commands("aa st;");
    // stop dome/shutter
    pc34_send_commands("bh4; bh5; bh6; bh7;");  
#endif

    // G->dome_initialized = FALSE;
  }
}

//------------------------------------------------------------------
//      Name.......:    ocs_get_date_time
//
//  Purpose....:        return the WWV date & time
//
//      Input......:    none
//
//      Output.....:    date and time of either the WWV clock or the system
//                                                              clock.
//
//                                                              error code
//
//------------------------------------------------------------------
unsigned ocs_get_date_time(struct date &dateRec, struct mytime &timeRec)
{
  unsigned status;

  mygettime(&dateRec, &timeRec);
//
// #ifdef no_hardware
//              // getdate(&dateRec);
//              mygettime(&dateRec, &timeRec);
// #else
//              if (!sysGlobal->wwv_type)
//                      return TCSERR_NA;
//              status = uts10_get_time(dateRec, timeRec);
// #endif
  return status;
}

//------------------------------------------------------------------
//      Name.......:    ocs_get_set_date_time
//
//  Purpose....:        get the WWV date & time and set the PC clock to match
//
//      Input......:    none
//
//      Output.....:    error code if the WWV is not installed
//
//------------------------------------------------------------------
unsigned ocs_get_set_date_time()
{
  struct date dateRec;
  struct time timeRec;
  struct mytime mytimeRec;

  unsigned status = ocs_get_date_time(dateRec, mytimeRec);
  if (!status) {
    timeRec.ti_hour = mytimeRec.ti_hour;
    timeRec.ti_min = mytimeRec.ti_min;
    timeRec.ti_sec = mytimeRec.ti_sec;
    timeRec.ti_hund = mytimeRec.ti_hund;
    setdate(&dateRec);
    settime(&timeRec);
  }

  return status;
}

//------------------------------------------------------------------
//      Name.......:    ocs_get_last_wwv_sync
//
//  Purpose....:        returns the minutes since the last WWV signal lock
//
//      Input......:    none
//
//      Output.....:    minutes since last sync
//                                                              error code
//
//------------------------------------------------------------------
unsigned ocs_get_last_wwv_sync(unsigned &last_sync)
{
  if (!sysGlobal->wwv_type)
  return TCSERR_NA;

  return uts10_get_sync(last_sync);
}


//------------------------------------------------------------------
//      Name.......:    ocs_initialize
//
//  Purpose....:        initialize the PC-34 card
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void ocs_initialize()
{
#ifndef no_hardware
  if (!ocs_installed()) return;

  sprintf(outbuf,"Initializing OCS");
  writeline(outbuf,0);

  // disable the OMS interrupts - we use a polling method
  pc34_disable_interrupts();

  // reset the PC38
  pc34_send_commands("aa rs;");

  // make sure that the card is finished initializing
  while (pc34_read_status_reg() & 0x02);
  delay(10);

  pc34_send_commands("bh1; bh2; bh3; bh4; bh5; bh6; bh7;");
  ocs_reset_watchdog();
#endif

  ocs_initialize_lookup_table();

  og_last_dome_az_error = 0.0;
  og_dome_done = TRUE;
  og_move_counts = 0L;

}

//------------------------------------------------------------------
//      Name.......:    ocs_initialize_lookup_table
//
//  Purpose....:        initialize the arrays used for interpolated dome moves
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void ocs_initialize_lookup_table()
{
  if (sysGlobal->enclosure_type != 2) return;

  for (int i = 0; i < 13; i++) d_arr[i] = (30.0 * i) - 180.0;

  if (sysGlobal->dome_use_lookup_table)
    for (i = 0; i < 12; i++) {
      dif_arr[i] = sysGlobal->dome_lookup_table[i+1] -
			    sysGlobal->dome_lookup_table[i];
      if (labs(dif_arr[i]) > (long)(450.0 * sysGlobal->dome_counts_degree))
        dif_arr[i] -= max_tech80_encoder_reading;
    }
}

//------------------------------------------------------------------
//      Name.......:    ocs_installed
//
//  Purpose....:        determine whether or not the OCS is installed
//
//      Input......:    none
//
//      Output.....:    TRUE if the OCS is installed
//
//------------------------------------------------------------------
BOOL ocs_installed()
{
  return (sysGlobal->ups_installed ||
          sysGlobal->rain_detector_installed ||
	  sysGlobal->watchdog_installed ||
	  sysGlobal->enclosure_type);
}

//------------------------------------------------------------------
//      Name.......:    ocs_is_dome_home
//
//  Purpose....:        return dome home status
//
//      Input......:    none
//
//      Output.....:    TRUE if the dome is home
//
//------------------------------------------------------------------
BOOL ocs_is_dome_home()
{
#ifdef no_hardware
  return TRUE;
#else
  if (!ocs_installed()) return FALSE;

  //unsigned rval = ocs_read_inputs();
  unsigned rval = pcx_read_inputs(PC34);
  rval &= OCS_DOME_HOME_BIT;
  return (rval) ? TRUE : FALSE;
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_kill_ups
//
//  Purpose....:        kill the UPS
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void ocs_kill_ups()
{
#ifndef no_hardware
  if (ocs_installed()) pc34_send_commands("bl1;");
#endif
}

unsigned ocs_open_shutters()
{
  EventTimer upperTimer, lowerTimer, shutterTimer;
  int timer, stop;

  // Clear shutdown flag
  if (G->shutdown_state) {
    writeline("Shutdown flags set. Cannot open dome",0);
    return(TCSERR_WEATHERSHUT);
  }

  // start upper opening, wait 10 secs before lower
  delay(2000);
  sprintf(outbuf,"Starting upper opening");
  writeline(outbuf,0);
  if (sysGlobal->shutter_gear_normal)
    pc34_send_commands("bh5;");
  else
    pc34_send_commands("bl5;");
  delay(500);
  pc34_send_commands("bl4;");
  timer = (int)sysGlobal->lower_shutter_delay;
  upperTimer.NewTimerSecs(timer);
  shutterTimer.NewTimerSecs(10);
  do {
    // update the tracking rates
    update_tracking_rates();
    update_status(0);
    update_display();
  } while (!shutterTimer.Expired());

  // Start lower opening
  sprintf(outbuf,"Starting lower opening");
  writeline(outbuf,0);
  pc34_send_commands("bl3;");
  delay(2000);
  pc34_send_commands("bl2;");
  timer = (int)sysGlobal->lower_shutter_delay;
  lowerTimer.NewTimerSecs(timer);

  sprintf(outbuf,"Time to open = %d seconds",timer);
  writeline(outbuf,0);

  stop=1;
  do {
    // update the tracking rates
    update_tracking_rates();
    update_status(0);
    update_display();
    if (upperTimer.Expired() && stop==1) {
      sprintf(outbuf,"Powering off upper");
      writeline(outbuf,0);
      stop = 0;
      pc34_send_commands("bh4; bh5;");
    }
  } while (!lowerTimer.Expired());

  // power off upper
  sprintf(outbuf,"Powering off upper");
  writeline(outbuf,0);
  pc34_send_commands("bh4; bh5;");
  delay(2000);
  // power off lower
  sprintf(outbuf,"Powering off lower");
  writeline(outbuf,0);
  pc34_send_commands("bh2;");

  autoGlobal->shutter_opened = TRUE;
  autoGlobal->lower_shutter_opened = TRUE;
  G->dome_open = TRUE;
  G->lower_dome_open = TRUE;
  sprintf(outbuf,"Upper and lower dome opened");
  writelog(outbuf,1);
  writetoccscf(autoGlobal);
  return TCSERR_OK;
}

unsigned ocs_close_shutters()
{
  EventTimer upperTimer, lowerTimer, shutterTimer;
  int timer;

  // Start lower closing
  delay(2000);
  sprintf(outbuf,"Powering on lower");
  writeline(outbuf,0);
  pc34_send_commands("bh3;");
  delay(2000);
  pc34_send_commands("bl2;");
  timer = (int)sysGlobal->lower_shutter_delay + 30;
  sprintf(outbuf,"lower timer: %d",timer);
  writeline(outbuf,0);
  lowerTimer.NewTimerSecs(timer);

  // start upper closing, but stop 10 seconds short
  timer = (int)sysGlobal->shutter_delay - 18;
  upperTimer.NewTimerSecs(timer);

  sprintf(outbuf,"Time to close = %d seconds",timer);
  writeline(outbuf,0);
  if (sysGlobal->shutter_gear_normal)
    pc34_send_commands("bl5;");
  else
    pc34_send_commands("bh5;");
  delay(2000);
  sprintf(outbuf,"Powering on upper");
  writeline(outbuf,0);
  pc34_send_commands("bl4;");
  do {
    // update the tracking rates
    update_tracking_rates();
    update_status(0);
    update_display();
  } while (!upperTimer.Expired());
  sprintf(outbuf,"Powering off upper");
  writeline(outbuf,0);
  pc34_send_commands("bh4; bh5;");

  // Now wait for lower to finish
  do {
    // update the tracking rates
    update_tracking_rates();
    update_status(0);
    update_display();
  } while (!lowerTimer.Expired());
  sprintf(outbuf,"Powering off lower");
  writeline(outbuf,0);
  pc34_send_commands("bh2;");

  // finish up upper
  delay(2000);
  if (sysGlobal->shutter_gear_normal)
    pc34_send_commands("bl5;");
  else
    pc34_send_commands("bh5;");
  delay(500);
  sprintf(outbuf,"Powering on upper");
  writeline(outbuf,0);
  pc34_send_commands("bl4;");
  timer = 25;
  upperTimer.NewTimerSecs(timer);

  do {
    // update the tracking rates
    update_tracking_rates();
    update_status(0);
    update_display();
  } while (!upperTimer.Expired());
  // power off upper
  sprintf(outbuf,"Powering off upper");
  writeline(outbuf,0);
  pc34_send_commands("bh4; bh5;");

  autoGlobal->shutter_opened = FALSE;
  autoGlobal->lower_shutter_opened = FALSE;
  G->dome_open = FALSE;
  G->lower_dome_open = FALSE;
  sprintf(outbuf,"Upper and lower dome closed");
  writelog(outbuf,1);
  writetoccscf(autoGlobal);
  return TCSERR_OK;

}
//------------------------------------------------------------------
//      Name.......:    ocs_open_shutter
//
//  Purpose....:        open the shutter/roll-off roof
//
//      Input......:    none
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_open_shutter(int opentime)
{
#ifdef no_hardware
  return TCSERR_OK;
#else
  EventTimer shutterTimer;
  int timer;
  unsigned status;

  // Clear shutdown flag
  if (G->shutdown_state) {
    writeline("Shutdown flags set. Cannot open dome",0);
    return(TCSERR_WEATHERSHUT);
  }

  timer = opentime;
  if (opentime == 0) timer = (int)sysGlobal->shutter_delay;

  // Wait a couple of secs in case we just commanded a dome motor
  delay(2000);

  switch (sysGlobal->enclosure_type) {
    case 0: return TCSERR_NA;

    case 2: if (!sysGlobal->slip_rings_installed) {
      status = ocs_home_dome(FALSE, TRUE);
      if (status) return status;
    }

    // fall into case below
    case 1: if (sysGlobal->shutter_gear_normal)
	      pc34_send_commands("bh5;");
	    else
	      pc34_send_commands("bl5;");
	    delay(500);
	    pc34_send_commands("bl4;");

	    sprintf(outbuf,"Time to open = %d seconds", timer);
	    writeline(outbuf,0);

	    // start the timer
	    shutterTimer.NewTimerSecs(timer);

	    do {
	      // sprintf(outbuf,"Remaining time = %d",
	      //    shutterTimer.RemainingTimeInSecs());
	      // writeline(outbuf,1);
	      // delay(300);

	      if (!check_priority(status)) return status;

	      // update the tracking rates
	      update_tracking_rates();
	      update_status(0);
	      update_display();
	    } while (!shutterTimer.Expired());
	    break;
  }

  pc34_send_commands("bh4; bh5;");
  delay(2000);
  if (opentime == 0) {
    autoGlobal->shutter_opened = TRUE;
    G->dome_open = TRUE;
    writetoccscf(autoGlobal);
    sprintf(outbuf,"Upper dome opened");
    writelog(outbuf,1);
  } else {
    G->dome_part_open = TRUE;
    sprintf(outbuf,"Upper dome partially opened");
    writelog(outbuf,1);
  }
  return TCSERR_OK;
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_close_shutter
//
//  Purpose....:        close the shutter/roll-off roof
//
//      Input......:    none
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_close_shutter()
{
#ifdef no_hardware
  return TCSERR_OK;
#else
  EventTimer shutterTimer;
  unsigned status;

  // Wait a couple of secs in case we just commanded a dome motor
  delay(2000);

  switch (sysGlobal->enclosure_type) {
    case 0: return TCSERR_NA;

    case 2: if (!sysGlobal->slip_rings_installed) {
              status = ocs_home_dome(FALSE, TRUE);
              if (status) return status;
            }
    // fall into case below
    
    case 1: if (sysGlobal->shutter_gear_normal)
              pc34_send_commands("bl5;");
            else
              pc34_send_commands("bh5;");
            delay(2000);
            pc34_send_commands("bl4;");
    
            sprintf(outbuf,"Time to close = %d seconds",
                    (int)sysGlobal->shutter_delay);
            writeline(outbuf,0);
    
            // start the timer
            shutterTimer.NewTimerSecs(sysGlobal->shutter_delay);
    
            do {
              // sprintf(outbuf,"Remaining time = %d",
              //   shutterTimer.RemainingTimeInSecs());
              // writeline(outbuf,1);
              // delay(300);
    
              // update the tracking rates
              update_tracking_rates();
              update_status(0);
              update_display();
            } while (!shutterTimer.Expired());
            break;
  }
    
  pc34_send_commands("bh4; bh5;");
  delay(2000);
  autoGlobal->shutter_opened = FALSE;
  G->dome_open = FALSE;
  G->dome_part_open = FALSE;
  writetoccscf(autoGlobal);
  sprintf(outbuf,"Upper dome closed");
  writelog(outbuf,1);
  return TCSERR_OK;
#endif
}


//------------------------------------------------------------------
//      Name.......:    ocs_open_lower_shutter
//
//  Purpose....:        open the lower shutter
//
//      Input......:    none
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_open_lower_shutter(int opentime)
	{
		#ifdef no_hardware
		return TCSERR_OK;
		#else
		EventTimer shutterTimer;
		unsigned status;
                int timer;

		if (!G->dome_open) {
		  writeline(
	    "Upper dome shutter is closed. Cannot open lower dome shutter.",0);
		  return TCSERR_SHUTTERPOS;
		}
                timer = opentime;
                if (opentime == 0) timer = (int)sysGlobal->lower_shutter_delay;

	// Wait a couple of secs in case we just commanded a dome motor
		delay(2000);

		pc34_send_commands("bl3;");
		delay(2000);
		pc34_send_commands("bl2;");

		sprintf(outbuf,"Time to open = %d seconds",timer);
		writeline(outbuf,0);

		// start the timer
		shutterTimer.NewTimerSecs(timer);

		do {
			// update the tracking rates
			update_tracking_rates();
			update_status(0);
			update_display();
		} while (!shutterTimer.Expired());

		pc34_send_commands("bh2;");
		delay(2000);
                if ( opentime == 0) {
		  autoGlobal->lower_shutter_opened = TRUE;
		  G->lower_dome_open = TRUE;
		  writetoccscf(autoGlobal);
		  sprintf(outbuf,"Lower dome opened");
		  writelog(outbuf,2);
		} else {
		  G->lower_dome_part_open = TRUE;
		  sprintf(outbuf,"Lower dome partially opened");
		  writelog(outbuf,2);
                }
		return TCSERR_OK;
		#endif
	}

//------------------------------------------------------------------
//      Name.......:    ocs_close_lower_shutter
//
//  Purpose....:        close the lower shutter
//
//      Input......:    none
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_close_lower_shutter()
	{
		#ifdef no_hardware
		return TCSERR_OK;
		#else
		EventTimer shutterTimer;
		unsigned status;

		if (!G->dome_open) {
		  writeline(
	    "Upper dome shutter is closed. Cannot close lower dome shutter.",0);
		  return TCSERR_SHUTTERPOS;
		}
	// Wait a couple of secs in case we just commanded a dome motor
		delay(2000);

		pc34_send_commands("bh3;");
		delay(2000);
		pc34_send_commands("bl2;");

		sprintf(outbuf,"Time to close = %d seconds", sysGlobal->lower_shutter_delay+30);
		writeline(outbuf,0);

		// start the timer
		shutterTimer.NewTimerSecs(sysGlobal->lower_shutter_delay+30);

		do {
			// update the tracking rates
			update_tracking_rates();
			update_status(0);
			update_display();
		} while (!shutterTimer.Expired());

		pc34_send_commands("bh2;");
		delay(2000);
		autoGlobal->lower_shutter_opened = FALSE;
		G->lower_dome_open = FALSE;
		G->lower_dome_part_open = FALSE;
		writetoccscf(autoGlobal);
		sprintf(outbuf,"Lower dome closed");
		writelog(outbuf,2);
		return TCSERR_OK;
		#endif
	}


//------------------------------------------------------------------
//      Name.......:    ocs_rain_detector_active
//
//  Purpose....:        check the rain detector
//
//      Input......:    none
//
//      Output.....:    TRUE if the detector is active
//
//------------------------------------------------------------------
BOOL ocs_rain_detector_active()
{
#ifdef no_hardware
  return FALSE;
#else
  if (!ocs_installed() || !sysGlobal->rain_detector_installed) return FALSE;

  //unsigned rval = ocs_read_inputs();
sprintf(outbuf,"rain");
writeline(outbuf,1);
  unsigned rval = pcx_read_inputs(PC34);
  rval &= OCS_RAIN_BIT;
  return (rval) ? TRUE : FALSE;
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_read_inputs
//
//  Purpose....:        get user input bits value
//
//      Input......:    none
//
//      Output.....:    value of all user input bits
//
//------------------------------------------------------------------
unsigned ocs_read_inputs()
	{
		#ifdef no_hardware
		return 0;
		#else
		if (!ocs_installed())
			return 0;

		byte v = 0;
		char hexval[3];

	// clear the read register and send the BX command to get the bit value
		pc34_clear_read_register();
		pc34_send_commands("bx");

		// the number comes back as <lf><cr>##<lf><cr>
		while (v != 10)
			v = pcx_read_byte(PC34, 0);

		while (v != 13)
			v = pcx_read_byte(PC34, 0);

		v = pcx_read_byte(PC34, 0);
		while (!v)
			v = pcx_read_byte(PC34, 0);
		hexval[0] = v;

		v = pcx_read_byte(PC34, 0);
		while (!v)
			v = pcx_read_byte(PC34, 0);
		hexval[1] = v;
		hexval[2] = 0;

		v = pcx_read_byte(PC34, 0);
		while (v != 10)
			v = pcx_read_byte(PC34, 0);

		while (v != 13)
			v = pcx_read_byte(PC34, 0);

		// convert hexval to an integer
		char hexdigits[] = "0123456789ABCDEFabcdef";
		unsigned rval = 0;

		for (byte i = 0; i < 2; i++)
			for (v = 0; v < 22; v++)
			  if (hexval[i] == hexdigits[v])
				rval = (16 * rval) + ((v < 16) ? v : v - 6);

		return rval;
		#endif
	}

//------------------------------------------------------------------
//      Name.......:    ocs_reset_watchdog
//
//  Purpose....:        reset the dome shutter watchdog timer
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void ocs_reset_watchdog()
{
#ifndef no_hardware
if (ocs_installed() && sysGlobal->watchdog_installed)
			{
   pc34_send_commands("bl0;");
   delay(200);
   pc34_send_commands("bh0;");
   SysTimer[SYSTMR_WDOG_CHECK].NewTimerSecs(SYSTMR_WDOG_INC);
}
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_ups_status
//
//  Purpose....:        return the status of the UPS
//
//      Input......:    none
//
//      Output.....:    status record
//
//------------------------------------------------------------------
void ocs_ups_status(struct UPSPACK &upsStatus)
{
#ifdef no_hardware
  upsStatus.battery_low = FALSE;
  upsStatus.line_fail = FALSE;
  return;
#else
  if (ocs_installed()) {
sprintf(outbuf,"ups");
writeline(outbuf,1);
    unsigned rval = pcx_read_inputs(PC34);
    //unsigned rval = ocs_read_inputs();
    rval &= OCS_LINE_FAIL_BIT;
    upsStatus.line_fail = (rval) ? TRUE : FALSE;

    rval = pcx_read_inputs(PC34);
    //rval = ocs_read_inputs();
    rval &= OCS_BATT_LOW_BIT;
    upsStatus.battery_low = (rval) ? TRUE : FALSE;
  } else {
    upsStatus.line_fail = FALSE;
    upsStatus.battery_low = FALSE;
  }
#endif
}

//------------------------------------------------------------------
//      Name.......:    ocs_watchdog_active
//
//  Purpose....:        return watchdog timedout status
//
//      Input......:    none
//
//      Output.....:    TRUE is watchdog timer is timedout
//
//------------------------------------------------------------------
BOOL ocs_watchdog_active()
{
#ifdef no_hardware
  return FALSE;
#else
  if (!ocs_installed() || !sysGlobal->watchdog_installed) return FALSE;

    unsigned rval = pcx_read_inputs(PC34);
    //unsigned rval = ocs_read_inputs();
    rval &= OCS_WATCHDOG_BIT;
sprintf(outbuf,"watchdog: %d", rval);
writeline(outbuf,1);
    return (rval) ? TRUE : FALSE;
#endif
}

// local procedure for calculate_lookup_move_counts
long interpolate(double az, BOOL inc_az)
{
  double av1, av2, avdif;

	/*
	We already know the array of differences between each position pair
	in the lookup table (calculated upon initialization with call
	to ocs_initialize_lookup_table().

	What this routine returns is the differenece from where we are
	to the closest known encoder position in the direction of travel.

	For example:

	If az is -80 and we are increasing azimuth, then the two know
	encoder points av1 and av2 are -90 and -60 respectively.  If the
	encoder difference between -90 and -60 (in the lookup table) is
	50 counts, then the encoder difference between -80 and -60
	should be 2/3rds the difference between -90 and -60, or in this
	case, 33 counts.
	*/
		if (az <= -150)
			{
				av1 = -180;
				av2 = -150;
				avdif = dif_arr[0];
			}
		else
			if (az <= -120)
				{
					av1 = -150;
					av2 = -120;
					avdif = dif_arr[1];
				}
		else
			if (az <= -90)
				{
					av1 = -120;
					av2 = -90;
					avdif = dif_arr[2];
				}
		else
			if (az <= -60)
				{
					av1 = -90;
					av2 = -60;
					avdif = dif_arr[3];
				}
		else
			if (az <= -30)
				{
					av1 = -60;
					av2 = -30;
					avdif = dif_arr[4];
				}
		else
			if (az <= 0)
				{
					av1 = -30;
					av2 = 0;
					avdif = dif_arr[5];
				}
		else
			if (az <= 30)
				{
					av1 = 0;
					av2 = 30;
					avdif = dif_arr[6];
				}
		else
			if (az <= 60)
				{
					av1 = 30;
					av2 = 60;
					avdif = dif_arr[7];
				}
		else
			if (az <= 90)
				{
					av1 = 60;
					av2 = 90;
					avdif = dif_arr[8];
				}
		else
			if (az <= 120)
				{
					av1 = 90;
					av2 = 120;
					avdif = dif_arr[9];
				}
		else
			if (az <= 150)
				{
					av1 = 120;
					av2 = 150;
					avdif = dif_arr[10];
				}
		else
			if (az <= 180)
				{
					av1 = 150;
					av2 = 180;
					avdif = dif_arr[11];
				}

		return ((inc_az) ? (long)(avdif * (av2 - az) / (av2 - av1)) :
											 (long)(avdif * (az - av1) / (av2 - av1)));
	}

// local procedure for calculate_lookup_move_counts
long calc_exact_lookup(double az1, double az2, double delta)
{
  // returns exact counts between two known lookup azimuths

  int i = 0;
  long sum = 0L;

  if (az1 == az2) return 0L;

  if (delta > 0) {
    if (az1 > az2) {
      while (d_arr[i] != az2) i++;
      while (d_arr[i] != az1) sum += dif_arr[i++];
    } else {
      while (d_arr[i] != az2) i++;
      while (i < 12) sum += dif_arr[i++];
      i = 0;
      while (d_arr[i] != az1) sum += dif_arr[i++];
    }
  } else {
    // delta < 0
    if (az1 < az2) {
      while (d_arr[i] != az1) i++;
      while (d_arr[i] != az2) sum += dif_arr[i++];
    } else {
      while (d_arr[i] != az2) i++;
      i--;
      while (i >= 0) sum += dif_arr[i--];
      i = 12;
      while (d_arr[i] != az1) sum += dif_arr[--i];
    }
  }

  if (sum > (long)(360.0 * sysGlobal->dome_counts_degree)) {
    error_sound();
    sprintf(outbuf, "\nDome lookup failure.  Calculated steps = %f",sum);
    writeline(outbuf,0);
    sprintf(outbuf,"No move will be made.\n");
    writeline(outbuf,0);
    return 0;
  }
  return sum;
}

//------------------------------------------------------------------
//      Name.......:    calculate_lookup_move_counts
//
//  Purpose....:        support for move by lookup routines
//
//      Input......:    naz             - new azimuth
//                                                              delta   - degrees to move
//
//      Output.....:    encoder counts to move
//
//------------------------------------------------------------------
long calculate_lookup_move_counts(double naz, double delta)
{
  int i;
  BOOL spos_ex;
  BOOL epos_ex;
  double az1;
  double az2;
  long mv_counts;

  /*
	There are 3 cases to consider when calculating the move steps from
	the lookup table.

	1)      Both the starting and ending azimuths are exact multiples of
	30 degrees and, therefore, the exact differences between
	then azimuth pairs in the table are known.  Therefore, we don't
	need to interpolate.

	2)      The move will occur within the boudries of one table entry.
	e.g. a move from -179 to -150, 92 to 118, etc. where the
	starting and ending azimuth are within the same table "cell".

	3)      The move will start and end in positions which both require
	interpolation of the partial differences as well as the
	summation of the known differences.
  */
  cout    << "\nDOME LOOKUP TABLE INTERPOLATION RESULTS"
          << "\n---------------------------------------\n";

  spos_ex = FALSE;
  epos_ex = FALSE;
    
  for (i = 0; i < 13; i++)
    if (d_arr[i] == autoGlobal->dome_azimuth) spos_ex = TRUE;

  for (i = 0; i < 13; i++)
    if (d_arr[i] == naz) epos_ex = TRUE;

  // case 1
  if (spos_ex && epos_ex) {
    cout << "Move from one exact position to another.\n";
    mv_counts = calc_exact_lookup(naz, autoGlobal->dome_azimuth, delta);
    if (delta < 0) mv_counts = -mv_counts;
    cout << "Move counts = " << mv_counts << endl;
    return mv_counts;
  }

  // check for case 2
  i = 0;
  az1 = naz;
  az2 = autoGlobal->dome_azimuth;
  spos_ex = FALSE;

  while ((!spos_ex) && (i < 12)) {
    spos_ex = (     (az1 >= d_arr[i]) &&
                    (az1 <= d_arr[i+1]) &&
                    (az2 >= d_arr[i]) &&
                    (az2 <= d_arr[i+1]));
    i++;
  }

  // case 2
  if (spos_ex) {
    cout << "Move is within the same lookup cell.\n";
    mv_counts = (long)(dif_arr[i-1] * fabs(delta) / 30.0);
    if (mv_counts > (long)(360.0 * sysGlobal->dome_counts_degree)) {
      error_sound;
      cout    << "\nDome lookup failure.  Calculated steps = " << mv_counts
          << endl
          << "Move is > 360 degrees.  No move will be made.\n";
      return 0L;
    }
    if (delta < 0) mv_counts = -mv_counts;
    cout << "Move counts = " << mv_counts << endl;
    return mv_counts;
  }

  // case 3
  // first, calculate the known differences
  az1 = naz;
  az2 = autoGlobal->dome_azimuth;
  i = 0;

  if (delta > 0) {
    // find the closest current azimuth (on the larger side)
    while ((az2 > d_arr[i]) && (i < 12)) i++;
    az2 = d_arr[i];
    
    // find the closest known new azimuth (on the smaller side)
    i = 12;
    while ((az1 < d_arr[i]) && i) i--;
    az1 = d_arr[i];
  } else {
    // find the closest known current azimuth (on the smaller side)
    i = 12;
    while ((az2 < d_arr[i]) && i) i--;
    az2 = d_arr[i];
    
    // find the closest known new azimuth (on the larger size)
    i = 0;
    while ((az1 > d_arr[i]) && (i < 12)) i++;
    az1 = d_arr[i];
  }
    
  // given these known positions, calculate counts as in case 1 above
  mv_counts = calc_exact_lookup(az1, az2, delta);
    
  // now, calculate the partial differences at each end of the move
  if (delta > 0) {
    mv_counts += interpolate(autoGlobal->dome_azimuth, TRUE);
    mv_counts += interpolate(naz, FALSE);
  } else {
    mv_counts += interpolate(naz, TRUE);
    mv_counts += interpolate(autoGlobal->dome_azimuth, FALSE);
  }

  if (mv_counts > (long)(360.0 * sysGlobal->dome_counts_degree)) {
    error_sound;
    cout    << "\nDome lookup failure.  Calculated steps = " << mv_counts
            << endl
            << "Move is > 360 degrees.  No move will be made.\n";
    return 0L;
  }

  if (delta < 0) mv_counts = -mv_counts;
  cout << "Move counts = " << mv_counts << endl;
  return mv_counts;
}

//------------------------------------------------------------------
//      Name.......:    ocs_aux_off
//
//  Purpose....:        turn an auxiliary channel off
//
//      Input......:    channel number
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_aux_off(int channel)
{
  int user_bit;
  char cmd[10];

  if (!ocs_installed()) return TCSERR_NA;

  if (channel == 1)
    user_bit = 3;
  else if (channel == 2)
    user_bit = 2;
  else
    return TCSERR_NOAUX;

  sprintf(cmd, "bh%d;", user_bit);
#ifndef no_hardware
  pc34_send_commands(cmd);
#endif
  return TCSERR_OK;
}

//------------------------------------------------------------------
//      Name.......:    ocs_aux_on
//
//  Purpose....:        turn an auxiliary channel on
//
//      Input......:    channel number
//
//      Output.....:    error code
//
//------------------------------------------------------------------
unsigned ocs_aux_on(int channel)
{
  int user_bit;
  char cmd[10];

  if (!ocs_installed()) return TCSERR_NA;

  if (channel == 1)
    user_bit = 3;
  else if (channel == 2)
    user_bit = 2;
  else
    return TCSERR_NOAUX;
  
  sprintf(cmd, "bl%d;", user_bit);
#ifndef no_hardware
  pc34_send_commands(cmd);
#endif
  return TCSERR_OK;
}

//------------------------------------------------------------------
//      Name.......:    ocs_aux_status
//
//  Purpose....:        return the status of the aux input bits
//
//      Input......:    none
//
//      Output.....:    state of input bits
//                                                              error code
//
//------------------------------------------------------------------
unsigned ocs_aux_status(unsigned &status)
	{
		#ifdef no_hardware
		status = 0;
		return TCSERR_OK;
		#else
		if (!ocs_installed())
			return TCSERR_NA;

sprintf(outbuf,"aux");
writeline(outbuf,1);
                unsigned bitvalues = pcx_read_inputs(PC34);
		//unsigned bitvalues = ocs_read_inputs();
		status = 0;

		// channel 1 is bit 0, channel 2 is bit 1, etc.
		if (bitvalues & OCS_AUX1_BIT)
			status |= 0x01;

		if (bitvalues & OCS_AUX2_BIT)
			status |= 0x02;

		if (bitvalues & OCS_AUX3_BIT)
			status |= 0x04;

		return TCSERR_OK;
		#endif
	}

//------------------------------------------------------------------
//      Name.......:    check_fill_status
//
//      Purpose....:    calculate time since last dewar fill to recommend
//                      if we need to fill
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
void check_fill_status()
{
  if ( !SysTimer[SYSTMR_CCD_FILL_STATUS].Expired() ) return;

   // calculate when we last filled the dewar
   struct mytime t;
   struct date d;
   struct ALLTIMES timeRec;
   mygettime(&d,&t);
   get_all_times_at(d, t, timeRec);

   G->ccd_fill_dtime = timeRec.mjd_utc - autoGlobal->last_fill_utc;

   int wait;
   if (G->ccd_fill_dtime*24. > sysGlobal->ccd_hold_time ) {
     sprintf(outbuf,"WARNING: dewar is probably WARM");
     writeline(outbuf,0);
   } else if (G->ccd_fill_dtime*24. > 0.9 *sysGlobal->ccd_hold_time ) {
     sprintf(outbuf,"WARNING: dewar will need filling within ~%d minutes",
       (int)(60.*(sysGlobal->ccd_hold_time - G->ccd_fill_dtime*24.)));
     writeline(outbuf,0);
   }

  SysTimer[SYSTMR_CCD_FILL_STATUS].NewTimerSecs(SYSTMR_CCD_FILL_INC);
}

//------------------------------------------------------------------
//      Name.......:    check_ccd_temp
//
//  Purpose....:        return ccd temperature
//
//      Input......:    none
//
//      Output.....:    none
//
//------------------------------------------------------------------
int adc12_open(int);
int adc12_get_value();

void check_ccd_temp(int doitnow)
{
  int   i, value;
  int   port;
  static BOOL adcinit = FALSE;
  static int ninit = 5;

  if (!doitnow && !SysTimer[SYSTMR_CCD_TEMP_STATUS].Expired() ) return;

  port = 1;

#ifndef no_hardware
  if (!adcinit) {
    if ( !adc12_open (port)) {
     fprintf (stderr,"Unable to open printer port\n");
     return;
    }
    adcinit = TRUE;
  }
#endif
  
#ifndef no_hardware
  // First time read voltage ninit times, thereafter, just once
  for (i=0 ; i<ninit ; i++) {
    value = adc12_get_value ();
	delay(500);
  }
  ninit = 1;

  // Calculate temperature from voltage
  G->ccd_temp = (value-2048) * 5.0 / 2048 * sysGlobal->ccd_degrees_per_volt;
#endif
  sprintf(outbuf,"adc value = %d, dpv = %7.4f temp = %6.3f\n", value, 
    sysGlobal->ccd_degrees_per_volt, G->ccd_temp);
  writeline(outbuf,1);

  SysTimer[SYSTMR_CCD_TEMP_STATUS].NewTimerSecs(SYSTMR_CCD_TEMP_INC);
}
//------------------------------------------------------------------
//      Name.......:    ocs_ccd_fill_open
//
//  Purpose....:        return ccd filler status
//
//      Input......:    none
//
//      Output.....:    TRUE if the autofill valve is open
//
//------------------------------------------------------------------
BOOL ocs_ccd_fill_open()
{
#ifdef no_hardware
  return TRUE;
#else
  if (!ocs_installed()) return FALSE;

sprintf(outbuf,"ccd_fill");
writeline(outbuf,1);
  unsigned rval = pcx_read_inputs(PC34);
  //unsigned rval = ocs_read_inputs();
  rval &= OCS_AUX1_BIT;
  return (rval) ? TRUE : FALSE;
#endif
}

void ocs_heater(int on)
{
#ifndef no_hardware
  if (on ==1) 
    pc34_send_commands("bh3;");
  else
    pc34_send_commands("bl3;");
#endif
    
}
/********************************* EOF ***********************************/

