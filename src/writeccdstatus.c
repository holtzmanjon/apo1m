/* program to accept commands from command program over fifo and send them
    to PI camera, get responses and send them back */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "mytype.h"
#include "slamac.h"

#define MAXCOMMAND 80

struct CCDSTATUS ccdinfo;

#ifdef CURSES
extern void update_ccd_display(struct CCDSTATUS *, int, char *);

void writeccdstatus(struct CCDSTATUS ccdinfo,char *cstatusfile,char *cstatusreadyfile)
{
  update_ccd_display(&ccdinfo,-9,"DIS slitviewer: Apogee CCD");
}
#else
/* function to write out CCD status information to a file */
void writeccdstatus(struct CCDSTATUS ccdinfo,char *cstatusfile,char *cstatusreadyfile)
{
      char command[80];
      FILE *cstatus;

      /* Delete the status ready file */
      remove(cstatusreadyfile);

      /* Open the status file */
      cstatus = fopen(cstatusfile,"w");
      if (cstatus==NULL) {
        fprintf(stderr,"error opening file: %s %d\n",cstatusfile,strlen(cstatusfile));
        perror("fopening cstatusfile:");
        return;
      }

      /* Write out the status information to the status file */
      fprintf(cstatus,
          "%d %f %d %f %s %d %d %d %d %d %d %d %f %d %d %d %d %f %f %f %f %f %d %d %f %f %f %f %f %f %f %f %f %f %d %s\n",
          ccdinfo.cleans, ccdinfo.exposure, ccdinfo.expstatus, ccdinfo.end_time,
          ccdinfo.filename, ccdinfo.filetype, ccdinfo.autodisplay,
          ccdinfo.autoxfer, ccdinfo.autodark, ccdinfo.autoflat,
          ccdinfo.filter, ccdinfo.filtfoc, ccdinfo.focus,
          ccdinfo.numseq, ccdinfo.incval, ccdinfo.shutter, 
          ccdinfo.guiding, ccdinfo.guide_x0, ccdinfo.guide_y0,
          ccdinfo.guide_pa, ccdinfo.guide_rad, ccdinfo.guide_mag,
          ccdinfo.guide_size, ccdinfo.guide_update,
          ccdinfo.ax, ccdinfo.bx, ccdinfo.ay, ccdinfo.by, 
          ccdinfo.sx, ccdinfo.sy, ccdinfo.theta, ccdinfo.cx, ccdinfo.cy,
	  ccdinfo.ccd_temp, ccdinfo.ccd_temp_status,
          ccdinfo.object);
      fprintf(cstatus,"%s\n", ccdinfo.object);

      /* Close the status file */
      fclose(cstatus);

      /* Open the status ready file to communicate that status file is ready */
      cstatus = fopen(cstatusreadyfile,"w");
      fclose(cstatus); 

      return;
}

#endif
