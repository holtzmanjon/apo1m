/****************************************************************************/
/*                                                        */
/*  Module:    spec.cpp                               */
/*                                                        */
/*  Purpose:  command program loop for SpectrSource                          */
/*                                                        */
/****************************************************************************/

/* Define BIGCCD for 512x524 camera, else assume 192x165 
   These have different include files, and different default base addresses */

#define BIGCCD

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <mem.h>
#include <alloc.h>
#include <float.h>
#include <math.h>
#include <dos.h>
#include "error.h"
#include "io.h"
#include "systimer.h"
#include "mytype.h"

#ifdef BIGCCD
#include "camera2.h"
#define BASE 0x120
#else
#include "camera.h"
#define BASE 0x320
#endif

#define NHEADER 2880
#ifdef no_hardware
#define TEMPDIR "e:"
#else
#define TEMPDIR "e:"
#endif

void inheadset(char *, int, char *);
void fheadset(char *, double, char *);
void lheadset(char *, int, char *);
void cheadset(char *, char *, char *);
void packfit(unsigned int *, unsigned int *, int);
void command_help();

int ccd_expose(WORD **, unsigned int, BOOL, int, int, int, int, int);
int ccd_write(WORD **, unsigned int,int, int, int, int);
int findcent(WORD **,float *,float *,float *, int);
int guide(WORD **buf,double x0,double y0, int size,unsigned int exptime,int update,
      double ax, double bx, double ay, double by);
int nintf(float);
extern float median(unsigned long , float *);
FILE *guide_file;
FILE *debug_file;
int outfile;

BOOL guiding = FALSE;
BOOL writeguide = FALSE;
BOOL offguiding = FALSE;
BOOL remote_on = TRUE;
BOOL remote_command = FALSE;
CameraType pCamera;
int nrow, ncol;
unsigned nbytes;
WORD **buf;
double x0, y0, ax, bx, ay, by;
int size, update, nupdate = 0;
unsigned int exptime;	

