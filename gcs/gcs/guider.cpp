#include <stdio.h>
#include <stdlib.h>
#include "pcx.h"
#include "io.h"
#include "guider.h"
#include "evtimer.h"
#include "error.h"
#include "scf.h"

long return_step_position(char);

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

   char buffer[81];
   sprintf(buffer, "ef pa0 ax ac%lu vl%lu ln;",
	   gcsGlobal->x_acceleration,
	   gcsGlobal->x_max_velocity);
   pc38_send_commands(buffer);
   sprintf(buffer, "ay ac%lu vl%lu ln;",
	   gcsGlobal->y_acceleration,
	   gcsGlobal->y_max_velocity);
   pc38_send_commands(buffer);


   if (fullinit) {
     pc38_send_commands("ax km; ay km;");
     BOOL x_done = FALSE;
     BOOL y_done = FALSE;
     AXISPACK axisRec;

     EventTimer TimeoutTimer;
     TimeoutTimer.NewTimerSecs(150);
     do {
       if (!x_done) {
	 pc38_get_axis_status('x', axisRec);
	 x_done = axisRec.limit;
       }

       if (!y_done) {
	 pc38_get_axis_status('y', axisRec);
	 y_done = axisRec.limit;
       }
     } while (!(x_done && y_done) && !TimeoutTimer.Expired());

     if (!TimeoutTimer.Expired()) {
       pc38_send_commands("ax lp0; ay lp0;");
  
       gcsGlobal->x_step_pos = 0L;
       gcsGlobal->y_step_pos = 0L;
       guider_initialized = TRUE;
       writegcsscf(gcsGlobal);
       return(GCSERR_OK);
     } else {
       pc38_send_commands("ax st; ay st;");
       writeline("Error initializing guider",0);
       guider_initialized = FALSE;
       return(GCSERR_GUIDER_NOTINIT);
     }
  
   } else {
     sprintf(buffer, "ax lp%ld; ay lp%ld;", x_pos, y_pos);
	 writeline(buffer,1);
     pc38_send_commands(buffer);
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
   xtimeout = labs(del_pos_x) * 150 / 725000 ;
   ytimeout = labs(del_pos_y) * 150 / 4000000 ;
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
     pc38_send_commands(buffer);
     x_done = FALSE;
   }
   if (y_axis||absolute) {
     sprintf(buffer, "ay ac%lu; vl%lu; ma%ld; gd; id;",
	       gcsGlobal->y_acceleration,
	       gcsGlobal->y_max_velocity,
	       new_pos_y);
     writeline(buffer,1);
     pc38_send_commands(buffer);
     y_done = FALSE;
   }

   // wait for done flags
   AXISPACK axisRec;
   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(timeout);
   do {
       if (!x_done) {
	 pc38_get_axis_status('x', axisRec);
	 x_done = axisRec.done;
       }

       if (!y_done) {
	 pc38_get_axis_status('y', axisRec);
	 y_done = axisRec.done;
       }
   } while (!(x_done && y_done) && !TimeoutTimer.Expired());
   pc38_clear_done_flags(pcx_x_axis | pcx_y_axis );

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
     pc38_send_commands("ax st; ay st lf;");
     gcsGlobal->x_step_pos = return_step_position('x');
     gcsGlobal->y_step_pos = return_step_position('y');
     guider_initialize(0,gcsGlobal->x_step_pos,gcsGlobal->y_step_pos);

   }
  }
  writeline("error during move! ",0);
  writeline("error during move! Guider de-initialized!",0);
  guider_initialized = FALSE;
  return GCSERR_GUIDER_NOTINIT;
}

long return_step_position(char axis)
{
   #ifdef no_pc38
   return 0L;
   #else
   char buffer[11];
   char number[21];
   int idx = 0;
   byte rbyte;

   sprintf(buffer, "a%c rp;", axis);
   pc38_send_commands(buffer);
   do
     rbyte = pc38_read_byte();
   while ((rbyte == 0) || (rbyte == 10) || (rbyte == 13));
   number[idx++] = rbyte;

   do {
       rbyte = pc38_read_byte();
       if ((rbyte != 0) && (rbyte != 10) && (rbyte != 13))
	 number[idx++] = rbyte;
   } while (rbyte != 10);
   number[idx] = 0;

   do
     rbyte = pc38_read_byte();
   while (rbyte);

   return atol(number);
   #endif
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
}
