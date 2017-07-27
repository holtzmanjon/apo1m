#include "slalib.h"
#include "slamac.h"
float slaGresid ( float s )
/*
**  - - - - - - - - - -
**   s l a G r e s i d
**  - - - - - - - - - -
**
**  Generate pseudo-random normal deviate ( = "Gaussian residual").
**
**  (single precision)
**
**  Given:
**     s      float     standard deviation
**
**  Called:  slaRandom
**
**  The results of many calls to this routine will be normally
**  distributed with mean zero and standard deviation s.
**
**  The Box-Muller algorithm is used.  This is described in
**  Numerical Recipes, Section 7.2.
**
**  Defined in slamac.h:  TRUE, FALSE
**
**  Last revision:   16 November 1993
**
**  Copyright P.T.Wallace.  All rights reserved.
*/

#define TINY 1e-30

{
   static float seed = 1;
   static int first = TRUE;
   static float x, y;
   float w;
   double rsq;


/* Second normal deviate of the pair available? */
   if ( first ) {

   /* No - generate random x,y in unit-radius circle */
      do {
         x = ( 2.0f * slaRandom ( seed ) ) - 1.0f;
         y = ( 2.0f * slaRandom ( seed ) ) - 1.0f;
         rsq = (double) ( x * x + y * y );
      } while ( rsq >= 1.0 );

   /* Box-Muller transformation, generating two deviates */
      w = ( rsq > TINY ) ? (float) sqrt ( - 2.0 * log ( rsq ) / rsq ) : 0.0f ;
      x *= w;
      y *= w;

   /* Set flag to indicate availability of next deviate */
      first = FALSE;

   } else {

   /* Return second deviate of the pair & reset flag */
      x = y;
      first = TRUE;
   }

/* Scale the deviate by the required standard deviation */
   return x * s;
}
