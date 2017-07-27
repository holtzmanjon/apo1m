/****************************************************************************/
/*																																				 	*/
/*	Module:			window.h																										*/
/*																																					*/
/*	Purpose:		Check coordinates against the observation window						*/
/*																																				 	*/
/****************************************************************************/
/*																																				 	*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION                    	*/
/*                        2637 Midpoint Dr., Suite D                       	*/
/*                         Fort Collins, CO  80525                         	*/
/*																																				 	*/
/*                            Copyright 1995                               	*/
/*              Unauthorized duplication or use is prohibited.						 	*/
/*																																				 	*/
/*																																				 	*/
/*	Author:		M. Donahue																										*/
/*																																					*/
/****************************************************************************/
#ifndef _WINDOW_H
	#define _WINDOW_H

// NOTE: all parameters are in radians except for the epoch

#include "mytype.h"
extern BOOL is_star_in_window(double az, double alt);

extern BOOL is_star_in_window(double ra_hrs, double dec_degs,
															double epoch, double pmra,
															double pmdec, double parallax,
															double radial_velocity);
#endif
/********************************* EOF **************************************/

