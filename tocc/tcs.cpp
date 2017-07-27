#undef AOPPAT
/****************************************************************************/
/*                                                                          */
/* Module:    tcs.cpp                                                       */
/*                                                                          */
/* Purpose:  Telescope Control System functions                             */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*                    PROPERTY OF AUTOSCOPE CORPORATION                     */
/*                        2637 Midpoint Dr., Suite D                        */
/*                         Fort Collins, CO  80525                          */
/*                                                                          */
/*                            Copyright 1995                                */
/*              Unauthorized duplication or use is prohibited.              */
/*                                                                          */
/* Author:    M. Donahue                                                    */
/*                                                                          */
/****************************************************************************/
/*
   The azimuth is measured eastward from the north.  So, North is 0°, East is
   90°, South is +180° and West +270°.

   Normal geartrains should increase counts on the X axis for increasing
   azimuth or increasing hour-angle.  Y axis steps should increase for
   increasing altitude or increasing Dec.

   NOTE:  In SLALIB convention, increasing hour-angle is decreasing RA.
	 Therefore, hour-angle increases to the west.
*/

//***********************************
//
// Possible defines in this module
//
// echo_pc38     - echo commands send to PC38 during initialization/moves
//
//***********************************

//#define echo_pc38
//#define echo_en_pc38

#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <dos.h>
#include <malloc.h>

#include "tcs.h"
#include "globals.h"
#include "status.h"

#include "pcx.h"
#include "cp4016.h"

#include "ocs.h"
#include "sounds.h"
#include "systimer.h"
#include "shutdown.h"
#include "tracking.h"
#include "slamac.h"
#include "slalib.h"
#include "window.h"
#include "keypad.h"
#include "io.h"
#include "weather.h"

#ifdef echo_pc38
 #define ECHOCH "\x01"
#else
 #define ECHOCH ""
#endif

#ifdef echo_en_pc38
 #define ENECHOCH "\x01"
#else
 #define ENECHOCH ""
#endif

extern void update_date_time();

