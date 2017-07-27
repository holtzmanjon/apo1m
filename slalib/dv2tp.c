#include "slalib.h"
#include "slamac.h"
void slaDv2tp ( double v[3], double v0[3], double *xi, double *eta, int *j )
/*
**  - - - - - - - - -
**   s l a D v 2 t p
**  - - - - - - - - -
**
**  Given the direction cosines of a star and of the tangent point,
**  determine the star's tangent-plane coordinates.
**
**  (double precision)
**
**  Given:
**     v         double[3]    direction cosines of star
**     v0        double[3]    direction cosines of tangent point
**
**  Returned:
**     *xi,*eta  double       tangent plane coordinates of star
**     j         int          status:   0  =  OK
**                                      1  =  error, star too far from axis
**                                      2  =  error, antistar too far from axis
**                                      3  =  error, antistar on tangent plane
**
**  Notes:
**
**  1  If vector v0 is not of unit length, or if vector v is of zero
**     length, the results will be wrong.
**
**  2  If v0 points at a pole, the returned xi,eta will be based on the
**     arbitrary assumption that the RA of the tangent point is zero.
**
**  3  This routine is the Cartesian equivalent of the routine slaDs2tp.
**
**  Last revision:   12 February 1995
**
**  Copyright P.T.Wallace.  All rights reserved.
*/
{
   double x, y, z, x0, y0, z0, r2, r, w, d;
   double tiny = 1e-6;


   x = v[0];
   y = v[1];
   z = v[2];
   x0 = v0[0];
   y0 = v0[1];
   z0 = v0[2];
   r2 = x0 * x0 + y0 * y0;
   r = sqrt ( r2 );
   if ( r == 0.0 ) {
      r = 1e-20;
      x = r;
   }
   w = x * x0 + y * y0;
   d = w + z * z0;
   if ( d > tiny ) {
      *j = 0;
   } else  if ( d >= 0.0 ) {
      *j = 1;
      d = tiny;
   } else if (d > - tiny ) {
      *j = 2;
      d = - tiny;
   } else {
      *j = 3;
   }
   d *= r;
   *xi = ( y * x0 - x * y0 ) / d;
   *eta = ( z * r2 - z0 * w ) / d;
}
