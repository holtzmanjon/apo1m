#include <stdio.h>
#include <fcntl.h>
#include <alloc.h>
#include <math.h>
#include <dos.h>
#include <io.h>
#include <sys/stat.h>
#include <string.h>
#include "ccd.h"
#include "error.h"
#include "io.h"
#include "mytype.h"

#define NEWMEM

extern BOOL writeguide;
extern BOOL guiding;
extern BOOL offguiding;
extern BOOL do_disable;
extern BOOL use_shutter;
extern BOOL lowres;
int outfile;

/* Define BIGCCD for 512x524 camera, else assume 192x165
   These have different include files, and different default base addresses */

#ifdef no_hardware
#define TEMPDIR "d:"
#else
#define TEMPDIR "e:"
#endif

int nintf(float val);
extern float median(unsigned long , float *);

void inheadset(char *, int, char *);
void fheadset(char *, double, char *);
void lheadset(char *, int, char *);
void cheadset(char *, char *, char *);
void packfit(unsigned int *, unsigned int *, int);

CameraType pCamera;
int nrow, ncol;
unsigned nbytes;
WORD **buf;
#ifdef NEWMEM
#define MAXSIZE 25
WORD *tmpbuf[MAXSIZE+1];
#endif
double x0, y0, ax, bx, ay, by;
int size, update, nupdate = 0;
unsigned int exptime;	
double badthresh;
long nguide;
int verbose = 0;
int domedian = 2;
long iseq = 0;

void ccd_initialize()
{
  int iline;

#ifdef no_hardware
  #ifdef BIGCCD
  pCamera.XSize = 524;
  pCamera.YSize = 512;
  #else
  pCamera.XSize = 192;
  pCamera.YSize = 165;
  #endif
  pCamera.HiResBPP = 12;
  pCamera.LoResBPP = 0;
#else
// Initialize Spectrasource
  sprintf(outbuf,"opening camera: %x\n",BASE);
  writeline(outbuf,1);
  CameraInitialize(BASE);

  writeline("turning on cooler\n",1);
// Turn on cooler
  CameraCooler(TRUE);

  writeline("clearing\n",1);
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
#endif

  ncol = (int)pCamera.XSize + 10;
  nrow = (int)pCamera.YSize;
  nbytes = ncol*2;
  // Allocate buffer arrays. buf is array which holds nrow different addresses
  // for each line. However, we can't allocate a full column of memory for
  // each line. Allocate MAXSIZE rows worth of columns in tmpbuf, and we'll
  // use these addresses as needed
  buf = (WORD **)malloc(nrow*sizeof(WORD *));
  if (buf ==NULL) {
    sprintf(outbuf,"error allocating buf\n");
    writeline(outbuf,0);
  }
  for (iline=0;iline<nrow;iline++) {
    *(buf+iline) = NULL;
  }
#ifdef NEWMEM
  for (iline=0;iline<=MAXSIZE;iline++) {
    *(tmpbuf+iline) = (WORD *)malloc(ncol*sizeof(WORD));
    if (*(tmpbuf+iline) ==NULL) {
      sprintf(outbuf,"error allocating tmpbuf, iline: %d\n",iline);
      writeline(outbuf,0);
    }
  }

#endif

}

// Function to expose SpectraSource and read image into buffer
int ccd_expose(WORD **buf, unsigned int exptime, BOOL use_shutter, 
               int xs, int xe, int ys, int ye, int writeout)
{
  int i, iline, status, nread, offset, nalloc, itmp;
  long nfree, nbyte;
  WORD *line;
  static int xs0=0;
  static int xe0=0;
  static int ys0=0;
  static int ye0=0;

#ifndef NEWMEM
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
#endif

  status = 0;
// Clear CCD, open shutter, and integrate
#ifndef no_hardware
  asm cli;
  CameraClearCCD();
  if (use_shutter)
    CameraOpenShutter();
  CameraBeginIntegrate();
  asm sti;
#endif
  fprintf(stderr,"exptime: %d\n",exptime);
  fprintf(stderr,"use_shutter: %d\n",use_shutter);
  if (exptime > 30000) {
    sleep((unsigned)(exptime/1000.));
  }
  else
    delay(exptime);
//  CameraExpose(exptime,0,0);
#ifndef no_hardware
  asm cli;
  CameraEndIntegrate();
  if (use_shutter && use_shutter>=0)
    CameraCloseShutter();
  asm sti;
#endif

// Read out CCD
  if (do_disable) disable();
  itmp = 1;
  for (iline=0; iline<=ye; iline++) {
#ifdef TEST
    line = buf[0];
    asm cli;
    CameraDigitizeLine(line, nread, offset, lowres, 1, 1);
    asm sti;
    write(outfile,buf[0],ncol*sizeof(WORD)); 
#else


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

#ifdef NEWMEM
    if (writeout==1 || nread == 1)
      *(buf+iline) = tmpbuf[0];
    else {
      *(buf+iline) = tmpbuf[itmp++];
/*
      sprintf(outbuf+strlen(outbuf),
        "buf[iline]: %ld offset: %d  line: %ld iline: %d\n",
        buf[iline],offset,buf[iline]+offset,iline);
*/
    }
    line = buf[iline]+offset;
#else
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
#endif

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
    asm cli;
    CameraDigitizeLine(line, nread, offset, lowres, 1, 1);
/*
    CameraCCDParallel(1);
    CameraSkipPixels(offset);
    for (i=xs; i<=xe; i++)
      *line++ = CameraDigitizePixel(1);
*/
    asm sti;
#endif
#ifdef NEWMEM
    if (writeout==1) write(outfile,buf[iline],ncol*sizeof(WORD)); 
#else
    if (writeout==1) write(outfile,buf[0],ncol*sizeof(WORD)); 
#endif
    if (writeout==2 && nread > 1) {
      line = buf[iline]+offset;
      packfit(line,line,nread*sizeof(WORD)); 
      write(outfile,buf[iline]+offset,nread*sizeof(WORD)); 
      packfit(line,line,nread*sizeof(WORD)); 
    }
#endif
  }
  if (do_disable) enable();
  xs0 = xs;
  xe0 = xe;
  ys0 = ys;
  ye0 = ye;

  if (writeout>0) close(outfile);
  return(status);
}

