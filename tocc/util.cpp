#include "globals.h"
#include "io.h"
#include "status.h"

void print_toccscf();
void print_sysscf();
void print_sysscf2();
void askval();

void askval()
{
  sprintf(outbuf,"Enter new value: ");
  writeline(outbuf,0);
}

int edit_toccscf()
{

   int i;
   unsigned char line[80];

   print_toccscf();

  while(1) {
   sprintf(outbuf,"Enter number to change, 0 to quit: ");
   writeline(outbuf,0);
   getline(line,sizeof(line));
   sscanf(line,"%d",&i);

   if (i==0) {
     print_toccscf();
     sprintf(outbuf,"Enter 1 to save SCF file now, else won't save to disk: ");
     writeline(outbuf,0);
     getline(line,sizeof(line));
     sscanf(line,"%d",&i);
     if (i==1) {
       writetoccscf(autoGlobal);
       return (TCSERR_OK);
     }
     else
       return(TCSERR_OK);
   } 
   else if (i==1) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->home_dec);
   }
   else if (i==2) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->home_ha);
   }
   else if (i==3) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->td1);
   }
   else if (i==4) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&autoGlobal->t_step_pos);
   }
   else if (i==5) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&autoGlobal->u_step_pos);
   }
   else if (i==6) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&autoGlobal->v_step_pos);
   }
   else if (i==7) {
     askval();
     getline(line,sizeof(line));
     autoGlobal->shutter_opened = !autoGlobal->shutter_opened ;
   }
   else if (i==8) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->dome_azimuth);
   }
   else if (i==9) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&autoGlobal->z_axis_pos);
   }
   else if (i==10) {
     askval();
     getline(line,sizeof(line));
     autoGlobal->dome_slaved = !autoGlobal->dome_slaved;
   }
   else if (i==11) {
     askval();
     getline(line,sizeof(line));
     autoGlobal->mc_enabled = !autoGlobal->mc_enabled;
   }
   else if (i==12) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->home_alt);
   }
   else if (i==13) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->home_az);
   }
   else if (i==14) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->home_rot);
   }
   else if (i==15) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->saved_alt);
   }
   else if (i==16) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->saved_az);
   }
   else if (i==17) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&autoGlobal->saved_rot);
   }
  }

}

void print_toccscf()
{
  sprintf(outbuf,"1. home_dec:  %f   "
         "2. home_ha: %f   "
         "3. td1: %f\r\n"
         "4. t_step_pos: %ld   "
         "5. u_step_pos: %ld   "
         "6. v_step_pos: %ld\r\n"
         "7. shutter_opened: %d   "
         "8. dome_azimuth: %f   "
         "9. z_axis_pos: %ld\r\n"
         "10. dome_slaved: %d   "
         "11. mc_enabled: %d\r\n"
         "12. home_alt: %f   "
         "13. home_az: %f   "
         "14. home_rot: %f\r\n"
         "15. saved_alt: %f   "
         "16. saved_az: %f   "
         "17. saved_rot: %f\r\n",
         autoGlobal->home_dec,       // home position declination (degrees)
         autoGlobal->home_ha,        // home position hour-angle (hours)
         autoGlobal->td1,            // used to be home epoch
         autoGlobal->t_step_pos,     // current t-axis position
         autoGlobal->u_step_pos,     // current u-axis position
         autoGlobal->v_step_pos,     // current v-axis position
         autoGlobal->shutter_opened, // current position of dome shutter
         autoGlobal->dome_azimuth,   // current azimuth of the dome
         autoGlobal->z_axis_pos,     // last position of the instrument rotator
         autoGlobal->dome_slaved,    // current status of dome slaving to telescope
         autoGlobal->mc_enabled,     // mount corrections enabled
         autoGlobal->home_alt,       // home position declination (degrees)
         autoGlobal->home_az,        // home position hour-angle (hours)
         autoGlobal->home_rot,        // home position hour-angle (hours)
         autoGlobal->saved_alt,      // home position declination (degrees)
         autoGlobal->saved_az,       // home position hour-angle (hours)
         autoGlobal->saved_rot);       // home position hour-angle (hours)

  writeline(outbuf,0);
}

