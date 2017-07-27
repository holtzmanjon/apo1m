/****************************************************************************/
/*								*/
/*	Module:		sounds.cpp				*/
/*								*/
/*	Purpose:	various sounds used throughout the program										*/
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
#include <dos.h>

#include "sounds.h"

void action_sound()
	{
		sound(3000);
		delay(15);
		nosound();
	}

void error_sound()
	{
		sound(100);
		delay(400);
		nosound();
	}

/********************************* EOF **************************************/

