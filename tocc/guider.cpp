#include <stdio.h>
#include <stdlib.h>
#include "pcx.h"
#include "io.h"
#include "guider.h"
#include "evtimer.h"
#include "status.h"
#include "scf.h"
#include "globals.h"

// Assumes x is radial stage, y is focus stage
// Assumes positive motion moves away from the limit switch
//long x_step_pos;
//long y_step_pos;
//long guider_x_acceleration = 100000; 
//long guider_x_max_velocity = 80000;
//long guider_x_travel = 1738000;
//long guider_y_acceleration = 10000;
//long guider_y_max_velocity = 50000;
//long guider_y_travel = 6595000;

BOOL guider_initialized = FALSE;

// Send guider stage and focus to the limit position
int guider_initialize(int fullinit, long x_pos, long y_pos)
{
   #ifdef no_guider
   return GCSERR_OK;
   #else
   unsigned status;

   // reset the PC38
   sprintf(outbuf,"Initilizing guider");
   writeline(outbuf,1);
   pcx_disable_interrupts(GUIDERPC38);
   pcx_send_commands(GUIDERPC38,"aa rs;");

   char buffer[81];
   sprintf(buffer, "ef pa0 ax ac%lu vl%lu ln;",
	   gcsGlobal->x_acceleration,
	   gcsGlobal->x_max_velocity);
   pcx_send_commands(GUIDERPC38,buffer);
   sprintf(buffer, "ay ac%lu vl%lu ln;",
	   gcsGlobal->y_acceleration,
	   gcsGlobal->y_max_velocity);
   pcx_send_commands(GUIDERPC38,buffer);


   if (fullinit) {
     pcx_send_commands(GUIDERPC38,"ax km; ay km;");
     BOOL x_done = FALSE;
     BOOL y_done = FALSE;
     AXISPACK axisRec;

     EventTimer TimeoutTimer;
     TimeoutTimer.NewTimerSecs(150);
     do {
       if (!x_done) {
	 pcx_get_axis_status(GUIDERPC38,'x', axisRec);
	 x_done = axisRec.limit;
       }

       if (!y_done) {
	 pcx_get_axis_status(GUIDERPC38,'y', axisRec);
	 y_done = axisRec.limit;
       }
       update_all();
     } while (!(x_done && y_done) && !TimeoutTimer.Expired());

     if (!TimeoutTimer.Expired()) {
       pcx_send_commands(GUIDERPC38,"ax lp0; ay lp0;");
  
       gcsGlobal->x_step_pos = 0L;
       gcsGlobal->y_step_pos = 0L;
       guider_initialized = TRUE;
       writegcsscf(gcsGlobal);
       return(GCSERR_OK);
     } else {
       pcx_send_commands(GUIDERPC38,"ax st; ay st;");
       writeline("Error initializing guider",0);
       guider_initialized = FALSE;
       gcsGlobal->x_step_pos = -1;
       gcsGlobal->y_step_pos = -1;
       return(GCSERR_GUIDER_NOTINIT);
     }
  
   } else {
     sprintf(buffer, "ax lp%ld; ay lp%ld;", x_pos, y_pos);
	 writeline(buffer,1);
     pcx_send_commands(GUIDERPC38,buffer);
     guider_initialized = TRUE;
   }
   #endif
}

