#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include "mytype.h"
#include "pcx.h"
#include "io.h"
#include "tracking.h"
#include "systimer.h"
#include "evtimer.h"
#include "filter.h"
#include "status.h"
#include "scf.h"

#define KLUDGE

#define FILTER0_BIT 0x01
#define FILTER1_BIT 0x02
#define FILTER2_BIT 0x04
#define FILTER3_BIT 0x08
#define DETENTOUT_BIT 0x10
#define DETENTIN_BIT 0x20

// Assumes z is focus motor
//long filter_z_acceleration = 40000; 
//long filter_z_max_velocity = 25000;
int nfilt = 10;
//long steps_per_rot = -170000;

BOOL filter_initialized = FALSE;
//int current_filter_pos;

int new_steps_per_rot(long steps)
{
  gcsGlobal->z_steps_rot = steps;
  writegcsscf(gcsGlobal);
  return GCSERR_OK;
}

// Send guider stage and focus to the limit position
int filter_initialize(long sign)
{
   char buffer[81];
   int ntry;
   #ifdef no_pc38
   return GCSERR_OK;
   #else
   unsigned status;

// Turn off echo and limits
   pcx_send_commands(GUIDERPC38,"ef pa0 az lf;");
   //sprintf(buffer, "az lp0 ac%lu vl%lu;", gcsGlobal->z_acceleration,
   sprintf(buffer, "az ac%lu vl%lu;", gcsGlobal->z_acceleration,
           gcsGlobal->z_max_velocity/4);
   writeline(buffer,1);
   pcx_send_commands(GUIDERPC38,buffer);

// Put the detent in and see if this is successful. If so, we are at a
//  valid position. If not, move the wheel slowly until detent is in

   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(8);
   status = move_detent_out();
   status = move_detent_in();
#ifdef OLDWAY
   if (status) {
     pcx_send_commands(GUIDERPC38,"bl9;");
     delay(500);
     if (sign >= 0)
       sprintf(buffer, "az jg%ld;", gcsGlobal->z_max_velocity/100);
     else
       sprintf(buffer, "az jg%ld;", -gcsGlobal->z_max_velocity/100);
	 writeline(buffer,1);
     pcx_send_commands(GUIDERPC38,buffer);
     do {
     } while (!is_detent_in() && !TimeoutTimer.Expired());
     pcx_send_commands(GUIDERPC38,"az st;");
     pcx_send_commands(GUIDREPC38,"bh9;");
	 if (TimeoutTimer.Expired() ) {
	   writeline("Error: filter wheel initialization timed out",0);
	   sprintf(outbuf,"is_detent: %d %d\n",is_detent_in(),is_detent_out());
	   writeline(outbuf,0);
	   return(GCSERR_FILTER_NOTINIT);
	 }
	 status = 0;
	 delay(500);
   }
#else
   if (!is_detent_in()) {
    ntry=0;
    gcsGlobal->current_filter_pos=11;
    while (gcsGlobal->current_filter_pos>10 && ntry < 5) {
sprintf(outbuf,"current filter pos: %d ntry: %d\n",gcsGlobal->current_filter_pos,ntry);
writeline(outbuf,1);
     ntry++;
     if (sign >= 0)
       sprintf(buffer, "az km id;");
     else
       sprintf(buffer, "az kr id;");
     writeline(buffer,1);
     pcx_send_commands(GUIDERPC38,buffer);
     do { 
           update_tracking_rates();
           update_status(0);
           update_display();
     } while (!pcx_done(GUIDERPC38,pcx_z_axis) && !TimeoutTimer.Expired());
     if (TimeoutTimer.Expired() ) {
       writeline("Error: filter wheel initialization timed out",0);
       pcx_send_commands(GUIDERPC38,"az st;");
       return(GCSERR_FILTER_NOTINIT);
     }
     delay(500);
     gcsGlobal->current_filter_pos = get_filter_pos();
     sprintf(outbuf,"in filt_init, pos: %d\n",gcsGlobal->current_filter_pos);
     writeline(outbuf,0);
    }
   }
   
   //delay(1000);
   delay(500);
  
#endif
   gcsGlobal->current_filter_pos = get_filter_pos();
   filter_initialized = TRUE;
   sprintf(outbuf,"z returns: %ld filtpos: %d\n",return_position(GUIDERPC38,'z',"rp",0),gcsGlobal->current_filter_pos);
   writeline(outbuf,0);

   gcsGlobal->z_step_pos = 0;
   sprintf(buffer, "az lp0 ac%lu vl%lu;", gcsGlobal->z_acceleration,
           gcsGlobal->z_max_velocity);
   pcx_send_commands(GUIDERPC38,buffer);
   writegcsscf(gcsGlobal);

   return(status);


   #endif
}
int move_detent_in()
{
   int status;

   //Drive detent in
   pcx_send_commands(GUIDERPC38,"bl9;");
 //  delay(500);

   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(5);
   while (!is_detent_in() && !TimeoutTimer.Expired()) {}

//  sprintf(outbuf,"%d %d %d\n",TimeoutTimer.ElapsedTimeInMSecs(),is_detent_in(),is_detent_out());
//  writeline(outbuf,0);

   if (TimeoutTimer.Expired()||is_detent_out()||!is_detent_in())  {
     sprintf(outbuf,"detent_in failed: %d %d %d\n",TimeoutTimer.Expired(),
	    is_detent_in(), is_detent_out());
     writeline(outbuf,0);
     status = GCSERR_FILTER_TIMEOUT;
   } else
     status = GCSERR_OK;

   //Turn off detent motor
   pcx_send_commands(GUIDERPC38,"bh9;");
   delay(500);

   return(status);

}
int move_detent_out()
{
   int status;

   //Drive detent in
   pcx_send_commands(GUIDERPC38,"bl8;");
   delay(500);

   EventTimer TimeoutTimer;
   TimeoutTimer.NewTimerSecs(5);
   //TimeoutTimer.NewTimerSecs(1);
   //TimeoutTimer.NewTimer(9);
   while (!is_detent_out() && !TimeoutTimer.Expired()) {}

//  sprintf(outbuf,"%d %d %d\n",TimeoutTimer.ElapsedTimeInMSecs(),is_detent_in(),is_detent_out());
//  writeline(outbuf,0);

   if (TimeoutTimer.Expired()||!is_detent_out||is_detent_in()) {
   //if (is_detent_in()) {
     sprintf(outbuf,"detent_out failed: %d %d %d\n",TimeoutTimer.Expired(),
	    is_detent_in(), is_detent_out());
     writeline(outbuf,0);
     status = GCSERR_FILTER_TIMEOUT;
   } else
     status = GCSERR_OK;

   //Turn off detent motor
   pcx_send_commands(GUIDERPC38,"bh8;");
   delay(500);

   return(status);
}

