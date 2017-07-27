/****************************************************************************/
/*								*/
/*	Module:			tech80.h			*/
/*								*/
/*	Purpose:		routines to handle the dome encoder*/
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
#ifndef _TECH80_H
	#define _TECH80_H

#define max_tech80_encoder_reading 16777215L    // (256^2 * 255)+(256 * 255)+255

extern void init_dome_encoder();
extern long read_dome_encoder();

#endif
/********************************* EOF **************************************/

