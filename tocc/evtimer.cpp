/****************************************************************************/
/*									*/
/*	Module:		Timer functions					*/
/*									*/
/*	Purpose:	Inplements a BIOS timer 			*/
/*									*/
/****************************************************************************/
/*									*/
/*                    PROPERTY OF AUTOSCOPE CORPORATION                 */
/*                        2637 Midpoint Dr., Suite D                    */
/*                         Fort Collins, CO  80525                      */
/*									*/
/*                            Copyright 1995                            */
/*              Unauthorized duplication or use is prohibited.		*/
/*									*/
/*	Author:		M. Donahue					*/
/*									*/
/****************************************************************************/
#include <dos.h>
#include <stdio.h>

#include "mytype.h"
#include "evtimer.h"

static BOOL EVTIMER_CAL = FALSE;

const long far *BiosTics = (long far *)MK_FP(0x0040, 0x006c);
	// data area for BIOS tics value

unsigned long CountsPerMs;

long Tics2Secs(long Tics)
	{
		return ((Tics + 9L) * SecsFreq) / TicsFreq;
	}

long Secs2Tics(long Secs)
	{
		return (Secs * TicsFreq) / SecsFreq;
	}

void DelayTics(long Tics)
	{
		if (Tics <= 0)
			return;
		else
			if (Tics > TicsPerDay)
				Tics = TicsPerDay;

		EventTimer ev;
		ev.NewTimer(Tics);
		while (!ev.Expired());
	}

void Delay(unsigned long Ms)
	{
		if (!Ms) return;

		if (!EVTIMER_CAL)
			CalibrateDelay();

		unsigned far *DummyLow = (unsigned far *)MK_FP(0x0000, 0x0000);
		unsigned InitTics;
		unsigned long Counts;
		BOOL Done;
		unsigned long T;
	fprintf(stderr,"Ms: %lu\n", Ms);
	fprintf(stderr,"CountsPerMs: %lu\n", CountsPerMs);

		for (T = 0; T < Ms; T++)
			{
				InitTics = *DummyLow;
				Counts = 0;
				do
					{
						Counts++;
						Done = ((Counts == CountsPerMs)                                                     || (*DummyLow != InitTics));
					}
				while (!Done);
			}
	}

void CalibrateDelay()
	{
		unsigned far *BiosTicsLow = 
                             (unsigned far *)MK_FP(0x0040, 0x006c);
		unsigned InitTics;
		unsigned long Counts = 0;
		BOOL Done;

		asm sti

		CountsPerMs = 1000000U;	// upper limit for counts per tic
		//CountsPerMs = 50000U;	// upper limit for counts per tic

		// wait for tick to change
		InitTics = *BiosTicsLow;
		while (*BiosTicsLow == InitTics) {}

		// now count until it changes again or it reaches a limit
		InitTics = *BiosTicsLow;
		do
			{
				Counts++;
				Done = ((Counts == CountsPerMs) || 
                                        (*BiosTicsLow != InitTics));
			}
		while (!Done);

		// convert to counts per millisecond
		CountsPerMs = Counts / 55;
	}

EventTimer::EventTimer()
	{
		StartTics = 0L;
		ExpireTics = 0L;
		SetTics = 0L;
	}

void EventTimer::NewTimer(long Tics)
	{
		if (Tics > TicsPerDay)
			Tics = TicsPerDay;

		StartTics = *BiosTics;
		ExpireTics = StartTics + Tics;
		SetTics = Tics;
	}

void EventTimer::NewTimerSecs(long Secs)
	{
		NewTimer(Secs2Tics(Secs));
	}

BOOL EventTimer::Expired()
	{
		long CurTics = *BiosTics;

		// check normal expiration
		if (CurTics > ExpireTics)
			return TRUE;

		// check wrapped CurTics
		if ((CurTics < StartTics) && 
                   ((CurTics + TicsPerDay) > ExpireTics))
			return TRUE;

		// if we get here, the timer has not expired yet
		return FALSE;
	}

long EventTimer::ElapsedTime()
	{
		long CurTics = *BiosTics;

		if (CurTics >= StartTics)
			// no midnight wrap yet
			return CurTics - StartTics;
		else
			// got a midnight wrap
			return (TicsPerDay - StartTics) + CurTics;
	}

long EventTimer::ElapsedTimeInSecs()
	{
		return Tics2Secs(ElapsedTime());
	}

long EventTimer::ElapsedTimeInMSecs()
	{
		return ElapsedTime() * 55L;
	}

long EventTimer::RemainingTime()
	{
		long RemainingTics;
		long CurTics = *BiosTics;

		if (CurTics >= StartTics)
			// no midnight wrap yet
			RemainingTics = ExpireTics - CurTics;
		else
			// midnight wrap
			RemainingTics = (ExpireTics - TicsPerDay) - CurTics;

		return (RemainingTics < 0L) ? 0L : RemainingTics;
	}

long EventTimer::RemainingTimeInSecs()
	{
		return Tics2Secs(RemainingTime());
	}

long EventTimer::RemainingTimeInMsecs()
	{
		return RemainingTime() * 55L;
	}

void EventTimer::ResetTimer()
	{
		NewTimer(SetTics);
	}

/********************************* EOF **************************************/

