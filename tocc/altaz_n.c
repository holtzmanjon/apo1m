#include <math.h>
#include "slamac.h"

#include <stdio.h>
#include <conio.h>

void altaz_n ( double az_o, double el_o, double an, double aw,
		double nrx, double nry, double ca,
		double npae, double tf, double ia, double ie,
		double *az_e, double *el_e )
/*
**  - - - - - - - -
**   a l t a z _ n
**  - - - - - - - -
**
**  For a Nasmyth altazimuth telescope, transform observed azimuth
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
**     nrx      double     Nasmyth misalignment horizontal
**     nry      double     Nasmyth misalignment vertical
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
**  2  All azimuths are N=0, E=90.
**
**  3  Sign conventions are consistent with the TPOINT pointing-analysis
**     package:  the arguments an, aw, nrx, nry, ca, npae, tf, ia and ie are
**     the same as the TPOINT coefficients AN AW NRX NRY CA NPAE TF IA IE.
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
**  6  The Nasmyth misalignment corrections are not strictly rigorous
**     but are of more than adequate accuracy for the envisaged
**     applications.  Increased accuracy could be achieved through
**     iterative techniques.
**
**  Last revision:   19 July 1995
**
**  Copyright P.T.Wallace.  All rights reserved.
*/

{
   double w, xo, yo, zo, x1, y1, z1, rxy2, rxy, dca, dce, eta, xi, xi2,
          eta2p1, sdf, r2, r, c, x2, y2, z2, f, x3, y3, z3, az, el;

/* Observed az/el spherical to Cartesian */
   w = cos ( el_o );
   xo = - cos ( az_o ) * w;
   yo = sin ( az_o ) * w;
   zo = sin ( el_o );

/* Rotate into frame of (slightly tilted) mount */
   x1 = xo + an * zo;
   y1 = yo + aw * zo;
   z1 = - an * xo - aw * yo + zo;

/* Nasmyth corrections (approximate) */
   rxy2 = x1 * x1 + y1 * y1;
   rxy = sqrt ( rxy2 );
   dca = nrx * rxy + nry * z1;
   dce = - nrx * z1 + nry * rxy;

/* Rotator axis position (on sky xi +ve left, eta +ve up) */
   eta = dce;
   xi = ca + dca + npae * ( z1 - eta * rxy + tf * rxy2 );

/* Predict mount vector (provisionally assuming no tube flexure) */
   xi2 = xi * xi;
   eta2p1 = eta * eta + 1.0;
   sdf = z1 * sqrt ( xi2 + eta2p1 );
   r2 = rxy2 * eta2p1 - z1 * z1 * xi2;
   r = ( r2 >= 0.0 ) ? sqrt ( r2 ) : 0.0;
   w = rxy2 * ( r2 + xi2 );
   c = ( w > 0.0 ) ? ( sdf * eta + r ) / ( eta2p1 * sqrt ( w ) ) : 0.0;
   x2 = c * ( x1 * r + y1 * xi );
   y2 = c * ( y1 * r - x1 * xi );
   z2 = ( sdf - eta * r ) / eta2p1;

/* Allow for tube flexure */
   f = 1.0 - tf * z2;
   x3 = x2 * f;
   y3 = y2 * f;
   z3 = ( z2 + tf ) * f;

/* Cartesian to spherical */
   r = sqrt ( x3 * x3 + y3 * y3 );
   az = ( r != 0.0 ) ? atan2 ( y3, - x3 ) : 0.0;
   el = atan2 ( z3, r );

/* Allow for index errors. */
   w = fmod ( az + ia, D2PI );
   *az_e = ( w >= 0.0 ) ? w : w + D2PI;
   *el_e = el - ie;

}
