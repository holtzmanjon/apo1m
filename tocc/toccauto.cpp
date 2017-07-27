/****************************************************************************/
/*                                                        */
/*  Module:    toccauto.cpp                               */
/*                                                        */
/*  Purpose:  main program loop                           */
/*                                                        */
/****************************************************************************/
/*                                                        */
/*                    PROPERTY OF AUTOSCOPE CORPORATION   */
/*                        2637 Midpoint Dr., Suite D      */
/*                         Fort Collins, CO  80525        */
/*                                                        */
/*                            Copyright 1995              */
/*              Unauthorized duplication or use is prohibited. */
/*                                                             */
/*  Author:    M. Donahue                                 */
/*                                                        */
/*                                                        */
/*  Defines:  no_hardware      - disable all hardware (pcx, tcs, ocs, wwv,*/
/*                              etc) so that simulations can be ran. */
/*                              The network DOES run.                */
/*            debug_main      - use debug main() instead of normal main() */
/*            debug_move_file  - write information to a debug file on each*/
/*                              move                                 */
/*            use_old_keypad  -  use the old keypad move_no_update routines  */
/*                                                                   */
/****************************************************************************/
/*                                                                   */
/*                  ***** NOTES AND STANDARDS *****                  */
/*                                                                   */
/*  1)  SLALIB is expecting longitude to be + to the East.  Our SETUP     */
/*      program enters longitude as + West so the globals module takes care*/
/*      of this for the TOCCAUTO variable G->longitude by taking the      */
/*      negative before converting to radians.                       */
/*                                                                   */
/*  2)  Hour-angle increases to the West and RA increases to the East.    */
/*                                                                   */
/*  3)  RA = LAST - HA                                               */
/*                                                                   */
/*  4)  Azimuth runs 0N, 90E, 180S and 270W          */
/*                                                                   */
/*  5)  The x-axis geartrain is normal if:                           */
/*        Positive steps increase hour-angle or azimuth              */
/*                                                                   */
/*  6)  The y-axis geartrain is normal if:                           */
/*        Positive steps increase declination or altitude            */
/*                                                                   */
/****************************************************************************/
#include <stdlib.h>
#include <conio.h>
#include <iostream.h>
#include <dos.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include "mytype.h"
#include "tcs.h"
#include "ocs.h"
#include "key_util.h"

#include "globals.h"
#include "systimer.h"
#include "shutdown.h"
#include "sounds.h"
#include "keypad.h"
#include "tcs_math.h"
#include "tracking.h"
#include "slalib.h"
#include "slamac.h"
#include "io.h"
#include "filter.h"
#include "guider.h"
#include "tertiary.h"

#ifndef SOCKET
void check_restart();
#endif

void display_menu()
{
clrscr();
cout  << "\t\tTOCCAUTO.EXE v4.0 Beta 1\n\n";
cout   << "Function Keys:\n\n"
<< "  F1:  Exit                            Alt-F1 Toggle 24/50 line mode\n"
<< "  F2:  Reset keypad counters           Alt-F2 Display timer information\n"
<< "  F3:  Display keypad counters         Alt-F3 Display current times\n"
<< "  F4:  Display step counters           Alt-F4 Toggle handpaddle (0-2)\n"
<< "  F5:  Display encoder counters        Alt-F5 Toggle update (off/on)\n"
<< "  F6:  Display current tracking rates  Alt-F6 Toggle z-axis-enable\n"
<< "  F7:  Stopwatch start/reset           Alt-F7 Adjust fudge factors\n"
<< "  F8:  Stopwatch display               Alt-F8 Toggle alt-az handpaddle\n" 
<< "  F9:  Start square-spiral search      Alt-F9 Move telescope steps\n"
<< "  F10: Display this menu               Alt-F10 Enter command mode\n"
<< endl;
}

void update_date_time()
  {
    if (!sysGlobal->wwv_type)
      return;

    if (!SysTimer[SYSTMR_WWV_CHECK].Expired())
      return;

    // get the WWV date & time
    sprintf(outbuf,"Reading the WWV clock");
    writeline(outbuf,1);
    unsigned status = ocs_get_set_date_time();
    if (status) {
      sprintf(outbuf,"Failed. Code = %d", status);
      writeline(outbuf,1);
    }
    else {
      sprintf(outbuf,"Success.");
      writeline(outbuf,1);
    }

    SysTimer[SYSTMR_WWV_CHECK].NewTimerSecs(SYSTMR_WWV_INC);
  }

