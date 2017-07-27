#ifndef _SCF_H
	#define _SCF_H

void initgcsscf(struct GCSSCF *);
void readgcsscf(struct GCSSCF *);
void writegcsscf(struct GCSSCF *);
void printgcsscf(struct GCSSCF *);

#include "mytype.h"

struct GCSSCF
{
	long    x_step_pos;	// current t-axis position
	long	y_step_pos;	// current u-axis position
	long	z_step_pos;	// current v-axis position
        int     current_filter_pos;
	long	x_max_velocity;
	long	x_acceleration;
	double	x_steps_degree;
	long	x_travel;
	long	y_max_velocity;
	long	y_acceleration;
	double	y_steps_degree;
	long	y_travel;
	long	z_max_velocity;
	long	z_acceleration;
	long	z_steps_rot;
        double  ysteps_per_telsteps;
        double  xsteps_per_ysteps;
	long    x_home_pos;	// home x-axis position (steps)
	long	y_home_pos;	// home y-axis position (steps)
	int	z_home_pos;	// home z-axis position (filter position)
        double  microns_per_xsteps;
        double  arcsec_per_micron;
        double  x_home_pos_arcsec;
        int     timeout;
} ;

extern struct GCSSCF *gcsGlobal, gcsGlobalNew;
#endif
