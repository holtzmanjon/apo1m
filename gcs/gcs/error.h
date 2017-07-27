/****************************************************************************/
/*								*/
/*	Module:		status.h				*/
/*								*/
/*	Purpose:	All possible return error codes		*/
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
/*								*/
/*	Author:		M. Donahue				*/
/*								*/
/****************************************************************************/
#ifndef _STATUS_H
	#define _STATUS_H

#define	GCSERR_OK		0	// good status
#define GCSERR_COMIO		1	// serial i/o error
#define GCSERR_NOFILE          2
#define GCSERR_OUTOFIMAGE      3
#define GCSERR_BADCENTROID     4
#define GCSERR_BADTOT         14

#define GCSERR_FILTER_NOTINIT 5
#define GCSERR_FILTER_TIMEOUT 6
#define GCSERR_FILTER_WRONG 7

#define GCSERR_GUIDER_NOTINIT 8
#define GCSERR_GUIDER_OUTOFRANGE 9


void error_code(int status);
#endif
/********************************* EOF **************************************/