int edit_sysscf()
{

   int i;
   unsigned char line[80];

   print_sysscf();

  while(1) {
   sprintf(outbuf,"Enter number to change, 0 to quit: ");
   writeline(outbuf,0);
   getline(line,sizeof(line));
   sscanf(line,"%d",&i);

   if (i==0) {
     print_sysscf();
     sprintf(outbuf,"Enter 1 to save SCF file now, else won't save to disk: ");
     writeline(outbuf,0);
     getline(line,sizeof(line));
     sscanf(line,"%d",&i);
     if (i==1) {
       writesysscf(sysGlobal);
       return(TCSERR_OK);
     }
     else
       return(TCSERR_OK);
   } 
   else if (i==1) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_max_velocity);
   }
   else if (i==2) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_acceleration);
   }
   else if (i==3) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->x_steps_degree);
   }
   else if (i==4) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->tb1 = !sysGlobal->tb1;
   }
   else if (i==5) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->td1);
   }
   else if (i==6) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_park_steps);
   }
   else if (i==7) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->x_geartrain_normal = !sysGlobal->x_geartrain_normal;
   }
   else if (i==8) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_pos_soft_limit);
   }
   else if (i==9) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_neg_soft_limit);
   }
   else if (i==10) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->x_home_dir_reversed = !sysGlobal->x_home_dir_reversed;
   }
   else if (i==11) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_max_velocity);
   }
   else if (i==12) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_acceleration);
   }
   else if (i==13) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->y_steps_degree);
   }
   else if (i==14) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->tb2 = !sysGlobal->tb2;
   }
   else if (i==15) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->td2);
   }
   else if (i==16) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_park_steps);
   }
   else if (i==17) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->y_geartrain_normal = !sysGlobal->y_geartrain_normal;
   }
   else if (i==18) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_pos_soft_limit);
   }
   else if (i==19) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_neg_soft_limit);
   }
   else if (i==20) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->y_home_dir_reversed = !sysGlobal->y_home_dir_reversed;
   }
   else if (i==21) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_max_velocity);
   }
   else if (i==22) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_acceleration);
   }
   else if (i==23) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->z_steps_degree);
   }
   else if (i==24) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_park_steps);
   }
   else if (i==25) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->z_geartrain_normal = !sysGlobal->z_geartrain_normal;
   }
   else if (i==26) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->z_axis_enabled = !sysGlobal->z_axis_enabled;
   }
   else if (i==27) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->td3);
   }
   else if (i==28) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_pos_soft_limit);
   }
   else if (i==29) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_neg_soft_limit);
   }
   else if (i==30) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->z_soft_limits_enabled = !sysGlobal->z_soft_limits_enabled;
   }
   else if (i==31) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->z_home_dir_reversed = !sysGlobal->z_home_dir_reversed;
   }
  }

}

