#include "slalib.h"
#include "slamac.h"
#include <time.h>
#include <stdio.h>
void slaWait ( float delay )
/*
**  - - - - - - - -
**   s l a W a i t
**  - - - - - - - -
**
**  Interval wait.
**
**  Given:
**     delay     float      delay in seconds
**
**  Note that this routine is extremely crude, in that the wait
**  is implemented via a CPU-bound loop.  The hibernate and
**  scheduled wakeup facilities that all operating systems offer
**  and which are required for a proper implementation are
**  unfortunately not available through any standard C library
**  functions.
**
**  Last revision:   9 December 1993
**
**  Copyright P.T.Wallace.  All rights reserved.
*/
{
   time_t then,      /* Calendar time on entry        */
          now;       /* Calendar time during the wait */

/* Calendar time at start */
   then = time ( NULL );

/* Proceed if OK */
   if ( then != -1 ) {

   /* Loop */
      for ( ; ; ) {

      /* Calendar time now */
         now = time ( NULL );

      /* Break if requested amount of time has elapsed */
         if ( now == -1 ||
              (float) difftime ( now, then ) >= delay ) break;
      }
   }
}
