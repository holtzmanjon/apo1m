#include <stdio.h>

#include "globals.h"
#include "scf.h"
#include "io.h"

#ifdef no_hardware
#define HOME "c:"
#else
#ifdef SOCKET
#define HOME "C:"
#else
#define HOME "e:"
#endif
#endif

struct TELESCF *autoGlobal, autoGlobalNew;
struct SYSSCF *sysGlobal, sysGlobalNew;
struct GCSSCF *gcsGlobal, gcsGlobalNew;

void readtoccscf(struct TELESCF *t) 
{
  char line[300];
  FILE *fp;

  fp = fopen(HOME"\\tocc\\toccscf.new","r");
  fprintf(stderr,"fp: %d\n",fp);

  fgets(line,299,fp);
  sscanf(line,"%lf", &t->home_dec);
  fgets(line,299,fp);
  sscanf(line,"%lf",  &t->home_ha);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->td1);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->t_step_pos);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->u_step_pos);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->v_step_pos);
  fgets(line,299,fp);
  sscanf(line,"%u",&t->shutter_opened);
  fgets(line,299,fp);
  sscanf(line,"%u",&t->lower_shutter_opened);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->dome_azimuth);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->z_axis_pos);
  fgets(line,299,fp);
  sscanf(line,"%u",&t->dome_slaved);
  fgets(line,299,fp);
  sscanf(line,"%u",&t->mc_enabled);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->home_alt);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->home_az);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->home_rot);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->saved_alt);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->saved_az);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->saved_rot);
  fgets(line,299,fp);
  sscanf(line,"%u",&t->mirror_covers_open);
  fgets(line,299,fp);
  sscanf(line,"%s",&t->mc_file);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->last_fill_utc);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_ia);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_ie);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_npae);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_ca);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_an);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_aw);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_nrx);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->tpoint_nry);
  fgets(line,299,fp);
  sscanf(line,"%d",&t->tertiary_port);

  fclose(fp);
}
void writetoccscf(struct TELESCF *t) 
{
  FILE *fp;
  fp = fopen(HOME"\\tocc\\toccscf.new","w");
  fprintf(fp,
             "%lf : home position declination (degrees)\n"
             "%lf : home position hour-angle (hours)\n"
             "%lf : used to be home epoch\n"
             "%ld : current t-axis position\n"
             "%ld : current u-axis position\n"
             "%ld : current v-axis position\n"
             "%u  : current position of dome shutter (open=1/closed=0)\n"
             "%u  : current position of lower dome shutter (open=1/closed=0)\n"
             "%lf : current azimuth of the dome\n"
             "%ld : last position of the instrument rotator\n"
             "%u  : current status of dome slaving to telescope\n"
             "%u  : mount corrections enabled\n"
             "%lf :  home position alt (degrees)\n"
             "%lf :  home position az-angle (degrees)\n"
             "%lf :  home position rotator (degrees)\n"
             "%lf :  saved position alt (degrees)\n"
             "%lf :  saved position az (degrees)\n"
             "%lf :  saved position rot (degrees)\n"
             "%u  :  mirror covers open\n"
             "%s  :  mount correction file\n"
             "%lf  : utc of last CCD fill \n"
             "%lf  : tpoint_ia \n"
             "%lf  : tpoint_ie \n"
             "%lf  : tpoint_npae \n"
             "%lf  : tpoint_ca \n"
             "%lf  : tpoint_an \n"
             "%lf  : tpoint_aw \n"
             "%lf  : tpoint_nrx \n"
             "%lf  : tpoint_nry \n"
             "%d   : tertiary_port \n",
                t->home_dec,
                t->home_ha,
                t->td1,
                t->t_step_pos,
                t->u_step_pos,
                t->v_step_pos,
                t->shutter_opened,
                t->lower_shutter_opened,
                t->dome_azimuth,
                t->z_axis_pos,
                t->dome_slaved,
                t->mc_enabled,
                t->home_alt,
                t->home_az,
                t->home_rot,
                t->saved_alt,
                t->saved_az,
                t->saved_rot,
                t->mirror_covers_open,
                t->mc_file,
                t->last_fill_utc,
                t->tpoint_ia,
                t->tpoint_ie,
                t->tpoint_npae,
                t->tpoint_ca,
                t->tpoint_an,
                t->tpoint_aw,
                t->tpoint_nrx,
                t->tpoint_nry,
                t->tertiary_port);
   fclose(fp);
}
void readsysscf(struct SYSSCF *s) 
{
  char line[300];
  FILE*fp;

  fp = fopen(HOME"\\tocc\\sysscf.new","r");
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->latitude);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->longitude);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->altitude);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->mount_type);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->aperture);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->x_steps_degree);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tb1);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->td1);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_park_steps);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_geartrain_normal);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_pos_soft_limit);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_neg_soft_limit);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_home_dir_reversed);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->y_steps_degree);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tb2);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->td2);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_park_steps);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_geartrain_normal);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_pos_soft_limit);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_neg_soft_limit);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_home_dir_reversed);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->z_steps_degree);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_park_steps);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_geartrain_normal);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_axis_enabled);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->td3);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_pos_soft_limit);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_neg_soft_limit);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_soft_limits_enabled);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_home_dir_reversed);

