/****************************************************************************/
/*								*/
/*	Module:		evtimer.h				*/
/*								*/
/*	Purpose:	Inplements a BIOS timer 		*/
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
#ifndef _EVTIMER_H
	#define _EVTIMER_H

#include "mytype.h"

#define TicsPerDay	1573040L	// assumes 18.20648 tics/sec
#define SecsPerDay	86400L
#define TicsFreq		1675L
#define SecsFreq		92L

class EventTimer
	{
		public:
		EventTimer();

		void NewTimer(long Tics);
			// set the timer to expire in Tics tics
		void NewTimerSecs(long Secs);
			// set the timer to expire in Secs secs
		BOOL Expired();
			// returns TRUE if the timer has expired
		long ElapsedTime();
			// returns the elapsed time in tics
		long ElapsedTimeInSecs();
			// returns the elapsed time in seconds
		long ElapsedTimeInMSecs();
			// returns the elapsed time in milliseconds
		long RemainingTime();
			// returns the remaining time in tics
		long RemainingTimeInSecs();
			// returns the remaining time in seconds
		long RemainingTimeInMsecs();
			// returns the remaining time in milliseconds
		void ResetTimer();
			// restart the timer with the last set time

		private:
		long StartTics;
		long ExpireTics;
		long SetTics;
	};

extern unsigned long CountsPerMs;
	// tight loop count for 1 millisecond

extern long Tics2Secs(long Tics);
	// returns seconds value for Tics tics

extern long Secs2Tics(long Secs);
	// returns tics value for Secs seconds

extern void DelayTics(long Tics);
	// delay for Tics tics

extern void Delay(unsigned long Ms);
	// delay for Ms milliseconds - replacement for dos.h delay

extern void CalibrateDelay();
	// delay calibration routine
#endif
/********************************* EOF **************************************/

