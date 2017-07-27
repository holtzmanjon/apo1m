/****************************************************************************/
/*								*/
/*	Module:			keypad.h			*/
/*								*/
/*	Purpose:		module to handle commands from the telescope keypad					*/
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
#include <iostream.h>

#include "keypad.h"
#include "globals.h"
#include "sounds.h"
#include "status.h"
#include "slamac.h"
#include "tcs.h"
#include "slalib.h"

void reset_keypad_counters()
	{
		kp_ra_sec_error = kp_dec_arcsec_error = 0.0;
	}

void display_keypad_counters()
	{
		printf(	"\nKeypad position:\n"
						"\tRA:  %.6f seconds\n"
						"\tDec: %.6f arcseconds\n",
						kp_ra_sec_error,
						kp_dec_arcsec_error);
	}

void display_move_increment()
	{
		cout << "\nKeypad move increment = ";
		if (kp_move_increment < 60)
			cout << kp_move_increment << " arcseconds.\n";
		else
			if (kp_move_increment < 3600)
				cout << (kp_move_increment / 60) << " minutes.\n";
			else
				cout << (kp_move_increment / 3600) << " degrees.\n";
	}

void keypad_action(KEYCOMMAND command)
	{
		unsigned status = TCSERR_OK;
		double ra_seconds;

		if (sysGlobal->keypad_ud_reversed)
			{
				if (command == TELE_UP)
					command = TELE_DOWN;
				else
					if (command == TELE_DOWN)
						command = TELE_UP;
			}

		if (sysGlobal->keypad_lr_reversed)
			{
				if (command == TELE_LEFT)
					command = TELE_RIGHT;
				else
					if (command == TELE_RIGHT)
						command = TELE_LEFT;
			}

		if (!G->telescope_initialized || G->telescope_at_home)
			switch (command)
				{
					case TELE_UP:
					case TELE_DOWN:
					case TELE_LEFT:
					case TELE_RIGHT:  error_sound();
														if (G->telescope_at_home)
															cout << "Error!  The telescope is at home.\n";
														else
															cout << "Error!  The telescope"
																			" is not initialized\n";
														return;
				};

		switch (command)
			{
		  	case RESET:
				reset_keypad_counters();
				break;

			case DISPLAY:
				display_keypad_counters();
				break;

			case TELE_UP:
                                if (G->handpaddle==1){
				status = old_tcs_move_noupdate(0, kp_move_increment);
                                } else
				status = tcs_move_noupdate(0, kp_move_increment);
				if (!status)
				kp_dec_arcsec_error += kp_move_increment;
				display_keypad_counters();
				break;

			case TELE_DOWN:
                                if (G->handpaddle==1){
				status = old_tcs_move_noupdate(0, -kp_move_increment);
                                } else
				status = tcs_move_noupdate(0, -kp_move_increment);
				if (!status)
				kp_dec_arcsec_error -= kp_move_increment;
				display_keypad_counters();
				break;

			case TELE_LEFT:
				ra_seconds = -(double)kp_move_increment / 15.0
                                             / cos(G->current_mean_dec);
                                if (G->handpaddle==1){
				status = old_tcs_move_noupdate(ra_seconds, 0);
                                } else
				status = tcs_move_noupdate(ra_seconds, 0);
				if (!status)
				kp_ra_sec_error += ra_seconds;
				display_keypad_counters();
				break;

			case TELE_RIGHT:
				ra_seconds = (double)kp_move_increment / 15.0
                                             / cos(G->current_mean_dec);
                                if (G->handpaddle==1){
				status = old_tcs_move_noupdate(ra_seconds, 0);
                                } else
				status = tcs_move_noupdate(ra_seconds, 0);
				if (!status)
				kp_ra_sec_error += ra_seconds;
				display_keypad_counters();
				break;

			case FOCUS_UP:
				break;

			case FOCUS_DOWN:
				break;

			case SIZE_INC:
				if (move_idx < 7)
				kp_move_increment = move_vals[++move_idx];
				else
				error_sound();
				display_move_increment();
				break;

			case SIZE_DEC:
				if (move_idx > 0)
				kp_move_increment = move_vals[--move_idx];
				else
				error_sound();
				display_move_increment();
				break;
			}

		if (status)
			{
				error_sound();
				cout << "Error code: " << status << endl;
			}
	}

/********************************* EOF **************************************/

