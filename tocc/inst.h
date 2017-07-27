#ifndef _INST_H
	#define _INST_H

extern void apply_instrument_correction(double ra, double dec, 
                                 double &inst_ra, double &inst_dec);
extern void update_instrument_correction(int inst, double sx, double sy,
                                 double xc, double yc, double rot, double rot0, BOOL fixed);
extern void apply_instrument_offset(int inst, double dx, double dy);

#endif
/********************************* EOF **************************************/

