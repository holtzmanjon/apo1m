#include "slalib.h"
#include "slamac.h"
#include <stdlib.h>
#include <limits.h>
#ifndef RAND_MAX /* To fix problem with SunOS */
#define RAND_MAX 2147483647
#endif
float slaRandom ( float seed )
/*
**  - - - - - - - - - -
**   s l a R a n d o m
**  - - - - - - - - - -
**
**  Generate pseudo-random real number in the range 0 <= x < 1.
**
**  (single precision)
**
**  Given:
**     seed     float     initializes random number generator: used
**                        first time through only.
**
**  Last revision:   5 October 1994
**
**  Copyright P.T.Wallace.  All rights reserved.
*/
{
   double as;
   unsigned int is;
   static int ftf = 1;

   if ( ftf )
   {
      ftf = 0;
      as = fabs ( (double) seed );
      is = (unsigned int) gmax ( 1.0, as );
      srand ( gmin ( UINT_MAX, is ) );
   }
   return (float) rand() / (float) RAND_MAX;
}
