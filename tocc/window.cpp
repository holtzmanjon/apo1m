/****************************************************************************/
/*								*/
/*	Module:		window.cpp				*/
/*								*/
/*	Purpose:	check coordinates against the observation window*/
/*								*/
/****************************************************************************/
/*								*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION         */
/*                        2637 Midpoint Dr., Suite D            */
/*                         Fort Collins, CO  80525              */
/*								*/
/*                            Copyright 1995                    */
/*              Unauthorized duplication or use is prohibited.	*/
/*								*/
/*	Author:		M. Donahue				*/
/*								*/
/****************************************************************************/
#include "window.h"
#include "globals.h"
#include "tcs_math.h"
#include "slamac.h"

//------------------------------------------------------------------
// 	Name.......:	is_star_in_window
//
//  Purpose....:	determines whether or not the coordinates given are
//			within the telescope's observation window.
//
//	Input......:	ra		- decimal RA (radians)
//			dec	 -	decimal Dec (radians)
//
//	Output.....:	TRUE - coordinates are within window
//
//------------------------------------------------------------------
BOOL is_star_in_window(double ra, double dec,
			double epoch, double pmra,
			double pmdec, double parallax,
			double radial_velocity)
	{
		double alt;
		double az;

		// convert RA Dec into Alt/Az
		equtohor(ra, dec, epoch, pmra, pmdec, parallax,
			radial_velocity, az, alt);
		return is_star_in_window(az, alt);
	}

BOOL is_star_in_window(double az, double alt)
	{
		/*
			The window works as follows:
The altitude value at sysGlobal->window_values[0] is for the
azimuth 0°.  This azimuth is the midpoint so the altitude at
this azimuth point covers azimuths between 345° and 15° through 0°.
		*/
		az *= DR2D;
		alt *= DR2D;

		if (alt >= 89.5)
			return FALSE;

		if ((az <= 15.0) || (az >= 345.0))
			return (alt >= sysGlobal->window_values[0]);

		double taz, tazl, tazh;
		BOOL good = FALSE;
		for (int i = 1; i < 12; i++)
			{
				taz = 30.0 * i;
				tazl = taz - 15.0;
				tazh = taz + 15.0;
				if ((az > tazl) && (az <= tazh))
				  good = (alt >= sysGlobal->window_values[i]);
			}

	// we should not get here at all!  Return from 165 to 108 to be safe
		return good;
	}

/********************************* EOF **************************************/

