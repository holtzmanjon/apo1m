#include "slalib.h"
#include "slamac.h"
void slaTprd ( float xi, float eta, float ra, float dec,
               float *raz, float *decz, int *j )
/*
**  - - - - - - - -
**   s l a T p r d
**  - - - - - - - -
**
**  From the tangent plane coordinates of a star of known RA,Dec,
**  determine the RA,Dec of the tangent point.
**
**  (single precision)
**
**  Given:
**     xi,eta      float   tangent plane rectangular coordinates
**     ra,dec      float   spherical coordinates
**
**  Returned:
**     *raz,*decz  float   spherical coordinates of tangent point
**     *j          int     status: 0 = OK, -1 = no solution
**
**
**  Notes:
**
**  1  The raz value is returned in the range 0-2pi.
**
**  2  The decz value is returned in the range +/-pi, but in the
**     ordinary, non-pole-crossing, case, the range is +/-pi/2.
**
**  3  Cases where there is no solution can only arise near the poles.
**     For example, it is clearly impossible for a star at the pole
**     itself to have a non-zero xi value, and hence it is
**     meaningless to ask where the tangent point would have to be
**     to bring about this combination of xi and dec.
**
**  Called:  slaRanorm
**
**  P.T.Wallace   Starlink   6 October 1994
*/
{
  float x2, y2, sd, cd, sdf, r2, r, s, c;

  x2 = xi * xi;
  y2 = eta * eta;
  sd = (float) sin ( dec );
  cd = (float) cos ( dec );
  sdf = sd * (float) sqrt ( 1.0f + x2 + y2 );
  r2 = cd * cd * ( 1.0f + y2 ) - sd * sd * x2;
  if ( r2 >= 0.0f ) {
     r = (float) sqrt ( r2 );
     s = sdf - eta * r;
     c = sdf * eta + r;
     if ( xi == 0.0f && r == 0.0f ) {
        r = 1.0f;
     }
     *raz = slaRanorm ( ra - (float) atan2 ( xi, r ) );
     *decz = (float) atan2 ( s, c );
     *j = 0;
  } else {
     *j = -1;
  }
}
