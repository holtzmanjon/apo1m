/* wcsconfig_utils.h.  Generated from wcsconfig_utils.h.in by configure.  */
/*============================================================================
*
*   wcsconfig_utils.h is generated from wcsconfig_utils.h.in by 'configure'.
*   It contains C preprocessor macro definitions for compiling the WCSLIB 4.13
*   utilities.
*
*   Author: Mark Calabretta, Australia Telescope National Facility
*   http://www.atnf.csiro.au/~mcalabre/index.html
*   $Id: wcsconfig_utils.h.in,v 4.13.1.1 2012/03/14 07:40:38 cal103 Exp cal103 $
*===========================================================================*/

#include <wcsconfig.h>

/* Definitions for Large File Support (LFS), i.e. files larger than 2GiB, for
 * the fitshdr utility. */

/* Define to 1 if fseeko() is available (for small or large files). */
#define HAVE_FSEEKO 1

/* Define _LARGEFILE_SOURCE to get prototypes from stdio.h for the LFS
 * functions fseeko() and ftello() which use an off_t argument in place of a
 * long. */
/* #undef _LARGEFILE_SOURCE */

/* There seems to be a bug in autoconf that causes _LARGEFILE_SOURCE not to be
 * set in Linux.  This dreadful kludge gets around it for now. */
#if (defined HAVE_FSEEKO && !defined _LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE
#endif

/* Number of bits in a file offset (off_t) on systems where it can be set. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files needed on AIX-type systems. */
/* #undef _LARGE_FILES */