// secondary
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tuv_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tuv_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tuv_travel);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tuv_tilt);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tuv_type);

  // mirror covers
  fgets(line,299,fp);
  sscanf(line,"%u", &s->mirror_covers_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->mirror_cover_delay);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->mirror_sets_normal);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->mirror_set1_normal);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->mirror_set2_normal);
// enclosure
  fgets(line,299,fp);
  sscanf(line,"%u", &s->enclosure_type);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->dome_counts_degree);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->dome_max_init_time);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->dome_home_azimuth);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->dome_encoder_normal);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->dome_geartrain_normal);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->dome_use_lookup_table);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->dome_lookup_table[13]);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->slip_rings_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->shutter_delay);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->lower_shutter_delay);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->shutter_size);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->shutter_gear_normal);
// observation window
  fgets(line,299,fp);
  sscanf(line,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
     &s->window_values[0],&s->window_values[1],&s->window_values[2],
     &s->window_values[3],&s->window_values[4],&s->window_values[5],
     &s->window_values[6],&s->window_values[7],&s->window_values[8],
     &s->window_values[9],&s->window_values[10],&s->window_values[11]);
// shutdown parameters
  fgets(line,299,fp);
  sscanf(line,"%u", &s->shutdown_wind_speed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->shutdown_humidity);
  fgets(line,299,fp);
  sscanf(line,"%d", &s->shutdown_int_low_temp);
  fgets(line,299,fp);
  sscanf(line,"%d", &s->shutdown_int_high_temp);
  fgets(line,299,fp);
  sscanf(line,"%d", &s->shutdown_out_low_temp);
  fgets(line,299,fp);
  sscanf(line,"%d", &s->shutdown_out_high_temp);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->shutdown_enabled);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->shutdown_delay_ws);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->shutdown_delay_rain);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->max_wstation_failures);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->weather_check_frequency);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->check_35m_closed);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->a35m_check_frequency);
  fgets(line,299,fp);
  sscanf(line,"%s", &s->a35m_check_closed);
  fgets(line,299,fp);
  sscanf(line,"%s", &s->a35m_check_opened);
// observatory parameters
  fgets(line,299,fp);
  sscanf(line,"%u", &s->weather_station_com_ch);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->tw1);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->default_outside_temp);
// installed instruments
  fgets(line,299,fp);
  sscanf(line,"%u", &s->is400_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->ssp3_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->ssp5_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->ssp7_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->ccd100_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->inst_extra[10]);
// installed subsystems
  fgets(line,299,fp);
  sscanf(line,"%u", &s->weather_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->ups_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->rain_detector_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->watchdog_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->wwv_type);
