/****************************************************************************/
/*								*/
/*	Module:		status.h				*/
/*								*/
/*	Purpose:	All possible return error codes		*/
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
#ifndef _STATUS_H
	#define _STATUS_H

#define	TCSERR_OK		0	// good status
#define TCSERR_NA		1	// equipment not available or disabled
#define TCSERR_DOMEHOME		2 	// dome could not find home in time allotted
#define TCSERR_SHUTTERPOS	3	// unable to verify dome shutter position
#define	TCSERR_SCFREAD		4	// unable to read .SCF file
#define TCSERR_TELENOTINIT	5	// the telescope is not initialized
#define TCSERR_NETWORKDOWN	6	// the network link is down
#define TCSERR_DOMEMOVE		7	// dome could not move in time allotted
#define TCSERR_DOMENOTINIT	8	// the dome has not been initialized
#define TCSERR_CANTINITTELE	9	// cannot initialize the telescope
#define TCSERR_SECONDARY	10	// tuv tilt or range exceeded
#define TCSERR_MOVELIMIT	11	// move would run telescope into the limits
#define TCSERR_MOVEWINDOW	12	// move would run telescope beyond window
#define TCSERR_NOFILTER		13	// the specified filter is not available
#define TCSERR_NOACQUIRE	14	// unable to acquire object
#define TCSERR_CCDNOCAL		15	// the CCD camera is not calibrated
#define TCSERR_WEATHERSHUT	16	// the observatory shut down due to weather
#define TCSERR_STATIONSHUT	17	// the observatory shut down due to bad sensors
#define TCSERR_NOAUX		18 	// the auxiliary channel is not available
#define TCSERR_MCREADSCF	19	// the MC cannot read the .SCF files
#define TCSERR_MCWRITESCF	20	// the MC cannot write the .SCF files
#define TCSERR_TOCCWRITESCF	21	// the TOCC cannot write the .SCF files
#define TCSERR_PHTNOPULSE	22	// the SSP-3/SSP-5 photometer is not counting
#define TCSERR_USERABORT	23	// a user abort was received
#define TCSERR_WDOGTOUT		24	// the watchdog timer timed out
#define TCSERR_MOVETIMEOUT	25	// the move took too long
#define TCSERR_TELEATHOME	26	// the telescope is at its home position
#define TCSERR_NOMCFILE		27	// unable to locate mount correction file
#define TCSERR_BADMCFILE	28	// bad mount correction file - missing terms

#define TCSERR_COMNA		30	// the COM channel is not available
#define TCSERR_COMIO		31	// serial i/o error
#define TCSERR_COMTIMEOUT	32	// serial channel timed out
#define TCSERR_COMNORESP	33	// serial channel no response
#define TCSERR_COMBADRESP	34	// serial channel bad response
#define	TCSERR_COMNOTINIT	35	// serial device not initialized
#define TCSERR_UTSNOTIME	36	// UTS-10 date/time not available
#define TCSERR_UTSFAULT		37	// UTS-10 hardware fault
#define TCSERR_CTSNOTRDY	38	// CTS-10 not ready
#define TCSERR_REQNOTACK	39	// request not acknowledged
#define TCSERR_BADCOMMAND	40	// command not recognized
#define TCSERR_INPUTMISSING	41	// input parameter missing

#define TCSERR_OUTOFMEMORY	50	// out of memory

#define TCSERR_DBASENOMATCH	60	// database no match found
#define TCSERR_DBASEBADNUM	61	// invalid database catalog number
#define TCSERR_DBASEIOERR	62	// database file i/o error

#define TCSERR_OUTOFRANGE	999	// parameter out of range

#define GCSERR_OK               0       // good status
#define GCSERR_COMIO            101       // serial i/o error
#define GCSERR_NOFILE          102
#define GCSERR_OUTOFIMAGE      103
#define GCSERR_BADCENTROID     104
#define GCSERR_BADTOT         114

#define GCSERR_FILTER_NOTINIT 105
#define GCSERR_FILTER_TIMEOUT 106
#define GCSERR_FILTER_WRONG 107

#define GCSERR_GUIDER_NOTINIT 108
#define GCSERR_GUIDER_OUTOFRANGE 109

#define GCSERR_TERTIARY_NOTINIT 110
#define GCSERR_BRAKE_TIMEOUT 111

void error_code(int status);
#endif
/********************************* EOF **************************************/

