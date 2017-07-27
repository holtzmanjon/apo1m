/****************************************************************************/
/*								*/
/*	Module:		mytype.h				*/
/*								*/
/*	Purpose:	common typedefs				*/
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
#ifndef _MYTYPE_H
	#define _MYTYPE_H

typedef int BOOL;

#define TRUE 	1
#define FALSE 0

typedef unsigned char byte;
typedef unsigned int word;
struct mytime {
  int ti_hour;
  int ti_min;
  int ti_sec;
  double ti_hund;
};
void mygettime(struct date *, struct mytime *);

#endif
/********************************* EOF **************************************/