// encoders
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_encoder_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_encoder_tb1);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_encoder_motor_steps_rev);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_encoder_encoder_steps_rev);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->x_encoder_encoder_steps_deg);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_encoder_slew_hold_vel);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_encoder_slew_hold_gain);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_encoder_slew_hold_deadband);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->x_encoder_fine_hold_vel);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_encoder_fine_hold_gain);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->x_encoder_fine_hold_deadband);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_encoder_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_encoder_tb1);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_encoder_motor_steps_rev);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_encoder_encoder_steps_rev);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->y_encoder_encoder_steps_deg);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_encoder_slew_hold_vel);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_encoder_slew_hold_gain);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_encoder_slew_hold_deadband);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->y_encoder_fine_hold_vel);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_encoder_fine_hold_gain);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->y_encoder_fine_hold_deadband);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_encoder_installed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_encoder_tb1);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_encoder_motor_steps_rev);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_encoder_encoder_steps_rev);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->z_encoder_encoder_steps_deg);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_encoder_slew_hold_vel);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_encoder_slew_hold_gain);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_encoder_slew_hold_deadband);
  fgets(line,299,fp);
  sscanf(line,"%ld", &s->z_encoder_fine_hold_vel);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_encoder_fine_hold_gain);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->z_encoder_fine_hold_deadband);

  fgets(line,299,fp);
  sscanf(line,"%lf", &s->default_humidity);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->move_fudge_factor);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->encoder_tracking);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->keypad_lr_reversed);
  fgets(line,299,fp);
  sscanf(line,"%u", &s->keypad_ud_reversed);
  fgets(line,299,fp);
  sscanf(line,"%lf", &s->keypad_fudge_factor);

  fgets(line,299,fp);
  sscanf(line,"%lf %lf %lf %lf %lf", 
         &s->sx[0],&s->sx[1],&s->sx[2],&s->sx[3],&s->sx[4]);
  fgets(line,299,fp);
  sscanf(line,"%lf %lf %lf %lf %lf", 
         &s->sy[0],&s->sy[1],&s->sy[2],&s->sy[3],&s->sy[4]);
  fgets(line,299,fp);
  sscanf(line,"%lf %lf %lf %lf %lf", 
         &s->xc[0],&s->xc[1],&s->xc[2],&s->xc[3],&s->xc[4]);
  fgets(line,299,fp);
  sscanf(line,"%lf %lf %lf %lf %lf", 
         &s->yc[0],&s->yc[1],&s->yc[2],&s->yc[3],&s->yc[4]);
  fgets(line,299,fp);
  sscanf(line,"%lf %lf %lf %lf %lf", 
         &s->rot[0],&s->rot[1],&s->rot[2],&s->rot[3],&s->rot[4]);
  fgets(line,299,fp);
  sscanf(line,"%lf %lf %lf %lf %lf", 
         &s->rot0[0],&s->rot0[1],&s->rot0[2],&s->rot0[3],&s->rot0[4]);
  fgets(line,299,fp);
  sscanf(line,"%u %u %u %u %u", 
         &s->fixed[0],&s->fixed[1],&s->fixed[2],&s->fixed[3],&s->fixed[4]);

  fgets(line,299,fp);
  fprintf(stderr,"%s\n",line);
  sscanf(line,"%lf", &s->ccd_hold_time);

  fgets(line,299,fp);
  fprintf(stderr,"%s\n",line);
  sscanf(line,"%lf", &s->ccd_degrees_per_volt);

  fclose(fp);
}