void print_sysscf2()
{

   sprintf(outbuf,
           "X encoder:  "
           "1. installed: %d "
           "2. tb1: %d \r\n"
           "3. mot_steps_rev: %ld "
           "4. enc_steps_rev: %ld "
           "5. enc_steps_deg: %f \r\n"
           "6. slew_hold_vel: %ld "
           "7. slew_hold_gain: %d "
           "8. slew_hold_deadband: %d\r\n"
           "9. fine_hold_vel: %ld "
           "10. fine_hold_gain: %d "
           "11. fine_hold_deadband: %d\r\n"
           "Y encoder:  "
           "12. installed: %d "
           "13. tb1: %d \r\n"
           "14. mot_steps_rev: %ld "
           "15. enc_steps_rev: %ld "
           "16. enc_steps_deg: %f \r\n"
           "17. slew_hold_vel: %ld "
           "18. slew_hold_gain: %d "
           "19. slew_hold_deadband: %d\r\n"
           "20. fine_hold_vel: %ld "
           "21. fine_hold_gain: %d "
           "22. fine_hold_deadband: %d\r\n"
           "Z encoder:  "
           "23. installed: %d "
           "24. tb1: %d \r\n"
           "25. mot_steps_rev: %ld "
           "26. enc_steps_rev: %ld "
           "27. enc_steps_deg: %f \r\n"
           "28. slew_hold_vel: %ld "
           "29. slew_hold_gain: %d "
           "30. slew_hold_deadband: %d\r\n"
           "31. fine_hold_vel: %ld "
           "32. fine_hold_gain: %d "
           "33. fine_hold_deadband: %d\r\n",

		sysGlobal->x_encoder_installed,
		sysGlobal->x_encoder_tb1,
		sysGlobal->x_encoder_motor_steps_rev,
		sysGlobal->x_encoder_encoder_steps_rev,
		sysGlobal->x_encoder_encoder_steps_deg,
		sysGlobal->x_encoder_slew_hold_vel,
		sysGlobal->x_encoder_slew_hold_gain,
		sysGlobal->x_encoder_slew_hold_deadband,
		sysGlobal->x_encoder_fine_hold_vel,
		sysGlobal->x_encoder_fine_hold_gain,
		sysGlobal->x_encoder_fine_hold_deadband,

		sysGlobal->y_encoder_installed,
		sysGlobal->y_encoder_tb1,
		sysGlobal->y_encoder_motor_steps_rev,
		sysGlobal->y_encoder_encoder_steps_rev,
		sysGlobal->y_encoder_encoder_steps_deg,
		sysGlobal->y_encoder_slew_hold_vel,
		sysGlobal->y_encoder_slew_hold_gain,
		sysGlobal->y_encoder_slew_hold_deadband,
		sysGlobal->y_encoder_fine_hold_vel,
		sysGlobal->y_encoder_fine_hold_gain,
		sysGlobal->y_encoder_fine_hold_deadband,

		sysGlobal->z_encoder_installed,
		sysGlobal->z_encoder_tb1,
		sysGlobal->z_encoder_motor_steps_rev,
		sysGlobal->z_encoder_encoder_steps_rev,
		sysGlobal->z_encoder_encoder_steps_deg,
		sysGlobal->z_encoder_slew_hold_vel,
		sysGlobal->z_encoder_slew_hold_gain,
		sysGlobal->z_encoder_slew_hold_deadband,
		sysGlobal->z_encoder_fine_hold_vel,
		sysGlobal->z_encoder_fine_hold_gain,
		sysGlobal->z_encoder_fine_hold_deadband);

      writeline(outbuf,0);
}

int edit_sysscf2()
{

   int i;
   unsigned char line[80];

   print_sysscf2();

  while(1) {
   sprintf(outbuf,"Enter number to change, 0 to quit: ");
   writeline(outbuf,0);
   getline(line,sizeof(line));
   sscanf(line,"%d",&i);

   if (i==0) {
     print_sysscf2();
     sprintf(outbuf,"Enter 1 to save SCF file now, else won't save to disk: ");
     writeline(outbuf,0);
     getline(line,sizeof(line));
     sscanf(line,"%d",&i);
     if (i==1) {
       writesysscf(sysGlobal);
       return(TCSERR_OK);
     }
     else
       return(TCSERR_OK);
   } 
   else if (i==1) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->x_encoder_installed = !sysGlobal->x_encoder_installed;
   }
   else if (i==2) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->x_encoder_tb1 = !sysGlobal->x_encoder_tb1;
   }
   else if (i==3) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_motor_steps_rev);
   }
   else if (i==4) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_encoder_steps_rev);
   }
   else if (i==5) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->x_encoder_encoder_steps_deg);
   }
   else if (i==6) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_slew_hold_vel);
   }
   else if (i==7) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_slew_hold_gain);
   }
   else if (i==8) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_slew_hold_deadband);
   }
   else if (i==9) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_fine_hold_vel);
   }
   else if (i==10) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_fine_hold_gain);
   }
   else if (i==11) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->x_encoder_fine_hold_deadband);
   }
   else if (i==12) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->y_encoder_installed = !sysGlobal->y_encoder_installed;
   }
   else if (i==13) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->y_encoder_tb1 = !sysGlobal->y_encoder_tb1;
   }
   else if (i==14) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_motor_steps_rev);
   }
   else if (i==15) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_encoder_steps_rev);
   }
   else if (i==16) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->y_encoder_encoder_steps_deg);
   }
   else if (i==17) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_slew_hold_vel);
   }
   else if (i==18) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_slew_hold_gain);
   }
   else if (i==19) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_slew_hold_deadband);
   }
   else if (i==20) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_fine_hold_vel);
   }
   else if (i==21) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_fine_hold_gain);
   }
   else if (i==22) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->y_encoder_fine_hold_deadband);
   }
   else if (i==23) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->z_encoder_installed = !sysGlobal->z_encoder_installed;
   }
   else if (i==24) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->z_encoder_tb1 = !sysGlobal->z_encoder_tb1;
   }
   else if (i==25) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_motor_steps_rev);
   }
   else if (i==26) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_encoder_steps_rev);
   }
   else if (i==27) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->z_encoder_encoder_steps_deg);
   }
   else if (i==28) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_slew_hold_vel);
   }
   else if (i==29) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_slew_hold_gain);
   }
   else if (i==30) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_slew_hold_deadband);
   }
   else if (i==31) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_fine_hold_vel);
   }
   else if (i==32) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_fine_hold_gain);
   }
   else if (i==33) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->z_encoder_fine_hold_deadband);
   }
  }

}

