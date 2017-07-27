/****************************************************************************/
/*								*/
/*	Module:		globals.cpp				*/
/*								*/
/*	Purpose:	all global variables used in this program*/
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
/*	Author:		M. Donahue				*/
/*								*/
/****************************************************************************/
#include <mem.h>

#include "globals.h"
#include "scf.h"
#include "tcs_math.h"
#include "slamac.h"
#include "evtimer.h"

double mail_var_array[MAX_MAIL_VARS];
const char *ENV_SCF_DIR = "V4SCFDIR";

double kp_ra_sec_error = 0;
double kp_dec_arcsec_error = 0;

long move_vals[8] = { 2, 10, 60, 600, 3600, 18000L, 36000L, 72000L };
int move_idx = 2;
long kp_move_increment = 60;

double d_arr[13];
long dif_arr[12];
double og_last_dome_az_error;
long og_move_counts;
long og_new_lookup_count;
long og_max_move_counts;
long og_starting_counts;
long og_current_counts;
BOOL og_enc_pos_dir;
BOOL og_dome_inc_az;
BOOL og_dome_done;
EventTimer og_dome_timer;

BOOL move_the_dome = FALSE;

TOCGLOBAL *G = NULL;

TOCGLOBAL::TOCGLOBAL()
{
   // set all variables to 0
   memset(this, 0, sizeof(*this));

   // set default here
   current_tlr = 0.0065;

   // calculate some other globals
   re_calc();
}

TOCGLOBAL::~TOCGLOBAL() {}

void TOCGLOBAL::re_calc()
{
   latitude = sysGlobal->latitude * DD2R;
   longitude = -sysGlobal->longitude * DD2R;
   current_barometer = 1013.25 * exp(-sysGlobal->altitude / 8149.9415);

   if (sysGlobal->x_encoder_installed)
     x_enc_to_motor = (sysGlobal->x_steps_degree != 0.0) ?
     (double)sysGlobal->x_encoder_encoder_steps_rev /
     (double)sysGlobal->x_encoder_motor_steps_rev : 0.0;

   if (sysGlobal->y_encoder_installed)
     y_enc_to_motor = (sysGlobal->y_steps_degree != 0.0) ?
     (double)sysGlobal->y_encoder_encoder_steps_rev /
     (double)sysGlobal->y_encoder_motor_steps_rev : 0.0;

   current_out_temp = sysGlobal->default_outside_temp;
   current_humidity = sysGlobal->default_humidity;

   x_tracking_factor = 1.0;
   y_tracking_factor = 1.0;
   z_tracking_factor = 1.0;
   x_encoder_error = 1.0;
   y_encoder_error = 1.0;
   z_encoder_error = 1.0;
   tracking_dtime = 100;
   tracking_dtime = 30;

   guide_factor = 1.0;

   current_mean_epoch = 1950.;

   //  Set these into G so they can easily be passed over remote line
   x_encoder_installed = sysGlobal->x_encoder_installed;
   y_encoder_installed = sysGlobal->y_encoder_installed;
   z_encoder_installed = sysGlobal->z_encoder_installed;
   encoder_tracking = sysGlobal->encoder_tracking;
   dome_slaved = autoGlobal->dome_slaved;
   dome_open = autoGlobal->shutter_opened;
   lower_dome_open = autoGlobal->lower_shutter_opened;
   mc_enabled = autoGlobal->mc_enabled;
   max_rate_change = 10.;
}

void initialize_globals()
{
  G = new TOCGLOBAL();
  G->update = 0;
  G->handpaddle = 0;
  G->delaltaz = 0; 
  G->use_encoders = FALSE; 
  G->move_file = NULL;
  G->track_file = NULL;
  G->pcx_file = NULL;
  G->debug_file = NULL;
  G->check_35m_closed = sysGlobal->check_35m_closed;
#ifdef debug_move_file
  G->move_file = fopen("e:\\tocc\\moves.dbx", "w");
  setbuf(G->move_file,NULL);
#endif
#ifdef debug_tracking_file
  // G->track_file = fopen("e:\\tocc\\track.dbx", "w");
  G->track_file = fopen("track.dbx", "w");
  setbuf(G->track_file,NULL);
#endif
#ifdef debug_pcx_file
  G->pcx_file = fopen("e:\\tocc\\pcx.dbx", "w");
  setbuf(G->pcx_file,NULL);
#endif
#ifdef have_debug_file
  G->debug_file = fopen("e:\\tocc\\debug.dbx", "w");
  setbuf(G->debug_file,NULL);
#endif
#ifndef no_hardware
  G->move_file = fopen("e:\\tocc\\track.dat", "w");
#endif
}
/********************************* EOF ***********************************/