void write_telescope_position(int force)
{
  if (!force && !SysTimer[SYSTMR_WRITEPOS_CHECK].Expired()) return;
  update_telescope_position();
  autoGlobal->saved_az = G->current_obs_az;
  autoGlobal->saved_alt = G->current_obs_alt;
  autoGlobal->saved_rot = G->current_obs_rot;
  writetoccscf(autoGlobal);
  sprintf(outbuf,"Wrote telescope position");
  writeline(outbuf,1);
  SysTimer[SYSTMR_WRITEPOS_CHECK].NewTimerSecs(SYSTMR_WRITEPOS_INC);
}

// routine to update telescope position and time in global variables
void update_telescope_position()
  {
    double az, alt, ha, dec;
    struct mytime t;
    struct date d;
    struct ALLTIMES timeRec;

    if (!SysTimer[SYSTMR_UPDATE_POSITION].Expired()) return;
    SysTimer[SYSTMR_UPDATE_POSITION].NewTimerSecs(3);

    // Get the encoder positions    
    update_encoder_position();
    // Store the current time 
    mygettime(&d,&t);

    // Convert encoder position to observed alt-az in degrees
    steps_to_degrees(G->current_enc_az, G->current_enc_alt, G->current_enc_rot,
        G->current_obs_az, G->current_obs_alt, G->current_obs_rot);
    autoGlobal->z_axis_pos = G->current_enc_rot;

    // Finally convert the observed alt-az to the observed ra-dec using the
    //  error signal in alt-az and applying this to the last commanded
    //  ra-dec

    get_all_times_at(d, t, timeRec);
    G->current_utc = timeRec.mjd_utc ;
    G->current_lasth = timeRec.lasth ;

    if (G->telescope_is_slewing || 
        G->tracking_on == FALSE || 
        G->telescope_at_home ) {
      slaDh2e(G->current_obs_az*DD2R, G->current_obs_alt*DD2R, G->latitude, &ha, &dec);
    } else {
      // Get the ideal alt-az for the ideal position
      slaDe2h(
 slaDrange(timeRec.last-(G->current_mean_ra+G->dra*(timeRec.mjd_utc-G->epoch0))),
 G->current_mean_dec+G->ddec*(timeRec.mjd_utc-G->epoch0),
            G->latitude, &az, &alt);

      // Add in the error signal
      az = slaDranrm(az + G->last_az_error*DD2R);
      alt = alt + G->last_alt_error*DD2R;

      // Now convert this back into an ra-dec pair
      slaDh2e(az, alt, G->latitude, &ha, &dec);
    }

    G->current_obs_ra = slaDranrm(timeRec.last - ha);
    G->current_obs_dec = dec;
    if (G->telescope_at_home || G->tracking_on == FALSE) {
      G->current_mean_ra = G->current_obs_ra;
      G->current_mean_dec = G->current_obs_dec;
    }

  }