void print_sysscf()
{

   sprintf(outbuf,
           "1. x_max_vel: %ld   "
           "2. x_acc: %ld   "
           "3. x_steps_degree: %f\r\n"
           "4. tb1: %d "
           "5. td1: %f "
           "6. x_park_steps: %ld "
           "7. x_gear_norm: %d\r\n"
           "8. x_pos_soft_lim: %ld "
           "9. x_neg_soft_lim: %ld "
           "10. x_home_dir_rev: %d\r\n"

           "11. y_max_vel: %ld   "
           "12. y_acc: %ld   "
           "13. y_steps_degree: %f\r\n"
           "14. tb2: %d "
           "15. td2: %f "
           "16. y_park_steps: %ld "
           "17. y_geartrain_norm: %d\r\n"
           "18. y_pos_soft_lim: %ld "
           "19. y_neg_soft_lim: %ld "
           "20. y_home_dir_rev: %d\r\n"

           "21. z_max_vel: %ld   "
           "22. z_acc: %ld   "
           "23. z_steps_degree: %f\r\n"
           "24. z_park_steps: %ld "
           "25. z_geartrain_norm: %d "
           "26. z_axis_enab: %d "
           "27. td3: %f\r\n"
           "28. z_pos_soft_lim: %ld "
           "29. z_neg_soft_lim: %ld "
           "30. z_soft_lim_enab: %d "
           "31. z_home_dir_rev: %d\r\n",

		sysGlobal->x_max_velocity,
		sysGlobal->x_acceleration,
		sysGlobal->x_steps_degree,
		sysGlobal->tb1,
		sysGlobal->td1,
		sysGlobal->x_park_steps,
		sysGlobal->x_geartrain_normal,
		sysGlobal->x_pos_soft_limit,
		sysGlobal->x_neg_soft_limit,
		sysGlobal->x_home_dir_reversed,

		sysGlobal->y_max_velocity,
		sysGlobal->y_acceleration,
		sysGlobal->y_steps_degree,
		sysGlobal->tb2,
		sysGlobal->td2,
		sysGlobal->y_park_steps,
		sysGlobal->y_geartrain_normal,
		sysGlobal->y_pos_soft_limit,
		sysGlobal->y_neg_soft_limit,
		sysGlobal->y_home_dir_reversed,


		sysGlobal->z_max_velocity,
		sysGlobal->z_acceleration,
		sysGlobal->z_steps_degree,
		sysGlobal->z_park_steps,
		sysGlobal->z_geartrain_normal,
		sysGlobal->z_axis_enabled,
		sysGlobal->td3,
		sysGlobal->z_pos_soft_limit,
		sysGlobal->z_neg_soft_limit,
		sysGlobal->z_soft_limits_enabled,
		sysGlobal->z_home_dir_reversed);
      writeline(outbuf,0);
}

