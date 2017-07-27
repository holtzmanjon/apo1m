#include <stdio.h>
#include "status.h"
#include "io.h"

void error_code(int status)
{
	int n;

  if (status == TCSERR_OK) {
    if (outfile[idepth] != NULL) {
       n=fprintf(outfile[idepth],"STATUS: %d\n",status);
       fflush(outfile[idepth]);
    }
    return;
  } 
  else if (status == UNKNOWN_ERROR)
    sprintf(outbuf,"unspecified error - command not executed!");
  else if (status == TCSERR_NA)
    sprintf(outbuf,"equipment not available or disabled");
  else if (status == TCSERR_DOMEHOME)
    sprintf(outbuf,"dome could not find home in time allotted");
  else if (status == TCSERR_SHUTTERPOS)
    sprintf(outbuf,"unable to verify dome shutter position");
  else if (status == TCSERR_SCFREAD)
    sprintf(outbuf,"unable to read .SCF file");
  else if (status == TCSERR_TELENOTINIT)
    sprintf(outbuf,"the telescope is not initialized");
  else if (status == TCSERR_NETWORKDOWN)
    sprintf(outbuf,"the network link is down");
  else if (status == TCSERR_DOMEMOVE)
    sprintf(outbuf,"dome could not move in time allotted");
  else if (status == TCSERR_DOMENOTINIT)
    sprintf(outbuf,"the dome has not been initialized");
  else if (status == TCSERR_CANTINITTELE)
    sprintf(outbuf,"cannot initialize the telescope");
  else if (status == TCSERR_SECONDARY)
    sprintf(outbuf,"tuv tilt or range exceeded");
  else if (status == TCSERR_MOVELIMIT)
    sprintf(outbuf,"move would run telescope into the limits");
  else if (status == TCSERR_MOVEWINDOW)
    sprintf(outbuf,"move would run telescope beyond window");
  else if (status == TCSERR_NOFILTER)
    sprintf(outbuf,"the specified filter is not available");
  else if (status == TCSERR_NOACQUIRE)
    sprintf(outbuf,"unable to acquire object");
  else if (status == TCSERR_CCDNOCAL)
    sprintf(outbuf,"the CCD camera is not calibrated");
  else if (status == TCSERR_WEATHERSHUT)
    sprintf(outbuf,"the observatory shut down due to weather");
  else if (status == TCSERR_STATIONSHUT)
    sprintf(outbuf,"the observatory shut down due to bad sensors");
  else if (status == TCSERR_NOAUX)
    sprintf(outbuf,"the auxiliary channel is not available");
  else if (status == TCSERR_MCREADSCF)
    sprintf(outbuf,"the MC cannot read the .SCF files");
  else if (status == TCSERR_MCWRITESCF)
    sprintf(outbuf,"the MC cannot write the .SCF files");
  else if (status == TCSERR_TOCCWRITESCF)
    sprintf(outbuf,"the TOCC cannot write the .SCF files");
  else if (status == TCSERR_PHTNOPULSE)
    sprintf(outbuf,"the SSP-3/SSP-5 photometer is not counting");
  else if (status == TCSERR_USERABORT)
    sprintf(outbuf,"a user abort was received");
  else if (status == TCSERR_WDOGTOUT)
    sprintf(outbuf,"the watchdog timer timed out");
  else if (status == TCSERR_MOVETIMEOUT)
    sprintf(outbuf,"the move took too long");
  else if (status == TCSERR_TELEATHOME)
    sprintf(outbuf,"the telescope is at its home position");
  else if (status == TCSERR_NOMCFILE)
    sprintf(outbuf,"unable to locate mount correction file");
  else if (status == TCSERR_BADMCFILE)
    sprintf(outbuf,"bad mount correction file - missing terms");
  else if (status == TCSERR_COMNA)
    sprintf(outbuf,"the COM channel is not available");
  else if (status == TCSERR_COMIO)
    sprintf(outbuf,"serial i/o error");
  else if (status == TCSERR_COMTIMEOUT)
    sprintf(outbuf,"serial channel timed out");
  else if (status == TCSERR_COMNORESP)
    sprintf(outbuf,"serial channel no response");
  else if (status == TCSERR_COMBADRESP)
    sprintf(outbuf,"serial channel bad response");
  else if (status == TCSERR_COMNOTINIT)
    sprintf(outbuf,"serial device not initialized");
  else if (status == TCSERR_UTSNOTIME)
    sprintf(outbuf,"UTS-10 date/time not available");
  else if (status == TCSERR_UTSFAULT)
    sprintf(outbuf,"UTS-10 hardware fault");
  else if (status == TCSERR_CTSNOTRDY)
    sprintf(outbuf,"CTS-10 not ready");
  else if (status == TCSERR_REQNOTACK)
    sprintf(outbuf,"request not acknowledged");
  else if (status == TCSERR_BADCOMMAND)
    sprintf(outbuf,"command not recognized");
  else if (status == TCSERR_INPUTMISSING)
    sprintf(outbuf,"input parameter missing");
  else if (status == TCSERR_OUTOFMEMORY)
    sprintf(outbuf,"out of memory");
  else if (status == TCSERR_DBASENOMATCH)
    sprintf(outbuf,"database no match found");
  else if (status == TCSERR_DBASEBADNUM)
    sprintf(outbuf,"invalid database catalog number");
  else if (status == TCSERR_DBASEIOERR)
    sprintf(outbuf,"database file i/o error");
  else if (status == TCSERR_OUTOFRANGE)
    sprintf(outbuf,"parameter out of range");
  else if (status == GCSERR_COMIO)
    sprintf(outbuf,"gcs: communications error");
  else if (status == GCSERR_NOFILE)
    sprintf(outbuf,"gcs: cant open file");
  else if (status == GCSERR_OUTOFIMAGE)
    sprintf(outbuf,"gcs: desired region of CCD is out of image!");
  else if (status == GCSERR_BADCENTROID)
    sprintf(outbuf,"gcs: bad centroid");
  else if (status == GCSERR_FILTER_NOTINIT)
    sprintf(outbuf,"gcs: filter wheel not initialized");
  else if (status == GCSERR_FILTER_TIMEOUT)
    sprintf(outbuf,"gcs: filter wheel timed out");
  else if (status == GCSERR_FILTER_WRONG)
    sprintf(outbuf,"gcs: filter wheel appears to have gone to wrong position");
  else if (status == GCSERR_GUIDER_NOTINIT)
    sprintf(outbuf,"gcs: guider not initialized");
  else if (status == GCSERR_GUIDER_OUTOFRANGE)
    sprintf(outbuf,"gcs: guider stage commanded out of range");
  else if (status == CCDERR_RESTART)
    sprintf(outbuf,"ccd: CCD commanded to restart");
  else 
    sprintf(outbuf,"command returns %d",status);
 
  writeline(outbuf,0);
  if (outfile[idepth] != NULL) {
//    fprintf(tomasterfp,"%s\n",outbuf);
    fprintf(outfile[idepth],"STATUS: %d\n",status);
    fflush(outfile[idepth]);
  }
/*
  if (havepipe[idepth]) {
//    fprintf(tomasterfp,"%s\n",outbuf);
    fprintf(tomasterfp,"STATUS: %d\n",status);
    fflush(tomasterfp);
  }
*/

} 