void writesysscf(struct SYSSCF *s) 
{
  FILE *fp;
  fp = fopen(HOME"\\tocc\\sysscf.new","w");
  fprintf(fp,
                "%lf : latitude\n"
                "%lf : longitude\n"
                "%lf : altitude (meters)\n"
                "%u  : mount_type (0 equatorial, 1 alt-az cassegrain, 2 lt-az naysmith)\n"
                "%u  : aperture (cm)\n"

                // x-axis
                "%ld : x_max_velocity\n"
                "%ld : x_acceleration\n"
                "%lf : x_steps_degree\n"
                "%u  : tb1\n"
                "%lf : td1\n"
                "%ld : x_park_steps\n"
                "%u  : x_geartrain_normal\n"
                "%ld : x_pos_soft_limit\n"
                "%ld : x_neg_soft_limit\n"
                "%u  : x_home_dir_reversed\n"

                // y-axis
                "%ld : y_max_velocity\n"
                "%ld : y_acceleration\n"
                "%lf : y_steps_degree\n"
                "%u  : tb2\n"
                "%lf : td2\n"
                "%ld : y_park_steps\n"
                "%u  : y_geartrain_normal\n"
                "%ld : y_pos_soft_limit\n"
                "%ld : y_neg_soft_limit\n"
                "%u  : y_home_dir_reversed\n"

                // z-axis
                "%ld : z_max_velocity\n"
                "%ld : z_acceleration\n"
                "%lf : z_steps_degree\n"
                "%ld : z_park_steps\n"
                "%u  : z_geartrain_normal\n"
                "%u  : z_axis_enabled\n"
                "%lf : td3\n"
                "%ld : z_pos_soft_limit\n"
                "%ld : z_neg_soft_limit\n"
                "%u  : z_soft_limits_enabled\n"
                "%u  : z_home_dir_reversed\n"

                // secondary
                "%u  : tuv_acceleration\n"
                "%u  : tuv_max_velocity\n"
                "%u  : tuv_travel\n"
                "%u  : tuv_tilt\n"
                "%u  : tuv_type  (0 none 1 3-axis 2 1-axis)\n"

                // mirror covers
                "%u  : mirror_covers_installed\n"
                "%u  : mirror_cover_delay\n"
                "%u  : mirror_sets_normal\n"
                "%u  : mirror_set1_normal\n"
                "%u  : mirror_set2_normal\n"

                // enclosure
                "%u  : enclosure_type (0 none; 1 roll-off; 2 dome)\n"
                "%lf : dome_counts_degree\n"
                "%u  : dome_max_init_time\n"
                "%lf : dome_home_azimuth\n"
                "%u  : dome_encoder_normal\n"
                "%u  : dome_geartrain_normal\n"
                "%u  : dome_use_lookup_table\n"
                "%ld : dome_lookup_table[13] ( -180M-0 to +180M-0 in 30M-0 steps)\n"
                "%u  : slip_rings_installed\n"
                "%u  : shutter_delay\n"
                "%u  : lower_shutter_delay\n"
                "%lf : shutter_size\n"
                "%u  : shutter_gear_normal\n"

                // observation window
                "%.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf: window_values[12] ( 0M-0 .. 360M-0 in 30M-0 steps)\n"

                // shutdown parameters
                "%u  : shutdown_wind_speed\n"
                "%u  : shutdown_humidity\n"
                "%d  : shutdown_int_low_temp\n"
                "%d  : shutdown_int_high_temp\n"
                "%d  : shutdown_out_low_temp\n"
                "%d  : shutdown_out_high_temp\n"
                "%u  : shutdown_enabled\n"
                "%u  : shutdown_delay_ws (minutes)\n"
                "%u  : shutdown_delay_rain (minutes)\n"
                "%u  : max_wstation_failures\n"
                "%ld : weather_check_frequency (seconds)\n"
                "%u  : check_35m_closed\n"
                "%ld : 35m_check_frequency (seconds)\n"
                "%s  : 35m_check_closed\n"
                "%s  : 35m_check_opened\n"

                // observatory parameters
                "%u  : weather_station_com_ch\n"
                "%u  : tw1\n"
                "%lf : default_outside_temp\n"

                // installed instruments
                "%u  : is400_installed\n"
                "%u  : ssp3_installed\n"
                "%u  : ssp5_installed\n"
                "%u  : ssp7_installed\n"
                "%u  : ccd100_installed\n"
                "%u  : inst_extra[10]\n"

                // installed subsystems
                "%u  : weather_installed\n"
                "%u  : ups_installed\n"
                "%u  : rain_detector_installed\n"
                "%u  : watchdog_installed\n"
                "%u  : wwv_type (0 none; 1 UTS-10; 2 CTS-10)\n"

                // encoders
                "%u  : x_encoder->installed\n"
                "%u  : x_encoder->tb1 (used to be geartrain normal)\n"
                "%ld : x_encoder->motor_steps_rev\n"
                "%ld : x_encoder->encoder_steps_rev\n"
                "%lf : x_encoder->encoder_steps_deg\n"
                "%ld  : x_encoder->slew_hold_vel\n"
                "%u  : x_encoder->slew_hold_gain\n"
                "%u  : x_encoder->slew_hold_deadband\n"
                "%ld  : x_encoder->fine_hold_vel\n"
                "%u  : x_encoder->fine_hold_gain\n"
                "%u  : x_encoder->fine_hold_deadband\n"

                "%u  : y_encoder->installed\n"
                "%u  : y_encoder->tb1 (used to be geartrain normal)\n"
                "%ld : y_encoder->motor_steps_rev\n"
                "%ld : y_encoder->encoder_steps_rev\n"
                "%lf : y_encoder->encoder_steps_deg\n"
                "%ld  : y_encoder->slew_hold_vel\n"
                "%u  : y_encoder->slew_hold_gain\n"
                "%u  : y_encoder->slew_hold_deadband\n"
                "%ld  : y_encoder->fine_hold_vel\n"
                "%u  : y_encoder->fine_hold_gain\n"
                "%u  : y_encoder->fine_hold_deadband\n"

                "%u  : z_encoder->installed\n"
                "%u  : z_encoder->tb1 (used to be geartrain normal)\n"
                "%ld : z_encoder->motor_steps_rev\n"
                "%ld : z_encoder->encoder_steps_rev\n"
                "%lf : z_encoder->encoder_steps_deg\n"
                "%ld  : z_encoder->slew_hold_vel\n"
                "%u  : z_encoder->slew_hold_gain\n"
                "%u  : z_encoder->slew_hold_deadband\n"
                "%ld  : z_encoder->fine_hold_vel\n"
                "%u  : z_encoder->fine_hold_gain\n"
                "%u  : z_encoder->fine_hold_deadband\n"

                "%lf : default_humidity\n"
                "%u  : move_fudge_factor\n"
                "%u  : encoder_tracking\n"
                "%u  : keypad_lr_reversed\n"
                "%u  : keypad_ud_reversed\n"
                "%lf : keypad_fudge_factor\n"

                "%lf %lf %lf %lf %lf : sx\n"
                "%lf %lf %lf %lf %lf : sy\n"
                "%lf %lf %lf %lf %lf : xc\n"
                "%lf %lf %lf %lf %lf : yc\n"
                "%lf %lf %lf %lf %lf : rot\n"
                "%lf %lf %lf %lf %lf : rot0\n"
                "%u %u %u %u %u : fixed\n"

                "%lf : ccd_hold_time (hours)\n"
                "%lf : ccd_degrees_per_volt\n",

                s->latitude,
                s->longitude,
                s->altitude,
                s->mount_type,
                s->aperture,
                s->x_max_velocity,
                s->x_acceleration,
                s->x_steps_degree,
                s->tb1,
                s->td1,
                s->x_park_steps,
                s->x_geartrain_normal,
                s->x_pos_soft_limit,
                s->x_neg_soft_limit,
                s->x_home_dir_reversed,
                s->y_max_velocity,
                s->y_acceleration,
                s->y_steps_degree,
                s->tb2,
                s->td2,
                s->y_park_steps,
                s->y_geartrain_normal,
                s->y_pos_soft_limit,
                s->y_neg_soft_limit,
                s->y_home_dir_reversed,
                s->z_max_velocity,
                s->z_acceleration,
                s->z_steps_degree,
                s->z_park_steps,
                s->z_geartrain_normal,
                s->z_axis_enabled,
                s->td3,
                s->z_pos_soft_limit,
                s->z_neg_soft_limit,
                s->z_soft_limits_enabled,
                s->z_home_dir_reversed,

                // secondary
                s->tuv_acceleration,
                s->tuv_max_velocity,
                s->tuv_travel,
                s->tuv_tilt,
                s->tuv_type,

                // mirror covers
                s->mirror_covers_installed,
                s->mirror_cover_delay,
                s->mirror_sets_normal,
                s->mirror_set1_normal,
                s->mirror_set2_normal,

                // enclosure
                s->enclosure_type,
                s->dome_counts_degree,
                s->dome_max_init_time,
                s->dome_home_azimuth,
                s->dome_encoder_normal,
                s->dome_geartrain_normal,
                s->dome_use_lookup_table,
                s->dome_lookup_table[13],
                s->slip_rings_installed,
                s->shutter_delay,
                s->lower_shutter_delay,
                s->shutter_size,
                s->shutter_gear_normal,

                // observation window
                s->window_values[0],s->window_values[1],s->window_values[2],
                s->window_values[3],s->window_values[4],s->window_values[5],
                s->window_values[6],s->window_values[7],s->window_values[8],
                s->window_values[9],s->window_values[10],s->window_values[11],

                // shutdown parameters
                s->shutdown_wind_speed,
                s->shutdown_humidity,
                s->shutdown_int_low_temp,
                s->shutdown_int_high_temp,
                s->shutdown_out_low_temp,
                s->shutdown_out_high_temp,
                s->shutdown_enabled,
                s->shutdown_delay_ws,
                s->shutdown_delay_rain,
                s->max_wstation_failures,
                s->weather_check_frequency,
                s->check_35m_closed,
                s->a35m_check_frequency,
                s->a35m_check_closed,
                s->a35m_check_opened,

                // observatory parameters
                s->weather_station_com_ch,
                s->tw1,
                s->default_outside_temp,

                // installed instruments
                s->is400_installed,
                s->ssp3_installed,
                s->ssp5_installed,
                s->ssp7_installed,
                s->ccd100_installed,
                s->inst_extra[10],

                // installed subsystems
                s->weather_installed,
                s->ups_installed,
                s->rain_detector_installed,
                s->watchdog_installed,
                s->wwv_type,

                // encoders
                s->x_encoder_installed,
                s->x_encoder_tb1,
                s->x_encoder_motor_steps_rev,
                s->x_encoder_encoder_steps_rev,
                s->x_encoder_encoder_steps_deg,
                s->x_encoder_slew_hold_vel,
                s->x_encoder_slew_hold_gain,
                s->x_encoder_slew_hold_deadband,
                s->x_encoder_fine_hold_vel,
                s->x_encoder_fine_hold_gain,
                s->x_encoder_fine_hold_deadband,

                s->y_encoder_installed,
                s->y_encoder_tb1,
                s->y_encoder_motor_steps_rev,
                s->y_encoder_encoder_steps_rev,
                s->y_encoder_encoder_steps_deg,
                s->y_encoder_slew_hold_vel,
                s->y_encoder_slew_hold_gain,
                s->y_encoder_slew_hold_deadband,
                s->y_encoder_fine_hold_vel,
                s->y_encoder_fine_hold_gain,
                s->y_encoder_fine_hold_deadband,

                s->z_encoder_installed,
                s->z_encoder_tb1,
                s->z_encoder_motor_steps_rev,
                s->z_encoder_encoder_steps_rev,
                s->z_encoder_encoder_steps_deg,
                s->z_encoder_slew_hold_vel,
                s->z_encoder_slew_hold_gain,
                s->z_encoder_slew_hold_deadband,
                s->z_encoder_fine_hold_vel,
                s->z_encoder_fine_hold_gain,
                s->z_encoder_fine_hold_deadband,

                s->default_humidity,
                s->move_fudge_factor,
                s->encoder_tracking,
                s->keypad_lr_reversed,
                s->keypad_ud_reversed,
                s->keypad_fudge_factor,

                s->sx[0],s->sx[1],s->sx[2],s->sx[3],s->sx[4],
                s->sy[0],s->sy[1],s->sy[2],s->sy[3],s->sy[4],
                s->xc[0],s->xc[1],s->xc[2],s->xc[3],s->xc[4],
                s->yc[0],s->yc[1],s->yc[2],s->yc[3],s->yc[4],
                s->rot[0],s->rot[1],s->rot[2],s->rot[3],s->rot[4],
                s->rot0[0],s->rot0[1],s->rot0[2],s->rot0[3],s->rot0[4],
                s->fixed[0],s->fixed[1],s->fixed[2],s->fixed[3],s->fixed[4],

                s->ccd_hold_time, s->ccd_degrees_per_volt);

  fclose(fp);
}

