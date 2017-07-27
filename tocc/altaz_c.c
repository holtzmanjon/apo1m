#include <math.h>

#include "slamac.h"

void altaz_c ( double az_o, double el_o, double an, double aw, double ca,
               double npae, double tf, double ia, double ie,
               double *az_e, double *el_e )
/*
**  - - - - - - - -
**   a l t a z _ c
**  - - - - - - - -
**
**  For a Cassegrain altazimuth telescope, transform observed azimuth
**  and elevation into encoder azimuth and elevation, correcting
**  for various telescope imperfections which affect pointing.
**
**  "Observed" az/el are obtained from a star's catalogue mean
**  RA/Dec by allowing for the following effects:
**
**      proper motion
**      parallax (small)
**      precession/nutation
**      annual aberration
**      light deflection (small)
**      Earth rotation
**      diurnal aberration (small)
**      HA/Dec to az/el
**      refraction
**
**  Given:
**     az_o     double     observed azimuth
**     el_o     double     observed elevation
**     an       double     azimuth-axis misalignment north
**     aw       double     azimuth-axis misalignment west
**     ca       double     horizontal collimation error
**     npae     double     az/el nonperpendicularity
**     tf       double     tube flexure
**     ia       double     azimuth index error
**     ie       double     elevation index error
**
**  Returned:
**     *az_e    *double    encoder azimuth
**     *el_e    *double    encoder elevation
**
**  Notes:
**
**  1  All arguments are in radians.
**
**  2  Azimuth arguments are N=0, E=90.
**
**  3  Sign conventions are consistent with the TPOINT pointing-analysis
**     package:  the arguments an, aw, ca, npae, tf, ia and ie are the
**     same as the TPOINT coefficients AN AW CA NPAE TF IA IE.
**
**  4  The predictions are correct for the case where:
**
**       either  (i)  the pointing-origin (eyepiece crosswires,
**                    center of CCD, etc) is located on the
**                    instrument-mount rotation axis,
**
**       or      (ii) the instrument-mount is stationary (so
**                    that the field rotates).
**
**  5  The azimuth axis misalignment coefficients an and aw are
**     assumed to be small (less than say 1000 arcsec).  The code
**     can be made rigorous at the expense of four trig calls and
**     some extra arithmetic.
**
**  6  The calculations are equally valid for prime focus,
**     Newtonian, and Cassegrain/Newtonian designs, but are only
**     a partial model of the coude and Nasmyth cases.
**
**  Last revision:   19 July 1995
**
**  Copyright P.T.Wallace.  All rights reserved.
*/

{
   double w, xo, yo, zo, x1, y1, z1, xi, rxy2, xi2, r2, r, c,
          x2, y2, z2, f, x3, y3, z3, az, el;

/* Observed az/el spherical to Cartesian (south, east, up) */
   w = cos ( el_o );
   xo = - cos ( az_o ) * w;
   yo = sin ( az_o ) * w;
   zo = sin ( el_o );

/* Rotate into frame of (slightly tilted) mount */
   x1 = xo + an * zo;
   y1 = yo + aw * zo;
   z1 = - an * xo - aw * yo + zo;

/* Rotator axis left/right position (on sky xi is +ve left) */
   xi = ca + npae * z1;

/* Predict mount vector, provisionally assuming no tube flexure */
   rxy2 = x1 * x1 + y1 * y1;
   xi2 = xi * xi;
   r2 = rxy2 - z1 * z1 * xi2;
   r = ( r2 >= 0.0 ) ? sqrt ( r2 ) : 0.0;
   w = rxy2 * ( r2 + xi2 );
   c = ( w > 0.0 ) ? r / sqrt ( w ) : 0.0;
   x2 = c * ( x1 * r + y1 * xi );
   y2 = c * ( y1 * r - x1 * xi );
   z2 = z1 * sqrt ( xi2 + 1.0 );

/* Allow for tube flexure */
   f = 1.0 - tf * z2;
   x3 = x2 * f;
   y3 = y2 * f;
   z3 = ( z2 + tf ) * f;

/* Cartesian to spherical */
	 r = sqrt ( x3 * x3 + y3 * y3 );
	 az = ( r != 0.0 ) ? atan2 ( y3, - x3 ) : 0.0;
	 el = atan2 ( z3, r );

/* Allow for index errors */
	 w = fmod ( az + ia, D2PI );
	 *az_e = ( w >= 0.0 ) ? w : w + D2PI;
	 *el_e = el - ie;
}