int guider_move_steps(long x_axis, long y_axis, int absolute)
{
   unsigned status;
   char buffer[81];
   double sky_per_xsteps, x0, y0;
   long new_pos_x, new_pos_y;
   long del_pos_x, del_pos_y;
   long xtimeout, ytimeout, timeout;
   int ntry;

   if (!guider_initialized) {
     writeline("ERROR: guider not initialized\n",0);
     return GCSERR_GUIDER_NOTINIT;
   }

   sprintf(outbuf,"MOVE GUIDER\n\r"
	 "X Steps = %ld\n\r"
	 "Y Steps = %ld\n\r",
	  x_axis, y_axis);
   writeline(outbuf,1);

   // check travel limits
   if (absolute) 
     new_pos_x = x_axis;
   else 
     new_pos_x = x_axis + gcsGlobal->x_step_pos;
   if ((new_pos_x > 0) || (labs(new_pos_x) > gcsGlobal->x_travel)) {
       writeline("X axis out of range.  No move.",0);
       return GCSERR_GUIDER_OUTOFRANGE;
   }

   if (absolute)
     new_pos_y = y_axis;
   else
     new_pos_y = y_axis + gcsGlobal->y_step_pos;
   if ((new_pos_y > 0) || (labs(new_pos_y) > gcsGlobal->y_travel)) {
       writeline("Y axis out of range.  No move.",0);
       return GCSERR_GUIDER_OUTOFRANGE;
   }
   del_pos_x = new_pos_x - gcsGlobal->x_step_pos;
   del_pos_y = new_pos_y - gcsGlobal->y_step_pos;
   xtimeout = labs(del_pos_x) * 150 / 725000 + 5;
   ytimeout = labs(del_pos_y) * 150 / 4000000 + 5;
   timeout = xtimeout > ytimeout ? xtimeout : ytimeout ;
   timeout = timeout>10 ? timeout : 10 ;
   sprintf(buffer,"timeout: %ld %ld %ld",xtimeout,ytimeout,timeout);
   writeline(buffer,1);

  ntry = (timeout < 150 ? 0 : 2);

  while (ntry++ < 3 ) {
   // command the moves
   BOOL x_done = TRUE;
   BOOL y_done = TRUE;

   if (x_axis||absolute) {
     sprintf(buffer, "ax ac%lu; vl%lu; ma%ld; gd; id;",
	       gcsGlobal->x_acceleration,
	       gcsGlobal->x_max_velocity,
	       new_pos_x);
     writeline(buffer,1);
     pcx_send_commands(GUIDERPC38,buffer);
     x_done = FALSE;
   }
   if (y_axis||absolute) {
     sprintf(buffer, "ay ac%lu; vl%lu; ma%ld; gd; id;",
	       gcsGlobal->y_acceleration,
	       gcsGlobal->y_max_velocity,
	       new_pos_y);
     writeline(buffer,1);
     pcx_send_commands(GUIDERPC38,buffer);
     y_done = FALSE;
   }

   // wait for done flags
   AXISPACK axisRec;
   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(timeout);
   do {
       if (!x_done) {
	 pcx_get_axis_status(GUIDERPC38,'x', axisRec);
	 x_done = axisRec.done;
       }

       if (!y_done) {
	 pcx_get_axis_status(GUIDERPC38,'y', axisRec);
	 y_done = axisRec.done;
       }
       update_all();
   } while (!(x_done && y_done) && !TimeoutTimer.Expired());
   pcx_clear_done_flags(GUIDERPC38,pcx_x_axis | pcx_y_axis );

   if (!TimeoutTimer.Expired()) {
     if (absolute) {
       gcsGlobal->x_step_pos = x_axis;
       gcsGlobal->y_step_pos = y_axis;
     } else {
       gcsGlobal->x_step_pos += x_axis;
       gcsGlobal->y_step_pos += y_axis;
     }
     sky_per_xsteps = gcsGlobal->microns_per_xsteps*gcsGlobal->arcsec_per_micron;
     x0 = gcsGlobal->x_home_pos;
     y0 = gcsGlobal->y_home_pos;
     writegcsscf(gcsGlobal);
     return GCSERR_OK;
   } else {
     pcx_send_commands(GUIDERPC38,"ax st; ay st lf;");
     gcsGlobal->x_step_pos = return_position(GUIDERPC38,'x',"rp",0);
     gcsGlobal->y_step_pos = return_position(GUIDERPC38,'y',"rp",0);
     guider_initialize(0,gcsGlobal->x_step_pos,gcsGlobal->y_step_pos);

   }
  }
  writeline("error during move! ",0);
  writeline("error during move! Guider de-initialized!",0);
  guider_initialized = FALSE;
  gcsGlobal->x_step_pos = -1;
  gcsGlobal->y_step_pos = -1;
  return GCSERR_GUIDER_NOTINIT;
}

int guider_focus(int telsteps, int absolute)
{
   long xsteps, ysteps, ypos;
   double skypos, sky_per_xsteps;

   if (absolute == 0) {
     ysteps = telsteps * gcsGlobal->ysteps_per_telsteps;
     xsteps = ysteps * gcsGlobal->xsteps_per_ysteps;
     guider_move_steps(xsteps, ysteps, 0);
   } else {
     ysteps = telsteps * gcsGlobal->ysteps_per_telsteps + 
              gcsGlobal->y_home_pos - gcsGlobal->y_step_pos;
     xsteps = ysteps * gcsGlobal->xsteps_per_ysteps;
     guider_move_steps(xsteps, ysteps, 0);
   }
}

int guider_skypos(double position)
{
   double xpos, ypos, x0, y0, sky_per_xsteps;

   x0 = gcsGlobal->x_home_pos;
   y0 = gcsGlobal->y_home_pos;
   sky_per_xsteps = gcsGlobal->microns_per_xsteps*gcsGlobal->arcsec_per_micron;
   //ypos = y0 ; // + correction for sky position
   ypos = gcsGlobal->y_step_pos;
   xpos = (position - gcsGlobal->x_home_pos_arcsec +
	     (ypos-y0)*gcsGlobal->xsteps_per_ysteps*sky_per_xsteps ) /
	     sky_per_xsteps + x0;
   guider_move_steps(xpos, ypos, 1);

   // update instrument block
   sysGlobal->xc[2] = -1 * position;
   writesysscf(sysGlobal);

}

int guider_set_home(long xpos, long ypos, double skypos)
{
   gcsGlobal->x_home_pos = xpos;
   gcsGlobal->y_home_pos = ypos;
   gcsGlobal->x_home_pos_arcsec = skypos;
   writegcsscf(gcsGlobal);
   sprintf(outbuf,"x home: %ld  y home: %ld  sky home: %lf",
     gcsGlobal->x_home_pos, gcsGlobal->y_home_pos,gcsGlobal->x_home_pos_arcsec);
   writeline(outbuf,1);
}