void update_encoder_position()
  {
    long enc_x, enc_y, enc_z;

    // if telescope not initialized, exit
    if (!G->encoder_initialized) return;

#ifndef no_hardware
    // get the current telescope position from the encoders if possible,
    //   otherwise from motor counts

    if (sysGlobal->x_encoder_installed) {
          enc_x = tcs_return_encoder_position('x');
          if (!sysGlobal->x_geartrain_normal) enc_x = -enc_x;
          if (sysGlobal->mount_type) 
            G->current_enc_az = enc_x ;
    } else {
          enc_x = tcs_return_step_position('x');
          if (!sysGlobal->x_geartrain_normal) enc_x = -enc_x;
          if (sysGlobal->mount_type) 
            G->current_enc_az = enc_x ;
    }

    if (sysGlobal->y_encoder_installed) {
          enc_y = tcs_return_encoder_position('y');
          if (!sysGlobal->y_geartrain_normal) enc_y = -enc_y;
          if (sysGlobal->mount_type) 
            G->current_enc_alt = enc_y ;
    } else {
          enc_y = tcs_return_step_position('y');
          if (!sysGlobal->y_geartrain_normal) enc_y = -enc_y;
          if (sysGlobal->mount_type) 
            G->current_enc_alt = enc_y ;
    }

    if (sysGlobal->z_encoder_installed) {
          enc_z = tcs_return_encoder_position('z');
          if (!sysGlobal->z_geartrain_normal) enc_z = -enc_z;
          if (sysGlobal->mount_type) 
            G->current_enc_rot = enc_z ;
    } else {
          enc_z = tcs_return_step_position('z');
          if (!sysGlobal->z_geartrain_normal) enc_z = -enc_z;
          if (sysGlobal->mount_type) 
            G->current_enc_rot = enc_z ;
    }
#else
    double curr_az, curr_alt, curr_rot;
    if (G->telescope_at_home) {
  
      curr_az = autoGlobal->home_az;
      curr_alt = autoGlobal->home_alt;
      curr_rot = autoGlobal->home_rot;
      degrees_to_steps(curr_az, curr_alt, curr_rot, 
             G->current_enc_az, G->current_enc_alt, G->current_enc_rot);
 
    } else {

    // get the current time
    struct mytime t1, t2;
    struct date d1, d2;
    struct ALLTIMES timeRec1, timeRec2;

    mygettime(&d1,&t1);
    //getdate(&d1);
    get_all_times_at(d1, t1, timeRec1);

    mean_to_mount_corrected(timeRec1,
                            G->current_mean_ra,
                            G->current_mean_dec,
                            G->current_mean_epoch,
                            G->current_mean_pmra,
                            G->current_mean_pmdec,
                            G->current_mean_parallax,
                            G->current_mean_radial_velocity,
                            curr_az, curr_alt, curr_rot);
    degrees_to_steps(curr_az*DR2D, curr_alt*DR2D, curr_rot*DR2D, 
             G->current_enc_az, G->current_enc_alt, G->current_enc_rot);

    }
#endif

  }

void check_limits()
  {
    long pos;
    BOOL x_bad;
    BOOL y_bad;
    BOOL z_bad;

    if (!G->telescope_initialized)
      return;

    pos = tcs_return_step_position('x');
    x_bad = ((pos >= sysGlobal->x_pos_soft_limit) ||
	    (pos <= sysGlobal->x_neg_soft_limit));

    pos = tcs_return_step_position('y');
    y_bad = ((pos >= sysGlobal->y_pos_soft_limit) ||
	    (pos <= sysGlobal->y_neg_soft_limit));

    pos = tcs_return_step_position('z');
    z_bad = ((pos >= sysGlobal->z_pos_soft_limit) ||
	    (pos <= sysGlobal->z_neg_soft_limit));

    if (x_bad || y_bad || z_bad) {
	//tcs_emergency_stop();
        tcs_telescope_stop(FALSE);
        G->tracking_on = FALSE;
	sprintf(outbuf,"The telescope has tracked to the soft limits.\n");
        writeline(outbuf,0);
    }
  }

void check_dome()
  {
    if ((sysGlobal->enclosure_type != 2) ||
	!autoGlobal->dome_slaved ||
	!G->telescope_initialized ||
	!G->dome_initialized ||
	G->telescope_at_home)
      return;

    // convert the current ra/dec to alt/az at this time
    double app_ra;
    double app_dec;
    double ha;
    double az;
    double alt;
    struct mytime t;
    struct date d;
    struct ALLTIMES timeRec;

    mygettime(&d,&t);
    //getdate(&d);
    get_all_times_at(d, t, timeRec);

    slaMapqk(G->current_mean_ra, G->current_mean_dec,
	      G->current_mean_pmra, G->current_mean_pmdec,
	      G->current_mean_parallax, G->current_mean_radial_velocity,
	      G->mean_to_app_parms, &app_ra, &app_dec);

    ha = slaDrange(timeRec.last - app_ra);
    slaDe2h(ha, app_dec, G->latitude, &az, &alt);
    az *= DR2D;

    // get the difference
    G->dome_azimuth = ocs_return_dome_azimuth();
    double dome_dif = slaDrange((az - G->dome_azimuth) * DD2R);
    dome_dif = fabs(dome_dif) * DR2D;

    // check this difference against the shutter window size
    if (dome_dif >= sysGlobal->shutter_size)
      {
	unsigned status = ocs_rotate_dome(az);
	if (status)
	  {
	    sprintf(outbuf,
              "Could not rotate the dome to match the telescope.\n"
	      "Error code: %d ",status);
            writeline(outbuf,0);
	    autoGlobal->dome_slaved = FALSE;
	    writetoccscf(autoGlobal);
	  }
      }
  }