ccd_write(WORD **buf, unsigned int exptime, int nc, int nr, int sc, int sr, int ndark)
{
  double dexptime;
  int status, iline, i;
  char header[NHEADER], card[80], cardname[9];
  WORD *line;
  struct time timeobs;

  // Now open the FITS file and write header
  if (guiding && writeguide)  {
    sprintf(card,TEMPDIR"\\spec\\t%07ld.spe",iseq++);
  } else if (ndark<0) {
    sprintf(card,TEMPDIR"\\spec\\temp.spe");
  } else {
    sprintf(card,TEMPDIR"\\spec\\dark%1d.spe",ndark);
  }

  outfile = open(card,O_RDWR|O_BINARY|O_CREAT,S_IWRITE|S_IREAD);
  if (outfile <= 0) {
    sprintf(outbuf,"error opening output FITS file\n%s\n",
       TEMPDIR"\\spec\\temp.spe");
    writeline(outbuf,0);
    perror("error:");
    status = GCSERR_NOFILE;
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
    inheadset("ISEQ",iseq-1,header);
    dexptime = (double)exptime / 1000.;
// This next line crashes code sometimes with floating-point emulation, as
//  far as I can tell, from a Borland bug in the emulation library
//  fheadset("EXPOSURE",dexptime,header);
    gettime(&timeobs);
    sprintf(card,"%2.2d:%2.2d:%2.2d",
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
    status = GCSERR_OK;
  }
  return(status);
}

// Function guide takes care of taking guide exposures, reading partial
//   frames, centroiding image on frame, computing offsets, and sending
//   position errors to telescope computer
int guide(WORD **buf,double x0, double y0, int size, unsigned int exptime, 
      int update, double ax, double bx, double ay, double by,
      float *peak, float *tot)
{
  int xs, xe, ys, ye, writeout;
  float row, col;
  static float totsave[50];
  double dx, dy, dra, ddec, meantot=0;
  static double totdra, totddec, totdx, totdy, tottot=0;
  int status;

// Take the guide exposure, only digitizing relevant region of frame
// If desired region is out of frame, return with error
  xs = x0-size/2-2;
  xe = x0+size/2+2;
  ys = y0-size/2-2;
  ye = y0+size/2+2;
  if (xs<0 || ys<0 || xe>ncol || ye>nrow) return(GCSERR_OUTOFIMAGE);

  sprintf(outbuf,"\n Guider subimage: (%d:%d,%d:%d)  exptime: %u\n",
          xs,xe,ys,ye,exptime);
//  writeline(outbuf,1);
  writeout = 0;

//  Write out individual guide frames
  if (writeguide) {
    ccd_write(buf,exptime,xe-xs+1,ye-ys+1,xs,ys,-1);
    writeout=2;
  }

  status = ccd_expose(buf, exptime, use_shutter, xs, xe, ys, ye, writeout);
  if (status<0) return(status);

// Find centroid using x0,y0 as starting guess. 
// If centroid fails, return with error
  row = y0;
  col = x0;
/*  printarr(buf,&row,&col); */
  // Get centroid of image. If we can't get a good centroid, return here with
  //   no correction issued
  if (findcent(buf, &row, &col, peak, tot, size)!=0) {
      if (verbose) writeline(outbuf,1);
      return(GCSERR_BADCENTROID);
  }

  // Accumulate mean counts for last update exposures. Once mean has been
  //   determined, reject observations where total counts is less than
  //   some fraction (badthresh) of mean rate from last update exposures.
  if (nguide<update) {
    totsave[nguide] = *tot;
    if (nguide==0) tottot=0;
    tottot += *tot;
    nguide++;
  } else {
    meantot = tottot / update;
    if (*tot > badthresh*meantot) {
      tottot -= totsave[nguide%update];
      totsave[nguide%update] = *tot;
      tottot += totsave[nguide%update];
      nguide++;
    } else {
      if (verbose) writeline(outbuf,1);
      return(GCSERR_BADTOT);
    }
  }
  // Debugging output
  sprintf(outbuf+strlen(outbuf),
    "new position: (%f,%f) original position: (%f:%f)\n", col,row,x0,y0); 
  sprintf(outbuf+strlen(outbuf),
    "nguide: %ld total cnts: %f meantot: %lf peak: %f\n",
     nguide,*tot,meantot,*peak);
  if (verbose) writeline(outbuf,1);


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

//sprintf(outbuf,"%f %f %f %f %f %f %d",dx, dy, dra, ddec, 
//        totdra/nupdate, totddec/nupdate, nupdate);
// writeline(outbuf,2);

// Send errors to telescope computer
  if (nupdate == update) {
    if (offguiding) 
      sprintf(outbuf,"GUIDEINST 2 %f %f",totdx/nupdate,totdy/nupdate);
    else
      sprintf(outbuf,"GUIDE %f %f",totdra/nupdate,totddec/nupdate);
    writeline(outbuf,6);
    if (verbose) writeline(outbuf,1);
  }

  if (nupdate == update || update==0) {
    nupdate = 0;
    totdra = 0.;
    totddec = 0.;
    totdx = 0.;
    totdy = 0.;
  }

  return(GCSERR_OK);
}

int findcent(WORD **data,float *row,float *col, float *peak, float *tot, int size)
{
#define MAXBOX 25
   int iter, imin, imax, jmin, jmax, i, j, icol, irow;
   int niter = 1;
   unsigned long n;
   float temp[4*MAXBOX+1];
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
/*
      imin = (imin<0 ? 0 : imin);
      jmin = (jmin<0 ? 0 : jmin);
      imax = (imax>size+4 ? size+4 : imax);
      jmax = (jmax>size+4 ? size+4 : jmax);
*/

/* Compute a background value using a median of the box edge values */
      thresh=0;
      if (domedian>0) {
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
        sprintf(outbuf+strlen(outbuf),"median n: %d\n",n);
      }
      if (domedian>1) thresh = median(n,temp);

      sum = sumx = sumy = 0; 
      *peak = 0;
      outbuf2[0] = 0;
      sprintf(outbuf2+strlen(outbuf2),"thresh: %f\n",thresh);
      sprintf(outbuf2+strlen(outbuf2),
              "centroid subimage: (%d:%d,%d:%d) size: %d\n",
              imin, imax, jmin, jmax, size);
      for (j=jmin;j<=jmax;j++) {
        add = *(data+j);
        for (i=imin;i<=imax;i++) {
/* Note that passed row,col are for (0,0) origin */
           px = *(add+i);
           val = (double)px - thresh;
           sprintf(outbuf2+strlen(outbuf2),"%4.4u ",px);
           *peak = (val>*peak ? (float)val : *peak);
           sum += val;
           sumx += val*i;
           sumy += val*j;
        }
        sprintf(outbuf2+strlen(outbuf2),"\n");
      }
      sprintf(outbuf2+strlen(outbuf2),"sumx: %lf sumy: %lf sum: %lf\n",
              sumx, sumy, sum);
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
      sprintf(outbuf2+strlen(outbuf2)," centroid: (%lf,%lf) dx: %lf dy: %lf",
        sumx, sumy, dx, dy);
      if (verbose) writeline(outbuf2,1);
      *col = sumx;
      *row = sumy;
      *tot = sum;
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

int printarr(WORD **data, float *row, float *col)
{
#define MAXBOX 25
   int iter, imin, imax, jmin, jmax, i, j, icol, irow;
   int niter = 1;
   unsigned long n;
   float temp[4*MAXBOX+1];
   double sum, sumx, sumy, dx, dy;
   float thresh;
   WORD px, *add, *add2;
   double val;
   double eps=1.e-3;

   icol = nintf(*col);
   irow = nintf(*row);
 
      imin = icol-size/2;
      imax = icol+size/2;
      jmin = irow-size/2;
      jmax = irow+size/2;

      outbuf2[0] = 0;
sprintf(outbuf2+strlen(outbuf2),"centroid subimage: (%d:%d,%d:%d) size: %d\n",
        imin, imax, jmin, jmax, size);
      for (j=jmin;j<=jmax;j++) {
        add = *(data+j);
        for (i=imin;i<=imax;i++) {
/* Note that passed row,col are for (0,0) origin */
           px = *(add+i);
 sprintf(outbuf2+strlen(outbuf2),"%4.4u ",px);
        }
sprintf(outbuf2+strlen(outbuf2),"\n");
      }
if (verbose) writeline(outbuf2,1);
}

ccd_cooler(BOOL on)
{
  if (on)
    CameraCooler(TRUE);
  else
    CameraCooler(FALSE);
}

ccd_shutter(BOOL open)
{
  if (open)
    CameraOpenShutter();
  else
    CameraCloseShutter();
  if (CameraGetStatus() & STAT_SHUTTER)
    writeline("Shutter open\n",0);
  else
    writeline("Shutter closed\n",0);
}