void initgcsscf(struct GCSSCF *t)
{
  t->x_step_pos = -1000000;
  t->y_step_pos = -2547500;
  t->z_step_pos = 0;
  t->x_max_velocity = 80000 ;
  t->x_acceleration = 100000 ;
  t->x_steps_degree = 0.;
  t->x_travel = 1738000 ;
  t->y_max_velocity = 50000 ;
  t->y_acceleration = 10000 ;
  t->y_steps_degree = 0.;
  t->y_travel = 6595000 ;
  t->z_max_velocity = 25000 ;
  t->z_acceleration = 40000 ;
  t->z_steps_rot = -170000 ;
  t->ysteps_per_telsteps =  -500000 / 100;
  t->xsteps_per_ysteps = -40000 / 1000000;
  t->x_home_pos = -1000000;
  t->y_home_pos = -2547500;
  t->z_home_pos = 4;
  t->microns_per_xsteps = 0.05;
  t->arcsec_per_micron = 0.03375;
  t->x_home_pos_arcsec = 0;
  t->timeout = 150;
}

void readgcsscf(struct GCSSCF *t) 
{
  char line[300];
  FILE *fp;

  fp = fopen(HOME"\\tocc\\gcsscf.new","r");
  fprintf(stderr,"fp: %d\n",fp);

  fgets(line,299,fp);
  sscanf(line,"%ld",&t->x_step_pos);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->y_step_pos);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->z_step_pos);
  fgets(line,299,fp);
  sscanf(line,"%d",&t->current_filter_pos);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->x_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->x_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->x_steps_degree);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->x_travel);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->y_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->y_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->y_steps_degree);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->y_travel);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->z_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->z_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->z_steps_rot);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->t_max_velocity);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->t_acceleration);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->t_pos);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->t_pos_na1);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->t_pos_na2);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->ysteps_per_telsteps);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->xsteps_per_ysteps);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->x_home_pos);
  fgets(line,299,fp);
  sscanf(line,"%ld",&t->y_home_pos);
  fgets(line,299,fp);
  sscanf(line,"%d",&t->z_home_pos);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->microns_per_xsteps);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->arcsec_per_micron);
  fgets(line,299,fp);
  sscanf(line,"%lf",&t->x_home_pos_arcsec);
  fgets(line,299,fp);
  sscanf(line,"%d",&t->timeout);
  fclose(fp);
}
void writegcsscf(struct GCSSCF *t) 
{
  FILE *fp;
  fp = fopen(HOME"\\tocc\\gcsscf.new","w");
  fprintf(fp,
             "%ld : current x-axis position\n"
             "%ld : current y-axis position\n"
             "%ld : current z-axis position\n"
             "%d : current filter position\n"
             "%ld : x_max_velocity\n"
             "%ld : x_acceleration\n"
             "%lf : x_steps_degree\n"
             "%ld : x_travel\n"
             "%ld : y_max_velocity\n"
             "%ld : y_acceleration\n"
             "%lf : y_steps_degree\n"
             "%ld : y_travel\n"
             "%ld : z_max_velocity\n"
             "%ld : z_acceleration\n"
             "%ld : z_steps_rot\n"
             "%ld : t_max_velocity\n"
             "%ld : t_acceleration\n"
             "%ld : t_pos\n"
             "%ld : t_pos_na1\n"
             "%ld : t_pos_na2\n"
             "%lf : ysteps_per_telsteps\n"
             "%lf : xsteps_per_ysteps\n"
             "%ld : home x-axis position\n"
             "%ld : home y-axis position\n"
             "%d : home z-axis position\n"
             "%lf : microns_per_xsteps\n"
             "%lf : arcsec_per_micron\n"
             "%lf : x_home_pos_arcsec\n"
             "%d  : timeout",
                t->x_step_pos,
                t->y_step_pos,
                t->z_step_pos,
                t->current_filter_pos,
                t->x_max_velocity,
                t->x_acceleration,
                t->x_steps_degree,
                t->x_travel,
                t->y_max_velocity,
                t->y_acceleration,
                t->y_steps_degree,
                t->y_travel,
                t->z_max_velocity,
                t->z_acceleration,
                t->z_steps_rot,
                t->t_max_velocity,
                t->t_acceleration,
                t->t_pos,
                t->t_pos_na1,
                t->t_pos_na2,
                t->ysteps_per_telsteps,
                t->xsteps_per_ysteps,
                t->x_home_pos,
                t->y_home_pos,
                t->z_home_pos,
                t->microns_per_xsteps,
                t->arcsec_per_micron,
                t->x_home_pos_arcsec,
                t->timeout
                );
  fclose(fp);
}
void printgcsscf(struct GCSSCF *t) 
{
  sprintf(outbuf,
             "%ld : current x-axis position\n"
             "%ld : current y-axis position\n"
             "%ld : current z-axis position\n"
             "%d : current filter position\n"
             "%ld : x_max_velocity\n"
             "%ld : x_acceleration\n"
             "%lf : x_steps_degree\n"
             "%ld : x_travel\n"
             "%ld : y_max_velocity\n"
             "%ld : y_acceleration\n"
             "%lf : y_steps_degree\n"
             "%ld : y_travel\n"
             "%ld : z_max_velocity\n"
             "%ld : z_acceleration\n"
             "%ld : z_steps_rot\n"
             "%ld : t_max_velocity\n"
             "%ld : t_acceleration\n"
             "%ld : t_pos\n"
             "%ld : t_pos_na1\n"
             "%ld : t_pos_na2\n"
             "%lf : ysteps_per_telsteps\n"
             "%lf : xsteps_per_ysteps\n"
             "%ld : home x-axis position\n"
             "%ld : home y-axis position\n"
             "%d : home z-axis position\n"
             "%lf : microns_per_xsteps\n"
             "%lf : arcsec_per_micron\n"
             "%lf : x_home_pos_arcsec\n"
             "%d  : timeout\n",
                t->x_step_pos,
                t->y_step_pos,
                t->z_step_pos,
                t->current_filter_pos,
                t->x_max_velocity,
                t->x_acceleration,
                t->x_steps_degree,
                t->x_travel,
                t->y_max_velocity,
                t->y_acceleration,
                t->y_steps_degree,
                t->y_travel,
                t->z_max_velocity,
                t->z_acceleration,
                t->z_steps_rot,
                t->t_max_velocity,
                t->t_acceleration,
                t->t_pos,
                t->t_pos_na1,
                t->t_pos_na2,
                t->ysteps_per_telsteps,
                t->xsteps_per_ysteps,
                t->x_home_pos,
                t->y_home_pos,
                t->z_home_pos,
                t->microns_per_xsteps,
                t->arcsec_per_micron,
                t->x_home_pos_arcsec,
                t->timeout
                );
  writeline(outbuf,1);
}
