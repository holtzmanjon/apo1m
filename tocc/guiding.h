#ifndef _GUIDING_H
	#define _GUIDING_H

extern void update_guide_correction(double dra, double ddec);
extern void update_guide_correction_inst(int inst, double dx, double dy);
extern void update_guide_correction_altaz(double daz, double dalt);
extern void zero_guide_correction();
extern void apply_guide_correction_altaz(double &az, double &alt);
extern void apply_guide_correction_radec(double *ra, double *dec);

#endif
/********************************* EOF **************************************/