//------------------------------------------------------------------
//  Name.......:  tcs_aux_off
//
//  Purpose....: turn an auxiliary channel off
//
// Input......:  channel  - i/o channel number (1 or 2)
//
// Output.....:  error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned tcs_aux_off(int channel)
#else
unsigned tcs_aux_off(int)
#endif
 {
   #ifndef no_hardware
   int user_bit;
   char cmd[10];

   switch (channel)
     {
       case 1  : user_bit = 8; break;
       case 2  : user_bit = 12; break;
       default  : return TCSERR_NOAUX;
     };

   sprintf(cmd, "bh%d;", user_bit);
   pc38_send_commands(cmd);
   #endif
   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_aux_on
//
//  Purpose....: turn an auxiliary channel on
//
// Input......:  channel  - i/o channel number (1 or 2)
//
// Output.....:  error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned tcs_aux_on(int channel)
#else
unsigned tcs_aux_on(int)
#endif
 {
   #ifndef no_hardware
   int user_bit;
   char cmd[10];

   switch (channel)
     {
       case 1  : user_bit = 8; break;
       case 2  : user_bit = 12; break;
       default  : return TCSERR_NOAUX;
     };

   sprintf(cmd, "bl%d;", user_bit);
   pc38_send_commands(cmd);
   #endif
   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_calibrate_soft_limits
//
//  Purpose....: calibrate the given axis' soft limits
//
// Input......:  axis    - 'X' or 'Y'
//
// Output.....:  error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned tcs_calibrate_soft_limits(char axis)
#else
unsigned tcs_calibrate_soft_limits(char)
#endif
 {
   #ifndef no_hardware
   unsigned status;
   char buffer[81];
   LIMITSTATUSPACK limStatus;

   if ((axis != 'x') && (axis != 'X') && (axis != 'y') && (axis != 'Y'))
     return TCSERR_OUTOFRANGE;

   BOOL x_axis = ((axis == 'X') || (axis == 'x'));

   // home the telescope
   writeline("Initializing telescope.",0);
   status = tcs_home_telescope();
   tcs_telescope_stop();
   tcs_set_deinit_telescope();
   if (status)
     return status;

   writeline("Moving to negative limit",1);;
   if (x_axis)
     sprintf(buffer, "ax jg-%ld;", sysGlobal->x_max_velocity / 8);
   else
     sprintf(buffer, "ay jg-%ld;", sysGlobal->y_max_velocity / 8);
   pc38_send_commands(buffer);

   do {
       // check for shutdown / reset watchdog
       if (!check_priority(status))
	 return status;
       tcs_return_limit_status(limStatus);
   } while ((x_axis) ? !limStatus.x_axis : !limStatus.y_axis);

   // get the oms card position for the limit
   long card_pos;
   if (x_axis)
     card_pos = tcs_return_step_position('x');
   else
     card_pos = tcs_return_step_position('y');
   cout << "Steps to negative limit =" << card_pos << endl;
   card_pos += 10000L;
   cout << "Soft limit = " << card_pos << endl;
   if (x_axis)
     sysGlobal->x_neg_soft_limit = card_pos;
   else
     sysGlobal->y_neg_soft_limit = card_pos;

   // move back fast, close to home
   sprintf(outbuf,"\nMove back close to home");
   writeline(outbuf,1);
   if (x_axis)
     sprintf(buffer, "ax vl%ld;", sysGlobal->x_max_velocity);
   else
     sprintf(buffer, "ay vl%ld;", sysGlobal->y_max_velocity);
   pc38_send_commands(buffer);
   if (x_axis)
     sprintf(buffer, "ax mr%ld; gd; id;", labs(sysGlobal->x_neg_soft_limit));
   else
     sprintf(buffer, "ay mr%ld; gd; id;", labs(sysGlobal->y_neg_soft_limit));
   pc38_send_commands(buffer);

   // wait until the axis stops
   do {
       // check for shutdown / reset watchdog
       if (!check_priority(status))
	 return status;
   } while ((x_axis) ? !pc38_done(pcx_x_axis) : !pc38_done(pcx_y_axis));

   // initialize telescope
   writeline("Initializing telescope.",0);
   status = tcs_home_telescope();
   tcs_telescope_stop();
   tcs_set_deinit_telescope();
   if (status)
     return status;

   // move to positive limit
   writeline("Moving to positive limit",1);
   if (x_axis)
     sprintf(buffer, "ax jg%ld;", sysGlobal->x_max_velocity / 8);
   else
     sprintf(buffer, "ay jg%ld;", sysGlobal->y_max_velocity / 8);
   pc38_send_commands(buffer);

   do {
       // check for shutdown / reset watchdog
       if (!check_priority(status))
	 return status;
       tcs_return_limit_status(limStatus);
   } while ((x_axis) ? !limStatus.x_axis : !limStatus.y_axis);

   // get the oms card position for the limit
   if (x_axis)
     card_pos = tcs_return_step_position('x');
   else
     card_pos = tcs_return_step_position('y');
   cout << "Steps to positive limit =" << card_pos << endl;
   card_pos -= 10000L;
   cout << "Soft limit = " << card_pos << endl;
   if (x_axis)
     sysGlobal->x_pos_soft_limit = card_pos;
   else
     sysGlobal->y_pos_soft_limit = card_pos;

   // move back fast, close to home
   sprintf(outbuf,"\nMove back close to home");
   writeline(outbuf,1);
   if (x_axis)
     sprintf(buffer, "ax vl%ld;", sysGlobal->x_max_velocity);
   else
     sprintf(buffer, "ay vl%ld;", sysGlobal->y_max_velocity);
   pc38_send_commands(buffer);
   if (x_axis)
     sprintf(buffer, "ax mr-%ld; gd; id;", labs(sysGlobal->x_pos_soft_limit));
   else
     sprintf(buffer, "ay mr-%ld; gd; id;", labs(sysGlobal->y_pos_soft_limit));
   pc38_send_commands(buffer);

   // wait until the axis stops
   do
     {
       if (!check_priority(status))
	 return status;
     }
   while ((x_axis) ? !pc38_done(pcx_x_axis) : !pc38_done(pcx_y_axis));

   // initialize telescope
   writeline("Initializing telescope.",0);
   tcs_home_telescope();
   tcs_telescope_stop();
   tcs_set_deinit_telescope();
   #endif

   // write data file and return
   writesysscf(sysGlobal);
   return(0);
 }

//------------------------------------------------------------------
//  Name.......:  tcs_calibrate_steps_degree
//
//  Purpose....: calibrate azimuth or rotator steps/° (alt-az only)
//
// Input......:  none
//
// Output.....:  error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned tcs_calibrate_steps_degree(char axis)
#else
unsigned tcs_calibrate_steps_degree(char)
#endif
 {
   double value1, value2;
   FILE *fp;
   char line[80];
   char ans;

   #ifndef no_hardware
   if ((!sysGlobal->mount_type) ||
       ((axis != 'x') && (axis != 'X') && (axis != 'z') && (axis != 'Z')))
     return TCSERR_OUTOFRANGE;

   writeline("Initialize telescope",0);
   autoGlobal->saved_az = -9999;
   autoGlobal->saved_alt = -9999;
   unsigned status = tcs_home_telescope();
   if (status)
     return status;
   else {

//  Old calibrate just ran in one direction and returned home. New way runs 
//    calibrate both directions

#ifdef OLDCAZ
     return (((axis == 'x') || (axis == 'X')) ? calibrate_az_steps() :
	   calibrate_rotator_steps());
#else
     if ( (axis == 'x') || (axis == 'X') ) {
       sprintf(outbuf,"Old encoder counts per degree = %f",
		     sysGlobal->x_encoder_encoder_steps_deg);
       writeline(outbuf,0);
       sprintf(outbuf,"Old motor counts per degree = %f",
		     sysGlobal->x_steps_degree);
       writeline(outbuf,0);
       // Get scale from home going away from limit
       status = calibrate_az_steps(value1);
       sprintf(outbuf,"New counts per degree - CCW = %f",value1);
       writeline(outbuf,0);

       // Back to home again
       sysGlobal->x_geartrain_normal = !sysGlobal->x_geartrain_normal;
       status = calibrate_az_steps(value2);
       sysGlobal->x_geartrain_normal = !sysGlobal->x_geartrain_normal;
       sprintf(outbuf,"New counts per degree - CW = %f",value2);
       writeline(outbuf,0);

       sprintf(outbuf,"Old encoder counts per degree = %f",
		     sysGlobal->x_encoder_encoder_steps_deg);
       writeline(outbuf,0);
       sprintf(outbuf,"Old motor counts per degree = %f",
		     sysGlobal->x_steps_degree);
       writeline(outbuf,0);
       sprintf(outbuf,"New average value of counts per degree = %f",
	   (value1+value2)/2.);
       writeline(outbuf,0);
       if (!sysGlobal->x_encoder_installed)
         sprintf(outbuf,"New average value of _encoder_ steps per degree = %f",
	   (value1+value2)/2.* sysGlobal->x_encoder_encoder_steps_rev/ sysGlobal->x_encoder_motor_steps_rev);

         writeline(outbuf,0);
/*
       sprintf(outbuf,"Do you want to change to the new value (Y or N)? ");
       writeline(outbuf,0);
       getline(line,sizeof(line));
       sscanf(line,"%c",&ans);
       sprintf(outbuf,"Received answer: %c",ans);
       writeline(outbuf,1);
       if (ans == 'Y' || ans == 'y') {
	 // Reset the global variables and update the configuration file
	 sysGlobal->x_encoder_encoder_steps_deg = (value1+value2) / 2.;
	 sysGlobal->x_steps_degree = sysGlobal->x_encoder_encoder_steps_deg *
		   sysGlobal->x_encoder_motor_steps_rev/
		   sysGlobal->x_encoder_encoder_steps_rev;
	 writesysscf(sysGlobal);
	 sprintf(outbuf,"Calibrate azimuth: %lf %lf %lf %lf %lf %lf %lf",
	     sysGlobal->x_encoder_encoder_steps_deg, value1, value2, (value1+value2)/2.,
	     G->current_out_temp,G->current_cab_temp,G->current_aux_temp);
	 writelog(outbuf,8);
       }
*/
       fp = fopen(TOCC"\\tocc\\caz.dat","a");
       fprintf(fp,"%lf %lf %lf %lf %lf %lf\n", value1, value2, (value1+value2)/2.,
	       G->current_out_temp,G->current_cab_temp,G->current_aux_temp);
       fclose(fp);

     } else {

       status = calibrate_rotator_steps(value1);
       sprintf(outbuf,"Old motor counts per degree = %f",
		     sysGlobal->z_steps_degree);
       writeline(outbuf,0);
       sprintf(outbuf,"New counts per degree = %lf",value1);
       writeline(outbuf,0);
/*
       sprintf(outbuf,"Do you want to change to the new value (Y or N)? ");
       writeline(outbuf,0);
       getline(line,sizeof(line));
       sscanf(line,"%c",&ans);

       if (ans == 'Y' || ans == 'y') {
	 // Reset the global variables and update the configuration file
	 sprintf(outbuf,"Calibrate rotator: %lf %lf %lf %lf %lf",
	     sysGlobal->z_steps_degree, value1,
	     G->current_out_temp,G->current_cab_temp,G->current_aux_temp);
	 writelog(outbuf,9);
	 sysGlobal->z_steps_degree = value1;
	 writesysscf(sysGlobal);
       }
*/
       fp = fopen(TOCC"\\tocc\\crot.dat","a");
       fprintf(fp,"%lf %lf %lf %lf %lf %lf", value1, value2, (value1+value2)/2.,
	       G->current_out_temp,G->current_cab_temp,G->current_aux_temp);
       fclose(fp);
     }

  // Reset shutdown timers so we don't immediately close!
     reset_shutdown_timers();
     return(status);
#endif
   }
   #else
   return TCSERR_OK;
   #endif
 }

#ifdef OLDCAZ
unsigned calibrate_az_steps()
#else
unsigned calibrate_az_steps(double &newvalue)
#endif
 {
   #ifndef no_hardware
   char buffer[81];
   BOOL go_pos_dir;
   AXISPACK axisRec;
   unsigned status;

   // send axis back around to home again
   writeline("Going around to home again...",1);

   // get starting position
   long startcount;
   if (sysGlobal->x_encoder_installed)
     startcount = labs(tcs_return_encoder_position('x'));
   else
     startcount = labs(tcs_return_step_position('x'));

   // if the geartrain is normal, then the greatest home to limit distance
   // is in the negative direction
   go_pos_dir = !sysGlobal->x_geartrain_normal;
   if (go_pos_dir)
     sprintf(buffer, "ax jg%ld;", sysGlobal->x_max_velocity);
   else
     sprintf(buffer, "ax jg-%ld;", sysGlobal->x_max_velocity);
   pc38_send_commands(buffer);

   // wait until we are off the home sensor
   do {
       pc38_get_axis_status('x', axisRec);
   } while (axisRec.home);

   // now wait until we hit home again
   do {
       pc38_get_axis_status('x', axisRec);

       // check shutdown / reset watchdog
       if (!check_priority(status))
	 return status;
       update_status(0);
       update_display();
   } while (!axisRec.home);

   // stop the axis
   pc38_send_commands("ax ca; st; id;");
   do {
       // check shutdown / reset watchdog
       if (!check_priority(status))
	 return status;
   } while (!pc38_done(pcx_x_axis));
   pc38_clear_done_flags(pcx_x_axis);

   /*  Now, we are either sitting on the home sensor or just past it.
       Since we want to always do the final pass toward the positive limit,
       we need to make sure that we move off of the home sensor in the
       right direction (negative).

       Get the current status of the home sensor.
   */
   writeline("Course adjustment...",1);
   pc38_get_axis_status('x', axisRec);
   BOOL home = axisRec.home;
   BOOL done = FALSE;
   EventTimer homeTimer;

   /*  state  0:  off home in the + direction
       state 1:  on home sensor
       state 2:  off home in the - direction (too far?)
       state 3:  loop until done
       state 4:  do nothing
       state 5:  off home (in timer)
       state 6:  wait until done and then go to state 1
   */
   byte state = (byte)home;
   if ((!home) && (!go_pos_dir))
     state = 2;

   if (state == 2)
     sprintf(buffer, "ax jg%ld;", (long)(sysGlobal->x_max_velocity / 4));
   else
     sprintf(buffer, "ax jg-%ld;", (long)(sysGlobal->x_max_velocity / 4));
   pc38_send_commands(buffer);

   do
     {
       // check shutdown / reset watchdog
       if (!check_priority(status))
	 return status;

       switch (state)
	 {
	   case 0:  // moving negative until on home sensor
		   pc38_get_axis_status('x', axisRec);
		   if (axisRec.home)
		     state++;
		   break;

	   case 1:  // on home sensor - start timer when off
		   pc38_get_axis_status('x', axisRec);
		   if (!axisRec.home)
		     {
		       homeTimer.NewTimer(9);  // 9 tics or 0.5 seconds
		       state = 5;
		     }
		   break;

	   case 2:  // too far negative - move until on home sensor and back off
		   pc38_get_axis_status('x', axisRec);
		   if (axisRec.home)
		     {
		       pc38_send_commands("ax ca; st; id;");
			 state = 6;
		     }
		   break;

	   case 3:  // wait for done
		   done = pc38_done(pcx_x_axis);
		   if (done)
		     {
		       pc38_clear_done_flags(pcx_x_axis);
		       state++;
		     }
		   break;

	   case 4:  // do nothing
		   break;

	   case 5:  // check the off-home timer
		   pc38_get_axis_status('x', axisRec);
		   if (axisRec.home)
		     state = 1;
		   else
		     if (homeTimer.Expired())  // wait 1/2 second
		       {
			 pc38_send_commands("ax st; id;");
			 state = 3;
		       }
		   break;

	   case 6:  // wait for done
		   if (pc38_done(pcx_x_axis))
		     {
		       pc38_clear_done_flags(pcx_x_axis);
		       state = 1;
		       sprintf(buffer, "ax; jg-%ld;",
				 (long)(sysGlobal->x_max_velocity / 4));
		       pc38_send_commands(buffer);
		     }
		   break;
	 }
     }
   while (!done);

   // now do the final adjustment
   writeline("Fine adjustment...",1);
/*  OLD WAY
   sprintf(buffer, "ax jg%ld;", (long)(sysGlobal->x_max_velocity / 100));
   do {  
	 pc38_get_axis_status('x', axisRec);
       // check shutdown / reset watchdog
       if (!check_priority(status))
	 return status;
   } while (!axisRec.home);
   pc38_send_commands("ax kl;");
*/
//  New way uses km command just like home telescope
   sprintf(buffer, "ax vl%ld; km; id;", 
	  (long)(sysGlobal->x_max_velocity / 100));
   pc38_send_commands(buffer);
   do
     {  
	 done = pc38_done(pcx_x_axis);
     }
   while (!done);

   // now, how many steps was that?
   long count;
   if (sysGlobal->x_encoder_installed)
     count = labs(tcs_return_encoder_position('x'));
   else
     count = labs(tcs_return_step_position('x'));
#ifndef OLDCAZ
   count = labs(count-startcount);
#endif
   sprintf(outbuf,"Encoder counts around = %ld\n",count);
   writeline(outbuf,0);

   newvalue = (double)count / 360.;

#ifdef OLDCAZ
   sprintf(buffer, "ax ac%ld; vl%ld; ma0; gd; id;", sysGlobal->x_acceleration,
	   sysGlobal->x_max_velocity);
   pc38_send_commands(buffer);
   do {
       // check shutdown / reset watchdog
       if (!check_priority(status))
	 return status;
   } while (!pc38_done(pcx_x_axis));
#endif
   pc38_clear_done_flags(pcx_x_axis);
   return status;
   #else
   return TCSERR_OK;
   #endif
 }

unsigned calibrate_rotator_steps(double &newvalue)
 {
   /*  The rotator is easier since we can force the direction of the move.
       Since the final adjustment in tcs_home_telescope is in the positive
       direction, movein the negative direction.
   */
   #if defined(no_hardware)
   return TCSERR_OK;
   #else
   if (!sysGlobal->z_axis_enabled)
     return TCSERR_NA;

   char buffer[81];
   AXISPACK axisRec;
   unsigned status;

   sprintf(outbuf, "Enter z maximum velocity (0 for standard system):");
   writeline(outbuf,0);
   getline(buffer,sizeof(buffer));
   long maxvel;
   sscanf(buffer,"%ld",&maxvel);
   if (maxvel <= 0 ) maxvel = sysGlobal->z_max_velocity;

   writeline("Going around to home again...",1);
   sprintf(buffer, "AZ JG-%ld;", maxvel);
   pc38_send_commands(buffer);

   // first, move off of the home sensor
   do {
       if (!check_priority(status))
	 return status;
       pc38_get_axis_status('z', axisRec);
   } while (axisRec.home);

   // now wait until we detect the sensor
   do {
       if (!check_priority(status))
	 return status;
       pc38_get_axis_status('z', axisRec);
       update_status(0);
       update_display();
   } while (!axisRec.home);

   // stop the axis
   pc38_send_commands("az ca; st; id;");
   do {
       if (!check_priority(status))
	 return status;
   } while (!pc38_done(pcx_z_axis));
   pc38_clear_done_flags(pcx_z_axis);

   writeline("Course adjustment",1);
   pc38_get_axis_status('z', axisRec);
   BOOL home = axisRec.home;
   BOOL done = FALSE;
   int state = ((home) ? 1 : 2);
   if (state == 2)
     sprintf(buffer, "AZ JG%ld;", (long)(sysGlobal->z_max_velocity / 4));
   else
     sprintf(buffer, "AZ JG-%ld;", (long)(sysGlobal->z_max_velocity / 4));
   pc38_send_commands(buffer);

   EventTimer homeTimer;

   do
     {
       if (!check_priority(status))
	 return status;

       switch(state)
	 {
	   case 0:  pc38_get_axis_status('z', axisRec);
		   if (axisRec.home)
		     state++;
		   break;

	   case 1:  pc38_get_axis_status('z', axisRec);
		   if (!axisRec.home) {
		       homeTimer.NewTimer(9);  // 9 tics or 0.5 seconds
		       state = 5;
		     }
		   break;

	   case 2:  pc38_get_axis_status('z', axisRec);
		   if (axisRec.home) {
		       pc38_send_commands("az ca; st; id;");
		       state = 6;
		     }
		   break;

	   case 3:  done = pc38_done(pcx_z_axis);
		   if (done) {
		       pc38_clear_done_flags(pcx_z_axis);
		       state++;
		     }
		   break;

	   case 4:  break;

	   case 5:  pc38_get_axis_status('z', axisRec);
		   if (axisRec.home)
		     state = 1;
		   else
		     if (homeTimer.Expired())  // wait 1/2 second
		       {
			 pc38_send_commands("az st; id;");
			 state = 3;
		       }
		   break;

	   case 6:  if (pc38_done(pcx_z_axis))
		     {
		       pc38_clear_done_flags(pcx_z_axis);
		       state = 1;
		       sprintf(buffer, "az jg-%ld;",
			       (long)(sysGlobal->z_max_velocity / 4));
		       pc38_send_commands(buffer);
		     }
		   break;
	 }
     }
   while (!done);

   // now do the final adjustment
   writeline("Fine adjustment...",1);
//   sprintf(buffer, "az jg%ld;", (long)(sysGlobal->z_max_velocity /100));
   sprintf(buffer, "az vl%ld; km; id;", 
     (long)(sysGlobal->z_max_velocity /100));
   pc38_send_commands(buffer);
   do {
       if (!check_priority(status))
	 return status;

       pc38_get_axis_status('z', axisRec);
   } while (!axisRec.home);
//   pc38_send_commands("az kl;");

   // how many steps?
   long count = labs(tcs_return_encoder_position('z'));
   sprintf(outbuf, "Encoder steps around = %ld", count);
   writeline(outbuf,0);
   count = labs(tcs_return_step_position('z'));
   sprintf(outbuf, "Motor steps around = %ld", count);
   writeline(outbuf,0);
   sprintf(outbuf,"\nOld Steps per degree = %12.5f", 
	   sysGlobal->z_steps_degree);
   writeline(outbuf,0);
   newvalue = (double)count / 360.0;

   // Go back home
   sprintf(buffer, "az ac%ld; vl%ld; ma0; gd; id;", sysGlobal->z_acceleration,
	   sysGlobal->z_max_velocity);
   pc38_send_commands(buffer);
   do {
       if (!check_priority(status))
	 return status;
   } while (!pc38_done(pcx_z_axis));
   pc38_clear_done_flags(pcx_z_axis);
   return status;
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_disable_limits
//
//  Purpose....: disable the axis limits
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
void tcs_disable_limits()
 {
   #ifndef no_hardware
   pc38_send_commands("aa lf;");
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_emergency_stop
//
//  Purpose....: stop all axes
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
void tcs_emergency_stop()
 {
   //error_sound;
   writeline("TCS emergency stop!",0);
   tcs_telescope_stop(FALSE);  // don't wait for done flags
   tcs_set_deinit_telescope();
 }

//------------------------------------------------------------------
//  Name.......:  tcs_enable_limits
//
//  Purpose....: enable axis limits (X, Y only else telescope won't work)
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
void tcs_enable_limits()
 {
   #ifndef no_hardware
   pc38_send_commands("ax ln; ay ln;");
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_home_secondary
//
//  Purpose....: put the secondary mirror at the top of travel (home)
//
// Input......:  none
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_home_secondary()
 {
   #ifdef no_hardware
   return TCSERR_OK;
   #else
   unsigned status;
   if (!sysGlobal->tuv_type)
     return TCSERR_NA;

   char buffer[81];
   sprintf(buffer, "at an; lf; ac%u; vl%u; km;",
	   sysGlobal->tuv_acceleration,
	   sysGlobal->tuv_max_velocity);
   pc38_send_commands(buffer);

   BOOL t_done = FALSE;
   BOOL u_done;
   BOOL v_done;
   AXISPACK axisRec;

   if (sysGlobal->tuv_type == 1)
     {
       sprintf(buffer, "au an; lf; ac%u; vl%u; km;",
	       sysGlobal->tuv_acceleration,
	       sysGlobal->tuv_max_velocity);
       pc38_send_commands(buffer);

       sprintf(buffer, "av an; lf; ac%u; vl%u; km;",
	       sysGlobal->tuv_acceleration,
	       sysGlobal->tuv_max_velocity);
       pc38_send_commands(buffer);
       u_done = FALSE;
       v_done = FALSE;
     }
   else
     {
       u_done = TRUE;
       v_done = TRUE;
     }

   do
     {
       // check watchdog / shutdown
       if (!check_priority(status))
	 return status;

       // update the tracking rates
       update_tracking_rates();

       if (!t_done) {
	   pc38_get_axis_status('t', axisRec);
	   t_done = axisRec.home;
       }

       if (!u_done) {
	   pc38_get_axis_status('u', axisRec);
	   u_done = axisRec.home;
       }

       if (!v_done) {
	   pc38_get_axis_status('v', axisRec);
	   v_done = axisRec.home;
       }
     }
   while (!(t_done && u_done && v_done));

   pc38_send_commands("at af lp0;");
   if (sysGlobal->tuv_type == 1)
     pc38_send_commands("au af lp0; av af lp0;");

   autoGlobal->t_step_pos = 0L;
   autoGlobal->u_step_pos = 0L;
   autoGlobal->v_step_pos = 0L;
   writetoccscf(autoGlobal);
   return TCSERR_OK;
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_home_telescope
//
//  Purpose....: initialize the telescope
//
// Input......:  none
//
// Output.....:  error_code
//
//------------------------------------------------------------------
unsigned tcs_home_telescope()
 {
   /*  It is important that the telescope is initialized in the exact
       same way each time ("cold" or "hot") to ensure proper initial
       pointing.

       In a "cold" situation, the control system cannot know for
       certain where the telescope is so it will move the telescope
       very slowly until either a limit or home is reached.

       In a "hot" situation, the control system know exactly where
       the telescope is so it will move quickly back to home to
       save time.

       In the Alt/Az case, the azimuth axis will ignore the home
       sensor in the "cold" situation and will travel until the
       negative limit is reached.  The prevents us from stopping at
       the wrong home position since the azimuth hits the home sensor
       twice in its normal travel.

       There is a parameter for each axis which determines which way the
       telescope will move initially in a "cold" move.  This allows the
       limit to be reached quickly from the parked position to speed things
       up a bit.

       The "normal" direction for "cold" initialization is to move the axis
       toward the positive limit initially.

       The telescope will ALWAYS do the final adjustment toward the positive
       limit so that all initializations put the telescope on the same
       side of the home magnet.

       Although the routine to initialize an equatorial telescope is almost
       idential to that of an alt/az, they are separated both for
       readability and debugging purposes.
   */

 #ifndef no_hardware
 unsigned status;

 char x_command[81];
 char y_command[81];
 char z_command[81];

 BOOL done;

 BOOL x_rev;
 BOOL y_rev;

 BOOL x_done;
 BOOL y_done;
 BOOL z_done;

 byte x_state;
 byte y_state;

 BOOL z_rev;
 byte z_state;

 AXISPACK axisRec;
 HOMESTATUSPACK homeRec;
// LIMITSTATUSPACK limits;

 EventTimer x_home_timer;
 EventTimer y_home_timer;
 EventTimer z_home_timer;

 // stop the telescope
 tcs_telescope_stop();
 tcs_set_deinit_telescope();

 int init_type = -1 ;

 // set the axis accelerations
 sprintf(x_command, ECHOCH"ax ac%ld;", sysGlobal->x_acceleration);
 pc38_send_commands(x_command);

 sprintf(y_command, ECHOCH"ay ac%ld;", sysGlobal->y_acceleration);
 pc38_send_commands(y_command);

// if (sysGlobal->z_axis_enabled) {
   sprintf(z_command, ECHOCH"az ac%ld;", sysGlobal->z_acceleration);
   pc38_send_commands(z_command);
// }
 if (sysGlobal->z_encoder_installed) cp4016_init();

 // Reset home positions to zero out any coordinate updates
 G->ref_alt = autoGlobal->home_alt;
 G->ref_az = autoGlobal->home_az;
 G->ref_rot = autoGlobal->home_rot;

  sprintf(outbuf,"saved alt, az, rot: %lf %lf %lf\n",
     autoGlobal->saved_alt, autoGlobal->saved_az, autoGlobal->saved_rot);
  writeline(outbuf,1);

// if we know the telescope position already, we just need to set the
//   encoders to match the known position and we are done! 

  if (autoGlobal->saved_alt >= -1000 && 
      autoGlobal->saved_az >= -1000 && 
      autoGlobal->saved_rot >= -1000) 
  {
    writeline("setting previous position",1);
    tcs_set_encoders();

    // Convert the current alt-az to ra and dec and load into variables 
    double ha, dec;
    struct mytime t;
    struct date d;
    struct ALLTIMES timeRec;

    mygettime(&d, &t);
    // getdate(&d);
    get_all_times_at(d, t, timeRec);

    slaDh2e(autoGlobal->saved_az*DD2R, autoGlobal->saved_alt*DD2R, 
	    G->latitude,&ha, &dec);

    G->current_mean_ra = slaDranrm(timeRec.last - ha);
    G->current_mean_dec = dec;
    G->current_mean_epoch = timeRec.equinox;

    tcs_store_z_axis_pos();
    G->telescope_at_home = FALSE;
    init_type = 0;

  } else {
   //  We will initialize the telescope by moving to the home sensor
   //**************************************
   //           alt-az mount
   //**************************************
    writeline("Course adjustment...",1);
    pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);

    if (G->telescope_session_init)
      {
	 // the system is "hot". Move the telescope 5 degrees off the home
	 //   sensor in the negative direction.
	 sprintf(x_command, ECHOCH"ax vl%ld; ma%ld; gd; id;",
	   sysGlobal->x_max_velocity,(long)(-5*sysGlobal->x_steps_degree));
	 sprintf(y_command, ECHOCH"ay vl%ld; ma%ld; gd; id;",
	   sysGlobal->y_max_velocity,(long)(-5*sysGlobal->y_steps_degree));
	 sprintf(z_command, ECHOCH"az vl%ld; ma%ld; gd; id;",
	   sysGlobal->z_max_velocity,(long)(-5*sysGlobal->z_steps_degree));

	 /*
	     Since the MA0 command will always place the home sensor on the
	     edge of detection, and since we would always have to make a
	     move in the positive direction to get back on the sensor,
	     force the x_rev, y_rev, z_rev flags to be FALSE so that the
	     state machine below will move the axes in the correct direction.
	 */
	 x_rev = FALSE;
	 y_rev = FALSE;

	 pc38_send_commands(x_command);
	 pc38_send_commands(y_command);

	 // wait for the axes to finish. Do x and y first, then z so we can
	 //  know where the telescope will be in azimuth when it does the
	 //  (potentially more damaging) rotator which we can look at with
	 //  a video camera
	 do {
	   // Update the telescope position and send it to the remote
	   update_display();
	   update_status(0);

	   // check the shutdown / reset watchdog
	   if (!check_priority(status))
	       return status;
	   done = pc38_done(pcx_x_axis | pcx_y_axis);
	 } while (!done);

	 //if (sysGlobal->z_axis_enabled) {
	   pc38_send_commands(z_command);
	   z_rev = FALSE;

	   do {
	     // Update the telescope position and send it to the remote
	     update_display();
	     update_status(0);
	     // check the shutdown / reset watchdog
	     if (!check_priority(status))
		 return status;
	     done = pc38_done(pcx_z_axis);
	   } while (!done);
	 //}
	 pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);
	 init_type = 1;
      }  // end "hot"
    else
      {
	 // the system is "cold"
	 /*
	   ON ALTITUDE AXIS ONLY
	   ---------------------
	   If we pass the home sensor on the way to the limit switch,
	   stop the axis so that the limit is not hit.

	   The z axis is a special case.  We may or may not know the
	   position of the z axis.  We will look at the last known
	   position of the z axis (stored in the TOCCAUTO.SCF file) and
	   will move the opposite number of steps.  If the z axis
	   position is known, this will work.  If this is the very first
	   time this telescope has been initialized or if the TOCCAUTO.SCF
	   file is corrupt or missing, this axis won't move because the
	   value will be 0. As above, if we are to move the z axis, we'll
	   do so after moving the x and y axes to (near) the home position,
	   so we'll know where we are in azimuth when doing the rotator so
	   we can take a look at it.

	   Mechanically, the azimuth axis passes the home sensor twice in
	   it full travel.  The soft limits are based on the home position
	   closest to the shortest limit.  ie, in a geartrain normal
	   azimuth situation, the home position is closer to the positive
	   limit.  To make sure that we start from the correct home position,
	   we will force the azimuth axis to run into the appropriate limit
	   switch regardless of whether or not we encounter the home sensor
	   on the way.
	 */
	 x_rev = !sysGlobal->x_geartrain_normal;  // true = toward neg limit
	 y_rev = sysGlobal->y_home_dir_reversed;

	 if (x_rev)
	   sprintf(x_command, ECHOCH"ax jg-%ld;",
		   sysGlobal->x_max_velocity / 3);
	 else
	   sprintf(x_command, ECHOCH"ax jg%ld;",
		   sysGlobal->x_max_velocity / 3);

	 if (y_rev)
	   sprintf(y_command, ECHOCH"ay jg-%ld;",
		   sysGlobal->y_max_velocity / 4);
	 else
	   sprintf(y_command, ECHOCH"ay jg%ld;",
		   sysGlobal->y_max_velocity / 4);

	 // send the commands
	 pc38_send_commands(x_command);
	 pc38_send_commands(y_command);

	 x_done = FALSE;
	 y_done = FALSE;
	 x_state = 0;
	 y_state = 0;

         EventTimer SafetyTimer;
         SafetyTimer.NewTimerSecs(300);

	 do
	   {
	     // Update the telescope position and send it to the remote
	     update_display();
	     update_status(0);

	     // check shutdown / reset watchdog
	     if (!check_priority(status))
	       return status;

	     switch (x_state)
	       {
		 case 0:  // moving
			 pc38_get_axis_status('x', axisRec);
			 if (axisRec.limit)
			   {
			     sprintf(x_command, ECHOCH"ax vl%ld;",
				       sysGlobal->x_max_velocity);
			     pc38_send_commands(x_command);
			     if (x_rev)
			       pc38_send_commands(ECHOCH"ax ca; hm; id;");
			     else
			       pc38_send_commands(ECHOCH"ax ca; hr; id;");
			     x_state++;
                             SafetyTimer.NewTimerSecs(60);

			   }
			 break;

		 case 1:  // wait for done
			 x_done = pc38_done(pcx_x_axis);
			 if (x_done)
			   {
			     pc38_clear_done_flags(pcx_x_axis);
			     x_state++;
			   }
			 break;
	       }

	     switch (y_state)
	       {
		 case 0:  // moving
			 pc38_get_axis_status('y', axisRec);
			 if (axisRec.home)
			   {
			     pc38_send_commands(ECHOCH"ay ca; st; id;");
			     y_state++;
			   }
			 else
			   if (axisRec.limit)
			     {
			     sprintf(y_command, ECHOCH"ay vl%ld;",
				       sysGlobal->y_max_velocity);
			     pc38_send_commands(y_command);
			     if (y_rev)
			       pc38_send_commands(ECHOCH"ay ca; hm; id;");
			     else
			       pc38_send_commands(ECHOCH"ay ca; hr; id;");
	sprintf(outbuf,"Hit limit, sending command %d",y_rev);
	writeline(outbuf,1);
			     y_state++;
			     }
			 break;

		 case 1:  // wait for done
			 y_done = pc38_done(pcx_y_axis);
			 if (y_done)
			   {
			     pc38_clear_done_flags(pcx_y_axis);
			     y_state++;
			   }
			 break;
	       }

	   }
	 while (!(x_done && y_done) &&  (x_state==0 || !SafetyTimer.Expired()));

	 if (SafetyTimer.Expired()) {
           writeline("Safety timer expired! Telescope not initialized!",0);
           tcs_telescope_stop();
           return TCSERR_CANTINITTELE;
         }

         // Now do z axis
	 //if (sysGlobal->z_axis_enabled) {
//           z_rev = autoGlobal->z_axis_pos < 0;
//           sprintf(z_command, ECHOCH"az vl%ld; mr%ld; gd; id;",
//                   sysGlobal->z_max_velocity, -autoGlobal->z_axis_pos);
	   sprintf(z_command, ECHOCH"az vl%ld; ma%ld; gd; id;",
		 sysGlobal->z_max_velocity, 
		 long(-5*fabs(sysGlobal->z_steps_degree)));
	   pc38_send_commands(z_command);
	   z_rev = 0;
	   z_done = FALSE;
	   z_state = 0;
	   do
	   {
	     // Update the telescope position and send it to the remote
	     update_display();
	     update_status(0);

	     // check shutdown / reset watchdog
	     if (!check_priority(status))
	       return status;

	     switch (z_state)
		 {
		   case 0:  // moving
			   z_done = pc38_done(pcx_z_axis);
			   if (z_done)
			     {
			       pc38_clear_done_flags(pcx_z_axis);
			       z_state++;
			     }
			   break;
		 }
	   }
	   while (!z_done);
	 //}
      }  // end "cold"

    /*
	 Now, we are either sitting on the home sensor or just past it.
	 Since we want to always do the final pass toward the positive limit,
	 we need to make sure that we move off of the home sensor in the
	 correct direction (ie toward the negative limit).

	 Get the current status of the home sensors.
    */
    tcs_return_home_status(homeRec);
    x_done = FALSE;
    y_done = FALSE;
    z_done = FALSE;

    // setup the velocities
    sprintf(x_command, ECHOCH"ax vl%ld; ay vl%ld; az vl%ld;",
		 (sysGlobal->x_max_velocity / 4),
		 (sysGlobal->y_max_velocity / 4),
		 (sysGlobal->z_max_velocity / 4));
    pc38_send_commands(x_command);

    /*
	 NOTE:  If the last move was fast (either from the limit switch or
	 due to the telescope being "hot", and was toward the negative
	 limit, and we overshot it, we are already on the correct side
	 of the home magnet, although we may be really far away.  If this
	 is the case, move back to home and then off again to get closer
	 for the final adjustment.

	 If the last move was toward the positive limit, we want to move
	 toward the negative limit until we detect home and then keep moving
	 until we don't detect home.

	 The state machine for each axis is set up as follows:

	 state 0:  off home in the positive direction
	       1:  on the home sensor
	       2:  off home in the negative direction
	       3:  loop until done
	       4:  do nothing
	       5:  off home (in home sensor hysteresis timer)
    */
     // figure out which state we are currently in
    x_state = homeRec.x_axis;
    y_state = homeRec.y_axis;
    z_state = homeRec.z_axis;

 sprintf(outbuf,"x_state: %d  y_state: %d  z_state: %d x_rev: %d y_rev: %d z_rev: %d  %ld",x_state,y_state,z_state,x_rev,y_rev,z_rev,sysGlobal->z_max_velocity);
 writeline(outbuf,1);
    if ((!homeRec.x_axis) && (!x_rev))
       x_state = 2;

    if ((!homeRec.y_axis) && (!y_rev))
       y_state = 2;

    if ((!homeRec.z_axis) && (!z_rev))
       z_state = 2;

    // move according to the state we are in
    if (x_state == 2)
       //pc38_send_commands(ECHOCH"ax hm; id;");
       pc38_send_commands(ECHOCH"ax km; id;");
    else {
       sprintf(x_command,ECHOCH"ax jg-%ld;", sysGlobal->x_max_velocity / 4);
       pc38_send_commands(x_command);
    }

    if (y_state == 2)
    //   pc38_send_commands(ECHOCH"ay ma10000; id;");
    //   pc38_send_commands(ECHOCH"ay hm; id;");
       pc38_send_commands(ECHOCH"ay km; id;");
    else {
       sprintf(y_command,ECHOCH"ay jg-%ld;", sysGlobal->y_max_velocity / 4);
       pc38_send_commands(y_command);
    }

    if (z_state == 2) {
      //writeline("sending az hm; id;",1);
      //pc38_send_commands(ECHOCH"az hm; id;");
      pc38_send_commands(ECHOCH"az km; id;");
    } else {
      writeline("sending az jg-maxvel/4;",1);
      sprintf(z_command,ECHOCH"az jg-%ld;", sysGlobal->z_max_velocity / 4);
      pc38_send_commands(z_command);
    }

    // We won't let things go more than 2 minutes before quitting!
    EventTimer SafetyTimer;
    SafetyTimer.NewTimerSecs(60);
int oldstate=-1;
    do {
	 // Update the telescope position and send it to the remote
	 update_display();
	 update_status(0);

	 // check shutdown / reset watchdog
	 if (!check_priority(status))
	   return status;

	 switch (x_state)
	   {
	     case 0:  pc38_get_axis_status('x', axisRec);
		     if (axisRec.home) x_state++;
		     break;

	     case 1:  pc38_get_axis_status('x', axisRec);
		     if (!axisRec.home) {
		       x_home_timer.NewTimer(9);
		       x_state = 5;
		     }
		     break;

	     case 2:  if (pc38_done(pcx_x_axis)) {
			 pc38_clear_done_flags(pcx_x_axis);
			 pc38_get_axis_status('x', axisRec);
			 x_state = axisRec.home;
			 sprintf(x_command, ECHOCH"ax jg-%ld;",
				   sysGlobal->x_max_velocity / 4);
			 pc38_send_commands(x_command);
		       }
		     break;

	     case 3:  x_done = pc38_done(pcx_x_axis);
		     if (x_done) {
			 pc38_clear_done_flags(pcx_x_axis);
			 x_state++;
		     }
		     break;

	     case 4: break;

	     case 5: pc38_get_axis_status('x', axisRec);
		     if (axisRec.home)
		       x_state = 1;    // still on home sensor
		     else
		       if (x_home_timer.Expired()) {
		         pc38_send_commands(ECHOCH"ax st; id;");
		         x_state = 3;
		       }
		     break;
	   }

	 switch (y_state)
	   {
	     case 0:  pc38_get_axis_status('y', axisRec);
		     if (axisRec.home) y_state++;
		     break;

	     case 1:  pc38_get_axis_status('y', axisRec);
		     if (!axisRec.home) {
			 y_home_timer.NewTimer(9);
			 y_state = 5;
		       }
		     break;

	     case 2:  if (pc38_done(pcx_y_axis)) {
			 pc38_clear_done_flags(pcx_y_axis);
			 pc38_get_axis_status('y', axisRec);
			 y_state = axisRec.home;
			 sprintf(y_command, ECHOCH"ay jg-%ld;",
				   sysGlobal->y_max_velocity / 4);
			 pc38_send_commands(y_command);
		       }
		     break;

	     case 3:  y_done = pc38_done(pcx_y_axis);
		     if (y_done) {
			 pc38_clear_done_flags(pcx_y_axis);
			 y_state++;
		       }
		     break;

	     case 4:  break;

	     case 5:  pc38_get_axis_status('y', axisRec);
		     if (axisRec.home)
		       y_state = 1;    // still on home sensor
		     else
		       if (y_home_timer.Expired())
			 {
			   pc38_send_commands(ECHOCH"ay st; id;");
			   y_state = 3;
			 }
		     break;
	   }

	 //if (sysGlobal->z_axis_enabled) {
    if (z_state != oldstate) {
	 sprintf(outbuf,"z_state: %d",z_state);
	 writeline(outbuf,1);
         oldstate=z_state;
    }
	     switch (z_state)
	       {
		 case 0:  pc38_get_axis_status('z', axisRec);
			 if (axisRec.home) z_state++;
			 break;

		 case 1:  pc38_get_axis_status('z', axisRec);
			 if (!axisRec.home)
			   {
			     z_home_timer.NewTimer(9);
			     z_state = 5;
			   }
			 break;

		 case 2:  if (pc38_done(pcx_z_axis))
			   {
			     pc38_clear_done_flags(pcx_z_axis);
			     pc38_get_axis_status('z', axisRec);
			     z_state = axisRec.home;
			     sprintf(z_command, ECHOCH"az jg-%ld;",
				       sysGlobal->z_max_velocity / 4);
			     pc38_send_commands(z_command);
			   }
			 break;

		 case 3:  z_done = pc38_done(pcx_z_axis);
			 if (z_done)
			   {
			     pc38_clear_done_flags(pcx_z_axis);
			     z_state++;
			   }
			 break;

		 case 4:  break;

		 case 5:  pc38_get_axis_status('z', axisRec);
			 if (axisRec.home)
			   z_state = 1;    // still on home sensor
			 else
			   if (z_home_timer.Expired())
			     {
			       pc38_send_commands(ECHOCH"az st; id;");
			       z_state = 3;
			     }
			 break;
	       }
	  // }
	 done = x_done && y_done;
	 //if (sysGlobal->z_axis_enabled)
	   done = done && z_done;
//sprintf(outbuf,"done: %d %d %d %d",done,x_done,y_done,z_done);
//writeline(outbuf,1);
       }
    while (!done && !SafetyTimer.Expired());
sprintf(outbuf,"done: %d %d %d %d",done,x_done,y_done,z_done);
writeline(outbuf,1);

    if (SafetyTimer.Expired()) {
      writeline("Safety timer expired! Telescope not initialized!",0);
      tcs_telescope_stop();
      return TCSERR_CANTINITTELE;
    }

    // now dow the final adjustment
    writeline("Fine adjustment...",1);

#define NEWWAY
#ifdef OLDWAY
    sprintf(x_command, ECHOCH"ax vl%ld; km; id;",
	     (long) (sysGlobal->x_max_velocity / 400));
    sprintf(y_command, ECHOCH"ay vl%ld; km; id;",
	     (long) (sysGlobal->y_max_velocity / 400));
    sprintf(z_command, ECHOCH"az vl%ld; km; id;",
	     (long) (sysGlobal->z_max_velocity / 100));
#else
#ifdef NEWWAY
    sprintf(x_command, ECHOCH"ax vl%ld; km; id;",
	     (long) (sysGlobal->x_max_velocity / 100));
    sprintf(y_command, ECHOCH"ay vl%ld; km; id;",
	     (long) (sysGlobal->y_max_velocity / 100));
    sprintf(z_command, ECHOCH"az vl%ld; km; id;",
	     (long) (sysGlobal->z_max_velocity / 100));
#else
    sprintf(x_command, ECHOCH"ax vl100000 hh hr vl1000 hl hm ma0 id go;");
    sprintf(y_command, ECHOCH"ay vl100000 hh hr vl1000 hl hm ma0 id go;");
    sprintf(z_command, ECHOCH"az vl100000 hh hr vl1000 hl hm ma0 id go;");
#endif
#endif

    pc38_send_commands(x_command);
    pc38_send_commands(y_command);

    //if (sysGlobal->z_axis_enabled)
       pc38_send_commands(z_command);

    SafetyTimer.NewTimerSecs(120);

    do {
	 // Update the telescope position and send it to the remote
	 update_display();
	 update_status(0);

	 // check shutdown / reset watchdog
	 if (!check_priority(status)) return status;
	 done = pc38_done(pcx_x_axis | pcx_y_axis);
	 //if (sysGlobal->z_axis_enabled) 
           done = done && pc38_done(pcx_z_axis);
    } while (!done && !SafetyTimer.Expired());

    pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);

    if (SafetyTimer.Expired()) {
      writeline("Safety timer expired! Telescope not initialized!",0);
      tcs_telescope_stop();
      return TCSERR_CANTINITTELE;
    }

   // load the step counter/encoders with 0
    Delay(500);
    sprintf(outbuf,
    "Old encoder xyz and motor xyz positions:\n%ld %ld %ld\n %ld %ld %ld\n",
    labs(tcs_return_encoder_position('x')),
    labs(tcs_return_encoder_position('y')),
    labs(tcs_return_encoder_position('z')),
    labs(tcs_return_step_position('x')),
    labs(tcs_return_step_position('y')),
    labs(tcs_return_step_position('z')));
    writeline(outbuf,1);
    pc38_send_commands(ECHOCH"ax lp0; ay lp0; az lp0;");
    if (sysGlobal->z_encoder_installed) cp4016_reset();
    G->encoder_initialized = TRUE;
    tcs_store_z_axis_pos();

    G->telescope_at_home = TRUE;

    // set the current approximate telescope position
    double ha, dec;
    struct mytime t;
    struct date d;
    struct ALLTIMES timeRec;
    mygettime(&d,&t);
    // getdate(&d);
    get_all_times_at(d, t, timeRec);
    if (!sysGlobal->mount_type){
      G->current_mean_ra = 
	slaDranrm(timeRec.last - autoGlobal->home_ha*DH2R);
      G->current_mean_dec = autoGlobal->home_dec*DD2R;
    }
    else {
      slaDh2e(autoGlobal->home_az*DD2R, autoGlobal->home_alt*DD2R,
	      G->latitude, &ha, &dec);
      G->current_mean_ra = slaDranrm(timeRec.last - ha);
      G->current_mean_dec = dec;
    }
    G->current_mean_epoch = timeRec.equinox;

  } /* end initialize telescope by moving to home sensor */

  // turn position mantainence mode off and set encoder ratios to unity
  //  to make sure we get correct tracking rates
  pc38_send_commands(ENECHOCH"ax hf; er1,1; ay hf; er1,1; az hf: er 1,1");

  // set some globals
  G->telescope_initialized = TRUE;
  G->telescope_session_init = TRUE;
  G->tracking_on = TRUE;

  writeline("init: calling update tracking rates",1);
  update_tracking_rates(TRUE);

  writeline("Encoder positions in home telescope",1);
  sprintf(outbuf,"x: %ld\n\r"
	  "y: %ld\n\r"
	  "z: %ld\n\r",
	  tcs_return_encoder_position('x'),
	  tcs_return_encoder_position('y'),
	  tcs_return_encoder_position('z'));
  writeline(outbuf,1);

  // reset shutdown timers
  reset_shutdown_timers();

   // reset the keypad
  reset_keypad_counters();
#else // no_hardware
  G->telescope_initialized = TRUE;
  G->telescope_session_init = TRUE;
  G->tracking_on = TRUE;
  G->telescope_at_home = TRUE;
#endif

  struct mytime t;
  struct date d;
  struct ALLTIMES timeRec;
  mygettime(&d,&t);
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

  if (init_type == 0)
    sprintf(outbuf,"Telescope initialized - quick init");
  else if (init_type == 1)
    sprintf(outbuf,"Telescope initialized - hot start");
  else
    sprintf(outbuf,"Telescope initialized - cold start");
  writelog(outbuf,5);

  write_telescope_position(1);

  return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_initialize
//
//  Purpose....: initialize the OMS card, etc.
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
void tcs_initialize()
 {
   #ifndef no_hardware
   writeline("Initializing TCS",0);

   // disable the OMS interrupts - we use a polling method
   pc38_disable_interrupts();

   // reset the PC38
   pc38_send_commands("aa rs;");

   // make sure that the card is finished initializing
   while (pc38_read_status_reg() & 0x02);
   Delay(10);

   // turn dust cover power off
   pc38_send_commands("bh7;");

   // select manual mode for the IS-400
   tcs_is400_select(0);

   // enable X & Y limits, disable T, U, V, Z limits
   pc38_send_commands("ax ln; ay ln;");
   pc38_send_commands("at lf; au lf; av lf; az lf;");

   // turn soft limits off
   pc38_send_commands("ax sf; ay sf; az sf;");

   // set base velocities
   pc38_send_commands("ax vb100; ay vb100; az vb100;");

   // set auxiliary bits for axes
   //pc38_send_commands("aa af1,1,1;");
   //pc38_send_commands("aa an,,,1,1,1;");
   pc38_send_commands("aa af1,1,1,1,1,1;");

   // home sensors are active low on X,Y,Z axes
   pc38_send_commands("ax hl; ay hl; az hl;");
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_is400_select
//
//  Purpose....: select a port on the instrument selector
//
// Input......:  port number (1-4, 0 manual)
//
// Output.....:  error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned tcs_is400_select(int port)
#else
unsigned tcs_is400_select(int)
#endif
 {
   #ifndef no_hardware
   if ((port < 0) || (port > 4))
     return TCSERR_OUTOFRANGE;

   if (!sysGlobal->is400_installed)
     return TCSERR_NA;

   switch(port)
     {
       case 0:  pc38_send_commands("bh0; bh1; bh2; bh3;"); break;
       case 1: pc38_send_commands("bl0; bh1; bh2; bh3;"); break;
       case 2: pc38_send_commands("bh0; bl1; bh2; bh3;"); break;
       case 3: pc38_send_commands("bh0; bh1; bl2; bh3;"); break;
       case 4: pc38_send_commands("bh0; bh1; bh2; bl3;"); break;
     }
   #endif
   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_mark_coordinates
//
//  Purpose....: set the telescope's coordinates
//
// Input......:  (see list)
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_mark_coordinates( double ra,      // radians
			       double dec,     // radians
			       double epoch,
			       double pmra,    // radians
			       double pmdec,    // radians
			       double parallax,
			       double radial_velocity,
			       double eff_wavelength,
			       double tlr,
			       BOOL sessiononly)

 {
  struct WEATHERPACK weather;

   if (!G->telescope_initialized)
     return TCSERR_TELENOTINIT;

   if (!is_star_in_window(ra, dec, epoch, pmra, pmdec,
			   parallax, radial_velocity))
     return TCSERR_MOVEWINDOW;

   // set the appropriate globals
   G->telescope_at_home = FALSE;
   G->telescope_is_slewing = FALSE;
   G->current_mean_ra = ra;
   G->current_mean_dec = dec;
   G->current_mean_epoch = epoch;
   G->current_mean_pmra = pmra;
   G->current_mean_pmdec = pmdec;
   G->current_mean_parallax = parallax;
   G->current_mean_radial_velocity = radial_velocity;
   G->current_mean_eff_wavelength = eff_wavelength;

   // get the time
   struct ALLTIMES timeRec;
   struct date d;
   struct mytime t;

   mygettime(&d,&t);
   //getdate(&d);
   get_all_times_at(d, t, timeRec);

   // update slalib parameters

   slaMappa(epoch, timeRec.mjd_tt, G->mean_to_app_parms);
#ifdef AOPPAT
   slaAoppat(timeRec.mjd_utc, G->app_to_obs_parms);
#else
   unsigned status = read_weather_station(weather);
   double out_temp = G->current_out_temp + 273.15;
   double humidity = G->current_humidity / 100.0;
   slaAoppa(timeRec.mjd_utc, G->ut1_minus_utc, G->longitude,
	     G->latitude, sysGlobal->altitude, G->polor_motion_x,
	     G->polor_motion_y, out_temp, G->current_barometer,
	     humidity, eff_wavelength, tlr, G->app_to_obs_parms);
#endif

   // reset the keypad
   reset_keypad_counters();

   // now reset the encoders for the new position
   tcs_reset_home_position(sessiononly);

   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_move_noupdate
//
//  Purpose....: move the telescope without updating the coordinates
//
// Input......:  ra_sec      - seconds of time in ra (+ to the East)
//               dec_arcsec  - seconds of arc in dec (+ to the North)
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_move_noupdate(double ra_sec, double dec_arcsec)
 {
   if (!G->telescope_initialized)
     return TCSERR_TELENOTINIT;

   if (G->telescope_at_home)
     return TCSERR_TELEATHOME;

   // local variables
   double curr_ha, curr_dec;
   double to_ra, to_dec, new_ha, new_dec, new_rot;
   double delta_ha, delta_dec;
   double curr_az, curr_alt, curr_rot;
   double new_az, new_alt;
   double delta_az, delta_alt, delta_rot;
   double move_time;
   long    steps_x, steps_y, steps_z;

   // calculate our "to" coordinates based on the current mean coordinates
   to_ra = G->current_mean_ra + (DS2R * ra_sec);
   to_dec = G->current_mean_dec + (DAS2R * dec_arcsec);
   sprintf(outbuf,"Using tcs_move_noupdate\n\r",
		    " from: %f %f\n\r"
		    " to: %f %f\n\r",
		    G->current_mean_ra,G->current_mean_dec,
		    to_ra,to_dec);
   writeline(outbuf,2);

   if (G->handpaddle == 2) {
     sprintf(outbuf,"Using tcs_move_to_coordinates");
     writeline(outbuf,1);
     return tcs_move_to_coordinates( to_ra,     // radians
				 to_dec,     // radians
				 G->current_mean_epoch,
				 G->current_mean_pmra,    // radians
				 G->current_mean_pmdec,    // radians
				 G->current_mean_parallax,
				 G->current_mean_radial_velocity,
				 G->current_mean_eff_wavelength,
				 G->current_tlr,0.);
   }

   // get the current time
   struct ALLTIMES timeRec;
   struct mytime t;
   struct date d;

   mygettime(&d,&t);
   //getdate(&d);
   writeline("Start tcs_move_noupdate\0",1);
   get_all_times_at(d, t, timeRec);


   //
   // first, get a rough time estimate for the move
   //
   // get our "to" mount corrected position
   if (!sysGlobal->mount_type)
     // equatorial
     mean_to_mount_corrected(timeRec, to_ra, to_dec,
			     G->current_mean_epoch,
			     G->current_mean_pmra,
			     G->current_mean_pmdec,
			     G->current_mean_parallax,
			     G->current_mean_radial_velocity,
			     new_ha, new_dec, new_rot);
   else
     // alt-az
     mean_to_mount_corrected(timeRec, to_ra, to_dec,
			     G->current_mean_epoch,
			     G->current_mean_pmra,
			     G->current_mean_pmdec,
			     G->current_mean_parallax,
			     G->current_mean_radial_velocity,
			     new_az, new_alt, new_rot);

   // get our "from" position
   if (!sysGlobal->mount_type)
     {
       // equatorial
       mean_to_mount_corrected(timeRec,
			       G->current_mean_ra,
			       G->current_mean_dec,
			       G->current_mean_epoch,
			       G->current_mean_pmra,
			       G->current_mean_pmdec,
			       G->current_mean_parallax,
			       G->current_mean_radial_velocity,
			       curr_ha, curr_dec, curr_rot);

       delta_ha = slaDrange(new_ha - curr_ha);
       delta_dec = new_dec - curr_dec;

       calc_equ_move_time_steps(delta_ha, delta_dec, move_time,
				 steps_x, steps_y);
     }
   else
     {
       // alt-az
       if (G->delaltaz==1) {
	 delta_az =ra_sec;
	 delta_alt = dec_arcsec;
       }
       else {
	 mean_to_mount_corrected(timeRec,
			       G->current_mean_ra,
			       G->current_mean_dec,
			       G->current_mean_epoch,
			       G->current_mean_pmra,
			       G->current_mean_pmdec,
			       G->current_mean_parallax,
			       G->current_mean_radial_velocity,
			       curr_az, curr_alt, curr_rot);

	 delta_az = slaDrange(new_az - curr_az);
	 delta_alt = new_alt - curr_alt;
	 delta_rot = new_rot - curr_rot;
       }

       calc_altaz_move_time_steps(delta_az, delta_alt, delta_rot,
				   move_time,
				   steps_x, steps_y, steps_z);
       sprintf(outbuf,"delta_az: %f\n\r"
		      "delta_alt: %f\n\r"
		      "move_time: %f\n\r"
		      "steps: %ld %ld %ld\n\r",
		      delta_az,delta_alt,
		      move_time,steps_x,steps_y,steps_z);
       writeline(outbuf,2);

     }
   writeline("tcs_move_noupdate: calculated rough time, starting timer",1);

   // now, get the actual stop time using the rough time + a fudge factor
   // start the move timer
   unsigned totalTimerTime = ceil(move_time + sysGlobal->keypad_fudge_factor);
   EventTimer TotalMoveTimer;
   TotalMoveTimer.NewTimerSecs(totalTimerTime);
   sprintf(outbuf,"totalTimerTime: %d",totalTimerTime);
   writeline(outbuf,2);

   // get the "from" time
   mygettime(&d,&t);
   //getdate(&d);

   // stop the tracking
   #ifndef no_hardware
   byte flags =  pcx_x_axis | pcx_y_axis | pcx_z_axis;
   pc38_clear_done_flags(flags);
   pc38_send_commands("ax ca; st; id; ay ca; st; id; az ca; st; id;");
   while (!pc38_done(flags));
   pc38_clear_done_flags(flags);
   #endif
   writeline("tcs_move_noupdate: stopped tracking \0",1);

   get_all_times_at(d, t, timeRec);

   // calculate the ending time/date of the move
   struct mytime endTime;
   struct date endDate;
   struct ALLTIMES endTimeRec;

   get_new_date_time_at(d, t, totalTimerTime, endDate, endTime);
   get_all_times_at(endDate, endTime, endTimeRec);

   // recalculate exact move delta based on the exact end time
   //
   // get exact "from" position
   if (!sysGlobal->mount_type)
     // equatorial
     mean_to_mount_corrected(timeRec,
			     G->current_mean_ra,
			     G->current_mean_dec,
			     G->current_mean_epoch,
			     G->current_mean_pmra,
			     G->current_mean_pmdec,
			     G->current_mean_parallax,
			     G->current_mean_radial_velocity,
			     curr_ha, curr_dec, curr_rot);
   else
     // alt-az
     mean_to_mount_corrected(timeRec,
			     G->current_mean_ra,
			     G->current_mean_dec,
			     G->current_mean_epoch,
			     G->current_mean_pmra,
			     G->current_mean_pmdec,
			     G->current_mean_parallax,
			     G->current_mean_radial_velocity,
			     curr_az, curr_alt, curr_rot);

   if (!sysGlobal->mount_type)
     {
       // equatorial
       mean_to_mount_corrected(endTimeRec, to_ra, to_dec,
			       G->current_mean_epoch,
			       G->current_mean_pmra,
			       G->current_mean_pmdec,
			       G->current_mean_parallax,
			       G->current_mean_radial_velocity,
			       new_ha, new_dec, new_rot);

       delta_ha = slaDrange(new_ha - curr_ha);
       delta_dec = new_dec - curr_dec;

       calc_equ_move_time_steps(delta_ha, delta_dec, move_time,
				 steps_x, steps_y);
     }
   else
     {
       // alt-az
       if (G->delaltaz==1) {
	 delta_az =ra_sec;
	 delta_alt = dec_arcsec;
       }
       else {
	 mean_to_mount_corrected(endTimeRec, to_ra, to_dec,
			       G->current_mean_epoch,
			       G->current_mean_pmra,
			       G->current_mean_pmdec,
			       G->current_mean_parallax,
			       G->current_mean_radial_velocity,
			       new_az, new_alt, new_rot);

	 delta_az = slaDrange(new_az - curr_az);
	 delta_alt = new_alt - curr_alt;
	 delta_rot = new_rot - curr_rot;
       }

       calc_altaz_move_time_steps(delta_az, delta_alt, delta_rot,
				  move_time,
				  steps_x, steps_y, steps_z);
       sprintf(outbuf,"delta_az: %f\n\r"
		      "delta_alt: %f\n\r"
		      "move_time: %f\n\r"
		      "steps: %ld %ld %ld\n\r",
		      delta_az,delta_alt,
		      move_time,steps_x,steps_y,steps_z);
       writeline(outbuf,2);
     }
   writeline("tcs_move_noupdate: calculated steps \0",1);

   // command the move
   move_the_dome = FALSE;
   tcs_move_telescope_steps(steps_x, steps_y, steps_z);
   writeline("tcs_move_noupdate: done move \0",1);

   #ifndef no_hardware
   if (TotalMoveTimer.Expired())
     {
       error_sound();
       sprintf(outbuf,"Keypad move timed out!\n\n");
       writeline(outbuf,0);
     }

   while (!TotalMoveTimer.Expired()) {}
   #endif

   if (G->update==1 && G->delaltaz==0) {
     sprintf(outbuf,"Old coodinates: %f %f",
	     G->current_mean_ra,G->current_mean_dec);
     writeline(outbuf,1);
     G->current_mean_ra = to_ra;
     G->current_mean_dec = to_dec;
     sprintf(outbuf,"New coodinates: %f %f",
	     G->current_mean_ra,G->current_mean_dec);
     writeline(outbuf,1);
   }

   writeline("tcs_move_noupdate: calling start track \0",1);
   update_tracking_rates(FALSE);
   writeline("tcs_move_noupdate: done\0",1);
   write_telescope_position(1);
   return TCSERR_OK;
 }

unsigned old_tcs_move_noupdate(double ra_sec, double dec_arcsec)
 {
   if (!G->telescope_initialized)
     return TCSERR_TELENOTINIT;

   if (G->telescope_at_home)
     return TCSERR_TELEATHOME;

   double ra1, dec1;
   double ra2, dec2;

   // get the current position
   ra1 = G->current_mean_ra;
   dec1 = G->current_mean_dec;

   // get the new position
   ra2 = slaDranrm(ra1 + (ra_sec * DS2R));  // 0 - 2pi
   dec2 = dec1 + (dec_arcsec * DAS2R);

   // calculate the deltas
   struct mytime t;
   struct date d;
   struct ALLTIMES timeRec;

   mygettime(&d,&t);
   //getdate(&d);
   get_all_times_at(d, t, timeRec);

   double ha1, ha2;
   ha1 = slaDrange(timeRec.last - ra1);
   ha2 = slaDrange(timeRec.last - ra2);

   double delta_ha = slaDrange(ha2 - ha1);
   double delta_dec;
   double move_time;
   long x_steps, y_steps, z_steps;
   double az1, az2, alt1, alt2, delta_az, delta_alt;

   if (!sysGlobal->mount_type)
     {
       // equatorial
       delta_dec = dec2 - dec1;
       calc_equ_move_time_steps(delta_ha,
			       delta_dec,
			       move_time,
			       x_steps,
			       y_steps);
     }
   else
     {
       // alt-az
       slaDe2h(ha1, dec1, G->latitude, &az1, &alt1);
       slaDe2h(ha2, dec2, G->latitude, &az2, &alt2);

       delta_az = slaDrange(az2 - az1);
       delta_alt = alt2 - alt1;

       calc_altaz_move_time_steps(delta_az,
				 delta_alt,
				 delta_az,
				 move_time,
				 x_steps,
				 y_steps,
				 z_steps);
     }

   // given the move time, calculate the effect of the sidereal rate on the
   // move and recalculate the move adjusting for this drift
   if (delta_ha != 0.0)
     {
       ra2 += (move_time * gc_stellar_rate);

       // calculate delta hour-angle
       ha2 = slaDrange(timeRec.last - ra2);
       delta_ha = slaDrange(ha2 - ha1);

       if (!sysGlobal->mount_type)
	 {
	   // equatorial
	   calc_equ_move_time_steps(delta_ha,
				   delta_dec,
				   move_time,
				   x_steps,
				   y_steps);
	 }
       else
	 {
	   // alt-az
	   slaDe2h(ha2, dec2, G->latitude, &az2, &alt2);

	   delta_az = slaDrange(az2 - az1);
	   delta_alt = alt2 - alt1;

	   calc_altaz_move_time_steps(delta_az,
				     delta_alt,
				     delta_az,
				     move_time,
				     x_steps,
				     y_steps,
				     z_steps);
	 }
     }

   // calculate the move steps
   if (!sysGlobal->mount_type)
     {
       x_steps = (delta_ha * DR2D) * sysGlobal->x_steps_degree;
       y_steps = (delta_dec * DR2D) * sysGlobal->y_steps_degree;
       z_steps = 0L;
     }
   else
     {
       x_steps = (delta_az * DR2D) * sysGlobal->x_steps_degree;
       y_steps = (delta_alt * DR2D) * sysGlobal->y_steps_degree;
       z_steps = (delta_az * DR2D) * sysGlobal->z_steps_degree;
     }

   // do the move
   #ifdef no_hardware
   unsigned status = TCSERR_OK;
   #else
   unsigned status = tcs_move_telescope_steps(x_steps, y_steps, z_steps);
   #endif

   if (G->update==1) {
     sprintf(outbuf,"Old coodinates: %f %f",
	     G->current_mean_ra,G->current_mean_dec);
     writeline(outbuf,1);
     G->current_mean_ra = ra2;
     G->current_mean_dec = dec2;
     sprintf(outbuf,"New coodinates: %f %f",
	     G->current_mean_ra,G->current_mean_dec);
     writeline(outbuf,1);
   }

   update_tracking_rates();
   write_telescope_position(1);
   return status;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_move_secondary_steps
//
//  Purpose....: move the secondary mirror
//
// Input......:  t, u, v steps
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_move_secondary_steps(long t_axis, long u_axis, long v_axis)
 {
   #ifdef no_hardware
   autoGlobal->t_step_pos += t_axis;
   autoGlobal->u_step_pos += u_axis;
   autoGlobal->v_step_pos += v_axis;
   return TCSERR_OK;
   #else
   unsigned status;
   char buffer[81];

   if (!sysGlobal->tuv_type)
     return TCSERR_NA;

   sprintf(outbuf,"MOVE SECONDARY\n\r"
	 "T Steps = %ld\n\r"
	 "U Steps = %ld\n\r"
	 "V Steps = %ld\n\r",
	  t_axis, u_axis, v_axis);
   writeline(outbuf,1);

   long new_pos_t;
   long new_pos_u;
   long new_pos_v;

   // check travel limits
   new_pos_t = t_axis + autoGlobal->t_step_pos;
   if ((new_pos_t > 0) || (labs(new_pos_t) > sysGlobal->tuv_travel))
     {
       error_sound();
       writeline("T axis out of range.  No move.",0);
       return TCSERR_SECONDARY;
     }

   if (sysGlobal->tuv_type == 1)
     {
       new_pos_u = u_axis + autoGlobal->u_step_pos;
       if ((new_pos_u > 0) || (labs(new_pos_u) > sysGlobal->tuv_travel))
	 {
	   error_sound();
	   writeline("U axis out of range.  No move.",0);
	   return TCSERR_SECONDARY;
	 }

       new_pos_v = v_axis + autoGlobal->v_step_pos;
       if ((new_pos_v > 0) || (labs(new_pos_v) > sysGlobal->tuv_travel))
	 {
	   error_sound();
	   writeline("V axis out of range.  No move.",0);
	   return TCSERR_SECONDARY;
	 }

       // check tilt limits
       long check = labs(new_pos_t - new_pos_u);
       BOOL tilt_excess = (check > sysGlobal->tuv_tilt);
       check = labs(new_pos_t - new_pos_v);
       tilt_excess |= (check > sysGlobal->tuv_tilt);
       check = labs(new_pos_u - new_pos_v);
       tilt_excess |= (check > sysGlobal->tuv_tilt);

       if (tilt_excess)
	 {
	   error_sound();
	   writeline("Tilt limit exceeded.  No Move.",0);
	   return TCSERR_SECONDARY;
	 }
     }

   // command the moves
   BOOL t_done = TRUE;
   BOOL u_done = TRUE;
   BOOL v_done = TRUE;

   if (t_axis)
     {
     sprintf(buffer, "at an; lf; ac%u; vl%u; mr%ld; gd; id;",
	       sysGlobal->tuv_acceleration,
	       sysGlobal->tuv_max_velocity,
	       t_axis);
     pc38_send_commands(buffer);
     t_done = FALSE;
     }

   if (sysGlobal->tuv_type == 1)
     {
       if (u_axis)
	 {
	 sprintf(buffer, "au an; lf; ac%u; vl%u; mr%ld; gd; id;",
		   sysGlobal->tuv_acceleration,
		   sysGlobal->tuv_max_velocity,
		   u_axis);
	 pc38_send_commands(buffer);
	 u_done = FALSE;
	 }

       if (v_axis)
	 {
	 sprintf(buffer, "av an; lf; ac%u; vl%u; mr%ld; gd; id;",
		   sysGlobal->tuv_acceleration,
		   sysGlobal->tuv_max_velocity,
		   v_axis);
	 pc38_send_commands(buffer);
	 v_done = FALSE;
	 }
     }

   // wait for done flags
   AXISPACK axisRec;
   do
     {
       // check shutdown / reset watchdog
       if (!check_priority(status))
	 return status;

       // update the tracking rates
       update_tracking_rates();

       if (!t_done)
	 {
	   pc38_get_axis_status('t', axisRec);
	   t_done = axisRec.done;
	 }

       if (!u_done)
	 {
	   pc38_get_axis_status('u', axisRec);
	   u_done = axisRec.done;
	 }

       if (!v_done)
	 {
	   pc38_get_axis_status('v', axisRec);
	   v_done = axisRec.done;
	 }
     }
   while (!(t_done && u_done && v_done));
   pc38_clear_done_flags(pcx_t_axis | pcx_u_axis | pcx_v_axis);

   sprintf(buffer, "at af; au af; av af;");
   pc38_send_commands(buffer);

   autoGlobal->t_step_pos += t_axis;
   autoGlobal->u_step_pos += u_axis;
   autoGlobal->v_step_pos += v_axis;
   writetoccscf(autoGlobal);
   return TCSERR_OK;
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_move_telescope_steps
//
//  Purpose....: move the telescope in the 3 axes
//
// Input......:  x, y, and z steps
//
//               NOTE:  The +/- convention here is +x = increasing RA or
//                      azimuth, +y = increasing Dec or Altitude,
//                     and +z = increasing rotator angle
//
//                     This routine takes care of the geartrain reversals
//                     to make sure that the telescope moves in the desired
//                     direction.
//
//               NOTE: For those axes that have encoders enabled, the move
//                     will be performed using the encoder and will take
//                     two moves to complete.
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_move_telescope_steps(long x_axis, long y_axis, long z_axis,
				 EventTimer *timer)
 {
//   #ifndef no_hardware
   unsigned status;
   BOOL finished;
//   #endif

   // if this is an equatorial, zero out the z_axis
   z_axis = (sysGlobal->mount_type) ? z_axis : 0;

   if (!sysGlobal->z_axis_enabled)
     z_axis = 0L;

   sprintf(outbuf,"*** MOVE TELESCOPE STEPS ***\n\r"
		  "x = %ld\r\n" 
		  "y = %ld\r\n"
		  "z = %ld\r\n",
		  x_axis, y_axis, z_axis);
   writeline(outbuf,1);

   // if all axes are 0, exit
   if ((!x_axis) && (!y_axis) && (!z_axis))
     return TCSERR_OK;

   writeline("Encoder positions in move_telescope_steps",1);
   sprintf(outbuf,
	  "x: %ld\n\r"
	  "y: %ld\n\r"
	  "z: %ld\n\r",
	  tcs_return_encoder_position('x'),
	  tcs_return_encoder_position('y'),
	  tcs_return_encoder_position('z'));
   writeline(outbuf,1);

   // check reversals to get actual card direction of move
   x_axis = (sysGlobal->x_geartrain_normal) ? x_axis : -x_axis;
   y_axis = (sysGlobal->y_geartrain_normal) ? y_axis : -y_axis;
   z_axis = (sysGlobal->z_geartrain_normal) ? z_axis : -z_axis;

   sprintf(outbuf,  "Move steps (geartrain modified):\n"
		    "\tx steps - %ld\n\r"
		    "\ty steps - %ld\n\r"
		    "\tz steps - %ld\n\r",
		    x_axis,
		    y_axis,
		    z_axis);
   writeline(outbuf,2);

   // test the move against the soft limits which are in card coordinates
   if (x_axis)
     {
       x_axis += tcs_return_step_position('x');
       if ((x_axis <= sysGlobal->x_neg_soft_limit) ||
	   (x_axis >= sysGlobal->x_pos_soft_limit))
	 {
	   error_sound();
	   writeline("Move would run telescope into the X soft limit!  "
		     "No move made.",0);
	   return TCSERR_MOVELIMIT;
	 }
     }

   if (y_axis)
     {
       y_axis += tcs_return_step_position('y');
       if ((y_axis <= sysGlobal->y_neg_soft_limit) ||
	   (y_axis >= sysGlobal->y_pos_soft_limit))
	 {
	   error_sound();
	   writeline("Move would run telescope into the Y soft limit!  "
		     "No move made.",0);
	   return TCSERR_MOVELIMIT;
	 }
     }

   if (z_axis) 
     {
       z_axis += tcs_return_step_position('z');
       if ((z_axis <= sysGlobal->z_neg_soft_limit) ||
	   (z_axis >= sysGlobal->z_pos_soft_limit))
	 {
	   error_sound();
	   writeline("Move would run telescope into the Z soft limit!  "
		     "No move made.",0);
	   return TCSERR_MOVELIMIT;
	 }
     }

   //  create the move strings now
   char x_command[129] = "";
   char x_commandb[129] = "";

   char y_command[129] = "";
   char y_commandb[129] = "";

   char z_command[129] = "";

   long new_pos;

   if (x_axis)
     {
       if (sysGlobal->x_encoder_installed)
	 {
	   // encoder enabled
	   new_pos = long(G->x_enc_to_motor * (double)x_axis);

	   // set up first move string
	   sprintf(x_command, ENECHOCH"ax vl%ld; ac%ld; er%ld,%ld; "
			      "hv%ld; hg%u; hd%u; hn; "
			      "ma%ld; gd; ip;",
		   sysGlobal->x_max_velocity,
		   sysGlobal->x_acceleration,
		   sysGlobal->x_encoder_encoder_steps_rev,
		   sysGlobal->x_encoder_motor_steps_rev,
		   sysGlobal->x_encoder_slew_hold_vel,
		   sysGlobal->x_encoder_slew_hold_gain,
		   sysGlobal->x_encoder_slew_hold_deadband,
		   new_pos);
	   writeline(x_command,1);

	   // set up final move string
	   sprintf(x_commandb, ENECHOCH"ax; ca; hv%ld; hg%u; hd%u; ip;",
		   sysGlobal->x_encoder_fine_hold_vel,
		   sysGlobal->x_encoder_fine_hold_gain,
		   sysGlobal->x_encoder_fine_hold_deadband);
	   writeline(x_commandb,1);
	 }
       else
	 // no encoder
	 sprintf(x_command, "ax vl%ld; ac%ld; ma%ld; gd; id;",
		 sysGlobal->x_max_velocity,
		 sysGlobal->x_acceleration,
		 x_axis);
     }

   if (y_axis)
     {
       if (sysGlobal->y_encoder_installed)
	 {
	   // encoder enabled
	   new_pos = long(G->y_enc_to_motor * (double)y_axis);

	   // set up first move string
	   sprintf(y_command, ENECHOCH"ay vl%ld; ac%ld; er%ld,%ld; "
			      "hv%ld; hg%u; hd%u; hn; "
			      "ma%ld; gd; ip;",
		   sysGlobal->y_max_velocity,
		   sysGlobal->y_acceleration,
		   sysGlobal->y_encoder_encoder_steps_rev,
		   sysGlobal->y_encoder_motor_steps_rev,
		   sysGlobal->y_encoder_slew_hold_vel,
		   sysGlobal->y_encoder_slew_hold_gain,
		   sysGlobal->y_encoder_slew_hold_deadband,
		   new_pos);
	   writeline(y_command,1);

	   // set up final move string
	   sprintf(y_commandb, ENECHOCH"ay; ca; hv%ld; hg%u; hd%u; ip;",
		   sysGlobal->y_encoder_fine_hold_vel,
		   sysGlobal->y_encoder_fine_hold_gain,
		   sysGlobal->y_encoder_fine_hold_deadband);
	   writeline(y_commandb,1);
	 }
       else
	 // no encoder
	 sprintf(y_command, "ay vl%ld; ac%ld; ma%ld; gd; id;",
		 sysGlobal->y_max_velocity,
		 sysGlobal->y_acceleration,
		 y_axis);
     }

   if (z_axis)
     sprintf(z_command, "az vl%ld; ac%ld; ma%ld; gd; id;",
	       sysGlobal->z_max_velocity,
	       sysGlobal->z_acceleration,
	       z_axis);

   // stop the axes that are moving and wait until they are done
//   #ifndef no_hardware
   BOOL x_done = !x_axis;
   BOOL y_done = !y_axis;
   BOOL z_done = !z_axis;

   if (x_axis)
     pc38_send_commands("ax ca; st; id;");

   if (y_axis)
     pc38_send_commands("ay ca; st; id;");

   if (z_axis)
     pc38_send_commands("az ca; st; id;");

   while (!(x_done && y_done && z_done))
     {
       if (x_axis)
	 x_done = pc38_done(pcx_x_axis);

       if (y_axis)
	 y_done = pc38_done(pcx_y_axis);

       if (z_axis)
	 z_done = pc38_done(pcx_z_axis);
       }
   pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);

   // start the dome if the dome is slaved
   if (move_the_dome)
     ocs_start_dome_move();

   // command the moves
   if (x_axis)
     pc38_send_commands(x_command);

   if (y_axis && !G->y_axis_separate)
     pc38_send_commands(y_command);

   if (sysGlobal->z_axis_enabled)
     if (z_axis)
       pc38_send_commands(z_command);

   x_done = !x_axis;
   if (G->y_axis_separate) 
     y_done = TRUE;
   else
     y_done = !y_axis;
   z_done = !z_axis;

   if (!sysGlobal->z_axis_enabled)
     z_done = TRUE;

   SysTimer[SYSTMR_REMOTE].NewTimer(SYSTMR_REMOTE_INC);
   do
     {
       if (timer && timer->Expired())
	 {
	   sprintf(outbuf,"timer expired in move_telescope_steps");
	   writeline(outbuf,1);
	   tcs_telescope_stop(TRUE);
	   pc38_send_commands(ENECHOCH"ax hf; er1,1; ay hf; er1,1;");
	   return TCSERR_MOVETIMEOUT;
	 }

       // Check to see if we need to shut down
       if (!check_priority(status))
	 return status;

       // Move the dome if necessary
       if (move_the_dome)
	 ocs_dome_finished(finished);

       // Update the telescope position and send it to the remote
       update_display();
       update_status(0);

       if (!x_done)
	 x_done = pc38_done(pcx_x_axis);

       if (!y_done)
	 y_done = pc38_done(pcx_y_axis);

       if (!z_done)
	 z_done = pc38_done(pcx_z_axis);
     }
   while (!(x_done && y_done && z_done));

   if (G->y_axis_separate) {
     y_done = !y_axis;
     pc38_send_commands(y_command);
     do {
       // Check to see if we need to shut down
       if (!check_priority(status))
	 return status;
       // Update the telescope position and send it to the remote
       update_display();
       update_status(0);
       if (!y_done) y_done = pc38_done(pcx_y_axis);
     } while (!y_done);
   }

   pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);

   // if the encoders are enabled, do the fine tuning move
   x_done = TRUE;
   y_done = TRUE;

   sprintf(outbuf,"fine tuning move in move_telescope_steps");
   writeline(outbuf,1);

   if (x_axis && sysGlobal->x_encoder_installed)
     {
       x_done = FALSE;
       pc38_send_commands(x_commandb);
     }

   if (y_axis && sysGlobal->y_encoder_installed)
     {
       y_done = FALSE;
       pc38_send_commands(y_commandb);
     }

   if (sysGlobal->x_encoder_installed || sysGlobal->y_encoder_installed)
     {
       while (!(x_done && y_done))
	 {
	   if (timer && timer->Expired())
	     {
             sprintf(outbuf,"time out in fine tuning move in move_telescope_steps");
             writeline(outbuf,1);

	     tcs_telescope_stop(TRUE);
	     pc38_send_commands(ENECHOCH"ax hf; er1,1; ay hf; er1,1;");
	     return TCSERR_MOVETIMEOUT;
	     }

	   if (!x_done)
	     x_done = pc38_done(pcx_x_axis);

	   if (!y_done)
	     y_done = pc38_done(pcx_y_axis);
	 }
     }
   pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);

   // turn position mantainence mode off
   pc38_send_commands(ENECHOCH"ax hf; er1,1; ay hf; er1,1;");

   // save the new Z position
   if ((sysGlobal->mount_type) && (sysGlobal->z_axis_enabled))
     tcs_store_z_axis_pos();
//   #endif
   writeline("Encoder positions at end of move_telescope_steps",1);
   sprintf(outbuf,
	  "x: %ld\n\r"
	  "y: %ld\n\r"
	  "z: %ld\n\r",
	  tcs_return_encoder_position('x'),
	  tcs_return_encoder_position('y'),
	  tcs_return_encoder_position('z'));
   writeline(outbuf,1);

   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_move_to_az_alt
//
//  Purpose....: move the telescope to new az/alt
//
// Input......:  (see list)
//
// Output.....:  error code
//
// NOTE:    This routine is not very accurate and is really only used
//         for debugging.
//
//------------------------------------------------------------------
unsigned tcs_move_to_az_alt(double az, double alt)
 {
   double ha, ra, dec;
   struct mytime t;
   struct date d;
   struct ALLTIMES timeRec;

   mygettime(&d,&t);
   //getdate(&d);
   get_all_times_at(d, t, timeRec);

   slaDh2e(az, alt, G->latitude, &ha, &dec);

   ra = slaDranrm(timeRec.last - ha);

   sprintf(outbuf,  "\nMOVE TO ALT AZ\n\r"
			 "--------------\n\r");
   writeline(outbuf,2);
   sprintf(outbuf,  "Azimuth       : %.10f\n\r"
		    "Altitude      : %.10f\n\r"
		    "<Hour-angle>  : %.10f\n\r"
		    "<Declination> : %.10f\n\r",
		    az,
		    alt,
		    (ha * DR2H),
		    (dec * DR2D));
   writeline(outbuf,2);
   return tcs_move_to_coordinates(ra, dec, timeRec.equinox,
				   0, 0, 0, 0, 0.55, 0.0065, 0.);
 }

//------------------------------------------------------------------
//  Name.......:  tcs_move_to_coordinates
//
//  Purpose....: move the telescope to new coordinates
//
// Input......:  (see list)
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_move_to_coordinates( double ra,     // radians
				 double dec,     // radians
				 double epoch,
				 double pmra,    // radians
				 double pmdec,    // radians
				 double parallax,
				 double radial_velocity,
				 double eff_wavelength,
				 double tlr,
				 double pa)

 {
   double new_ha, new_dec;
   double curr_ha, curr_dec;
   double delta_ha, delta_dec;

   double new_az, new_alt, new_rot;
   double curr_az, curr_alt, curr_rot;
   double delta_az, delta_alt, delta_rot;

   double move_time;
   long steps_x, steps_y, steps_z;

   double new_dome_az;

   struct ALLTIMES timeRec, endTimeRec;
   struct mytime t, endTime;
   struct date d, endDate;

   struct WEATHERPACK weather;

   BOOL stopped;

   double *save_mtoa_parms_to, *save_mtoa_parms_from;

   sprintf(outbuf,  "\nMOVE TO COORDINATES\n\r"
			   "-------------------\n\r"
			   "RA              : %.10f\n\r"
			   "Dec             : %.10f\n\r"
			   "Epoch           : %.10f\n\r"
			   "PMRA            : %.10f\n\r"
			   "PMDEC           : %.10f\n\r"
			   "Parallax        : %.10f\n\r"
			   "Radial Velocity : %.10f\n\r"
			   "Eff wavelength  : %.10f\n\r"
			   "TLR             : %.10f\n\r",
			   (ra * DR2H),
			   (dec * DR2D),
			   epoch,
			   (pmra * DR2S),
			   (pmdec * DR2AS),
			   parallax,
			   radial_velocity,
			   eff_wavelength,
			   tlr);
   writeline(outbuf,2);
tcs_math_verbose = 0;

   // if the telescope has not been initialized, exit
   if (!G->telescope_initialized) {
       sprintf(outbuf,  "Error - Telescope not initialized");
       writeline(outbuf,0);
       return TCSERR_TELENOTINIT;
   }

   // If we are shutdown, exit
   if (G->shutdown_state) {
       sprintf(outbuf,  "Error - Telescope is current in shutdown state");
       writeline(outbuf,0);
       return TCSERR_WEATHERSHUT;
   }

   // Update the WWV clock
   update_date_time();

   // get the current date/time

   mygettime(&d,&t);
   //getdate(&d);
   get_all_times_at(d, t, timeRec);

   // update slalib parameters for the new position

/*
   sprintf(outbuf,  "Current time/date\n\r"
			 "\t%0.2d:%0.2d:%0.2d.%0.2d  %0.2d.%0.2d.%d\n\r",
			 t.ti_hour,
			 t.ti_min,
			 t.ti_sec,
			 t.ti_hund,
			 d.da_day,
			 d.da_mon,
			 d.da_year);
   writeline(outbuf,2);

   sprintf(outbuf, "Current time record\n\r"
			 "\tmjd_tt         : %.10f\n\r"
			 "\tmjd_utc        : %.10f\n\r"
			 "\tLAST (radians) : %.10f\n\r"
			 "\tLAST (hours)   : %.10f\n\r"
			 "\tequinox        : %.10f\n\r",
			 timeRec.mjd_tt,
			 timeRec.mjd_utc,
			 timeRec.last,
			 timeRec.lasth,
			 timeRec.equinox);
   writeline(outbuf,2);

   sprintf(outbuf, "Current outside temperature (C): %.2f\n\r"
			 "Current humidity               : %.2f\n\r"
			 "Current barometer (mBar)       : %.2f\n\r",
			G->current_out_temp,
			 G->current_humidity,
			 G->current_barometer);
   writeline(outbuf,2);
*/

   // it is possible that the user is moving between two stars
   // of differing epochs.  So, save the "from" and "to" parameters since
   // other routines use the quick conversions which depend on the
   // parameters being correct for the given star.
   //
   // save the "from" parameters
   sprintf(outbuf, "Calculating \"from\" SLALIB parameters");
   writeline(outbuf,1);
   if (G->telescope_at_home == 1)
     slaMappa(timeRec.equinox, timeRec.mjd_tt, G->mean_to_app_parms);
   else
     slaMappa(G->current_mean_epoch, timeRec.mjd_tt, G->mean_to_app_parms);

   save_mtoa_parms_from =
	   (double *)malloc(sizeof(G->mean_to_app_parms));
   memcpy(save_mtoa_parms_from, G->mean_to_app_parms,
	     sizeof(G->mean_to_app_parms));

   // save the "to" parameters
   //if (epoch != G->current_mean_epoch && !G->telescope_at_home) {
   //if (epoch != G->current_mean_epoch || G->telescope_at_home) {
     sprintf(outbuf,"Calculating \"to\" SLALIB parameters for epoch %f", epoch);
     writeline(outbuf,1);
     slaMappa(epoch, timeRec.mjd_tt, G->mean_to_app_parms);
   //}
   save_mtoa_parms_to =
	   (double *)malloc(sizeof(G->mean_to_app_parms));
   memcpy(save_mtoa_parms_to, G->mean_to_app_parms,
	     sizeof(G->mean_to_app_parms));

   // calculate the apparent to observed parameters, which do not depend
   // on the epoch
   sprintf(outbuf,"Calculating apparent to observed parameters");
   writeline(outbuf,1);
#ifdef AOPPAT
   slaAoppat(timeRec.mjd_utc, G->app_to_obs_parms);
#else
   unsigned status = read_weather_station(weather);
   double out_temp = G->current_out_temp + 273.15;
   double humidity = G->current_humidity / 100.0;
   slaAoppa(timeRec.mjd_utc, G->ut1_minus_utc, G->longitude,
	     G->latitude, sysGlobal->altitude, G->polor_motion_x,
	     G->polor_motion_y, out_temp, G->current_barometer,
	     humidity, eff_wavelength, tlr,
	     G->app_to_obs_parms);
#endif

   // Set the desired pa
   double parang;
   if (pa < -360)  {
     mean_to_mount_corrected_sub(timeRec, ra, dec, epoch, pmra, pmdec,
			       parallax, radial_velocity,
			       new_az, new_alt, new_rot, parang);
     update_encoder_position();
     steps_to_degrees(
	       G->current_enc_az,G->current_enc_alt,G->current_enc_rot,
	       curr_az,curr_alt,curr_rot);
     curr_rot *= DD2R;
     G->current_pa = curr_rot + new_alt + parang;
   } else {
     G->current_pa = pa * DD2R;
   }
   sprintf(outbuf,"pa: %f %f %f %f %f",pa,curr_rot,new_alt,parang,G->current_pa);
   writeline(outbuf,1);

   // get the corrected new coordinates to check the window
   sprintf(outbuf,"Checking the window");
   writeline(outbuf,1);
   if (!sysGlobal->mount_type) {
       mean_to_mount_corrected(timeRec, ra, dec, epoch, pmra, pmdec,
			       parallax, radial_velocity,
			       new_ha, new_dec, new_rot);
       slaDe2h(new_ha, new_dec, G->latitude, &new_az, &new_alt);
       if (!is_star_in_window(new_az, new_alt)) {
	   free(save_mtoa_parms_to);
	   free(save_mtoa_parms_from);
	   sprintf(outbuf,  "Error - star is out of the window");
	   writeline(outbuf,0);
	   writeline(outbuf,2);
	   return TCSERR_MOVEWINDOW;
       }
   } else {
       double parang;
       mean_to_mount_corrected_sub(timeRec, ra, dec, epoch, pmra, pmdec,
			       parallax, radial_velocity,
			       new_az, new_alt, new_rot, parang);

       if (!is_star_in_window(new_az, new_alt)) {
	   free(save_mtoa_parms_to);
	   free(save_mtoa_parms_from);
	   sprintf(outbuf,  "Error - star is out of the window");
	   writeline(outbuf,0);
	   writeline(outbuf,2);
	   return TCSERR_MOVEWINDOW;
       }
   }

   // make sure that the G->mean_to_app_parms contains the parameters for
   // the "from" star for the mean_to_mount_coorrected conversion
   memcpy(G->mean_to_app_parms, save_mtoa_parms_from,
	   sizeof(G->mean_to_app_parms));

   // get a rough time estimate for the move
   //
   sprintf(outbuf,"Getting rough time estimate");
   writeline(outbuf,1);

   // get our "from" position
   if (!sysGlobal->mount_type) {
       // equatorial
       //
       if (G->telescope_at_home != 1)
	 mean_to_mount_corrected(timeRec,
				 G->current_mean_ra,
				 G->current_mean_dec,
				 G->current_mean_epoch,
				 G->current_mean_pmra,
				 G->current_mean_pmdec,
				 G->current_mean_parallax,
				 G->current_mean_radial_velocity,
				 curr_ha, curr_dec, curr_rot);
       else {
	   curr_ha = autoGlobal->home_ha * DH2R;
	   curr_dec = autoGlobal->home_dec * DD2R;
       }

       delta_ha = slaDrange(new_ha - curr_ha);
       delta_dec = new_dec - curr_dec;

       sprintf(outbuf, "\nRough time estimate\n\r"
			       "-------------------\n\r"
			     "\tNew HA      : %.10f\n\r"
			     "\tCurrent HA  : %.10f\n\r"
			     "\tDelta HA    : %.10f\n\r"
			     "\tNew Dec     : %.10f\n\r"
			     "\tCurrent Dec : %.10f\n\r"
			     "\tDelta Dec   : %.10f\n\r",
			     (new_ha * DR2H),
			     (curr_ha * DR2H),
			     (delta_ha * DR2H),
			     (new_dec * DR2D),
			     (curr_dec * DR2D),
			     (delta_dec * DR2D));
       writeline(outbuf,2);

       calc_equ_move_time_steps(delta_ha, delta_dec, move_time,
				 steps_x, steps_y);
   } else {
       // alt-az. Get current position either from:
       //       1. encoders if option enabled
       //       2. current ra/dec
       //       3. home position if at home
       //
       if (G->use_encoders) {
	   update_encoder_position();
	   steps_to_degrees(
	       G->current_enc_az,G->current_enc_alt,G->current_enc_rot,
	       curr_az,curr_alt,curr_rot);
	   curr_az *= DD2R;
	   curr_alt *= DD2R;
	   curr_rot *= DD2R;
       } else if (G->telescope_at_home != 1)
	   mean_to_mount_corrected(timeRec,
				 G->current_mean_ra,
				 G->current_mean_dec,
				 G->current_mean_epoch,
				 G->current_mean_pmra,
				 G->current_mean_pmdec,
				 G->current_mean_parallax,
				 G->current_mean_radial_velocity,
				 curr_az, curr_alt, curr_rot);
       else {
	   curr_ha = autoGlobal->home_ha * DH2R;
	   curr_dec = autoGlobal->home_dec * DD2R;
	   slaDe2h(curr_ha, curr_dec, G->latitude, &curr_az, &curr_alt);
       }

       calc_alt_az_deltas(curr_az, curr_alt, curr_rot, 
			  new_az, new_alt, new_rot,
			  delta_az, delta_alt, delta_rot);

       if (sysGlobal->z_axis_enabled)
	 calc_altaz_move_time_steps(delta_az, delta_alt, delta_rot,
			   move_time,
			   steps_x, steps_y, steps_z);
       else
	 calc_altaz_move_time_steps(delta_az, delta_alt, 0,
			   move_time,
			   steps_x, steps_y, steps_z);
       sprintf(outbuf,"delta_az: %f\n\r"
		      "delta_alt: %f\n\r"
		      "delta_rot: %f\n\r"
		      "move_time: %f\n\r"
		      "steps: %ld %ld %ld\n\r",
		      delta_az,delta_alt,delta_rot,
		      move_time,steps_x,steps_y,steps_z);
       writeline(outbuf,2);
     }

   // start the move timer
   unsigned totalTimerTime = 
     (unsigned) ceil(sysGlobal->move_fudge_factor + move_time);
   long stopCheckingTime = Secs2Tics(totalTimerTime);  // in tics
   stopCheckingTime -= 9;

   sprintf(outbuf,"Move time: %f\n\r"
		  "Factor: %d\n\r"
		  "-------------\n\r"
		  "Total time: %d\n\r",
		  move_time,sysGlobal->move_fudge_factor,totalTimerTime);
   writeline(outbuf,1);


//   #ifndef no_hardware
   EventTimer TotalMoveTimer;
   EventTimer StopCheckingTimer;
   TotalMoveTimer.NewTimerSecs(totalTimerTime);

   if (stopCheckingTime > 0)
     StopCheckingTimer.NewTimer(stopCheckingTime);
//   #endif

   // get the "from" time
   mygettime(&d,&t);
   // getdate(&d);

   // stop the tracking
   sprintf(outbuf,"Stopping the telescope\n");
   writeline(outbuf,1);
//   #ifndef no_hardware
   tcs_telescope_stop();
//   #endif
   get_all_times_at(d, t, timeRec);

/*
   sprintf(outbuf,  "\nStart time/date\n\r"
			 "\t%0.2d:%0.2d:%0.2d.%0.2d  %0.2d.%0.2d.%d\n\r",
			 t.ti_hour,
			 t.ti_min,
			 t.ti_sec,
			 t.ti_hund,
			 d.da_day,
			 d.da_mon,
			 d.da_year);
   writeline(outbuf,2);

   sprintf(outbuf, "Start time record\n\r"
			 "\tmjd_tt         : %.10f\n\r"
			 "\tmjd_utc        : %.10f\n\r"
			 "\tLAST (radians) : %.10f\n\r"
			 "\tLAST (hours)   : %.10f\n\r"
			 "\tequinox        : %.10f\n\r",
			 timeRec.mjd_tt,
			 timeRec.mjd_utc,
			 timeRec.last,
			 timeRec.lasth,
			 timeRec.equinox);
   writeline(outbuf,2);
*/
   // calculate the ending time/date of the move. For the move time, use
   // calculated move time + 3 seconds for encoder fine tuning - dont use
   // fudge factor since we're not going to wait for the timer to expire
   // to start tracking

   get_new_date_time_at(d, t, totalTimerTime-sysGlobal->move_fudge_factor+3, 
			endDate, endTime);
   get_all_times_at(endDate, endTime, endTimeRec);

   sprintf(outbuf, "Total move time (secs) = %u", 
	totalTimerTime-sysGlobal->move_fudge_factor+3);
   writeline(outbuf,2);

/*
   sprintf(outbuf,  "End time/date\n\r"
			 "\t%0.2d:%0.2d:%0.2d.%0.2d  %0.2d.%0.2d.%d\n\r",
			 endTime.ti_hour,
			 endTime.ti_min,
			 endTime.ti_sec,
			 endTime.ti_hund,
			 endDate.da_day,
			 endDate.da_mon,
			 endDate.da_year);
   writeline(outbuf,2);

   sprintf(outbuf, "End time record\n\r"
			 "\tmjd_tt         : %.10f\n\r"
			 "\tmjd_utc        : %.10f\n\r"
			 "\tLAST (radians) : %.10f\n\r"
			 "\tLAST (hours)   : %.10f\n\r"
			 "\tequinox        : %.10f\n\r",
			 endTimeRec.mjd_tt,
			 endTimeRec.mjd_utc,
			 endTimeRec.last,
			 endTimeRec.lasth,
			 endTimeRec.equinox);
   writeline(outbuf,2);
*/
   // recalculate exact move delta based on the exact end time
   //
   // get exact "from" position - if we are at home, we already have it
   sprintf(outbuf,"Recalculating exact \"from\" and \"to\" positions");
   writeline(outbuf,1);
   long enc_x, enc_y;

   if (G->use_encoders) {
       update_encoder_position();

       steps_to_degrees(G->current_enc_az,G->current_enc_alt,G->current_enc_rot,
			   curr_az,curr_alt,curr_rot);

       curr_az *= DD2R;
       curr_alt *= DD2R;
       curr_rot *= DD2R;
   } else if (G->telescope_at_home != 1)
     if (!sysGlobal->mount_type)
       // equatorial
       //
       mean_to_mount_corrected(timeRec,
			       G->current_mean_ra,
			       G->current_mean_dec,
			       G->current_mean_epoch,
			       G->current_mean_pmra,
			       G->current_mean_pmdec,
			       G->current_mean_parallax,
			       G->current_mean_radial_velocity,
			       curr_ha, curr_dec, curr_rot);
     else
       // alt-az
       //
       mean_to_mount_corrected(timeRec,
			       G->current_mean_ra,
			       G->current_mean_dec,
			       G->current_mean_epoch,
			       G->current_mean_pmra,
			       G->current_mean_pmdec,
			       G->current_mean_parallax,
			       G->current_mean_radial_velocity,
			       curr_az, curr_alt, curr_rot);

   // get exact "to" position - calculate exact steps to move
   memcpy(G->mean_to_app_parms, save_mtoa_parms_to,
	   sizeof(G->mean_to_app_parms));

   if (!sysGlobal->mount_type) {
       // equatorial
       mean_to_mount_corrected(endTimeRec, ra, dec, epoch, pmra, pmdec,
			       parallax, radial_velocity,
			       new_ha, new_dec, new_rot);

       sprintf(outbuf, "\nExact time estimate\n\r"
			       "-------------------\n\r"
			     "\tNew HA      : %.10f\n\r"
			     "\tCurrent HA  : %.10f\n\r"
			     "\tDelta HA    : %.10f\n\r"
			     "\tNew Dec     : %.10f\n\r"
			     "\tCurrent Dec : %.10f\n\r"
			     "\tDelta Dec   : %.10f\n\r",
			     (new_ha * DR2H),
			     (curr_ha * DR2H),
			     (delta_ha * DR2H),
			     (new_dec * DR2D),
			     (curr_dec * DR2D),
			     (delta_dec * DR2D));
       writeline(outbuf,2);

       delta_ha = slaDrange(new_ha - curr_ha);
       delta_dec = new_dec - curr_dec;

       sprintf(outbuf,"Calculating exact move time");
       writeline(outbuf,1);
       calc_equ_move_time_steps(delta_ha, delta_dec, move_time,
				 steps_x, steps_y);

       // get new azimuth
       slaDe2h(new_ha, new_dec, G->latitude, &new_az, &new_alt);
   } else {
       // alt-az
       mean_to_mount_corrected(endTimeRec, ra, dec, epoch, pmra, pmdec,
			       parallax, radial_velocity,
			       new_az, new_alt, new_rot);

       sprintf(outbuf,"Calculating exact move time");
       writeline(outbuf,1);
       calc_alt_az_deltas(curr_az, curr_alt, curr_rot,
			  new_az, new_alt, new_rot,
			  delta_az, delta_alt, delta_rot);

       if (sysGlobal->z_axis_enabled)
	 calc_altaz_move_time_steps(delta_az, delta_alt, delta_rot,
			    move_time,
			    steps_x, steps_y, steps_z);
       else
	 calc_altaz_move_time_steps(delta_az, delta_alt, 0,
			    move_time,
			    steps_x, steps_y, steps_z);
       sprintf(outbuf,"delta_az: %f\n\r"
		      "delta_alt: %f\n\r"
		      "delta_rot: %f\n\r"
		      "move_time: %f\n\r"
		      "steps: %ld %ld %ld\n\r",
		      delta_az,delta_alt,delta_rot,
		      move_time,steps_x,steps_y,steps_z);
       writeline(outbuf,2);

   }

   // make sure that we don't try to update the tracking rates anywhere
   G->telescope_is_slewing = TRUE;

   // if the dome is slaved, set up dome move calculations
   move_the_dome = FALSE;

   #ifndef no_hardware
   if ((sysGlobal->enclosure_type == 2) && (autoGlobal->dome_slaved))
     {
       new_dome_az = new_az * DR2D;
       G->dome_azimuth = ocs_return_dome_azimuth();
       double dome_dif = slaDrange((new_dome_az - G->dome_azimuth) * DD2R);
       dome_dif = fabs(dome_dif) * DR2D;

       if (dome_dif >= sysGlobal->shutter_size) {
	 move_the_dome = TRUE;
	 ocs_calc_dome_counts(new_dome_az);
       }
     }
   #endif

   // command the move. tcs_move_telescope_steps won't return until move is 
   //   done or timed out. It does, however, start the dome moving.
   BOOL finished = FALSE;
   sprintf(outbuf,"Moving the telescope\n");
   writeline(outbuf,1);
   G->nmove = 1;
   status = tcs_move_telescope_steps(steps_x, steps_y, steps_z,
					       &TotalMoveTimer);
   sprintf(outbuf,"move telescope steps returns: %d",status);
   writeline(outbuf,1);

   // Was there an error? If so, deinit the telescope, but finish the dome move
   if (status) {
       if ((status == TCSERR_MOVETIMEOUT)) {
	 if (move_the_dome) {
	   while (!finished) ocs_dome_finished(finished);
	 }
	 autoGlobal->dome_azimuth = new_dome_az;
	 move_the_dome = FALSE;
	 tcs_set_deinit_telescope();
       }

       free(save_mtoa_parms_to);
       free(save_mtoa_parms_from);
       return status;
   }

   // if either move timer has alredy expired, abort and return the
   // appropriate error code
   if (StopCheckingTimer.Expired() || TotalMoveTimer.Expired()) {
       tcs_set_deinit_telescope();
       if (move_the_dome)
	 while (!finished)
	   ocs_dome_finished(finished);
       autoGlobal->dome_azimuth = new_dome_az;
       move_the_dome = FALSE;
       free(save_mtoa_parms_to);
       free(save_mtoa_parms_from);
       return TCSERR_MOVETIMEOUT;
   }

   // If we're not checking against encoders, sit around and wait until the 
   //    timer has expired, so we know we'll be in the right place at 
   //    the right time.
   // If we are using encoders, see how close we are, and let the tracking
   //    routine bring us in, but only let it go 20 seconds until we quit.
   //    Currently only implemented for alt-az.

   G->telescope_at_home = FALSE;
   tcs_math_verbose = 0;
   if (G->use_encoders && sysGlobal->mount_type) {

      double dx_pos, dy_pos, dz_pos;

      // save the new position
      G->current_mean_ra = ra;
      G->current_mean_dec = dec;
      G->current_mean_epoch = epoch;
      G->current_mean_pmra = pmra;
      G->current_mean_pmdec = pmdec;
      G->current_mean_parallax = parallax;
      G->current_mean_radial_velocity = radial_velocity;
      G->current_mean_eff_wavelength = eff_wavelength;
      G->current_tlr = tlr;
     
      G->telescope_is_slewing = FALSE;

//  Now track until we're close to desired position or timeout
      status = track_to_position();

   } else { 
     while (!StopCheckingTimer.Expired()) {
       if (!check_priority(status))
	 {
	   tcs_set_deinit_telescope();
	   free(save_mtoa_parms_to);
	   free(save_mtoa_parms_from);
	   return status;
	 }

       if (move_the_dome)
	 ocs_dome_finished(finished);
     }

     while (!TotalMoveTimer.Expired()) {}
     stopped = TRUE;
   }

   sprintf(outbuf,"Starting tracking rates\n");
   writeline(outbuf,1);

   tcs_math_verbose = 0;
   // save the new position
   G->current_mean_ra = ra;
   G->current_mean_dec = dec;
   G->current_mean_epoch = epoch;
   G->current_mean_pmra = pmra;
   G->current_mean_pmdec = pmdec;
   G->current_mean_parallax = parallax;
   G->current_mean_radial_velocity = radial_velocity;
   G->current_mean_eff_wavelength = eff_wavelength;
   G->current_tlr = tlr;

   // start the tracking rates
   G->telescope_is_slewing = FALSE;
   G->telescope_initialized = TRUE;
   update_tracking_rates(stopped);
   sprintf(outbuf,"Slew to: %lf %lf",ra*DR2H,dec*DR2D);
   writelog(outbuf,10);

   // wait for the dome to finish moving
   #ifndef no_hardware
   if (move_the_dome)
     {
       while (!finished)
	 ocs_dome_finished(finished);
       autoGlobal->dome_azimuth = new_dome_az;
//       ocs_correct_last_dome_move();
     }
   #endif

   // clean up
   move_the_dome = FALSE;
   free(save_mtoa_parms_to);
   free(save_mtoa_parms_from);

   sprintf(outbuf,"Z motor position: %ld %lf\n Z encoder position: %ld %lf\n",
       tcs_return_step_position('z'),
       tcs_return_step_position('z') / sysGlobal->z_steps_degree,
       tcs_return_encoder_position('z'),
       tcs_return_encoder_position('z') / sysGlobal->z_encoder_encoder_steps_deg);
   writeline(outbuf,1);

   // reset the keypad
   reset_keypad_counters();
   write_telescope_position(1);
   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_dust
//
//  Purpose....: open/close dust covers
//
// Input......:  none
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_dust(int open)
 {
   unsigned status;

   if ((!sysGlobal->mirror_covers_installed) ||
       (!sysGlobal->mirror_cover_delay))
     return TCSERR_NA;

   // Note that we require the telescope to be at altitude>80 for safe operation

   // if the telescope has not been initialized, we cannot move
   if (!G->telescope_initialized) {
       writeline("Telescope has not been initialized!",0);
       return TCSERR_TELENOTINIT;
   }

   // send the telescope to its home position
   writeline("Sending the telescope  to altitude 88 ...",0);
   char y_command[81];

   sprintf(y_command, ECHOCH"ay vl%ld; ma%ld; gd; id;",
	    sysGlobal->y_max_velocity,(long)(9*sysGlobal->y_steps_degree));
   pc38_send_commands(y_command);

   // wait for the axes to finish
   BOOL done;
   do {
       // check the shutdown / reset watchdog
       if (!check_priority(status))
	       return status;
       done = pc38_done(pcx_y_axis);

       // Update the telescope position and send it to the remote
       update_display();
       update_status(0);
   } while (!done);
   pc38_clear_done_flags(pcx_y_axis);

   // stop the telescope
   tcs_telescope_stop();

#ifdef OLD
   // If telescope isn't initialized, initialize it so we can be sure we
   //   are at a suitable position (the home position is suitable)

   if (!G->telescope_initialized) {
     sprintf(outbuf,"Moving telescope to home position...");
     writeline(outbuf,1);
     autoGlobal->saved_az = -9999;
     autoGlobal->saved_alt = -9999;
     tcs_home_telescope();
   }
   else {
     // Move to altitude 85 at current azimuth
     double az, alt, ra, dec;
     alt = 85.;
     az = G->current_obs_az;

     struct mytime t;
     struct date d;
     struct ALLTIMES timeRec;
     mygettime(&d, &t);
     get_all_times_at(d, t, timeRec);

     slaDh2e(az*DD2R, alt*DD2R, G->latitude, &ra, &dec);
     ra = slaDranrm(timeRec.last - ra);
     slaPreces("FK4",timeRec.equinox,1950.,&ra,&dec);

     int status;
     status = tcs_move_to_coordinates( ra, dec, 1950.,
				       0., 0., 0., 0., 0.55, 0.0065, 0.);
   }
#endif

   sprintf(outbuf,"Mirror cover delay =  %d seconds", 
     (int)sysGlobal->mirror_cover_delay);
   writeline(outbuf,1);

   sprintf(outbuf,"Moving mirror covers...");
   writeline(outbuf,1);

   // pick direction
   if (open==1) {
     pc38_send_commands("bh6;");
     // turn power on
     pc38_send_commands("bl5;");
     delay(sysGlobal->mirror_cover_delay * 1000);
     // turn power off
     pc38_send_commands("bh5;");
     autoGlobal->mirror_covers_open = TRUE;
     G->mirror_covers_open = TRUE;
     sprintf(outbuf,"Mirror covers opened");
     writelog(outbuf,3);

   }
   else {
     pc38_send_commands("bh5;");
     // turn power on
     pc38_send_commands("bl6;");
     delay(sysGlobal->mirror_cover_delay * 1000);
     // turn power off
     pc38_send_commands("bh6;");
     autoGlobal->mirror_covers_open = FALSE;
     G->mirror_covers_open = FALSE;
     sprintf(outbuf,"Mirror covers closed");
     writelog(outbuf,3);
   }
   writetoccscf(autoGlobal);
   delay(200);

   G->telescope_at_home = 3;

   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_pos_within_limits
//
//  Purpose....: check a step position against the soft limits
//
// Input......:  axis      - the axis to check ('x' or 'y')
//               step_pos  - the position to check
//
// Output.....:  TRUE if the step position is within the soft limits
//
//------------------------------------------------------------------
BOOL tcs_pos_within_limits(const char axis, const long step_pos)
 {
   switch (axis)
     {
       case  'x':
       case  'X':  return ((step_pos > sysGlobal->x_neg_soft_limit) &&
			     (step_pos < sysGlobal->x_pos_soft_limit));

       case   'y':
       case  'Y': return ((step_pos > sysGlobal->y_neg_soft_limit) &&
			     (step_pos < sysGlobal->y_pos_soft_limit));

       case   'z':
       case  'Z': return ((step_pos > sysGlobal->z_neg_soft_limit) &&
			     (step_pos < sysGlobal->z_pos_soft_limit));

       default: return TRUE;
     }
 }

//------------------------------------------------------------------
//  Name.......:  tcs_reset_home_position
//
//  Purpose....: reset the telescope's astronomical home position
//
// Input......:  none
//
// Output.....:  error code
//
//------------------------------------------------------------------
unsigned tcs_reset_home_position(BOOL sessiononly)
 {
   struct ALLTIMES timeRec;
   struct mytime t;
   struct date d;

   // get the LST
   mygettime(&d,&t);
   //getdate(&d);
   get_all_times_at(d, t, timeRec);

   // get x encoder or motor step position
   long x_count = (sysGlobal->x_encoder_installed) ?
		   ((sysGlobal->x_geartrain_normal) ?
		     tcs_return_encoder_position('x') :
		     -tcs_return_encoder_position('x'))  :
		   ((sysGlobal->x_geartrain_normal) ?
		     tcs_return_step_position('x') :
		     -tcs_return_step_position('x'));

   long y_count = (sysGlobal->y_encoder_installed) ?
		   ((sysGlobal->y_geartrain_normal) ?
		     tcs_return_encoder_position('y') :
		     -tcs_return_encoder_position('y'))  :
		   ((sysGlobal->y_geartrain_normal) ?
		     tcs_return_step_position('y') :
		     -tcs_return_step_position('y'));

   // calculate the distance of the star from home
   //
   // use either encoder steps/° or motor steps/°
   double dx_star_from_home = (sysGlobal->x_encoder_installed) ?
	     (double)x_count / sysGlobal->x_encoder_encoder_steps_deg :
	     (double)x_count / sysGlobal->x_steps_degree;

   double dy_star_from_home = (sysGlobal->y_encoder_installed) ?
	     (double)y_count / sysGlobal->y_encoder_encoder_steps_deg :
	     (double)y_count / sysGlobal->y_steps_degree;


   if (!sysGlobal->mount_type)
     {
       // equatorial
       double star_ha, star_dec, star_pa;
       mean_to_mount_corrected(timeRec,
			   G->current_mean_ra,
			   G->current_mean_dec,
			   G->current_mean_epoch,
			   G->current_mean_pmra,
			   G->current_mean_pmdec,
			   G->current_mean_parallax,
			   G->current_mean_radial_velocity,
			   star_ha, star_dec, star_pa);

       autoGlobal->home_ha = slaDrange(star_ha - (dx_star_from_home * DD2R))
						 * DR2H;
       autoGlobal->home_dec = (star_dec * DR2D) - dy_star_from_home;
     }
   else
     {
       // alt-az
       double star_az, star_el, star_pa, home_az, home_el;
       mean_to_mount_corrected(timeRec,
			   G->current_mean_ra,
			   G->current_mean_dec,
			   G->current_mean_epoch,
			   G->current_mean_pmra,
			   G->current_mean_pmdec,
			   G->current_mean_parallax,
			   G->current_mean_radial_velocity,
			   star_az, star_el, star_pa);

       home_az = slaDranrm(star_az - (dx_star_from_home * DD2R));
       home_el = star_el - (dy_star_from_home * DD2R);
       G->ref_alt = home_el * DR2D;
       G->ref_az = home_az * DR2D;
       if (sessiononly) {
	 sprintf(outbuf,"Update coordinates: %lf %lf %lf %lf",
	     autoGlobal->home_az, autoGlobal->home_alt,
	     G->ref_az, G->ref_alt);
	 writelog(outbuf,6);
       } else {
	 sprintf(outbuf,"Update home position: %lf %lf %lf %lf",
	     autoGlobal->home_az, autoGlobal->home_alt,
	     G->ref_az, G->ref_alt);
	 writelog(outbuf,7);
	 autoGlobal->home_alt = home_el * DR2D;
	 autoGlobal->home_az = home_az * DR2D;
	 autoGlobal->home_rot = G->ref_rot ;
	 slaDh2e(home_az, home_el, G->latitude,
	       &autoGlobal->home_ha, &autoGlobal->home_dec);
	 autoGlobal->home_ha *= DR2H;
	 autoGlobal->home_dec *= DR2D;
       }
     }

   if (sessiononly)
     return(TCSERR_OK);
   else {
     sprintf(outbuf,"Reset home coordinates results:\n"
	 "\tNew home az = %f\n\r"
	 "\tNew home alt = %f\n\r", 
	  autoGlobal->home_az,autoGlobal->home_alt);
     writeline(outbuf,0);
     writetoccscf(autoGlobal);
     return(0);
   }
 }

//------------------------------------------------------------------
//  Name.......:  tcs_return_encoder_position
//
//  Purpose....: return the axis encoder position - no geartrain reversals
//               are calculated
//
// Input......:  axis - 'x', 'y', or 'z'
//
// Output.....:  position
//
#ifndef no_hardware
long tcs_return_encoder_position(char axis)
#else
long tcs_return_encoder_position(char)
#endif
 {
   #ifdef no_hardware
   return 0L;
   #else
   if (axis == 'x' && !sysGlobal->x_encoder_installed) return(0);
   if (axis == 'y' && !sysGlobal->y_encoder_installed) return(0);
   if (axis == 'z' && !sysGlobal->z_encoder_installed) return(0);
   if (axis == 'z') return(cp4016_read_pos());
   return (return_position(PC38,axis,"re\0",0));
   #endif
 }

//------------------------------------------------------------------
//------------------------------------------------------------------
//  Name.......:  tcs_return_step_position
//
//  Purpose....: return the axis step position - no geartrain reversals
//               are calculated
//
// Input......:  axis - 'x', 'y', or 'z'
//
// Output.....:  position
//
#ifndef no_hardware
long tcs_return_step_position(char axis)
#else
long tcs_return_step_position(char)
#endif
 {
   #ifdef no_hardware
   return 0L;
   #else
     return (return_position(PC38,axis,"rp\0",0));
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_return_home_status
//
//  Purpose....: return the home status of all axes
//
// Input......:  none
//
// Output.....:  AXISPACK hstatus - BOOL for each axis
//
//------------------------------------------------------------------
void tcs_return_home_status(struct HOMESTATUSPACK &hstatus)
 {
   #ifdef no_hardware
   hstatus.t_axis = TRUE;
   hstatus.u_axis = TRUE;
   hstatus.v_axis = TRUE;
   hstatus.x_axis = TRUE;
   hstatus.y_axis = TRUE;
   hstatus.z_axis = TRUE;
   #else
   AXISPACK axisRec;

   pc38_get_axis_status('t', axisRec);
   hstatus.t_axis = axisRec.home;

   pc38_get_axis_status('u', axisRec);
   hstatus.u_axis = axisRec.home;

   pc38_get_axis_status('v', axisRec);
   hstatus.v_axis = axisRec.home;

   pc38_get_axis_status('x', axisRec);
   hstatus.x_axis = axisRec.home;

   pc38_get_axis_status('y', axisRec);
   hstatus.y_axis = axisRec.home;

   pc38_get_axis_status('z', axisRec);
   hstatus.z_axis = axisRec.home;
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_return_limit_status
//
//  Purpose....: return the limit status of each major axis
//
// Input......:  none
//
// Output.....:  LIMITSTATUSPACK - -1, 0, 1 for each axis
//
//------------------------------------------------------------------
void tcs_return_limit_status(struct LIMITSTATUSPACK &lstatus)
 {
   #ifdef no_hardware
   lstatus.x_axis = 0;
   lstatus.y_axis = 0;
   lstatus.z_axis = 0;
   #else
   AXISPACK axisRec;

   pc38_get_axis_status('x', axisRec);
   lstatus.x_axis = (axisRec.limit ? (axisRec.posdir ? 1 : -1) : 0);

   pc38_get_axis_status('y', axisRec);
   lstatus.y_axis = (axisRec.limit ? (axisRec.posdir ? 1 : -1) : 0);

   pc38_get_axis_status('z', axisRec);
   lstatus.z_axis = (axisRec.limit ? (axisRec.posdir ? 1 : -1) : 0);

   pc38_get_axis_status('t', axisRec);
   lstatus.t_axis = (axisRec.limit ? (axisRec.posdir ? 1 : -1) : 0);

   pc38_get_axis_status('u', axisRec);
   lstatus.u_axis = (axisRec.limit ? (axisRec.posdir ? 1 : -1) : 0);

   pc38_get_axis_status('v', axisRec);
   lstatus.v_axis = (axisRec.limit ? (axisRec.posdir ? 1 : -1) : 0);
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_set_deinit_telescope
//
//  Purpose....: set certain variables to reflect that the telescope is no
//               longer initialized and tracking
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
void tcs_set_deinit_telescope()
 {
   //
   // **** WARNING ****
   // do not set G->telescope_at_home to FALSE in here!!!
   //
   G->telescope_initialized = FALSE;
   G->telescope_is_slewing = FALSE;

   G->x_tracking_rate = 0.0;
   G->y_tracking_rate = 0.0;
   G->z_tracking_rate = 0.0;

   G->x_tracking_factor = 1.0;
   G->y_tracking_factor = 1.0;
   G->z_tracking_factor = 1.0;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_store_z_axis_pos
//
//  Purpose....: write the z-axis position to the toccauto scf file
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
void tcs_store_z_axis_pos()
 {
   #ifndef no_hardware

     if ((!sysGlobal->z_axis_enabled) || (!sysGlobal->mount_type))
       return;

     autoGlobal->z_axis_pos = tcs_return_step_position('z');
     writetoccscf(autoGlobal);
   #endif
 }

//------------------------------------------------------------------
//  Name.......:  tcs_set_tracking_rates
//
//  Purpose....: set the axis tracking rates
//
// Input......:  x_axis  - x-axis rate (arcsec/sec)
//               y_axis  - y-axis rate (arcsec/sec)
//               z_axis  - z-axis rate (arcsec/sec)
//
// Output.....:  error code
//
//------------------------------------------------------------------
#ifndef no_hardware
unsigned tcs_set_tracking_rates(double x_axis, double y_axis, double z_axis)
#else
unsigned tcs_set_tracking_rates(double, double, double)
#endif
 {
   #ifndef no_hardware
   char x_command[41] = "";
   char y_command[41] = "";
   char z_command[41] = "";

   if (!G->telescope_initialized)
     return TCSERR_TELENOTINIT;

   z_axis = (sysGlobal->mount_type) ? z_axis : 0.0;

   // convert arcsec/sec to steps/sec
   double x_steps_sec = x_axis * sysGlobal->x_steps_degree / 3600.0;
   double y_steps_sec = y_axis * sysGlobal->y_steps_degree / 3600.0;
   double z_steps_sec = (z_axis != 0.0) ?
	       z_axis * sysGlobal->z_steps_degree / 3600.0 : 0.0;

   // check velocities against max velocities
   if ((fabs(x_steps_sec) > sysGlobal->x_max_velocity) ||
       (fabs(y_steps_sec) > sysGlobal->y_max_velocity) ||
       ((z_axis != 0) && (fabs(z_steps_sec) > sysGlobal->z_max_velocity)))
     {
       error_sound();
      writeline("Tracking rate change is out of range.  Rates not adjusted.",0);
       sprintf(outbuf,"X rate = %10.4f\tMax = %ld", x_steps_sec,
	       sysGlobal->x_max_velocity);
       writeline(outbuf,1);
       sprintf(outbuf,"Y rate = %10.4f\tMax = %ld", y_steps_sec,
	       sysGlobal->y_max_velocity);
       writeline(outbuf,1);
       sprintf(outbuf,"Z rate = %10.4f\tMax = %ld", z_steps_sec,
	       sysGlobal->z_max_velocity);
       writeline(outbuf,1);
       return TCSERR_OUTOFRANGE;
     }

   // store the new values in the globals
   G->x_tracking_rate = x_steps_sec;
   G->y_tracking_rate = y_steps_sec;
   G->z_tracking_rate = z_steps_sec;

   // account for gear train reversals
   x_steps_sec = (sysGlobal->x_geartrain_normal ? x_steps_sec : -x_steps_sec);
   y_steps_sec = (sysGlobal->y_geartrain_normal ? y_steps_sec : -y_steps_sec);
   z_steps_sec = (sysGlobal->z_geartrain_normal ? z_steps_sec : -z_steps_sec);

   sprintf(x_command, "ax ac%ld; jf%011.4f", sysGlobal->x_acceleration,
	   x_steps_sec);
   sprintf(y_command, "ay ac%ld; jf%011.4f", sysGlobal->y_acceleration,
	   y_steps_sec);
   if (z_steps_sec != 0)
     sprintf(z_command, "az ac%ld; jf%011.4f", sysGlobal->z_acceleration,
	     z_steps_sec);

   // send the commands to the PC-38 card
   pc38_send_commands(x_command);
   pc38_send_commands(y_command);

   if (sysGlobal->z_axis_enabled)
     pc38_send_commands(z_command);

   #endif  // no_hardware
   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_telescope_park
//
//  Purpose....: put the telescope in its parked position and stop the axes
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
unsigned tcs_telescope_park(int pos)
 {
   unsigned status;

   // if the telescope has not been initialized, we can't move
   if (!G->telescope_initialized) {
       writeline("Telescope has not been initialized!",0);
       return TCSERR_TELENOTINIT;
   }

   // send the telescope to its parked position
   writeline("Sending the telescope to its parked position...",0);

   char buffer[81];
   sprintf(buffer, "ax ac%ld; vl%ld; ma%ld; gd; id;",
	     sysGlobal->x_acceleration,
	     sysGlobal->x_max_velocity,
	     sysGlobal->x_park_steps);
   writeline(buffer,1);
   pc38_send_commands(buffer);
   writeline(buffer,1);

   if (pos == 0) 
     sprintf(buffer, "ay ac%ld; vl%ld; ma%ld; gd; id;",
	     sysGlobal->y_acceleration,
	     sysGlobal->y_max_velocity,
	     sysGlobal->y_park_steps);
   else
     sprintf(buffer, "ay ac%ld; vl%ld; ma%ld; gd; id;",
	     sysGlobal->y_acceleration,
	     sysGlobal->y_max_velocity,
	     sysGlobal->y_pos_soft_limit-50000);
   writeline(buffer,1);
   pc38_send_commands(buffer);
   writeline(buffer,1);

   if (sysGlobal->z_axis_enabled) {
       if (sysGlobal->mount_type) {
	   sprintf(buffer, "az ac%ld; vl%ld; ma%ld; gd; id;",
		     sysGlobal->z_acceleration,
		     sysGlobal->z_max_velocity,
		     sysGlobal->z_park_steps);
	   writeline(buffer,1);
	   pc38_send_commands(buffer);
	   writeline(buffer,1);
       }
   }

   // wait until done
   if (sysGlobal->z_axis_enabled)
     {
       if (sysGlobal->mount_type)
	 do {
	   if (!check_priority(status))
	     return status;
	   update_status(0);
	   update_display();
//           sprintf(outbuf,"axis done: %d %d %d",
//           pc38_done(pcx_x_axis),pc38_done(pcx_y_axis),pc38_done(pcx_z_axis));
//           writeline(outbuf,1);
	 } while (!pc38_done(pcx_x_axis | pcx_y_axis | pcx_z_axis));
       else
	 do
	   if (!check_priority(status))
	     return status;
	 while (!pc38_done(pcx_x_axis | pcx_y_axis));
     }
   else
     {
       do {
	 if (!check_priority(status))
	   return status;
	 update_status(0);
	 update_display();
       } while (!pc38_done(pcx_x_axis | pcx_y_axis));
     }

   pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);

   // stop the telescope
   tcs_telescope_stop();
   G->telescope_is_slewing = FALSE;

   if (pos==0)
     G->telescope_at_home = 4;
   else
     G->telescope_at_home = 5;

//   G->tracking_on = FALSE;
//   sprintf(outbuf,
//      "Telescope is at stowed position - TRACKING HAS BEEN TURNED OFF");
   sprintf(outbuf,"Telescope is at stowed position");
   writeline(outbuf,0);
//   tcs_set_deinit_telescope();
   if (sysGlobal->mount_type) tcs_store_z_axis_pos();

   // save the current telescope position
   update_encoder_position();

   steps_to_degrees(
         G->current_enc_az,G->current_enc_alt,G->current_enc_rot,
         autoGlobal->saved_az,autoGlobal->saved_alt,autoGlobal->saved_rot);
   writetoccscf(autoGlobal);

   sprintf(outbuf,"Telescope stowed");
   writelog(outbuf,11);
   return TCSERR_OK;
 }

//------------------------------------------------------------------
//  Name.......:  tcs_telescope_service
//
//  Purpose....: put the telescope in it's "service" position
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
unsigned tcs_telescope_service()
 {
   unsigned status;

   // if the telescope has not been initialized, we cannot move
   if (!G->telescope_initialized) {
       writeline("Telescope has not been initialized!",0);
       return TCSERR_TELENOTINIT;
   }

   // send the telescope to its home position
   writeline("Sending the telescope to its service/home position...",0);

   char x_command[81];
   char y_command[81];
   char z_command[81];

   sprintf(x_command, ECHOCH"ax vl%ld; ma%ld; gd; id;",
	    sysGlobal->x_max_velocity,0L);
   sprintf(y_command, ECHOCH"ay vl%ld; ma%ld; gd; id;",
	    sysGlobal->y_max_velocity,0L);
   if (sysGlobal->z_axis_enabled)
     sprintf(z_command, ECHOCH"az vl%ld; ma%ld; gd; id;",
	    sysGlobal->z_max_velocity,0L);

   pc38_send_commands(x_command);
   pc38_send_commands(y_command);

   if (sysGlobal->z_axis_enabled)
     pc38_send_commands(z_command);

   // wait for the axes to finish
   BOOL done;
   do {
       // check the shutdown / reset watchdog
       if (!check_priority(status))
	       return status;
       done = pc38_done(pcx_x_axis | pcx_y_axis);
       if (sysGlobal->z_axis_enabled)
	     done = done && pc38_done(pcx_z_axis);
   } while (!done);
   pc38_clear_done_flags(pcx_x_axis | pcx_y_axis | pcx_z_axis);

   // stop the telescope
   tcs_telescope_stop();
   G->telescope_is_slewing = FALSE;
   G->tracking_on = FALSE;
   if (sysGlobal->mount_type) tcs_store_z_axis_pos();
   return TCSERR_OK;
 }
//------------------------------------------------------------------
//  Name.......:  tcs_telescope_fill
//
//  Purpose....: put the rotator in it's "fill" position and fill the dewar
//
// Input......:  none
//
// Output.....:  none
//
//------------------------------------------------------------------
unsigned tcs_telescope_fill(double fill_time)
 {
   unsigned status;

   // if the telescope has not been initialized, we cannot move

   if (!G->telescope_initialized) {
       writeline("Telescope has not been initialized! Filling in current position",0);
       //return TCSERR_TELENOTINIT;
   } else {

/*   JUST STOP THE TELESCOPE WHERE IT IS, DON'T MOVE IT
     // send the telescope to its home position
     writeline("Sending the rotator to its fill position...",0);

     char x_command[81];
     char y_command[81];
     char z_command[81];

     if (sysGlobal->z_axis_enabled) {
       sprintf(z_command, ECHOCH"az vl%ld; ma0; gd; id;",
  	    sysGlobal->z_max_velocity);
       pc38_send_commands(z_command);

       // wait for the axes to finish
       BOOL done;
       do {
         // check the shutdown / reset watchdog
         if (!check_priority(status))
  	       return status;
         done = pc38_done(pcx_z_axis);
  
         // Update the telescope position and send it to the remote
         update_display();
         update_status(0);
       } while (!done);
       pc38_clear_done_flags(pcx_z_axis);
     }
     G->telescope_at_home = 2;
*/

     // stop the telescope
     tcs_telescope_stop();
 
     if (sysGlobal->mount_type) tcs_store_z_axis_pos();
   }
   
   int retry=0;
   tryagain:

   // fill the dewar
   writeline("Tracking off. Starting the fill...",0);
   pc38_send_commands("bl8;");
   delay(2000);
   pc38_send_commands("bh8;");

   // calculate when we last filled the dewar
   struct mytime t;
   struct date d;
   struct ALLTIMES timeRec;
   mygettime(&d,&t);
   get_all_times_at(d, t, timeRec);

   double dtime;
   dtime = timeRec.mjd_utc - autoGlobal->last_fill_utc;

#undef Havefillsensor
#ifdef Havefillsensor
   fill_time = 20;
   sprintf(outbuf,"Waiting for fill to complete, or 20 minutes, whichever is less.\n"
" All operations (including status) will be suspended...",fill_time);
   writeline(outbuf,0);
#else
//   if (dtime*24. > sysGlobal->ccd_hold_time ) {
//     fill_time = 10;
//   } else {
//     fill_time = ceil(dtime / sysGlobal->ccd_hold_time * 10);
//     fill_time = (fill_time>3 ? fill_time : 3);
//   }
   sprintf(outbuf,"Waiting %f minutes for fill to complete.\n All operations (including status) will be suspended...",fill_time);
   writeline(outbuf,0);
#endif

//   Delay(fill_time*60*1000L);

//   EventTimer TempTimer; 
//   TempTimer.NewTimerSecs(15);

   EventTimer FillTimer;
   int fill_time_secs;
   fill_time_secs=fill_time*60;
   FillTimer.NewTimerSecs(fill_time_secs);
#ifdef Havefillsensor
   while (ocs_ccd_fill_open() && !FillTimer.Expired()) {
#else
   while (!FillTimer.Expired()) {
#endif
       if (!check_priority(status)) return(status);
       // Update the telescope position and send it to the remote
       update_display();
       update_status(0);
//       if (TempTimer.Expired()) {
//	 check_ccd_temp(1);
//	 TempTimer.NewTimerSecs(15);
//       }
       delay(500);
   }

#ifdef Havefillsensor
   if (FillTimer.Expired()) {
     sprintf(outbuf,"WARNING: timer expired before fill sensor triggered!");
     writeline(outbuf,0);
   } else if (retry==0 && FillTimer.RemainingTimeInSecs() > (fill_time-5)*60 ) {
     // We shut off in less than 5 minutes. Wait a minute and try again.
     sprintf(outbuf,"WARNING: fill sensor triggered in less than 5 minutes!");
     writeline(outbuf,0);
     sprintf(outbuf,"Waiting a minute and then will try again...");
     writeline(outbuf,0);
     TempTimer.NewTimerSecs(60);
     while (!TempTimer.Expired()) {
       // Update the telescope position and send it to the remote
       update_display();
       update_status(0);
       delay(500);
     }
     retry=1;
     goto tryagain;
   } else {
    // Just wait 30 seconds before moving
     FillTimer.NewTimerSecs(30);
     while (!FillTimer.Expired()) {
       // Update the telescope position and send it to the remote
       update_display();
       update_status(0);
     }
   }

#endif

   // Store the current time while we're at it
   mygettime(&d,&t);
   get_all_times_at(d, t, timeRec);
   autoGlobal->last_fill_utc = timeRec.mjd_utc ;
   writetoccscf(autoGlobal);

   sprintf(outbuf,"Fill complete."); 
   writeline(outbuf,0);
   sprintf(outbuf,"Dewar filled");
   writelog(outbuf,12);

   reset_shutdown_timers();

   G->telescope_at_home = 2;
   write_telescope_position(1);

   return TCSERR_OK;
 }

void set_ccd_speed(int fast)
{
  if (fast==1)
    pc38_send_commands("bl12;");
  else
    pc38_send_commands("bh12;");

}


//------------------------------------------------------------------
//  Name.......:  tcs_telescope_stop
//
//  Purpose....: stop all telescope axes
//
// Input......:  wait - TRUE (default) wait until stopped to return
//
// Output.....:  none
//
//------------------------------------------------------------------
#ifndef no_hardware
void tcs_telescope_stop(const BOOL wait)
#else
void tcs_telescope_stop(const BOOL)
#endif
 {
   int icount;
   #ifndef no_hardware
   byte flags =  pcx_t_axis | pcx_u_axis | pcx_v_axis | pcx_z_axis;

   pc38_clear_done_flags(flags);

   // stop secondary
   pc38_send_commands("at st; id; au st; id; av st; id; az st; id;");

   // check limits and stop each axis
   LIMITSTATUSPACK limits;
   tcs_return_limit_status(limits);

   if (!limits.x_axis) {
       flags |= pcx_x_axis;
       pc38_send_commands("ax ca; st; id;");
   }

   if (!limits.y_axis) {
       flags |= pcx_y_axis;
       pc38_send_commands("ay ca; st; id;");
   }

   if (!wait) {
       pc38_send_commands("ax st; ay st;");
       // we still must wait for the z-axis to stop
       if (sysGlobal->mount_type) {
           icount = 0;
	   do { 
            delay(1);
            icount++;
           } while (!pc38_done(pcx_z_axis) && icount<1000);
           if (icount == 1000) {
             sprintf(outbuf,"pc38_done failed ...");
             writeline(outbuf,1);
           } else
	     tcs_store_z_axis_pos();
	   // tcs_set_deinit_telescope();
       }
       return;
   }

   icount = 0;
   do {
     delay(1);
     icount++;
   } while (!pc38_done(flags) && icount<1000);
   if (icount == 1000) {
     sprintf(outbuf,"pc38_done failed ...");
     writeline(outbuf,1);
   } else {
     pc38_clear_done_flags(flags);
     if (sysGlobal->mount_type) tcs_store_z_axis_pos();
   }
   #endif
//   tcs_set_deinit_telescope();
 }

unsigned tcs_quick_move(double ra_sec, double dec_arcsec)
{
   unsigned status;

   if (!G->telescope_initialized)
     return TCSERR_TELENOTINIT;

   if (G->telescope_at_home)
     return TCSERR_TELEATHOME;

   sprintf(outbuf,"before: %f %f\n",G->current_mean_ra,G->current_mean_dec);
   writeline(outbuf,1);
   G->current_mean_ra += (DS2R * ra_sec);
   G->current_mean_dec += (DAS2R * dec_arcsec);
   sprintf(outbuf,"after: %f %f\n",G->current_mean_ra,G->current_mean_dec);
   writeline(outbuf,1);

   tcs_telescope_stop();
   status = track_to_position();
   write_telescope_position(1);
   return(status);
}

unsigned track_to_position()
{
   BOOL stopped;
   double xdead = 0.00055;
   double ydead = 0.00055;
   double zdead = 0.1;
   update_tracking_rates(TRUE);
   EventTimer ShortMoveTimer;
   struct date d;
   struct mytime t;

   if (sysGlobal->z_axis_enabled)
     zdead = 0.1;
   else
     zdead = 1.e10;

   ShortMoveTimer.NewTimerSecs(20);
   stopped = TRUE;
   outbuf2[0] = 0;
   do {
	update_tracking_rates(stopped);
   //     mygettime(&d,&t);
   //     sprintf(outbuf2+strlen(outbuf2),"%d:%d:%d.%d    %f %f %f\n",
   //             t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund,
   //             G->last_az_error,G->last_alt_error, G->last_rot_error); 
	stopped = FALSE;
   } while ((fabs(G->last_az_error)  > xdead || 
	     fabs(G->last_alt_error) > ydead || 
	     fabs(G->last_rot_error) > zdead) &&
	     !ShortMoveTimer.Expired() );

   //writeline(outbuf2,1);
   //writeline(outbuf2,2);

   if (ShortMoveTimer.Expired()) {
	writeline("short move time expired!",0);
	writeline("short move time expired!",2);
	return(-1);
   }
   return(0);
}
void tcs_set_encoders()
{
    long enc_x, enc_y, enc_z;
    char x_command[81];
    char y_command[81];
    char z_command[81];
    // Determine the appropriate encoder position for the saved position
    if (sysGlobal->x_encoder_installed)
      enc_x = (long) (autoGlobal->saved_az - autoGlobal->home_az) *
	     sysGlobal->x_encoder_encoder_steps_deg;
    else
      enc_x = (long) (autoGlobal->saved_az - autoGlobal->home_az) *
	     sysGlobal->x_steps_degree;
    if (!sysGlobal->x_geartrain_normal) enc_x = -enc_x;

    if (sysGlobal->y_encoder_installed)
      enc_y = (long) (autoGlobal->saved_alt - autoGlobal->home_alt) *
	     sysGlobal->y_encoder_encoder_steps_deg;
    else
      enc_y = (long) (autoGlobal->saved_alt - autoGlobal->home_alt) *
	     sysGlobal->y_steps_degree;
    if (!sysGlobal->y_geartrain_normal) enc_y = -enc_y;

    if (sysGlobal->z_encoder_installed)
      enc_z = (long) (autoGlobal->saved_rot - autoGlobal->home_rot) *
	     sysGlobal->z_encoder_encoder_steps_deg;
    else
      enc_z = (long) (autoGlobal->saved_rot - autoGlobal->home_rot) *
	     sysGlobal->z_steps_degree;
    if (!sysGlobal->z_geartrain_normal) enc_z = -enc_z;

    sprintf(outbuf,"enc_x, y, z: %ld %ld %ld \n",enc_x, enc_y, enc_z);
    writeline(outbuf,1);

    // Load the appropriate values into the counters
    if (sysGlobal->x_encoder_installed)
      sprintf(x_command,ECHOCH"ax er%ld,%ld lp%ld;",
		   sysGlobal->x_encoder_encoder_steps_rev,
		   sysGlobal->x_encoder_motor_steps_rev,
		   enc_x);
    else
      sprintf(x_command,ECHOCH"ax lp%ld;",enc_x);
    pc38_send_commands(x_command);

    if (sysGlobal->y_encoder_installed)
      sprintf(y_command,ECHOCH"ay er%ld,%ld lp%ld;",
		   sysGlobal->y_encoder_encoder_steps_rev,
		   sysGlobal->y_encoder_motor_steps_rev,
		   enc_y);
    else
      sprintf(y_command,ECHOCH"ay lp%ld;",enc_y);
    pc38_send_commands(y_command);

    if (sysGlobal->z_encoder_installed)
      sprintf(z_command,ECHOCH"az er%ld,%ld lp%ld;",
		   sysGlobal->z_encoder_encoder_steps_rev,
		   sysGlobal->z_encoder_motor_steps_rev,
		   enc_z);
    else
      sprintf(z_command,ECHOCH"az lp%ld;",enc_z);
    pc38_send_commands(z_command); 

    // Load secondary positions
    sprintf(x_command,ECHOCH"at lp%ld;",autoGlobal->t_step_pos);
    pc38_send_commands(x_command); 
    sprintf(x_command,ECHOCH"au lp%ld;",autoGlobal->u_step_pos);
    pc38_send_commands(x_command); 
    sprintf(x_command,ECHOCH"av lp%ld;",autoGlobal->v_step_pos);
    pc38_send_commands(x_command); 

    G->encoder_initialized = TRUE;
}
/********************************* EOF **************************************/

