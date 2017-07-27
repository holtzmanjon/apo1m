#include "slalib.h"
#include "slamac.h"
void slaCc62s ( float v[6],
                float *a, float *b, float *r,
                float *ad, float *bd, float *rd )
/*
**  - - - - - - - - -
**   s l a C c 6 2 s
**  - - - - - - - - -
**
**  Conversion of position & velocity in Cartesian coordinates
**  to spherical coordinates.
**
**  (single precision)
**
**  Given:
**     v     float[6]   Cartesian position & velocity vector
**
**  Returned:
**     *a    float      longitude (radians)
**     *b    float      latitude (radians)
**     *r    float      radial coordinate
**     *ad   float      longitude derivative (radians per unit time)
**     *bd   float      latitude derivative (radians per unit time)
**     *rd   float      radial derivative
**
**  Last revision:   5 October 1994
**
**  Copyright P.T.Wallace.  All rights reserved.
*/
{
   float rxy2, w, rxy, r2, xyp;

/* Components of vector */
   float x = v[0];
   float y = v[1];
   float z = v[2];
   float xd = v[3];
   float yd = v[4];
   float zd = v[5];

/* Component of r in xy plane squared (with crude precaution */
/* against polar problems and daft units)                    */
   rxy2 = x * x + y * y;
   w = ( xd * xd + yd * yd + zd * zd ) / 1.0e30f;
   rxy2 = gmax( rxy2, w );

/* Other useful functions */
   rxy = (float) sqrt ( rxy2 );
   r2  = rxy2 + z * z;
   xyp = x * xd + y * yd;

/* Position in spherical coordinates */
   *a = (float) atan2 ( y, x );
   *b = (float) atan2 ( z, rxy );
   *r = (float) sqrt ( r2 );

/* Velocity in spherical coordinates */
   *ad = ( x * yd - y * xd ) / rxy2;
   *bd = ( zd * rxy2 - z * xyp ) / ( r2 * rxy );
   *rd = ( xyp + z * zd ) / *r;
}
