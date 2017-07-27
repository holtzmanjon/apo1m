#undef no_hardware
/****************************************************************************/
/*                                                                     */
/*      Module:         uts10.cpp                                      */
/*                                                                     */
/*      Purpose:        routines to handle the UTS-10 WWV receiver     */
/*                                                                     */
/****************************************************************************/
/*                                                                     */
/*                    PROPERTY OF AUTOSCOPE CORPORATION                */
/*                        2637 Midpoint Dr., Suite D                   */
/*                         Fort Collins, CO  80525                     */
/*                                                                     */
/*                            Copyright 1995                           */
/*              Unauthorized duplication or use is prohibited.         */
/*                                                                     */
/*      Author:         M. Donahue                                     */
/*                                                                     */
/****************************************************************************/
#undef DEBUG_UTS10
#define DISPLAY_UTS10
#define UTS_DEBUG_COM_PORT COM2
#define UTS_DELAY_TIME 500            // delay between consequtive commands (ms)

#include <string.h>
#include <iostream.h>
#include <mem.h>
#include <dos.h>
#include <stdio.h>
#ifdef DEBUG_UTS10
	#include <conio.h>
#endif
#ifndef no_hardware
#include "serial.h"
#endif
#include "uts10.h"
#include "status.h"
#include "mytype.h"
#include "weather.h"
#include "io.h"
#ifndef DEBUG_UTS10
	#include "globals.h"
#endif

#define MAX_UTS_BUFFER_LEN 128

#ifndef no_hardware
int UTSinit = 1;
#endif
char UTSbuffer[MAX_UTS_BUFFER_LEN];
//unsigned UTSLastStatus = TCSERR_OK;
unsigned UTSLastStatus = 1;

#ifndef no_hardware
 unsigned uts10_access(char *message)
#else
unsigned uts10_access(char*)
#endif
	{
		#ifdef no_hardware
		return TCSERR_OK;
		#else
		unsigned char ch;

                // select the proper port
                UTSinit = setup_uts10();
		if (UTSinit != 0)
			return TCSERR_COMNA;

		// send the command to the UTS-10
		SendString(message);

		// if this was a 'Q' command, wait for the response
		if ((message[0] == 'q') || (message[0] == 'Q'))
			{
                          // Wait a second for the response to come it
                                delay(1000);
                                GetString(UTSbuffer);

// it is possible that the response contains unsolicited characters
// such as the #, * or ! character.  These characters have to do
// with signal sync conditions and are ignored by this software so strip them.

				strrev(UTSbuffer);
				ch = UTSbuffer[strlen(UTSbuffer) - 1];
				while ((ch == '!') || (ch == '#') 
                                       || (ch == '*'))
				  {
				    UTSbuffer[strlen(UTSbuffer) - 1] = '\x00';
				    ch = UTSbuffer[strlen(UTSbuffer) - 1];
				  }
				strrev(UTSbuffer);

			}

                // close this serial port
                closeserial();

		return TCSERR_OK;
		#endif
	}

unsigned init_uts10()
	{
		#ifndef no_hardware
                UTSinit = setup_uts10();

		if (UTSinit != 0)
			return TCSERR_COMNA;

		sprintf(outbuf,"Initializing the UTS-10 WWV receiver.");
                writeline(outbuf,1);

		// turn off automatic daylight savings
		// remember, non-zero return code is good.
		if (uts10_access("sd0")) goto leave;

		// disable the 'set' button on the physical unit so that the 
		// settings cannot be changed while this program is running
		delay(UTS_DELAY_TIME);
		if (uts10_access("se0")) goto leave;

		// set the signal loss interval
		delay(UTS_DELAY_TIME);
		if (uts10_access("sl00")) goto leave;

		// set 24 hour format
		delay(UTS_DELAY_TIME);
		if (uts10_access("sm0")) goto leave;

		// set the end of line character
		delay(UTS_DELAY_TIME);
		if (uts10_access("sn\x0D")) goto leave;

		// set the time zone
		delay(UTS_DELAY_TIME);
		if (uts10_access("so0")) goto leave;

		// set the pulse time
		delay(UTS_DELAY_TIME);
		if (uts10_access("sp2400")) goto leave;

		// set the update interval
		delay(UTS_DELAY_TIME);
		if (uts10_access("su0000")) goto leave;

		return TCSERR_OK;

		// error abort
		leave:
		UTSinit = 1;
                closeserial();
		return UTSLastStatus;
		#else   // no_hardware
		return TCSERR_OK;
		#endif
	}

