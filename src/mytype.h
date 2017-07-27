#ifndef _MYTYPE_H
#define _MYTYPE_H

#define TRUE    1
#define FALSE 0

typedef int BOOL;

struct ALLTIMES
{
    double mjd_tt;  /* terrestrial time as modified julian date */
    double mjd_utc; /* coordinated universal time as modified julian date */
    double last;    /* local apparent sidereal time in radians */
    double lasth;   /* local apparent sidereal time in hours (0 < lasth < 24) */
    double equinox; /* fractional equinox */
} ;

struct STATUS
{
    double current_obs_ra;  
    double current_obs_dec; 
    double current_pa;    
    double current_mean_ra;  
    double current_mean_dec; 
    double current_mean_epoch;
    double current_utc;    
    double current_lasth;  
    double current_obs_az; 
    double current_obs_alt; 
    double current_obs_rot;   
    int  tertiary_port;
    BOOL telescope_initialized;
    BOOL telescope_at_home;
    BOOL tracking_on;
    BOOL telescope_is_slewing;
    BOOL use_encoders;
    int encoder_tracking;
    int x_encoder_tracking;
    int y_encoder_tracking;
    int z_encoder_tracking;
    BOOL x_encoder_installed;
    BOOL y_encoder_installed;
    BOOL z_encoder_installed;
    int  nmove;
    double ut1_minus_utc;
    BOOL dome_initialized;
    BOOL dome_slaved;
    BOOL dome_open;
    BOOL lower_dome_open;
    int dome_azimuth;
    BOOL mirror_covers_open;
    double last_x_rate;
    double last_y_rate;
    double last_z_rate;
    double last_az_error;
    double last_alt_error;
    double last_rot_error;
    int t_step_pos;
    int u_step_pos;
    int v_step_pos;
    double current_cab_temp;
    double current_out_temp;
    double current_aux_temp;
    int current_winddir;
    int current_windspeed;
    int shutdown_state;
    BOOL check_35m_closed;
    double ccd_temp;
    BOOL mc_enabled;
    char mc_file[64];
    double foc;
    double foc_theta;
    double foc_phi;
    int filtfoc;
    double airmass;
    double sx[3];
    double sy[3];
    double theta[3];
    double cx[3];
    double cy[3];
    long guider_x_pos;
    long guider_y_pos;
    long guider_z_pos;
    int  guider_filtpos;
} ;

#define NOBJECT 64
struct CCDSTATUS
{
  int cleans;
  double exposure;
  double darkexposure;
  int ndark;
  char dirname[24];
  char filename[24];
  int filetype;
  BOOL autoxfer;
  BOOL autodisplay;
  BOOL autodark;
  BOOL autoflat;
  int expstatus;
  double end_time;
  int filter;
  int filtfoc;
  int numseq;
  int incval;
  int shutter;
  char object[NOBJECT];
  int objnum;
  int stannum;
  int guiding;
  double guide_x0;
  double guide_y0;
  double guide_theta0;
  double guide_pa;
  double guide_dist;
  double guide_rot0;
  double guide_xoff;
  double guide_yoff;
  double guide_rad;
  double guide_mag;
  int guide_size;
  int guide_update;
  double ax;
  double bx;
  double ay;
  double by;
  double sx;
  double sy;
  double theta;
  double cx;
  double cy;
  double ccd_temp;
  int ccd_temp_status;
  int x0;
  int y0;
  int nx;
  int ny;
  int nbias;
  double xc;
  double yc;
  int offsettype;
  double focus;
} ;
#endif
