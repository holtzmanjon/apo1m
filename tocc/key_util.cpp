/****************************************************************************/
/*                                                      */
/*      Module:         key_util.cpp                    */
/*                                                      */
/*      Purpose:        keyboard utilities              */
/*                                                      */
/****************************************************************************/
/*                                                      */
/*                    PROPERTY OF AUTOSCOPE CORPORATION */
/*                        2637 Midpoint Dr., Suite D    */
/*                         Fort Collins, CO  80525      */
/*                                                      */
/*                            Copyright 1995            */
/*              Unauthorized duplication or use is prohibited.                                                  */
/*                                                      */
/*      Author:         M. Donahue                      */
/*                                                      */
/****************************************************************************/
#include <conio.h>

#include "key_util.h"

BOOL inkey(BOOL &functionKey, unsigned char &key, BOOL echo)
	{
		if (!kbhit())
			return FALSE;

		if (echo)
		  key = getche();
		else
		  key = getch();

		functionKey = (!key);

		if (functionKey)
			if (echo) 
			  key = getche();
			else 
			  key = getch();

// Check for non-extended keys that will be redefined as function keys
		switch (key)
			{
				case CR: functionKey = TRUE; break;
				case ESC:functionKey = TRUE; break;
			}

		return TRUE;
	}

void clear_keyboard()
	{
		unsigned char key;

		while (kbhit())
			{
				key = getch();
				if (!key)
					key = getch();
			}
	}
/********************************* EOF **************************************/

