#include "slalib.h"
#include "slamac.h"
void slaDc62s ( double v[6], double *a, double *b, double *r,
                double *ad, double *bd, double *rd )
/*
**  - - - - - - - - -
**   s l a D c 6 2 s
**  - - - - - - - - -
**
**  Conversion of position & velocity in Cartesian coordinates
**  to spherical coordinates.
**
**  (double precision)
**
**  Given:
**     v     double[6]  Cartesian position & velocity vector
**
**  Returned:
**     *a    double     longitude (radians)
**     *b    double     latitude (radians)
**     *r    double     radial coordinate
**     *ad   double     longitude derivative (radians per unit time)
**     *bd   double     latitude derivative (radians per unit time)
**     *rd   double     radial derivative
**
**  Last revision:   5 October 1994
**
**  Copyright P.T.Wallace.  All rights reserved.
*/
{
   double rxy2, w, rxy, r2, xyp;

/* Components of vector */
   double x = v[0];
   double y = v[1];
   double z = v[2];
   double xd = v[3];
   double yd = v[4];
   double zd = v[5];

/* Component of r in xy plane squared (with crude precaution */
/* against polar problems and daft units)                    */
   rxy2 = x * x + y * y;
   w = ( xd * xd + yd * yd + zd * zd ) / 1.0e30;
   rxy2 = gmax( rxy2, w );

/* Other useful functions */
   rxy = sqrt ( rxy2 );
   r2  = rxy2 + z * z;
   xyp = x * xd + y * yd;

/* Position in spherical coordinates */
   *a = atan2 ( y, x );
   *b = atan2 ( z, rxy );
   *r = sqrt ( r2 );

/* Velocity in spherical coordinates */
   *ad = ( x * yd - y * xd ) / rxy2;
   *bd = ( zd * rxy2 - z * xyp ) / ( r2 * rxy );
   *rd = ( xyp + z * zd ) / *r;
}