unsigned uts10_get_time(struct date &dateRec, struct mytime &timeRec)
	{
#undef no_UTS10
		#ifdef no_UTS10
		mygettime(&dateRec, &timeRec);
		// getdate(&dateRec);
		return TCSERR_OK;
		#else
		unsigned status;
                struct date d;
                struct mytime t;

		#ifndef DEBUG_UTS10
		if (sysGlobal->wwv_type != 1)
			return TCSERR_NA;
		#endif

		if (UTSinit != 0)
		{
			status = init_uts10();
			if (status)
				return status;

			#ifdef DEBUG_UTS10
			cout << "\nPress any key to continue...";
			while(!kbhit()) {}
			getch();
			cout << endl << endl;
			#endif
		}

		// first, make sure that the unit is operational
		status = uts10_access("qm");
		if (status) return status;

		#ifdef DISPLAY_UTS10
		mygettime(&d,&t);
                sprintf(outbuf,"%d:%d:%d.%d    %s\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund,UTSbuffer);
		#endif

		// get status byte
		unsigned char stat_byte = UTSbuffer[9] - '0';
		if (stat_byte & 0x02)
			return TCSERR_UTSFAULT;

		if (!(stat_byte & 0x08))
			return TCSERR_UTSNOTIME;

		// get the date
		// NOTE:
		// To get the year, we could use either QD or QM.  Since QD only
		// returns the last 2 digits, we will use the year from the QM
		// command above which returns the numbers of years since 1986.
		// This will prevent problems at the turn of the century.
		//
		dateRec.da_year = 1986 + UTSbuffer[4] - '0';

		status = uts10_access("qd");
		if (status) return status;
		#ifdef DISPLAY_UTS10
		mygettime(&d,&t);
                sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    %s\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund,UTSbuffer);
		#endif

		dateRec.da_mon = (UTSbuffer[3] - '0') * 10 + UTSbuffer[4] - '0';
		dateRec.da_day = (UTSbuffer[6] - '0') * 10 + UTSbuffer[7] - '0';

		// get the time
		status = uts10_access("qt");
		if (status) return status;
		#ifdef DISPLAY_UTS10
		mygettime(&d,&t);
                sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    %s\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund,UTSbuffer);
		#endif

		timeRec.ti_hour =(UTSbuffer[1] - '0') * 10 + UTSbuffer[2] - '0';
		timeRec.ti_min = (UTSbuffer[4] - '0') * 10 + UTSbuffer[5] - '0';
		timeRec.ti_sec = (UTSbuffer[7] - '0') * 10 + UTSbuffer[8] - '0';
		timeRec.ti_hund =(UTSbuffer[10] - '0')* 10 + UTSbuffer[11] -'0';

		#ifdef DISPLAY_UTS10
                sprintf(outbuf+strlen(outbuf),
                        "Hour  - %d\n\r"
                        "Min   - %d\n\r"
                        "Sec   - %d\n\r"
                        "Hund  - %d\n\r"
                        "Year  - %d\n\r"
                        "Month - %d\n\r"
                        "Day  - %d\n\r",
                        (int)timeRec.ti_hour,
                        (int)timeRec.ti_min,
                        (int)timeRec.ti_sec,
                        (int)timeRec.ti_hund,
                        dateRec.da_year,
                        (int)dateRec.da_mon,
                        (int)dateRec.da_day);
                writeline(outbuf,1);
		#endif

		return TCSERR_OK;
	        #endif
	}

unsigned uts10_get_sync(unsigned &last_sync)
	{
		#ifdef no_hardware
		last_sync = 0;
		return TCSERR_OK;
		#else
		unsigned status;

		#ifndef DEBUG_UTS10
		if (sysGlobal->wwv_type != 1)
			return TCSERR_NA;
		#endif

		if (UTSinit != 0)
			{
				status = init_uts10();
				if (status)
					return status;

				#ifdef DEBUG_UTS10
				cout << "\nPress any key to continue...";
				while(!kbhit()) {}
				getch();
				cout << endl << endl;
				#endif
			}

		// first, make sure that the unit is operational
		status = uts10_access("qm");
		if (status) return status;
		#ifdef DISPLAY_UTS10
                sprintf(outbuf,"%s",UTSbuffer);
                writeline(outbuf,1);
		#endif

		// get status byte
		unsigned char stat_byte = UTSbuffer[9] - '0';
		if (stat_byte & 0x02)
			return TCSERR_UTSFAULT;

		if (!(stat_byte & 0x08))
			return TCSERR_UTSNOTIME;

		last_sync = 0;
		for (int i = 13; i < 17; i++)
			{
				last_sync *= 10;
				last_sync += UTSbuffer[i] - '0';
			}

		#ifdef DISPLAY_UTS10
                sprintf(outbuf,"Last time sync: %d minutes ago",last_sync);
                writeline(outbuf,1);
		#endif

		return TCSERR_OK;
		#endif  // no_hardware
	}

#ifdef DEBUG_UTS10
int main()
	{
		struct date dateRec;
		struct mytime timeRec;
		unsigned status;
		unsigned last;

		while (!kbhit())
			{
				status = uts10_get_time(dateRec, timeRec);
				if (status)
					cout << endl << "Error: " << status << endl << endl;
				else
					cout << endl;

				status = uts10_get_sync(last);
				if (status)
					cout << endl << "Error: " << status << endl << endl;
				else
					cout << endl;

				if (!kbhit())
					delay(5000);
			}
		getch();
		return 0;
	}
#endif

unsigned setup_uts10()
{
		#ifndef DEBUG_UTS10
		if (sysGlobal->weather_station_com_ch == 1)
                        UTSinit = SetSerial(COM2, 9600, NO_PARITY, 8, 1);    
		else
                        UTSinit = SetSerial(COM1, 9600, NO_PARITY, 8, 1);    
		#else
		        UTSinit = SetSerial(UTS_DEBUG_COM_PORT, 9600, 
                                            NO_PARITY, 8, 1);
		#endif
		if (UTSinit != 0)
			return TCSERR_COMNA;
                else {
                        initserial();
                        ctrlbrk(c_break);
                        return(0);
                }
}
/********************************* EOF **************************************/