void do_spiral_search()
  {
    if (G->telescope_at_home)
      {
	error_sound();
	cout << "Error!  The telescope is at home.\n";
	return;
      };

    if (!G->telescope_initialized)
      {
	error_sound();
	cout << "Error!  The telescope is not initialized\n";
	return;
      };

    long save_inc = kp_move_increment;
    kp_move_increment = 300L;

    int inc = 1;
    int i;

    do
      {
	for (i = 0; i < inc; i++)
	  {
	    keypad_action(TELE_LEFT);
	    if (kbhit())
	      goto leave;
	  }

	for (i = 0; i < inc; i++)
	  {
	    keypad_action(TELE_UP);
	    if (kbhit())
	      goto leave;
	  }

	inc++;
	for (i = 0; i < inc; i++)
	  {
	    keypad_action(TELE_RIGHT);
	    if (kbhit())
	      goto leave;
	  }

	for (i = 0; i < inc; i++)
	  {
	    keypad_action(TELE_DOWN);
	    if (kbhit())
	      goto leave;
	  }
	inc++;
      }
    while (TRUE);

leave:
    getch();
    action_sound();
    cout << "Search stopped.\n";
    kp_move_increment = save_inc;
  }

/******************************************************************/
/*                  **** MAIN PROGRAM LOOP ****                                                 */
/******************************************************************/
#ifndef debug_main
int main()
  {
    unsigned char inputline[132];

    EventTimer stopwatch;
    stopwatch.NewTimer(1);

    int mode = C80;
    textmode(mode);
    clrscr();


/*  Old SCF file stuff is commented out
    // ****
    // **** THE FOLLOWING INITIALIZATIONS MUST OCCUR IN THIS ORDER!!!
    // ****
    // create scf global structures
    initialize_scf_globals();

    // read the SCF files
    for (int i = 0; i < 100; i++)
      {
	if (sysGlobal->readData())
	  break;
	Delay(100);
      }

    if (i == 100)
      {
	sprintf(outbuf,"\x07System Message:  Error reading 'system.scf'.\n"
	      "TOCCAUTO is aborting.\n\n");
        writeline(outbuf,0);
	return 2;
      }

    if (!autoGlobal->readData())
      {
	sprintf(outbuf,"\x07System Message:  Error reading 'toccauto.scf'.\n"
	      "You will need to reset the telescope's home position\n"
	      "before using the telescope.\n");
        writeline(outbuf,0);
      }

    writeautoGlobal();
    writesysGlobal();
    readtoccscf(&autoGlobalNew);
    writetoccscf(&autoGlobalNew);
    readsysscf(&sysGlobalNew);
    writesysscf(&sysGlobalNew);
*/

    // Read configuration files
    fprintf(stderr,"readtoccscf...\n");
    readtoccscf(&autoGlobalNew);
    fprintf(stderr,"readsysscf...\n");
    readsysscf(&sysGlobalNew);
    autoGlobal = &autoGlobalNew;
    sysGlobal = &sysGlobalNew;
    fprintf(stderr,"readgcsscf...\n");
    readgcsscf(&gcsGlobalNew);
    gcsGlobal = &gcsGlobalNew;

    fprintf(stderr,"writetoccscf...\n");
    writetoccscf(&autoGlobalNew);
    fprintf(stderr,"writesysscf...\n");
    writesysscf(&sysGlobalNew);

    // Check to see if communications program has been started, and open
    //   communications files if so.
#ifdef SOCKET
    fprintf(stderr,"settinp up server socket...\n");
    setup_server(&command_serv, 1050);
    SysTimer[SYSTMR_NETWORK_LOST].NewTimerSecs(300);
#else
    fprintf(stderr,"check_restart...\n");
    check_restart();
#endif

    // Does TELPOS.SAV exist?  If not, last session was not gracefully exited
    //   and we don't know the telescope coordinates. 
    fprintf(stderr,"looking for telpos.sav...\n");
    int ifile;
#ifndef no_hardware
    ifile = open(TOCC"\\TOCC\\TELPOS.SAV",O_RDONLY);
    if (ifile<0) {
     sprintf(outbuf,"Telescope software was not quit gracefully!!!\n\n");
     sprintf(outbuf," Rotator position has probably been lost - BEWARE!!!\n\n");
     sprintf(outbuf," Report condition to Dave or Jon before doing anything\n");
     sprintf(outbuf,"     EVEN IF YOU KNOW THE PASSWORD!!!!\n");
     sprintf(outbuf," You must enter a password IN THE DOME to continue: ");
     writeline(outbuf,0);
     scanf("%s",inputline);
     if (strcmp(inputline,"clyde") != 0) return 0 ;

     autoGlobal->saved_alt = -9999;
     autoGlobal->saved_az = -9999;
     autoGlobal->saved_rot = -9999;
    }
    close(ifile);
#endif
    // Remove TELPOS.SAV
    remove(TOCC"\\TOCC\\TELPOS.SAV");

    // check home ha and dec
    if ((autoGlobal->home_ha == 0.0) && (autoGlobal->home_dec == 0.0)) {
	autoGlobal->home_dec = sysGlobal->latitude;
	writetoccscf(autoGlobal);
    }

    // initialize the globals
    initialize_globals();

    // compute home alt-az
    if (sysGlobal->mount_type) {
       // double home_ha_r = autoGlobal->home_ha * DH2R;
       // double home_dec_r = autoGlobal->home_dec * DD2R;
       // double home_az, home_alt;
       // slaDe2h(home_ha_r, home_dec_r, G->latitude, &home_az, &home_alt);

       // autoGlobal->home_az = home_az * DR2D;
       // autoGlobal->home_alt = home_alt * DR2D;

        // Start off with reference alt-az same as home alt-az
        G->ref_alt = autoGlobal->home_alt;
        G->ref_az = autoGlobal->home_az;
        G->ref_rot = autoGlobal->home_rot;
    }
 
    sprintf(outbuf,"longitude: %f",G->longitude);
    writeline(outbuf,1);
                   
    // start the system timers
    init_system_timers();

    // initialize the TCS
    tcs_initialize();

    // Set the encoders from the saved telescope position so that position
    // readout is approximately correct. However, we wont call this a valid
    // initialization of the telescope, so telescope still will need to
    // be initialized before moving.
    if (sysGlobal->mount_type && autoGlobal->saved_alt >= -1000 &&
     autoGlobal->saved_az >= -1000 && autoGlobal->saved_rot >= -1000)
    {
      tcs_set_encoders();
    }

    // initialize the OCS
    ocs_initialize();

    // Quick init on guider
    #ifndef no_guider
    guider_initialize(0,gcsGlobal->x_step_pos,gcsGlobal->y_step_pos);
    #endif
    printgcsscf(&gcsGlobalNew);

    // initialize the filter wheel
    #ifndef no_filter
    filter_initialize(0);
    #endif


    // Quick tertiary initialization from saved position
    tertiary_initialize();

    // calculate star-independent parameters for mean to apparent conversions
    struct mytime t;
    struct date d;
    struct ALLTIMES timeRec;

    mygettime(&d,&t);
    //getdate(&d);
    get_all_times_at(d, t, timeRec);
    sprintf(outbuf,"\nCalculating mean to apparent SLALIB parameters...");
    writeline(outbuf,1);
    slaMappa(timeRec.equinox, timeRec.mjd_tt, G->mean_to_app_parms);

    // calculate the apparent to observed place parameters
    sprintf(outbuf,"\n\nCalculating apparent to observed SLALIB parameters...");
    writeline(outbuf,1);
    double out_temp = G->current_out_temp + 273.15;
    double humidity = G->current_humidity / 100.0;
    slaAoppa(timeRec.mjd_utc, G->ut1_minus_utc, G->longitude,
	      G->latitude, sysGlobal->altitude, G->polor_motion_x,
	      G->polor_motion_y, out_temp, G->current_barometer,
	      humidity, 0.55, 0.0065, G->app_to_obs_parms);

    sprintf(outbuf,"\nThe TOCC is ready.\n");
    writeline(outbuf,1);

    //****************************
    // START THE MAIN PROGRAM LOOP
    // ***************************
    BOOL user_exit = FALSE;
    BOOL functionKey;
    unsigned char Key;

    while (!user_exit) {

//  Update everything we need to update regularly: tracking, clock, weather, etc
      update_all();

//  Command-mode commands here: go into a command driven loop
      if (command_mode) process_command();
      user_exit = TRUE;

//  We no longer use any of this input, so just set user_exit TRUE and get
//    out of the program!

	// process keyboard input
	if (inkey(functionKey, Key, command_mode))
	    if (functionKey)
	      switch (Key)
		{
		  case F1:
		    action_sound();
		    sprintf(outbuf,"\nAre you sure that you want"
			  " to exit? (Y/N)");
                    writeline(outbuf,0);
		    do
		      {
			while (!inkey(functionKey, Key, command_mode)) {}
		      }
		    while ((Key != 'Y') && (Key != 'y')
			    && (Key != 'n') && (Key != 'n')
			    && (Key != ESC));
		    user_exit = ((Key == 'Y') || (Key == 'y'));
		    break;

		  case ALT_F1:
		    mode = (mode == C80) ? C4350 : C80;
		    textmode(mode);
		    clrscr();
		    display_menu();
		    break;

		  case F2:
		    keypad_action(RESET);
		    keypad_action(DISPLAY);
		    break;

		  case ALT_F2:
		    display_timer_info();
		    break;

		  case F3:
		    keypad_action(DISPLAY);
		    break;

		  case ALT_F3:
		    mygettime(&d,&t);
		    //getdate(&d);
		    get_all_times_at(d, t, timeRec);

		    sprintf(outbuf,
                            "\nCurrent times\n"
			    "\tYear    : %d\n"
			    "\tMonth   : %d\n"
			    "\tDay     : %d\n\n"
			    "\tHour    : %d\n"
			    "\tMinute  : %d\n"
			    "\tSecond  : %d\n\n"
			    "\tlasth   : %7.5f\n"
			    "\tmjd_tt  : %13.4f\n"
			    "\tmjd_utc : %13.4f\n"
			    "\tjd_utc  : %13.4f\n"
			    "\tequinox : %9.4f\n\0",
			    d.da_year,
			    (int)d.da_mon,
			    (int)d.da_day,
			    (int)t.ti_hour,
			    (int)t.ti_min,
			    (int)t.ti_sec,
			    timeRec.lasth,
			    timeRec.mjd_tt,
			    timeRec.mjd_utc,
			    (timeRec.mjd_utc + 2400000.5),
			    timeRec.equinox);
                    writeline(outbuf,1);
		    break;

		  case F4:
		    cout  << "\nCurrent step positions (absolute)\n"
			  << "\tX: " << tcs_return_step_position('x') << endl
			  << "\tY: " << tcs_return_step_position('y') << endl;
		    if (sysGlobal->mount_type)
		      cout   << "\tZ: " << tcs_return_step_position('z') << endl;
		    break;

		  case ALT_F4:
		    G->handpaddle = ++G->handpaddle % 3;
		    if (G->handpaddle == 0)
		      printf("Using new AutoScope routine\n");
		    else if (G->handpaddle == 1)   
		      printf("Using old AutoScope routine\n"); 
		    else
		      printf("Using move_to_coordinates\n");
		    break;

		  case F5:
		    cout  << "\nCurrent encoder positions (absolute)\n"
			  << "\tX: " << tcs_return_encoder_position('x') << endl
			  << "\tY: " << tcs_return_encoder_position('y')
			  << endl;
		    break;

		  case ALT_F5:
		    G->update = ++G->update % 2;
		    if (G->update == 0)
		      printf("Not updating handpaddle coords\n");
		    else
		      printf("Updating handpaddle coords\n");
		    break;

		  case F6:
		    cout  << "\nCurrent tracking rates (steps/sec)\n";
		    printf("\tX: %.4f\n", G->x_tracking_rate);
		    printf("\tY: %.4f\n", G->y_tracking_rate);
		    if (sysGlobal->mount_type)
		      printf("\tZ: %.4f\n", G->z_tracking_rate);
		    printf("\nCurrent tracking factors:\n");
		    printf("\tX: %.4f\n", G->x_tracking_factor);
		    printf("\tY: %.4f\n", G->y_tracking_factor);
		    if (sysGlobal->mount_type)
		      printf("\tZ: %.4f\n", G->x_tracking_factor);
		    break;

		  case ALT_F6:
		    if (sysGlobal->z_axis_enabled)
		      printf("z-axis was enabled, now disabling it\n");
		    else
		      printf("z-axis was disabled, now enabling it\n");
		    sysGlobal->z_axis_enabled = !sysGlobal->z_axis_enabled;
		    break;

		  case F7:
		    cout  << "\nResetting stopwatch.\n";
		    stopwatch.ResetTimer();
		    break;

		  case ALT_F7:
		    printf("Old keypad, move fudge factors: %f %d\n",
		      sysGlobal->keypad_fudge_factor,
		      sysGlobal->move_fudge_factor);
		    printf("Enter new factors: ");
                    getline(inputline,sizeof(inputline));
		    sscanf(inputline,"%f %d",
		      &sysGlobal->keypad_fudge_factor,
		      &sysGlobal->move_fudge_factor);
		    break;

		  case F8:
		    cout  << "\nStopwatch elapsed time = "
			  << stopwatch.ElapsedTimeInSecs()
			  << " seconds.\n";
		    break;

		  case ALT_F8:
		    G->delaltaz = ++G->delaltaz % 2;
		    if (G->delaltaz == 0)
		      printf("Handpaddle offsets in ra-dec\n");
		    else
		      printf("Handpaddle offsets in alt-az\n");
		    break;

		  case F9:
		    cout  << "\nStarting square spiral search.  Press"
				" any key to stop.\n";
		    do_spiral_search();
		    break;

		  case ALT_F9:
		    printf("Enter no of x,y,z steps to move: ");
		    long xsteps, ysteps, zsteps;
                    getline(inputline,sizeof(inputline));
		    sscanf(inputline,"%ld %ld %ld",&xsteps,&ysteps,&zsteps);
		    tcs_move_telescope_steps(xsteps,ysteps,zsteps);
		    break;

		  case F10:
		    display_menu();
		    break;
	
		  case ALT_F10:
                    command_mode = !command_mode;
                    break;

		  case UP:
		    keypad_action(TELE_UP);
		    break;

		  case DOWN:
		    keypad_action(TELE_DOWN);
		    break;

		  case LEFT:
		    keypad_action(TELE_LEFT);
		    break;

		  case RIGHT:
		    keypad_action(TELE_RIGHT);
		    break;

		  case CR:
		    keypad_action(SIZE_DEC);
		    break;

		  case INS:
		    cout  << "\nStarting square spiral search.  Press"
				" any key to stop.\n";
		    do_spiral_search();
		    break;
		}
	    else
	      switch (Key)
		{
		  case '*':
		    keypad_action(FOCUS_UP);
		    break;

		  case '-':
		    keypad_action(FOCUS_DOWN);
		    break;

		  case '+':
		    keypad_action(SIZE_INC);
		    break;

		  case '0':
		    cout  << "\nStarting square spiral search.  Press"
				" any key to stop.\n";
		    do_spiral_search();
		    break;

		  case '2':
		    keypad_action(TELE_DOWN);
		    break;

		  case '4':
		    keypad_action(TELE_LEFT);
		    break;

		  case '6':
		    keypad_action(TELE_RIGHT);
		    break;

		  case '8':
		    keypad_action(TELE_UP);
		    break;
		}

	    // clear the keyboard buffer
	    clear_keyboard();
    }

    // stop the telescope
    tcs_telescope_stop();
    G->tracking_on = FALSE;

    // save the current telescope position
    update_encoder_position();

    steps_to_degrees(
         G->current_enc_az,G->current_enc_alt,G->current_enc_rot,
         autoGlobal->saved_az,autoGlobal->saved_alt,autoGlobal->saved_rot);
    autoGlobal->z_axis_pos = G->current_enc_rot;

    writetoccscf(autoGlobal);

    // Create the file TELPOS.SAV to show we exited gracefully
    ifile = creat(TOCC"\\TOCC\\TELPOS.SAV",S_IWRITE);

#ifdef have_debug_file
    system("copy debug.dbx e:\\tocc\\debug.dbx");
#endif
    textmode(C80);
    clrscr();
    return 0;
  }