void print_sysscf3()
{

   sprintf(outbuf,
           "1. encoder_tracking: %d\r\n"
           "2. move_fudge_factor: %d   "
           "3. keypad_fudge_factor: %f\r\n"
           "4. keypad_lr_reversed: %d  "
           "5. keypad_ud_reversed: %d \r\n"
           "6. dome_use_lookup_table: %d "
           "7. shutter_size: %f "
           "8. dome_counts_degree: %f\r\n"
           "9. weather_check_frequency: %ld "
           "10. mirror_cover_delay: %d "
           "11. mirror_covers_installed: %d\r\n"
           "12. tuv_travel: %d  "
           "13. tuv_tilt: %d \r\n",

		sysGlobal->encoder_tracking,
		sysGlobal->move_fudge_factor,
		sysGlobal->keypad_fudge_factor,
		sysGlobal->keypad_lr_reversed,
		sysGlobal->keypad_ud_reversed,
                sysGlobal->dome_use_lookup_table,
                sysGlobal->shutter_size,
                sysGlobal->dome_counts_degree,
                sysGlobal->weather_check_frequency,
                sysGlobal->mirror_cover_delay,
                sysGlobal->mirror_covers_installed,
                sysGlobal->tuv_travel,
                sysGlobal->tuv_tilt) ;
      writeline(outbuf,0);
}

int edit_sysscf3()
{

   int i;
   unsigned char line[80];

   print_sysscf3();

  while(1) {
   sprintf(outbuf,"Enter number to change, 0 to quit: ");
   writeline(outbuf,0);
   getline(line,sizeof(line));
   sscanf(line,"%d",&i);

   if (i==0) {
     print_sysscf3();
     sprintf(outbuf,"Enter 1 to save SCF file now, else won't save to disk: ");
     writeline(outbuf,0);
     getline(line,sizeof(line));
     sscanf(line,"%d",&i);
     if (i==1) {
       writesysscf(sysGlobal);
       return(TCSERR_OK);
     }
     else
       return(TCSERR_OK);
   } 
   else if (i==1) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%d",&sysGlobal->encoder_tracking);
   }
   else if (i==2) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%d",&sysGlobal->move_fudge_factor);
   }
   else if (i==3) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->keypad_fudge_factor);
   }
   else if (i==4) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->keypad_lr_reversed = !sysGlobal->keypad_lr_reversed;
   }
   else if (i==5) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->keypad_ud_reversed = !sysGlobal->keypad_ud_reversed;
   }
   else if (i==6) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->dome_use_lookup_table = !sysGlobal->dome_use_lookup_table;
   }
   else if (i==7) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->shutter_size);
   }
   else if (i==8) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%lf",&sysGlobal->dome_counts_degree);
   }
   else if (i==9) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%ld",&sysGlobal->weather_check_frequency);
   }
   else if (i==10) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%d",&sysGlobal->mirror_cover_delay);
   }
   else if (i==11) {
     askval();
     getline(line,sizeof(line));
     sysGlobal->mirror_covers_installed = !sysGlobal->mirror_covers_installed;
   }
   else if (i==12) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%d",&sysGlobal->tuv_travel);
   }
   else if (i==13) {
     askval();
     getline(line,sizeof(line));
     sscanf(line,"%d",&sysGlobal->tuv_tilt);
   }
  }

}

int setup_scf()
{
  int i, status;
  unsigned char line[80];

  sprintf(outbuf,"Choose which menu of setup parameters: \r\n"
                 "  1. Telescope setup/saved parameters\r\n"
                 "  2. Motor setup/configuration \r\n"
                 "  3. Encoder setup/configuration \r\n"
                 "  4. Miscellaneous setup/configuration \r\n");
  writeline(outbuf,0);
  sprintf(outbuf,"Enter number to change, 0 to quit: ");
  writeline(outbuf,0);
  getline(line,sizeof(line));
  sscanf(line,"%d",&i);

  if (i==1)
    status = edit_toccscf();
  else if (i==2)    
    status = edit_sysscf();
  else if (i==3)    
    status = edit_sysscf2();
  else if (i==4)    
    status = edit_sysscf3();

  return(status);

}
