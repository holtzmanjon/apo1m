#ifndef _SCF_H
	#define _SCF_H

void writeautoGlobal();
void writesysGlobal();
void readtoccscf(struct TELESCF *);
void writetoccscf(struct TELESCF *);
void readsysscf(struct SYSSCF *);
void writesysscf(struct SYSSCF *);
void initgcsscf(struct GCSSCF *);
void readgcsscf(struct GCSSCF *);
void writegcsscf(struct GCSSCF *);
void printgcsscf(struct GCSSCF *);

extern int setup_scf();
extern int edit_sysscf();
extern int edit_sysscf2();
extern int edit_sysscf3();
extern int edit_toccscf();

#include "mytype.h"

#define MAXINST 5

struct TELESCF
{
	double  home_dec;       // home position declination (degrees)
	double  home_ha;        // home position hour-angle (hours)
	double  td1;            // used to be home epoch
	long    t_step_pos;     // current t-axis position
	long    u_step_pos;     // current u-axis position
	long    v_step_pos;     // current v-axis position
	BOOL    shutter_opened; // current position of dome shutter
	BOOL    lower_shutter_opened; // current position of dome shutter
	double  dome_azimuth;   // current azimuth of the dome
	long    z_axis_pos;     // last position of the instrument rotator
	BOOL    dome_slaved;    // current status of dome slaving to telescope
	BOOL    mc_enabled;     // mount corrections enabled
	double  home_alt;       // home position declination (degrees)
	double  home_az;        // home position hour-angle (hours)
	double  home_rot;       // home position hour-angle (hours)
	double  saved_alt;      // home position alt (degrees)
	double  saved_az;       // home position az (degrees)
	double  saved_rot;      // home position rot (degrees)
	BOOL    mirror_covers_open; // mirror covers opened?
	char    mc_file[64];    // mount correction file
	double  last_fill_utc;  // time of last CCD fill
        double  tpoint_ia;
        double  tpoint_ie;
        double  tpoint_npae;
        double  tpoint_ca;
        double  tpoint_an;
        double  tpoint_aw;
        double  tpoint_nrx;
        double  tpoint_nry;
        int     tertiary_port;
} ;

struct SYSSCF
{
	// general
	double  latitude;
	double  longitude;
	double  altitude;       // meters
	byte    mount_type;  // 0 equat, 1 alt-az cassegrain, 2 alt-az naysmith
	word    aperture;       // primary aperture (cm)

	// x-axis
	long    x_max_velocity;
	long    x_acceleration;
	double  x_steps_degree;
	BOOL    tb1;
	double  td1;
	long    x_park_steps;
	BOOL    x_geartrain_normal;
	long    x_pos_soft_limit;
	long    x_neg_soft_limit;
	BOOL    x_home_dir_reversed;

	// y-axis
	long    y_max_velocity;
	long    y_acceleration;
	double  y_steps_degree;
	BOOL    tb2;
	double  td2;
	long    y_park_steps;
	BOOL    y_geartrain_normal;
	long    y_pos_soft_limit;
	long    y_neg_soft_limit;
	BOOL    y_home_dir_reversed;

	// z-axis
	long    z_max_velocity;
	long    z_acceleration;
	double  z_steps_degree;
	long    z_park_steps;
	BOOL    z_geartrain_normal;
	BOOL    z_axis_enabled;
	double  td3;
	long    z_pos_soft_limit;
	long    z_neg_soft_limit;
	BOOL    z_soft_limits_enabled;
	BOOL    z_home_dir_reversed;

	// secondary
	word    tuv_acceleration;
	word    tuv_max_velocity;
	word    tuv_travel;
	word    tuv_tilt;
	byte    tuv_type;       // 0 none; 1 3-axis; 2 1-axis

	// mirror covers
	BOOL    mirror_covers_installed;
	byte    mirror_cover_delay;
	BOOL    mirror_sets_normal;
	BOOL    mirror_set1_normal;
	BOOL    mirror_set2_normal;

	// enclosure
	byte    enclosure_type; // 0 none; 1 roll-off; 2 dome
	double  dome_counts_degree;
	word    dome_max_init_time;
	double  dome_home_azimuth;
	BOOL    dome_encoder_normal;
	BOOL    dome_geartrain_normal;
	BOOL    dome_use_lookup_table;
	long    dome_lookup_table[13];  // -1800 to +1800 in 300 steps
	BOOL    slip_rings_installed;
	byte    shutter_delay;
	byte    lower_shutter_delay;
	double  shutter_size;
	BOOL    shutter_gear_normal;

	// observation window
	double  window_values[12];      // 00 .. 3600 in 300 steps