#else

#include "tcs.h"

int main()
  {
    int mode = C80;
    textmode(mode);
    clrscr();
    // ****
    // **** THE FOLLOWING INITIALIZATIONS MUST OCCUR IN THIS ORDER!!!
    // ****

    // create scf global structures
    initialize_scf_globals();

    // read the SCF files
    for (int i = 0; i < 100; i++)
      {
	if (sysGlobal->readData())
	  break;
	delay(100);
      }

    if (i == 100)
      {
	sprintf(outbuf,"\x07System Message:  Error reading 'system.scf'.\n"
	      "TOCCAUTO is aborting.\n\n");
        writeline(outbuf,0);
	return 2;
      }

    if (!autoGlobal->readData())
      {
	sprintf(outbuf,"\x07System Message:  Error reading 'toccauto.scf'.\n"
	      "You will need to reset the telescope's home position\n"
	      "before using the telescope.\n");
        writeline(outbuf,0);
      }

    // check home ha and dec
    if ((autoGlobal->home_ha == 0.0) && (autoGlobal->home_dec == 0.0))
      {
	autoGlobal->home_dec = sysGlobal->latitude;
	writetoccscf(autoGlobal);
      }

    // initialize the globals
    initialize_globals();

    // calculate star-independent parameters for mean to apparent conversions
    struct mytime t;
    struct date d;
    struct ALLTIMES timeRec;

    mygettime(&d,&t);
    //getdate(&d);
    get_all_times_at(d, t, timeRec);
    sprintf(outbuf,"Calculating mean to apparent SLALIB parameters...");
    writeline(outbuf,1);
    slaMappa(timeRec.equinox, timeRec.mjd_tt, G->mean_to_app_parms);

    // calculate the apparent to observed place parameters
    sprintf(outbuf,"\n\nCalculating apparent to observed SLALIB parameters...");
    writeline(outbuf,1);
    double out_temp = G->current_out_temp + 273.15;
    double humidity = G->current_humidity / 100.0;
    slaAoppa(timeRec.mjd_utc, G->ut1_minus_utc, G->longitude,
	      G->latitude, sysGlobal->altitude, G->polor_motion_x,
	      G->polor_motion_y, out_temp, G->current_barometer,
	      humidity, 0.55, 0.0065, G->app_to_obs_parms);

    G->telescope_initialized = TRUE;
    G->telescope_at_home = TRUE;
    tcs_move_to_az_alt((180.0 * DD2R), (65.0 * DD2R));

    return 0;
  }
