#ifndef TPHDEF
#define TPHDEF

#ifdef __cplusplus
	extern "C" {
#endif

/*
**  - - - - -
**   t p . h
**  - - - - -
**
**  Prototype function declarations for telescope pointing routines.
**
**  Last revision:   6 June 1995
**
**  Copyright P.T.Wallace and Autoscope Corp.  All rights reserved.
*/

/*
**  - - - - -
**   t p . h
**  - - - - -
**
**  Prototype function declarations for telescope pointing routines.
**
**  Last revision:   6 June 1995
**
**  Copyright P.T.Wallace and Autoscope Corp.  All rights reserved.
*/

void equat_f ( double, double, double, double, double, double, double,
               double, double, double, double, double*, double* );

void altaz_c ( double, double, double, double, double, double, double,
               double, double, double*, double* );

void altaz_n ( double, double, double, double, double, double, double,
	       double, double, double, double, double*, double* );


#ifdef __cplusplus
	}
#endif
#endif
