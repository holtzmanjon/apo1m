#include <stdio.h>
#include "error.h"
#include "io.h"

void error_code(int status)
{

  if (status == GCSERR_COMIO)
    sprintf(outbuf,"communications error");
  else if (status == GCSERR_NOFILE)
    sprintf(outbuf,"cant open file");
  else if (status == GCSERR_OUTOFIMAGE)
    sprintf(outbuf,"desired region of CCD is out of image!");
  else if (status == GCSERR_BADCENTROID)
    sprintf(outbuf,"bad centroid");
  else if (status == GCSERR_BADTOT)
    sprintf(outbuf,"total below threshold fraction");
  else if (status == GCSERR_FILTER_NOTINIT)
    sprintf(outbuf,"filter wheel not initialized");
  else if (status == GCSERR_FILTER_TIMEOUT)
    sprintf(outbuf,"filter wheel timed out");
  else if (status == GCSERR_FILTER_WRONG)
    sprintf(outbuf,"filter wheel appears to have gone to wrong position");
  else if (status == GCSERR_GUIDER_NOTINIT)
    sprintf(outbuf,"guider not initialized");
  else if (status == GCSERR_GUIDER_OUTOFRANGE)
    sprintf(outbuf,"guider stage commanded out of range");

  // Output the message to the screen
  if (status != GCSERR_OK) writeline(outbuf,1);

  // For remote operation, notify that the command is completed, and 
  //     return status
  if (remote_command) {
      sprintf(outbuf,"DONE %6d",status);
      writeline(outbuf,10);
  }
 
} 