#endif


void update_all()
{
        //check_restart();

	// check sensors for shutdown
        writeline("checking sensors",3);
	check_sensors();

	// check weather for shutdown
        writeline("checking weather",3);
	check_weather();

#ifdef SOCKET
#ifdef NOTDEF
	writeline("checking socket aliveness",3);
        if (SysTimer[SYSTMR_NETWORK_LOST].Expired()) {
          writeline("resetting server!!",1);
          setup_server(&command_serv, 1050);
          SysTimer[SYSTMR_NETWORK_LOST].NewTimerSecs(30);
          SysTimerActive[SYSTMR_NETWORK_LOST] = TRUE;
        }
#endif
#else
	// check for network aliveness
	writeline("checking network",3);
	check_network();
#endif

	// check CCD for fill
        writeline("checking CCD",3);
	check_fill_status();
	//check_ccd_temp(0);

	// check the soft limits
        writeline("checking limits",3);
	if (!G->soft_limits_disabled) check_limits();

	// check the dome azimuth against the telescope azimuth
        writeline("checking dome",3);
	check_dome();

	// update the system date/time - put this in move to coords so we
        //  don't interrupt guiding
        // writeline("updating date time",3);
	// update_date_time();

	// update the tracking rates
        writeline("updating tracking",3);
	update_tracking_rates();

        // update the display for command mode
        if (do_display) {
        writeline("updating display",3);
          update_display();
        writeline("done updating display",3);
        }

        // periodically write out telescope position
        update_telescope_position();
        if (remote_on) {
        writeline("updating status",3);
          update_status(0);
        writeline("done updating status",3);
        }

        G->dome_open = autoGlobal->shutter_opened;
        G->lower_dome_open = autoGlobal->lower_shutter_opened;
        G->dome_slaved = autoGlobal->dome_slaved;
        G->mirror_covers_open = autoGlobal->mirror_covers_open;

}
/*
void check_restart()
{
        int ifile;
    // Does RESTART exist?  If it does, then tcomm has been restarted, and
    //  we need to open/reopen files
        ifile = open(restart,O_RDONLY);
        if (ifile >= 0){
          close(ifile);
          if (rfile != NULL) fclose(rfile);
          if (sfile != NULL) fclose(sfile);
          if (cfile != NULL) fclose(cfile);
          ropen = FALSE;
          statopen = FALSE;
          copen = FALSE;
          remove(restart);
        }
}
*/
/********************************* EOF **************************************/
