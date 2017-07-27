/****************************************************************************/
/*								*/
/*	Module:			uts-10.h			*/
/*								*/
/*	Purpose:		routines to handle the UTS-10 WWV receiver									*/
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
#ifndef _UTS10_H
	#define _UTS10_H

#include <dos.h>

extern unsigned uts10_get_sync(unsigned &last_sync);
extern unsigned uts10_get_time(struct date &dateRec, struct mytime &timeRec);
extern unsigned setup_uts10();

#endif
/********************************* EOF **************************************/