	// shutdown parameters
	word    shutdown_wind_speed;
	word    shutdown_humidity;
	int     shutdown_int_low_temp;
	int     shutdown_int_high_temp;
	int     shutdown_out_low_temp;
	int     shutdown_out_high_temp;
	BOOL    shutdown_enabled;
	word    shutdown_delay_ws;              // minutes
	word    shutdown_delay_rain;  // minutes
	byte    max_wstation_failures;
	long    weather_check_frequency;        // seconds

	BOOL    check_35m_closed;
	long    a35m_check_frequency;
	char    a35m_check_opened[64];
	char    a35m_check_closed[64];

	// observatory parameters
	byte    weather_station_com_ch;
	word    tw1;                                                                                    // default_barometer;
	double  default_outside_temp;

	// installed instruments
	BOOL    is400_installed;
	BOOL    ssp3_installed;
	BOOL    ssp5_installed;
	BOOL    ssp7_installed;
	BOOL    ccd100_installed;
	BOOL    inst_extra[10];

	// installed subsystems
	BOOL    weather_installed;
	BOOL    ups_installed;
	BOOL    rain_detector_installed;
	BOOL    watchdog_installed;
	byte    wwv_type;       // 0 none; 1 UTS-10; 2 CTS-10

		// encoders
	BOOL      x_encoder_installed;
	BOOL      x_encoder_tb1;                // used to be geartrain normal
	long      x_encoder_motor_steps_rev;
	long      x_encoder_encoder_steps_rev;
	double    x_encoder_encoder_steps_deg;
	long      x_encoder_slew_hold_vel;
	unsigned  x_encoder_slew_hold_gain;
	unsigned  x_encoder_slew_hold_deadband;
	long      x_encoder_fine_hold_vel;
	unsigned  x_encoder_fine_hold_gain;
	unsigned  x_encoder_fine_hold_deadband;
	BOOL      y_encoder_installed;
	BOOL      y_encoder_tb1;                // used to be geartrain normal
	long      y_encoder_motor_steps_rev;
	long      y_encoder_encoder_steps_rev;
	double    y_encoder_encoder_steps_deg;
	long      y_encoder_slew_hold_vel;
	unsigned  y_encoder_slew_hold_gain;
	unsigned  y_encoder_slew_hold_deadband;
	long      y_encoder_fine_hold_vel;
	unsigned  y_encoder_fine_hold_gain;
	unsigned  y_encoder_fine_hold_deadband;
	BOOL      z_encoder_installed;
	BOOL      z_encoder_tb1;                // used to be geartrain normal
	long      z_encoder_motor_steps_rev;
	long      z_encoder_encoder_steps_rev;
	double    z_encoder_encoder_steps_deg;
	long      z_encoder_slew_hold_vel;
	unsigned  z_encoder_slew_hold_gain;
	unsigned  z_encoder_slew_hold_deadband;
	long      z_encoder_fine_hold_vel;
	unsigned  z_encoder_fine_hold_gain;
	unsigned  z_encoder_fine_hold_deadband;

	double default_humidity;
	unsigned move_fudge_factor;
	int encoder_tracking;
	BOOL keypad_lr_reversed;
	BOOL keypad_ud_reversed;
	double keypad_fudge_factor;

	double sx[MAXINST];
	double sy[MAXINST];
	double xc[MAXINST];
	double yc[MAXINST];
	double rot[MAXINST];
	double rot0[MAXINST];
	BOOL fixed[MAXINST];

	double ccd_hold_time;   // ccd hold time in hours
	double ccd_degrees_per_volt;   // ccd hold time in hours
};

struct GCSSCF
{
        long    x_step_pos;     // current t-axis position
        long    y_step_pos;     // current u-axis position
        long    z_step_pos;     // current v-axis position
        int     current_filter_pos;
        long    x_max_velocity;
        long    x_acceleration;
        double  x_steps_degree;
        long    x_travel;
        long    y_max_velocity;
        long    y_acceleration;
        double  y_steps_degree;
        long    y_travel;
        long    z_max_velocity;
        long    z_acceleration;
        long    z_steps_rot;
        long    t_max_velocity;
        long    t_acceleration;
        long    t_pos;
        long    t_pos_na1;
        long    t_pos_na2;
        double  ysteps_per_telsteps;
        double  xsteps_per_ysteps;
        long    x_home_pos;     // home x-axis position (steps)
        long    y_home_pos;     // home y-axis position (steps)
        int     z_home_pos;     // home z-axis position (filter position)
        double  microns_per_xsteps;
        double  arcsec_per_micron;
        double  x_home_pos_arcsec;
        int     timeout;
} ;

extern struct SYSSCF *sysGlobal, sysGlobalNew;
extern struct TELESCF *autoGlobal, autoGlobalNew;
extern struct GCSSCF *gcsGlobal, gcsGlobalNew;

#endif
