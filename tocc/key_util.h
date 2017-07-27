/****************************************************************************/
/*							*/
/*	Module:			key_util.h		*/
/*							*/
/*	Purpose:		keyboard utilities	*/
/*							*/
/****************************************************************************/
/*							*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION */
/*                        2637 Midpoint Dr., Suite D    */
/*                         Fort Collins, CO  80525      */
/*							*/
/*                            Copyright 1995            */
/*              Unauthorized duplication or use is prohibited.*/
/*							*/
/*							*/
/*	Author:		M. Donahue			*/
/*							*/
/****************************************************************************/
#ifndef _KEY_UTIL_H
	#define _KEY_UTIL_H

#include "mytype.h"

typedef enum
	{ 	BS              =  8,
        NL              = 10,
        CR 		= 13,
		ESC 		= 27,
		F1 		= 59,	F2, F3, F4, F5, F6, F7, F8, F9, F10,
		HOME 		= 71,	UP, PGUP,
		LEFT 		= 75,
		RIGHT 	= 77,
		END 		= 79,	DOWN, PGDN,	INS, DEL,
		SHFT_F1 = 84, SHFT_F2, SHFT_F3, SHFT_F4, SHFT_F5, SHFT_F6,
		SHFT_F7, SHFT_F8, SHFT_F9, SHFT_F10,
		CTRL_F1, CTRL_F2, CTRL_F3, CTRL_F4, CTRL_F5,
		CTRL_F6, CTRL_F7, CTRL_F8, CTRL_F9, CTRL_F10,
		ALT_F1, ALT_F2, ALT_F3, ALT_F4, ALT_F5, ALT_F6,
		ALT_F7, ALT_F8, ALT_F9, ALT_F10
	} KEYS;

extern BOOL inkey(BOOL &functionKey, unsigned char &key, BOOL echo);	// TRUE if key hit
extern void clear_keyboard();
#endif
/********************************* EOF **************************************/