BOOL is_detent_in()
{
   #ifdef no_filter
   return TRUE;
   #else
   unsigned rval = pcx_read_inputs(GUIDERPC38);
   rval &= DETENTIN_BIT;
   return (rval) ? TRUE : FALSE;
   #endif
}
BOOL is_detent_out()
{
   #ifdef no_filter
   return TRUE;
   #else
   unsigned rval = pcx_read_inputs(GUIDERPC38);
   rval &= DETENTOUT_BIT;
   return (rval) ? TRUE : FALSE;
   #endif
}

int get_filter_pos()
{
   #ifdef no_filter
   return 0;
   #else
   unsigned rval = pcx_read_inputs(GUIDERPC38);
/*
   sprintf(outbuf,"rval: %u %u\n",  rval,((rval&FILTER3_BIT)>>3) + ((rval&FILTER2_BIT)>>1) +
      ((rval&FILTER1_BIT)<<1) + ((rval&FILTER0_BIT)<<3) );
writeline(outbuf,1);
*/
   return (
      ((rval&FILTER3_BIT)>>3) + ((rval&FILTER2_BIT)>>1) +
      ((rval&FILTER1_BIT)<<1) +((rval&FILTER0_BIT)<<3) );
   #endif
}

int move_filter(int pos)
{
   #ifdef no_filter
   return GCSERR_OK;
   #else
   char buffer[81];
   int status;

   if (!filter_initialized) {
     writeline("Filter wheel not initialized",0);
     return GCSERR_FILTER_NOTINIT;
   }
   int dpos;
   dpos = pos - gcsGlobal->current_filter_pos ;
   if (dpos==0) return(0);
   dpos = (dpos > nfilt-1 ? dpos-nfilt : dpos);
   dpos = (dpos < -nfilt+1 ? dpos+nfilt : dpos);

   // Choose quickest direction to go
   dpos = (dpos > nfilt/2 ? dpos-nfilt : dpos);
   dpos = (dpos < -nfilt/2 ? dpos+nfilt : dpos);

   sprintf(outbuf,"dpos : %d %d %d\n",dpos,pos,gcsGlobal->current_filter_pos);
   writeline(outbuf,0);

   // Disengage detent
   status = move_detent_out();
   if (status) {
     writeline("Error disengaging detent",0);
     writeline("Deinitializing filter wheel",0);
     filter_initialized = FALSE;
     gcsGlobal->current_filter_pos = -1;
   } else {
#ifdef KLUDGE
     EventTimer TimeoutTimer;
     TimeoutTimer.NewTimerSecs(15);
     sprintf(buffer, "az ac%lu vl%lu;", gcsGlobal->z_acceleration,
           gcsGlobal->z_max_velocity);
     pcx_send_commands(GUIDERPC38,buffer);
     sprintf(buffer, "az mr%ld gd id;", (long)(dpos*gcsGlobal->z_steps_rot/nfilt));
     writeline(buffer,1);
     pcx_send_commands(GUIDERPC38,buffer);
     do {
           update_tracking_rates();
           update_status(0);
           update_display();
     } while (!pcx_done(GUIDERPC38,pcx_z_axis) && !TimeoutTimer.Expired());
   }
   sprintf(outbuf,"Calling filter initialize: %ld",dpos*gcsGlobal->z_steps_rot);
   writeline(outbuf,1);
   status = filter_initialize(dpos*gcsGlobal->z_steps_rot);
#else
     sprintf(buffer, "az lp0 mr%ld gd id;", dpos*gcsGlobal->z_steps_rot/nfilt);
     pcx_send_commands(GUIDERPC38,buffer);
	 do {
	 } while (!pcx_done(GUIDERPC38,pcx_z_axis));
   }
   // Engage detent
   status = move_detent_in();
#endif
   if (status) {
     writeline("Error engaging detent",0);
     writeline("Deinitializing filter wheel",0);
     filter_initialized = FALSE;
     gcsGlobal->current_filter_pos = -1;
   } else {
     gcsGlobal->current_filter_pos = get_filter_pos();
     if (gcsGlobal->current_filter_pos != pos) {
       sprintf(outbuf,"Detected filter position: %d",gcsGlobal->current_filter_pos);
       writeline(outbuf,1);
       writeline("Error: filter appears to have gone to wrong position!",0);
       status = GCSERR_FILTER_WRONG;
 // we'll figure that it's more likely that encoders are reading wrong then
 // that we really went to a totally different slot successfully, so set
 // current_position to commanded position, so that next move will most likely
 // succeed.
 #ifndef KLUDGE
       gcsGlobal->current_filter_pos = pos;
 #endif
     }
   }

   writegcsscf(gcsGlobal);
   return(status);


   #endif
}
