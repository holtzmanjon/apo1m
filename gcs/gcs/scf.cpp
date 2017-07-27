#include <stdio.h>
#include "scf.h"
#include "io.h"

#ifdef no_hardware
#define GCS "c:"
#else
#define GCS "e:"
#endif

struct GCSSCF *gcsGlobal, gcsGlobalNew;

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

  fp = fopen(GCS"\\spec\\gcsscf.new","r");
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
  fp = fopen(GCS"\\spec\\gcsscf.new","w");
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
