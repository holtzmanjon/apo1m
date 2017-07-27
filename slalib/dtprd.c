#include "slalib.h"
#include "slamac.h"
void slaDtprd ( double xi, double eta, double ra, double dec,
                double *raz, double *decz, int *j )
/*
**  - - - - - - - - -
**   s l a D t p r d
**  - - - - - - - - -
**
**  From the tangent plane coordinates of a star of known RA,Dec,
**  determine the RA,Dec of the tangent point.
**
**  (double precision)
**
**  Given:
**     xi,eta      double  tangent plane rectangular coordinates
**     ra,dec      double  spherical coordinates
**
**  Returned:
**     *raz,*decz  double  spherical coordinates of tangent point
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
**  Called:  slaDranrm
**
**  P.T.Wallace   Starlink   6 October 1994
*/
{
  double x2, y2, sd, cd, sdf, r2, r, s, c;

  x2 = xi * xi;
  y2 = eta * eta;
  sd = sin ( dec );
  cd = cos ( dec );
  sdf = sd * sqrt ( 1.0 + x2 + y2 );
  r2 = cd * cd * ( 1.0 + y2 ) - sd * sd * x2;
  if ( r2 >= 0.0 ) {
     r = sqrt ( r2 );
     s = sdf - eta * r;
     c = sdf * eta + r;
     if ( xi == 0.0 && r == 0.0 ) {
        r = 1.0;
     }
     *raz = slaDranrm ( ra - atan2 ( xi, r ) );
     *decz = atan2 ( s, c );
     *j = 0;
  } else {
     *j = -1;
  }
}