main()
{
  BOOL command_mode = TRUE;
  char command[MAXCMD], inputline[MAXCMD], inputline2[MAXCMD];
  int iline, status, n;
  struct time t;

  guide_file = fopen("e:\\spec\\guide.log","a");
  debug_file = fopen("debug.log","w");

#ifndef no_hardware
  // start the system timers
//  init_system_timers();

// Initialize Spectrasource
  CameraInitialize(BASE);

// Turn on cooler
  CameraCooler(TRUE);

// Clear camera
  CameraClearCCD();

// Check for hardware
  if (CameraDetect() == TRUE) {
    writeline("Camera hardware detected!!",0);
  }
  else {
    writeline("No camera hardware detected!!",0);
  }

// Get camera information
  CameraGetEquipment(&pCamera);
  sprintf(outbuf,"cooler equipped: %d\n"
         "antibloom equipped: %d\n"
         "MPP equipped: %d\n"
         "binning supported: %d\n"
         "reverse parallel supported: %d\n\n"
         "CCD read time (ms): %d\n"
         "bits per color: %d\n"
         "number of colors: %d\n"
         "x size: %d\n"
         "y size: %d\n"
         "high-res bits per pixel: %d\n"
         "low-res bits per pixel: %d\n",
         pCamera.bCooler, 
         pCamera.bAntibloom, 
         pCamera.bMpp, 
         pCamera.bBinning, 
         pCamera.bReverseParallel, 
         pCamera.CCDReadTime, 
         pCamera.BitsPerColor, 
         pCamera.NumberOfColors, 
         pCamera.XSize, 
         pCamera.YSize, 
         pCamera.HiResBPP, 
         pCamera.LoResBPP); 
  writeline(outbuf,0);
        
// Get status
  if (CameraGetStatus() & STAT_COOLER)
    writeline("Cooler on\n",0);
  else
    writeline("Cooler off\n",0);  
  if (CameraGetStatus() & STAT_SHUTTER)
    writeline("Shutter open\n",0);
  else
    writeline("Shutter closed\n",0);  
  if (CameraGetStatus() & STAT_ANTIBLOOM)
    writeline("Antibloom enabled\n",0);
  else
    writeline("Antibloom disabled\n",0);  
  if (CameraGetStatus() & STAT_FRAMETRANSFER)
    writeline("Frame transfer enabled\n",0);
  else
    writeline("Frame transfer disabled\n",0);  

  CameraAntibloom(FALSE);
  if (CameraGetStatus() & STAT_ANTIBLOOM)
    writeline("Antibloom enabled\n",0);
  else
    writeline("Antibloom disabled\n",0);  
#else
#ifdef BIGCCD
  pCamera.XSize = 524; 
  pCamera.YSize = 512; 
#else
  pCamera.XSize = 192; 
  pCamera.YSize = 165; 
#endif
  pCamera.HiResBPP = 12; 
  pCamera.LoResBPP = 0;
#endif 
 
  ncol = (int)pCamera.XSize;
  nrow = (int)pCamera.YSize;
  nbytes = ncol*2;

  // Allocate buffer arrays
  buf = (WORD **)malloc(nrow*sizeof(WORD *));
  if (buf ==NULL) fprintf(stderr,"error allocating buf\n");
  for (iline=0;iline<nrow;iline++) {
    *(buf+iline) = NULL;
  }

  while (command_mode) {

    status = SPECERR_OK;

    writeline("Command: ",0);
//  Command-mode commands: these are not acted upon until a CR is received
    if ( getline(inputline2,sizeof(inputline2)) != 0) {
      status = SPECERR_COMIO;
    } else
    {

//  Parse out the first word in case more junk is on the input line
    command[0] = '\0';
    sscanf(inputline2,"%s",command);
      if (strlen(command) > 1) {

//      Convert to upper case
        strupr(command);

        // Help list
        if (strcmp(command,"HP")==0) {
              command_help();
        }
 
        else if (strcmp(command,"RT")==0) {
              remote_on = !remote_on;
              sprintf(outbuf,"remote_on: %d",remote_on);
              writeline(outbuf,0);
        }
 
        // Exposure
        else if (strcmp(command,"EXP")==0) {
              if ((n=sscanf(inputline2,"EXP %u",&exptime)) < 1) {
                writeline("Enter exposure time (ms): ",0);
                if (getline(inputline,sizeof(inputline)) ==0) {
                  sscanf(inputline,"%u",&exptime);
                } else
                status = SPECERR_COMIO;
              }
              if (status == SPECERR_OK) {
                gettime(&t);
                sprintf(outbuf,"%d:%d:%d.%d   calling write\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
                status = ccd_write(buf,exptime,ncol,nrow,1,1);
                gettime(&t);
                sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    calling expose\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
                status = ccd_expose(buf,exptime,FALSE,0,ncol-1,0,nrow-1,1);
                gettime(&t);
                sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    done\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
                writeline(outbuf,1);
              }
        }

        // Dark Exposure
        else if (strcmp(command,"DARK")==0) {
              if ((n=sscanf(inputline2,"DARK %u",&exptime)) < 1) {
                writeline("Enter exposure time (ms): ",0);
                if (getline(inputline,sizeof(inputline)) ==0) {
                  sscanf(inputline,"%u",&exptime);
                } else
                status = SPECERR_COMIO;
              }
              if (status == SPECERR_OK) {
                gettime(&t);
                sprintf(outbuf,"%d:%d:%d.%d    calling write\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
                status = ccd_write(buf,exptime,ncol,nrow,1,1);
                gettime(&t);
                sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    calling dark expose\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
                status = ccd_expose(buf,exptime,TRUE,0,ncol-1,0,nrow-1,1);
                gettime(&t);
                sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    done\n",
                        t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
                writeline(outbuf,1);
              }
        }

        // Start guiding at specified position
        else if (strcmp(command,"OLDGUIDE")==0) {
             sscanf(inputline2,"OLDGUIDE %lf %lf %d %u %d %lf %lf %lf %lf",
                    &x0,&y0,&size,&exptime,&update,&ax,&bx,&ay,&by);
             nupdate = 0;
             guiding = TRUE;
             offguiding = FALSE;
        }

        else if (strcmp(command,"NEWGUIDE")==0) {
             sscanf(inputline2,"NEWGUIDE %lf %lf %d %u %d",
                    &x0,&y0,&size,&exptime,&update);
             nupdate = 0;
             offguiding = TRUE;
             guiding = TRUE;
        }

        // Write guide images
        else if (strcmp(command,"WRITE")==0) {
             writeguide = !writeguide;
        }

        // Stop guiding
        else if (strcmp(command,"GUIDEOFF")==0) {
             guiding = FALSE;
        }

	else if (strcmp(command,"UPDATE")==0) {
             sscanf(inputline2,"UPDATE %d",&update);
             nupdate = 0;
        }

	else if (strcmp(command,"SIZE")==0) {
             sscanf(inputline2,"SIZE %d",&size);
        }

        // Set default exposure time
	else if (strcmp(command,"EXPOSURE")==0) {
              double dexptime;
              sscanf(inputline2,"EXPOSURE %lf",&dexptime);
              exptime = (unsigned int)dexptime*1000;
        }

        // Dummy commands to keep things compatible with PI commands
        else if (strcmp(command,"FILTER")==0 ||
                 strcmp(command,"NUMSEQ")==0 ||
                 strcmp(command,"SETINCVAL")==0 ||
                 strcmp(command,"SHUTTERCMD")==0 ||
                 strcmp(command,"NAME")==0 ||
                 strcmp(command,"CLEANS")==0) {}

        else if (strcmp(command,"TECON")==0) {
             CameraCooler(TRUE);
        }

        else if (strcmp(command,"TECOFF")==0) {
             CameraCooler(FALSE);
        }

        // quit command mode
        else if (strcmp(command,"QU")==0) {
              command_mode = !command_mode;
              sprintf(outbuf,"Leaving command mode and exiting program");
              writeline(outbuf,0);
        }
  
        else if (strlen(command)>0){
           writeline("Unknown command!!\r\n",0);
        }
        
      } // end if strlen(command > 1)

    } // end if (getline != TCSERR_OK)

    // Now process the returned status. This will also take care of
    //   sending a verification of returned status for remote operation
    error_code(status);
               
  }  // end while (command_mode)

  system("copy debug.log e:\\spec\\debug.log");

#ifndef no_hardware
  CameraCooler(FALSE);
#endif
  return(0);
}

void command_help()
{
   char ans;

   sprintf(outbuf,"General commands: \r\n"
                  " HP: print help menu\r\n"
                  " QU: quit command mode\r\n");
   writeline(outbuf,0);
}


// Function to expose SpectraSource and read image into buffer
int ccd_expose(WORD **buf, unsigned int exptime, BOOL dark, 
               int xs, int xe, int ys, int ye, int writeout)
{
  int i, iline, status, nread, offset, nalloc;
  long nfree, nbyte;
  WORD *line;
  static int xs0=0;
  static int xe0=0;
  static int ys0=0;
  static int ye0=0;

// If we're writing out the image, just allocate a single line
// If not, check to see if we have enough memory
  if (writeout==1) {
    if (*buf != NULL) free(*buf);
    *buf = (WORD *)malloc(ncol*sizeof(WORD));
  } else {
    nbyte =(long)(xe-xs+1)*(ye-ys+1)*sizeof(WORD);
    nfree = coreleft();
    if (nbyte > nfree) {
      fprintf(stderr,"%d %d %d %d %d\n",xs,xe,ys,ye,sizeof(WORD));
      fprintf(stderr,"%ld %ld\n",nbyte, nfree);
      return(-1);
    }
  }

  status = 0;
// Clear CCD, open shutter, and integrate
#ifndef no_hardware
  CameraClearCCD();
  if (!dark)
    CameraOpenShutter();
  CameraBeginIntegrate();
#endif
  if (exptime > 30000) {
    sleep((unsigned)(exptime/1000.));
  }
  else
    delay(exptime);
//  CameraExpose(exptime,0,0);
#ifndef no_hardware
  CameraEndIntegrate();
  if (!dark)
    CameraCloseShutter();
#endif

// Read out CCD
  for (iline=0; iline<=ye; iline++) {
//  Read out leading lines only 1 pixel per line. Otherwise read desired
//    subsection. Allocate either 1 or ncol pixels per line.
    if (iline < ys) {
      nread = 1;
      offset = 0;
      nalloc = 1;
    }
    else {
      nread = xe-xs+1;
      offset = xs;
      nalloc = ncol;
    }
    if (writeout==1)
      line = buf[0];
    else {
//    Allocate memory if we need to
      if (xs != xs0 || xe !=xe0 || ys != ys0 || ye != ye0) {
        fprintf(stderr," freeing: %d %lu\n",iline,*(buf+iline));
        if (*(buf+iline) != NULL) free(*(buf+iline));
        fprintf(stderr," allocating: %d %d\n",iline,nalloc);
        *(buf+iline) = (WORD *)malloc(nalloc);
        if (*(buf+iline) ==NULL) {
          fprintf(stderr,"error allocating  %d\n",iline);
          return(-1);
        }
        fprintf(stderr," allocated: %lu\n",*(buf+iline));
      }
      line = buf[iline]+offset;
    }
#ifdef no_hardware
    int xc = (xe+xs)/2;
    int yc = (ye+ys)/2;
    int data;
    double xdist, ydist, dist;
    double sig=2.;
    ydist = (iline-yc);
    ydist *= (iline-yc);
    for (i=xs; i<=xe; i++) {
      xdist = (i-xc);
      xdist *= (i-xc);
      dist = xdist + ydist;
      data = (int) 1000. * exp(-(dist)/2./sig/sig) / 2. / 3.14159 / sqrt(sig);
      *line++ = data;
    }
#else
    CameraDigitizeLine(line, nread, offset, TRUE, 1, 1);
#endif
    if (writeout==1) write(outfile,buf[0],ncol*sizeof(WORD)); 
    if (writeout==2 && nread > 1) {
      line = buf[iline]+offset;
      packfit(line,line,nread*sizeof(WORD)); 
      write(outfile,buf[iline]+offset,nread*sizeof(WORD)); 
      packfit(line,line,nread*sizeof(WORD)); 
    }
  }
  xs0 = xs;
  xe0 = xe;
  ys0 = ys;
  ye0 = ye;

  if (writeout>0) close(outfile);
  return(status);
}

ccd_write(WORD **buf, unsigned int exptime, int nc, int nr, int sc, int sr)
{
  double dexptime;
  int status, iline, i;
  char header[NHEADER], card[80], cardname[9];
  WORD *line;
  struct time timeobs;
  static long iseq = 0;

  // Now open the FITS file and write header
  if (guiding && writeguide)  {
    sprintf(card,TEMPDIR"\\spec\\t%07d.spe",iseq++);
    outfile = open(card,O_RDWR|O_BINARY|O_CREAT,S_IWRITE|S_IREAD);
  }
  else
    outfile = open(TEMPDIR"\\spec\\temp.spe",
               O_RDWR|O_BINARY|O_CREAT,S_IWRITE|S_IREAD);
  if (outfile <= 0) {
    sprintf(outbuf,"error opening output FITS file\n%s\n",
       TEMPDIR"\\spec\\temp.spe");
    writeline(outbuf,0);
    perror("error:");
    status = SPECERR_NOFILE;
  } else {
    // write out the image header
    memset(header,(int)' ',NHEADER);
    sprintf(header,"END ");
    header[4] = ' ';
    header[2779] = '\0';
    lheadset("SIMPLE",TRUE,header);
    inheadset("BITPIX",16,header);
    inheadset("NAXIS",2,header);
    inheadset("NAXIS1",nc,header);
    inheadset("NAXIS2",nr,header);
    inheadset("CRVAL1",sc,header);
    inheadset("CRVAL2",sr,header);
    inheadset("CDELT1",1,header);
    inheadset("CDELT2",1,header);
    dexptime = (double)exptime / 1000.;
// This next line crashes code sometimes with floating-point emulation, as
//  far as I can tell, from a Borland bug in the emulation library
//  fheadset("EXPOSURE",dexptime,header);
    gettime(&timeobs);
    sprintf(card,"%2d:%2d:%2d",
            timeobs.ti_hour,timeobs.ti_min,timeobs.ti_sec);
    cheadset("TIME",card,header);
    write(outfile,header,NHEADER);

#ifdef NOTDEF
    for (iline=0; iline<nrow; iline++) {
      // byte swap the data on the UNIX host
      // line = buf[iline];
      // packfit(line,line,nbytes); 

      // write out the image data line by line
      write(outfile,buf[iline],ncol*sizeof(WORD)); 
    }
    close(ifile);
#endif
    status = SPECERR_OK;
  }
  return(status);
}

// Function guide takes care of taking guide exposures, reading partial
//   frames, centroiding image on frame, computing offsets, and sending
//   position errors to telescope computer
int guide(WORD **buf,double x0,double y0, int size, unsigned int exptime, 
      int update, double ax, double bx, double ay, double by)
{
  int xs, xe, ys, ye, writeout;
  float row, col, peak;
  double dx, dy, dra, ddec;
  static double totdra, totddec, totdx, totdy;
  int status;

// Take the guide exposure, only digitizing relevant region of frame
// If desired region is out of frame, return with error
  xs = x0-size/2-5;
  xe = x0+size/2+5;
  ys = y0-size/2-5;
  ye = y0+size/2+5;
  if (xs<0 || ys<0 || xe>ncol || ye>nrow) return(SPECERR_OUTOFIMAGE);

  sprintf(outbuf,"\n xs: %d  xe: %d  ys: %d  ye: %d exptime: %u\n",
          xs,xe,ys,ye,exptime);
//  writeline(outbuf,1);
  writeout = 0;

//  Uncomment the following to write out individual guide frames
  if (writeguide) {
    ccd_write(buf,exptime,xe-xs+1,ye-ys+1,xs,ys);
    writeout=2;
  }

  status = ccd_expose(buf, exptime, FALSE, xs, xe, ys, ye, writeout);
  if (status<0) return(status);

// Find centroid using x0,y0 as starting guess. 
// If centroid fails, return with error
  row = y0;
  col = x0;
  if (findcent(buf, &row, &col, &peak, size)!=0) return(SPECERR_BADCENTROID);

  sprintf(outbuf+strlen(outbuf),"new x: %f  new y: %f  x0: %f  y0: %f  peak: %f\n",col,row,x0,y0,peak);
  writeline(outbuf,1);

// Now compute offsets, using transformation matrix to convert to ra/dec
  dy = row - y0;
  dx = col - x0;
  dra = ax*dx + bx*dy;
  ddec = ay*dx + by*dy;

// Accumulate offsets for averaging
  totdra += dra;
  totddec += ddec;
  totdx += dx;
  totdy += dy;
  nupdate++;

// sprintf(outbuf,"ax: %f bx: %f  ay: %f by:%f\n",ax,bx,ay,by);
// sprintf(outbuf+strlen(outbuf),"dx: %f dy: %f  dra: %f  ddec: %f\n",dx,dy,dra,ddec);
//sprintf(outbuf+strlen(outbuf),"%f %f %f %f %f %f %d",dx, dy, dra, ddec, 
//        totdra/nupdate, totddec/nupdate, nupdate);
//writeline(outbuf,1);

sprintf(outbuf,"%f %f %f %f %f %f %d",dx, dy, dra, ddec, 
        totdra/nupdate, totddec/nupdate, nupdate);
writeline(outbuf,2);

// Send errors to telescope computer
  if (nupdate == update) {
    if (offguiding) 
      sprintf(outbuf,"GUIDEINST 2 %f %f",totdx/nupdate,totdy/nupdate);
    else
      sprintf(outbuf,"GUIDE %f %f",totdra/nupdate,totddec/nupdate);
    writeline(outbuf,6);
    writeline(outbuf,1);
  }

  if (nupdate == update || update==0) {
    nupdate = 0;
    totdra = 0.;
    totddec = 0.;
    totdx = 0.;
    totdy = 0.;
  }

  return(SPECERR_OK);
}

int findcent(WORD **data,float *row,float *col, float *peak, int size)
{
#define MAXBOX 51
   int iter, imin, imax, jmin, jmax, i, j, icol, irow;
   int niter = 1;
   unsigned long n;
   float temp[MAXBOX*MAXBOX+1];
   double sum, sumx, sumy, dx, dy;
   float thresh;
   WORD px, *add, *add2;
   double val;
   double eps=1.e-3;

   icol = nintf(*col);
   irow = nintf(*row);
   for (iter=0;iter<niter;iter++) {
      imin = icol-size/2;
      imax = icol+size/2;
      jmin = irow-size/2;
      jmax = irow+size/2;

/* Compute a background value using a median of the box edge values */
      n=1;
      for (j=jmin;j<=jmax;j++) {
        add = *(data+j);
        temp[n++] = *(add+imin);
        temp[n++] = *(add+imax);
      }
      add = *(data+jmin);
      add2 = *(data+jmax);
      for (i=imin+1;i<=imax-1;i++) {
        temp[n++] = *(add+i);
        temp[n++] = *(add2+i);
      }
      thresh = median(n,temp);

      sum = sumx = sumy = 0; 
      *peak = 0;
      outbuf2[0] = 0;
 sprintf(outbuf2+strlen(outbuf2),"%d %d %d %d %d\n",jmin, jmax, imin, imax, size);
sprintf(outbuf2+strlen(outbuf2),"thresh: %f\n",thresh);
      for (j=jmin;j<=jmax;j++) {
        add = *(data+j);
        for (i=imin;i<=imax;i++) {
/* Note that passed row,col are for (0,0) origin */
           px = *(add+i);
           val = (double)px - thresh;
 sprintf(outbuf2+strlen(outbuf2),"%3.3u ",px);
           *peak = (val>*peak ? (float)val : *peak);
           sum += val;
           sumx += val*i;
           sumy += val*j;
        }
sprintf(outbuf2+strlen(outbuf2),"\n");
      }
sprintf(outbuf2+strlen(outbuf2),"sums: %lf %lf %lf",sumx, sumy, sum);
      if (sum != 0) {
        sumx /= sum;
        sumy /= sum;
      } else {
        sumx = *col;
        sumy = *row;
      }
      if (sumx<=imin || sumx>=imax || sumy<=jmin || sumy>=jmax) return(-1);

      dx = sumx - *col;
      dy = sumy - *row;
sprintf(outbuf2+strlen(outbuf2)," dx, dy: %lf %lf",dx, dy);
// writeline(outbuf2,1);
      *col = sumx;
      *row = sumy;
      if ((fabs(dx) < eps && fabs(dy) < eps) || niter==1)
        return(0);

      icol = nintf(*col);
      irow = nintf(*row);
   }
   return(-1);
}

int nintf(float val)
{
  if (fmod(val,1.) < 0.5)
    return((int)floor(val));
  else
    return((int)ceil(val));
}

void update_all()
{
  int status;

#ifdef NEWCOM
  int ifile;
  // Does RESTART exist?  If it does, then tcomm has been restarted, and
  //  we need to reopen files
  ifile = open(restart,O_RDONLY);
  if (ifile >= 0){
writeline("restart is present!",1);
    close(ifile);
    if (rfile != NULL) fclose(rfile);
    if (sfile != NULL) fclose(sfile);
    if (cfile != NULL) fclose(cfile);
    ropen = FALSE;
    statopen = FALSE;
    copen = FALSE;
    remove(restart);
  }
#endif

  if (guiding) {
    status=guide(buf,x0,y0,size,exptime,update,ax,bx,ay,by);
    if (status != SPECERR_OK) error_code(status);
  } else {
   EventTimer PauseTimer;
   PauseTimer.NewTimer(1);
   while (!PauseTimer.Expired()) {}
  }
}
