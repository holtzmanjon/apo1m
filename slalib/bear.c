#include "slalib.h"
#include "slamac.h"
float slaBear ( float a1, float b1, float a2, float b2 )
/*
**  - - - - - - - -
**   s l a B e a r
**  - - - - - - - -
**
**  Bearing (position angle) of one point on a sphere relative
**  to another.
**
**  (single precision)
**
**  Given:
**     a1,b1    float    spherical coordinates of one point
**     a2,b2    float    spherical coordinates of the other point
**
**  (The spherical coordinates are RA,Dec, Long,Lat etc, in radians.)
**
**  The result is the bearing (position angle), in radians, of point
**  a2,b2 as seen from point a1,b1.  It is in the range +/- pi.  If
**  a2,b2 is due east of a1,b1 the bearing is +pi/2.  Zero is returned
**  if the two points are coincident.
**
**  Called:  slaDbear
**
**  Last revision:   31 October 1993
**
**  Copyright P.T.Wallace.  All rights reserved.
*/
{
   return (float) slaDbear ( (double) a1, (double) b1,
                             (double) a2, (double) b2 );
}
