#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include "mytype.h"
#include "pcx.h"
#include "io.h"
#include "tracking.h"
#include "systimer.h"
#include "evtimer.h"
#include "status.h"
#include "scf.h"
#include "tertiary.h"
#include "globals.h"
#include "shutdown.h"
#include "tcs.h"

#define BRAKEOUT_BIT 0x40
#define BRAKEIN_BIT 0x80

#ifdef echo_pc38
 #define ECHOCH "\x01"
#else
 #define ECHOCH ""
#endif

// Assumes t is tertiary motor

// tertiary_initialize sets up motor parameters
// tertiary_home sends motor to nearest home switch in requested direction
// tertiary_move moves tertiary

// there appears to be significant backlash, so preferred initialization to
//  preserve reasonable repeatability is to home tertiary in one direction,
//  move past home in the same direction, then home tertiary in reverse
//  direction, then continue move to desired position. Ideally, move to
//  desired position should be done in one single step.

BOOL tertiary_initialized = FALSE;

// Send tertiary to the limit position
int tertiary_initialize()
{
   char buffer[81];
   #ifdef no_pc38
   tertiary_initialized = FALSE;
   return GCSERR_OK;
   #else
   unsigned status=GCSERR_OK;

   tertiary_initialized = FALSE;
   // Turn off echo and limits and load motor parameters
   pcx_send_commands(GUIDERPC38,"ef pa0 at lf;");
   sprintf(buffer, "at ac%lu vl%lu;", gcsGlobal->t_acceleration,
           gcsGlobal->t_max_velocity);
   pcx_send_commands(GUIDERPC38,buffer);
   sprintf(buffer, "at lp%lu;", gcsGlobal->t_pos);
   pcx_send_commands(GUIDERPC38,buffer);

   // Turn off all power to motor
   sprintf(buffer, "at af;");
   pcx_send_commands(GUIDERPC38,buffer);

   writegcsscf(gcsGlobal);
   tertiary_initialized = TRUE;
   return(0);
   #endif
}

int tertiary_home(int sign)
{
   char buffer[81];
   int r;
   #ifdef no_pc38
   tertiary_initialized = FALSE;
   return GCSERR_OK;
   #else
   unsigned status=GCSERR_OK;

   if (moveto_tertiary_position() != 0) return(1);

   // Disengage brake
   status = move_brake_out();
   if (status) {
     writeline("Error disengaging brake",0);
     writeline("Deinitializing tertiary",0);
     tertiary_initialized = FALSE;
     return(GCSERR_TERTIARY_NOTINIT);
   }
  
   // Turn on power to motor
   sprintf(buffer, "at an;");
   pcx_send_commands(GUIDERPC38,buffer);

   // First move at max_vel until we hit encoder home position
   if (sign==0)  {
     //sprintf(buffer, "at he hm id;");
     sprintf(buffer, "at hm id;");
   } else {
     //sprintf(buffer, "at he hr id;");
     sprintf(buffer, "at hr id;");
   }
   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(300);
   gcsGlobal->t_pos = return_position(GUIDERPC38,'t',"rp",0);
   sprintf(outbuf,"t_pos: %ld",gcsGlobal->t_pos);
   writeline(outbuf,1);
   sprintf(outbuf,"Moving tertiary to home position....");
   writeline(outbuf,1);
   pcx_clear_done_flags(GUIDERPC38,pcx_t_axis);
   pcx_send_commands(GUIDERPC38,buffer);
   do {
     update_tracking_rates();
     update_status(0);
     update_display();
   } while ((!pcx_done(GUIDERPC38,pcx_t_axis)) && !TimeoutTimer.Expired());
   if (TimeoutTimer.Expired() ) {
       writeline("Error: faster tertiary initialization timed out",0);
       pcx_send_commands(GUIDERPC38,"at st;");
       return(GCSERR_TERTIARY_NOTINIT);
   }
   // Move back off home position and approach home from negative slower
   gcsGlobal->t_pos = return_position(GUIDERPC38,'t',"rp",0);
   sprintf(outbuf,"t_pos: %ld",gcsGlobal->t_pos);
   writeline(outbuf,1);
   sprintf(outbuf,"Moving back off home position....");
   writeline(outbuf,1);
   sprintf(buffer, "at ma10000; gd; id;");
   pcx_clear_done_flags(GUIDERPC38,pcx_t_axis);
   pcx_send_commands(GUIDERPC38,buffer);
   TimeoutTimer.NewTimerSecs(60);
   do {
     update_tracking_rates();
     update_status(0);
     update_display();
   } while ((!pcx_done(GUIDERPC38,pcx_t_axis)) && !TimeoutTimer.Expired());
   gcsGlobal->t_pos = return_position(GUIDERPC38,'t',"rp",0);
   sprintf(outbuf,"t_pos: %ld",gcsGlobal->t_pos);
   writeline(outbuf,1);
   sprintf(buffer, "at ac500 vl500;");
   pcx_send_commands(GUIDERPC38,buffer);
   sprintf(buffer, "at hr id;");
   sprintf(outbuf,"Moving to home position again slower....");
   writeline(outbuf,1);
   pcx_clear_done_flags(GUIDERPC38,pcx_t_axis);
   pcx_send_commands(GUIDERPC38,buffer);
   TimeoutTimer.NewTimerSecs(60);
   do {
     update_tracking_rates();
     update_status(0);
     update_display();
   } while ((!pcx_done(GUIDERPC38,pcx_t_axis)) && !TimeoutTimer.Expired());
   if (TimeoutTimer.Expired() ) {
       writeline("Error: slow tertiary initialization timed out",0);
       pcx_send_commands(GUIDERPC38,"at st;");
       return(GCSERR_TERTIARY_NOTINIT);
   }
   gcsGlobal->t_pos = return_position(GUIDERPC38,'t',"rp",0);
   sprintf(outbuf,"t_pos: %ld",gcsGlobal->t_pos);
   writeline(outbuf,1);

   // Reset velocity
   sprintf(buffer, "at ac%lu vl%lu;", gcsGlobal->t_acceleration,
           gcsGlobal->t_max_velocity);
   pcx_send_commands(GUIDERPC38,buffer);

   // Engage brake
   status = move_brake_in();
   if (status) {
     writeline("Error engaging brake",0);
     writeline("Deinitializing tertiary",0);
     tertiary_initialized = FALSE;
     return(GCSERR_TERTIARY_NOTINIT);
   }

   // Turn off power to motor
   sprintf(buffer, "at af;");
   pcx_send_commands(GUIDERPC38,buffer);

   tertiary_initialized = TRUE;
   writegcsscf(gcsGlobal);
   return(status);
   #endif
}

