/****************************************************************************/
/*							*/
/*	Module:		globals.h			*/
/*							*/
/*	Purpose:	Definitions of globals used by all the other modules*/
/*							*/
/****************************************************************************/
/*							*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION */
/*                        2637 Midpoint Dr., Suite D    */
/*                         Fort Collins, CO  80525      */
/*							*/
/*                            Copyright 1995            */
/*              Unauthorized duplication or use is prohibited.*/
/*							*/
/*							*/
/*	Author:		M. Donahue			*/
/*							*/
/****************************************************************************/
#ifndef _GLOBALS_H
	#define _GLOBALS_H
	
#include <stdio.h>

//------------------------------------------------------------------
//	include our common type definitions
//------------------------------------------------------------------
#include "mytype.h"
#include "tcs_math.h"
#include "evtimer.h"

#include "scf.h"

//------------------------------------------------------------------
//	These are the various constants /variables used by other modules.
//------------------------------------------------------------------
// This array is used by the mail messaging system
#define MAX_MAIL_VARS 10
extern double mail_var_array[MAX_MAIL_VARS];

extern const char	*ENV_SCF_DIR;

// The standard sidereal rate
// radians per second decrease in RA (15.04107 arcsec/sec)
#define gc_stellar_rate		-7.2921165145e-5	

extern long move_vals[8];
extern int move_idx;
extern long kp_move_increment;

extern double		d_arr[13];
extern long 		dif_arr[12];
extern double	 	og_last_dome_az_error;
extern long		og_move_counts;
extern long		og_new_lookup_count;
extern long		og_max_move_counts;
extern long		og_starting_counts;
extern long		og_current_counts;
extern BOOL		og_enc_pos_dir;
extern BOOL		og_dome_inc_az;
extern BOOL		og_dome_done;
extern EventTimer	og_dome_timer;

extern BOOL move_the_dome;


#define MAXINST 5

