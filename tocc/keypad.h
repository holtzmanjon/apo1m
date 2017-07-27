/****************************************************************************/
/*																																				 	*/
/*	Module:			keypad.h																										*/
/*																																					*/
/*	Purpose:		module to handle commands from the telescope keypad					*/
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
#ifndef _KEYPAD_H
	#define _KEYPAD_H

typedef enum
	{
		TELE_UP, TELE_DOWN, TELE_LEFT, TELE_RIGHT, FOCUS_UP, FOCUS_DOWN,
		SIZE_INC, SIZE_DEC, RESET, DISPLAY
	} KEYCOMMAND;

extern void keypad_action(KEYCOMMAND command);
extern void reset_keypad_counters();

extern double kp_ra_sec_error;
extern double kp_dec_arcsec_error;
extern long kp_move_increment;
#endif
/********************************* EOF **************************************/