int move_brake_in()
{
   int status,i,r,s;

   if (is_brake_in()) return(GCSERR_OK);

   //Drive brake in
   pcx_send_commands(GUIDERPC38,"bl10;");

   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(5);
   i=0;
   while (((r=!is_brake_in())||(s=is_brake_out()))&&i<5000) {
     i++;
   }

   //Turn off brake motor
   delay(500);
   pcx_send_commands(GUIDERPC38,"bh10;");
   delay(500);
   if (i>=5000||!is_brake_in()||is_brake_out())  {
     sprintf(outbuf,"brake_in failed: %d %d %d\n",TimeoutTimer.Expired(),
	    is_brake_in(), is_brake_out());
     writeline(outbuf,0);
     status = GCSERR_BRAKE_TIMEOUT;
   } else
     status = GCSERR_OK;

   return(status);

}
int move_brake_out()
{
   int status,i,r,s;

   if (is_brake_out()) return(GCSERR_OK);

   //Drive brake in
   pcx_send_commands(GUIDERPC38,"bl10;");

   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(5);
   i=r=s=0;
   while (((r=!is_brake_out())||(s=is_brake_in()))&&i<5000) {
     i++;
   }

   //Turn off brake motor
   delay(500);
   pcx_send_commands(GUIDERPC38,"bh10;");
   delay(500);
   if (i>=5000||!is_brake_out()||is_brake_in()) {
     sprintf(outbuf,"brake_out failed: %d %d %d\n",TimeoutTimer.Expired(),
	    is_brake_in(), is_brake_out());
     writeline(outbuf,0);
     status = GCSERR_BRAKE_TIMEOUT;
   } else
     status = GCSERR_OK;


   return(status);
}

BOOL is_brake_in()
{
   #ifdef no_tertiary
   return TRUE;
   #else
   unsigned rval = pcx_read_inputs(GUIDERPC38);
   rval &= BRAKEIN_BIT;
   return (rval) ? TRUE : FALSE;
   #endif
}
BOOL is_brake_out()
{
   #ifdef no_tertiary
   return TRUE;
   #else
   unsigned rval = pcx_read_inputs(GUIDERPC38);
   rval &= BRAKEOUT_BIT;
   return (rval) ? TRUE : FALSE;
   #endif
}

int tertiary_move(long pos, int relative)
{
   #ifdef no_tertiary
   return GCSERR_OK;
   #else
   char buffer[81];
   int status;

   if (!tertiary_initialized) {
     writeline("Tertiary not initialized",0);
     return GCSERR_TERTIARY_NOTINIT;
   }

   if (relative>0) {
     if (moveto_tertiary_position() != 0) return(1);
   }

   // Turn on power to motor
   sprintf(buffer, "at an;");
   pcx_send_commands(GUIDERPC38,buffer);

   // Disengage brake
   status = move_brake_out();
   if (status) {
     writeline("Error disengaging brake",0);
     writeline("Deinitializing tertiary",0);
     tertiary_initialized = FALSE;
   } else {
     if (relative!=0) pos += gcsGlobal->t_pos;
     sprintf(outbuf,"Moving tertiary to position %ld....",pos);
     writeline(outbuf,1);
     sprintf(buffer, "at ma%ld gd id;", pos);
     pcx_send_commands(GUIDERPC38,buffer);
     pcx_clear_done_flags(GUIDERPC38,pcx_t_axis);
     do {
       update_tracking_rates();
       update_status(0);
       update_display();
     } while (!pcx_done(GUIDERPC38,pcx_t_axis));
     
     gcsGlobal->t_pos = pos;
   }

   // Engage brake
   status = move_brake_in();
   if (status) {
     writeline("Error engaging brake",0);
     writeline("Deinitializing tertiary",0);
     tertiary_initialized = FALSE;
   }

   // Turn off power to motor
   sprintf(buffer, "at af;");
   pcx_send_commands(GUIDERPC38,buffer);

   writegcsscf(gcsGlobal);
   return(status);

   #endif
}

int moveto_tertiary_position() 
{
   int status;

   if (!G->telescope_initialized) {
      writeline("Telescope has not been initialized! Can't move tertiary!",0);
      return(1);
   } else {

     if (G->current_obs_alt > 75 ) {
       writeline("Already at high (>75) altitude...",0);
       return(0);
     }
     // send the telescope to its home position
     writeline("Sending the telescope to high altitude...",0);

     char y_command[81];
     sprintf(y_command, ECHOCH"ay vl%ld; ma0; gd; id;",
            sysGlobal->y_max_velocity);
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
     G->telescope_at_home = 3;
     return(0);
  }

}