//------------------------------------------------------------------
//	These are the various globals used by other modules.
//------------------------------------------------------------------
class TOCGLOBAL
	{
		public:

		TOCGLOBAL();
		~TOCGLOBAL();
		void re_calc();

		FILE *move_file;
		FILE *track_file;
		FILE *pcx_file;
		FILE *debug_file;

		//
		// these are calculated once from the .SCF files
		//
		double longitude;		// radians
		double latitude;		// radians

		double x_enc_to_motor;	// encoder to motor ratio x-axis
		double y_enc_to_motor;	// encoder to motor ratio y-axis

		//
		// these are used by the TOCC routines and change frequently
		//
		// current mean position variables (catalog)
		double current_mean_ra;		// decimal RA in radians
		double current_mean_dec;	// decimal Dec in radians
		double current_mean_epoch;
		double current_mean_pmra;	// radians per year
		double current_mean_pmdec;	// radians per year
		double current_mean_parallax;
		double current_mean_radial_velocity;
		double current_mean_eff_wavelength;
		double current_tlr;
                BOOL apply_rates;
		double dra;
		double ddec;
		double epoch0;
		double telescope_move_lst; // lst when telescope starts tracking
		double current_obs_ra;	   // observed RA in decimal hours
		double current_obs_dec;	   // observed Dec in decimal degrees
		double current_pa;	   // current/desired PA in radians
		long current_enc_alt;	   // encoder alt in encoder steps
		long current_enc_az;	   // encoder az in encoder steps
		long current_enc_rot;	   // encoder rot in encoder steps
		double current_obs_alt;	   // observed alt in decimal degrees
		double current_obs_az;	   // observed az in decimal degrees
		double current_obs_rot;	   // observed rot in decimal degrees
                double last_az_error;      // last computed az error (degrees)
                double last_alt_error;     // last computed alt error (degrees)
                double last_rot_error;     // last computed rot error (degrees)
		double ref_alt;	           // reference alt in decimal degrees
		double ref_az;	           // reference az in decimal degrees
		double ref_rot;	           // reference rot in decimal degrees
                double current_utc;        // current UTC as modified JD
                double current_lasth;      // current LAST in hours
                double guide_err_alt;      // guider error in alt (radians)
                double guide_err_az;       // guider error in az (radians)
                double guide_err_ra;      // guider error in ra (radians)
                double guide_err_dec;       // guider error in dec (radians)
				double guide_factor;

		// tracking
                BOOL    tracking_on;
		double 	x_tracking_rate;   // current tracking rates in 
                                           // motor steps/sec not taking
		double 	y_tracking_rate;   // geartrain reversals into account:
		double 	z_tracking_rate;   // sign indicates desired direction
		double 	x_encoder_error;
		double 	y_encoder_error;
		double 	z_encoder_error;
		double	x_tracking_factor;
		double	y_tracking_factor;
		double	z_tracking_factor;
		double	x_tracking_factor_new;
		double	y_tracking_factor_new;
		double	z_tracking_factor_new;
		BOOL	tracking_factor_mod;
                double last_x_rate;
                double last_y_rate;
                double last_z_rate;
		int tracking_dtime;
                int encoder_tracking;
                int x_encoder_tracking;
                int y_encoder_tracking;
                int z_encoder_tracking;
                BOOL y_axis_separate;

		double max_rate_change;

		// slalib
		double ut1_minus_utc;
		double polor_motion_x;
		double polor_motion_y;

		// mean to apparent parameters
		double mean_to_app_parms[21];
		double app_to_obs_parms[14];

		// system status variables
		BOOL telescope_initialized;
		BOOL encoder_initialized;
		BOOL telescope_session_init;
		BOOL telescope_at_home;
		BOOL telescope_is_slewing;
                int  nmove;
                BOOL x_encoder_installed;
                BOOL y_encoder_installed;
                BOOL z_encoder_installed;

		BOOL dome_initialized;
		BOOL dome_slaved;
		BOOL dome_open;
		BOOL lower_dome_open;
		BOOL dome_part_open;
		BOOL lower_dome_part_open;
                int  dome_azimuth;

                BOOL mirror_covers_open;
 
                BOOL mc_enabled;
                char mc_file[64];

		double current_aux_temp;    // store as 0C
		double current_cab_temp;    // store as 0C
		double current_out_temp;    // store as 0C
		double current_barometer;   // store in millibars
		double current_humidity;    // store as number between 0 and 1
		int current_windspeed;   
		int current_winddir;     
		BOOL check_35m_closed;

                double ccd_fill_dtime;      // time since last CCD fill
                double ccd_temp;            // CCD temperature

		// shutdown
		BOOL		need_to_shutdown;
		unsigned  shutdown_state;   // which conditions are causing shutdown
		unsigned	weather_failures;

		// TPOINT model parameters for alt-az systems
		double	tpoint_an;	// azimuth axis misalignment north
		double	tpoint_aw;	// azimuth axis misalignment west
		double	tpoint_ca;	// horizontal colimation error
		double	tpoint_npae;	// az/el nonperpendicularity
		double	tpoint_tf;	// tube flexure
		double	tpoint_ia;	// azimuth index error
		double	tpoint_ie;	// elevation index error

		// additional TPOINT model parameters for alt-az Nasmyth systems
		double	tpoint_nrx;	// Nasmyth misalignment horizontal
		double	tpoint_nry;	// Nasmuth misalignment vertical

		// TPOINT model parameters for equatorial fork systems
		double	tpoint_me;	// polar axis misalignment up/down
		double	tpoint_ma;	// polar axis misalignment left/right
		double	tpoint_ch;	// east/west collimation error
		double	tpoint_np;	// HA/Dec nonperpendicularity
		double	tpoint_fo;	// fork flexure
		double	tpoint_ih;	// HA index error
		double	tpoint_id;	// Dec index error

                int current_inst;

                int update;
                int handpaddle;
                int delaltaz;
                BOOL use_encoders;
		BOOL verbose;
		BOOL const_tracking;

                BOOL soft_limits_disabled;

	};

extern TOCGLOBAL *G;

extern void initialize_globals();
extern void initialize_scf_globals();
#endif
/********************************* EOF **************************************/

