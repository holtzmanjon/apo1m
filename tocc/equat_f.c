#include <math.h>

#include "slamac.h"

void equat_f ( double az_o, double el_o, double phi,
               double me, double ma, double ch, double np,
               double fo, double tf, double ih, double id,
               double *ha_e, double *dec_e )
/*
**  - - - - - - - -
**   e q u a t _ f
**  - - - - - - - -
**
**  For an fork-mounted equatorial telescope, transform observed
**  azimuth and elevation into encoder hour angle and declination,
**  correcting for various telescope imperfections which affect
**  pointing.
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
**     phi      double     site latitude (geodetic)
**     me       double     polar-axis misalignment up/down
**     ma       double     polar-axis misalignment left/right
**     ch       double     east/west collimation error
**     np       double     HA/Dec nonperpendicularity
**     fo       double     fork flexure
**     tf       double     tube flexure
**     ih       double     HA index error
**     id       double     Dec index error
**
**  Returned:
**     *ha_e    *double    encoder HA
**     *dec_e   *double    encoder Dec
**
**  Notes:
**
**  1  All arguments are in radians.
**
**  2  Azimuth arguments are N=0, E=90.  HA is west-positive.
**
**  3  Sign conventions are consistent with the TPOINT pointing-analysis
**     package:  the arguments me, ma, ch, np, tf, ih and id are
**     the same as the TPOINT coefficients ME MA CH NP TF IH ID.
**
**  4  The predictions are correct for the case where the pointing-origin
**     (eyepiece crosswires, center of CCD, etc) is located on the
**     instrument-mount rotation axis.
**
**  Last revision:   19 July 1995
**
**  Copyright P.T.Wallace.  All rights reserved.
*/

{
   double c, xo, yo, zo, p, sp, cp, s, sps, cps, spc, cpc,
          x1, y1, z1, xi, rxy2, xi2, r2, r, w, x2, y2, z2,
          sha, cha, ha, dec;


/* Observed az/el spherical to Cartesian */
   c = cos ( el_o );
   xo = - cos ( az_o ) * c;
   yo = sin ( az_o ) * c;
   zo = sin ( el_o );

/* Rotate into frame of (misaligned) equatorial mount */
   p = phi - me;
   sp = sin ( p );
   cp = cos ( p );
   s = sin ( ma );
   c = cos ( ma );
   sps = sp * s;
	 cps = cp * s;
   spc = sp * c;
	 cpc = cp * c;
   x1 = sp * xo + cp * zo;
   y1 = cps * xo + c * yo - sps * zo;
   z1 = - cpc * xo + s * yo + spc * zo;

/* Rotator axis east/west position (on sky xi is +ve east) */
   xi = - ch - np * z1;

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

/* Cartesian to spherical */
   r = sqrt ( x2 * x2 + y2 * y2 );
   if ( r != 0.0 ) {
      sha = - y2 / r;
      cha = x2 / r;
	 } else {
      sha = 0.0;
			cha = 1.0;
   }
   ha = atan2 ( sha, cha );
   dec = atan2 ( z2, r );

/* Allow for fork flexure */
   dec = dec - fo * cha;

/* Allow for tube flexure */
   ha -= atan2 ( tf * cp * sha , r );
   dec -= tf * ( cp * cha * z2 - sp * r );

/* Allow for index errors */
   ha -= ih;
   w = fmod ( ha, D2PI );
   *ha_e = ( fabs ( w ) < DPI ) ? w : w - dsign ( D2PI, ha );
   *dec_e = dec - id;
}

