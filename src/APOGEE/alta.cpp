/*  Program : test_alta.cpp
 *  Version : 3.0
 *  Author  : Dave Mills
 *  Copyright : The Random Factory 2004-2012
 *  License : GPL
 *
 *
 *  This program provides a limited test environment for Apogee ALTA 
 *  series cameras. It utilises the same low-level API provided by the
 *  apogee_USB.so and apogee_NET.so shared object libraries.
 * 
 *  To build for ALTA-E
 * 
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ALTA  -DALTA_NET apgSampleCmn.cpp
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ALTA test_apogee.cpp
    g++ -g -o test_alta test_apogee.o apgSampleCmn.o -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl

 *  To build for ALTA-U
 *
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ALTA apgSampleCmn.cpp
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ALTA test_apogee.cpp
    g++ -g -o test_alta test_apogee.o apgSampleCmn.o -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl

 *  To build for ALTA-F
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ALTAF apgSampleCmn.cpp
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ALTAF test_apogee.cpp
    g++ -g -o test_ascent test_apogee.o apgSampleCmn.o -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl

 *  To build for ASCENT
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ASCENT apgSampleCmn.cpp
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ASCENT test_apogee.cpp
    g++ -g -o test_ascent test_apogee.o apgSampleCmn.o -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl
 *
 *  To build for Aspen
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ASPEN apgSampleCmn.cpp
    g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-2.1/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ASPEN test_apogee.cpp
    g++ -g -o test_ascent test_apogee.o apgSampleCmn.o -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl

 *  The program is controlled by a set of command line options
 *  Usage information is obtained by invoking the program with -h
 *
 *  eg   ./test_ascent -h
 *
 *  Functions provided include full frame, subregion, binning, image sequences,
 *  fan and cooling control.
 *
 *  Caveats : There is limited error checking on the input options, if you 
 * 	      hang the camera onboard software, simply power cycle the 
 * 	      camera.
 */


#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>
#include <sys/time.h>


#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "fitsio.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include "tcl.h"
#include "AltaF.h"

#include "FindDeviceUsb.h"
#include "CameraInfo.h"
#include "apgSampleCmn.h"
#include "ccd.h"
extern int printerror(int status);
int parse_options (int argc, char **argv);
int saveimage(unsigned short *src_buffer, char *filename, int nx, int ny);
int dobiassubtract(unsigned short *src,unsigned short *dest, int nx, int ny);


/* Declare the camera object. All camera functions and parameters are
 * accessed using the methods and instance variables of this object
 *
 * Their declarations can be found in ApnCamera.h
 */

/* Declare globals to store the input command line options */

char imagename[256];
double texposure=1.0;
int  shutter=1;
int ip[4]={0,0,0,0};
int xbin=1;
int ybin=1;
int xstart=0;
int xend=0;
int ystart=0;
int yend=0;
int biascols=0;
Apg::FanMode fanmode=Apg::FanMode_Off;
double cooling=99.0;
int numexp=1;
int firmware=33;
int modelnum=0;
int ipause=0;
int verbose=0;
int camnum=1;
Apg::AdcSpeed highspeed=Apg::AdcSpeed_Normal;
int tdimode=0;
int tdirows=0;
int bulkseq = 0;


/* Bias definitions used in the libccd functions */

extern int bias_start, bias_end, bcols;
typedef struct {
     unsigned short *pixels;
     int            size;
     int          xdim;
     int          ydim;
     int          zdim;
     int          xbin;
     int          ybin;
     int          type;
     char           name[64];
     int            shmid;
     size_t         shmsize;
     char           *shmem;
} CCD_FRAME;

typedef void *PDATA;
#define MAX_CCD_BUFFERS  1000
PDATA CCD_locate_buffer(char *name, int idepth, int imgcols, int imgrows, int hbin, int vbin);
int   CCD_free_buffer();
int   CCD_locate_buffernum(char *name);
extern CCD_FRAME CCD_Frame[MAX_CCD_BUFFERS];
extern int CCD_free_buffer(char *name);
extern void CCD_buffer_init();
void checkStatus( const Apg::Status status );
extern "C" void apogee_initialize();
extern "C" int apogee_temp(double *temp, int *tempstatus);
extern "C" int set_apogee_temp(double temp);
extern "C" int apogee_expose(unsigned short *buf, double exptime, int use_shutter, int fast, int xs, int xe, int ys, int ye, int writeout);
extern "C" int apogee_status();
extern "C" void apogee_close();

///////////////////////////
// MAKE	  TOKENS
std::vector<std::string> makeTokens(const std::string &str, const std::string &separator)
{
	std::vector<std::string> returnVector;
	std::string::size_type start = 0;
	std::string::size_type end = 0;

	while( (end = str.find(separator, start)) != std::string::npos)
	{
		returnVector.push_back (str.substr (start, end-start));
		start = end + separator.size();
	}

	returnVector.push_back( str.substr(start) );

	return returnVector;
}


////////////////////////////
//	GET		ADDRESS
std::string GetAddress( const std::string & msg )
{

	//search the find string for an attached camera address
	std::vector<std::string> params = makeTokens( msg, "," );
	std::vector<std::string>::iterator iter;

	for(iter = params.begin(); iter != params.end(); ++iter)
	{
	   if( std::string::npos != (*iter).find("address=") )
	   {
		 std::string DeviceAddr = makeTokens( (*iter), "=" ).at(1);
		 //only looking for one camera
		 return DeviceAddr ;
	   }
	} //for

	std::string noOp;
	return noOp;
}

/* Main executable starts here -----------------------------------------------*/

int oldmain (int argc, char **argv) 
{

	int mystatus;
	unsigned short *image;
	int bnum,i;
	int nx,ny;
	int iexposure;
        unsigned int ipaddr;
        double t;
        char seqname[256];
        unsigned short ir, Reg;
        std::vector<uint16_t> pImageData;
        unsigned short *pccdData;
	unsigned int maxrows,maxcols;
        struct timeval m_start;
	struct timeval m_end;
	int msreadout;

/*	Default the bias to no-bias-subtraction */
        bias_start = 0;
        bias_end = 0;
        bcols = 0;

/*	Obtain user provided options */
	mystatus = parse_options(argc,argv);

        std::string ioInterface("usb");
        FindDeviceUsb lookUsb;
        std::string msg = lookUsb.Find();                
        std::string addr = apgSampleCmn::GetUsbAddress( msg );
        uint16_t id = apgSampleCmn::GetID( msg );
        uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );

/*	Create the camera object , this will reserve memory */
        AltaF Apogee;
	Apogee.OpenConnection(ioInterface, addr, frmwrRev, id);

/*	Do a system Init to ensure known state, flushing enabled etc */
	Apogee.Init();

        CCD_buffer_init();

/*      Special verbosity to dump regs and exit */
        if (verbose == 99) {
           for(ir=0;ir<106;ir++) {
              Reg=Apogee.ReadReg(ir);
              printf ("Register %d = %d (%x)\n",ir,Reg,Reg);
           }
           exit(0);
        }

/*	If bias subtraction requested, set it up */
        if (biascols != 0) {
           bcols = biascols;
        }

/*	Setup binning, defaults to full frame */
        i = 0;

/*      Set up binning */
	Apogee.SetRoiBinCol(xbin);
	Apogee.SetRoiBinRow(ybin);

/*	Set up a region of interest, defaults to full frame */
        if (xstart > 0) {
          Apogee.SetRoiStartCol(xstart);
          Apogee.SetRoiStartRow(ystart);
          Apogee.SetRoiNumCols((xend-xstart+1)/xbin);
          Apogee.SetRoiNumRows((yend-ystart+1)/ybin);
        } else {
          maxcols = Apogee.GetMaxImgCols() / xbin;
          maxrows = Apogee.GetMaxImgRows() / ybin;
          Apogee.SetRoiStartCol(0);
          Apogee.SetRoiStartRow(0);
          Apogee.SetRoiNumCols(maxcols);
          Apogee.SetRoiNumRows(maxrows);
        }

/*	Set the required fan mode */
        Apogee.SetFanMode(fanmode,false);

/*	If a particular CCD temperature was requested, then enable 
	cooling and set the correct setpoint value */
        if (cooling < 99.0) {
           printf("Waiting for requested temperature of %6.1lf \r",cooling);
           Apogee.SetCooler(1);
           Apogee.SetCoolerSetPoint(cooling);
           t = Apogee.GetTempCcd();

/*	   Then loop until we get within 0.2 degrees, about the best we can hope for */
           while (fabs(t-cooling) > 0.2) {
               printf("Waiting for requested temperature of %6.1lf, current value is %6.1lf \r",cooling,t);
               sleep(1);
               t = Apogee.GetCoolerStatus();
               t = Apogee.GetTempCcd();
           }
           printf("\n	Temperature is now %6.1lf\n",t);
        }

/*	Add a second to ensure readout will be complete when we try to read */
        iexposure = (int)texposure+1;
        

/*	Loop until all exposures completed */
  	Apogee.SetImageCount(1);

/*	Start an exposure */
        Apogee.StartExposure(texposure,shutter);

// Check camera status to make sure image data is ready
	Apg::Status status = Apg::Status_Flushing;
	while( Apg::Status_ImageReady !=  status ) {
		status = Apogee.GetImagingStatus();
		//make sure there isn't an error
		//throw here if there is
		checkStatus( status );
	}


/*	Readout the image and save in a named buffer (tempobs) */
        char tbuffer[8];
        strcpy(tbuffer,"tempobs");

        nx = Apogee.GetRoiNumCols();
        ny = Apogee.GetRoiNumRows();
        pccdData = (unsigned short *)CCD_locate_buffer(tbuffer, 2 , nx, ny, 1, 1 );
        if (pccdData == NULL) {
                printf("ERROR - no CCD_Buffer\n");
                exit(1);
        }

        gettimeofday( &m_start, NULL);
        Apogee.GetImage(pImageData);
        gettimeofday( &m_end, NULL);
        copy(pImageData.begin(), pImageData.end(), pccdData);
        msreadout = ( ( (m_end.tv_sec - m_start.tv_sec) * 1000) + (m_end.tv_usec - m_start.tv_usec) / 1000.0) + 0.5;

/*	Use the libccd routine to find the corresponding buffer index */
        bnum = CCD_locate_buffernum(tbuffer);

/*	Print details about the buffer for debug purposes */
        printf("Buffer %4d %s = %d bytes cols=%d rows=%d depth=%d\n",bnum,CCD_Frame[bnum].name, 
        CCD_Frame[bnum].size,CCD_Frame[bnum].xdim,CCD_Frame[bnum].ydim,CCD_Frame[bnum].zdim);
        printf("CCD readout time = %d ms\n",msreadout);

/*	Obtain the memory address of the actual image data, and x,y dimensions */
        image = CCD_Frame[bnum].pixels;

/*	If this is part of a sequence, prefix image name with the number */
        saveimage(image, imagename, nx, ny);

/*	 All done, tidy up */
	 Apogee.CloseConnection();

}

/*  Helper routines start here-------------------------------------------------*/

/*  This routine provides a very simple command line parser
 *  Unknown options should be ignored, but strict type
 *  checking is NOT done.
 */

int parse_options (int argc, char **argv)
{
   int i;
   int goti,gott,gots,gota;

/* Zero out counters for required options */
   goti=0;
   gott=0;
   gots=0;
   gota=0;
   i=1;

/* Default fanmode to medium */
   fanmode = Apg::FanMode_Medium;

/* Loop thru all provided options */
   while (i<argc) {

/*     Image name */
       if (!strncmp(argv[i],"-i",2)) {
          strcpy(imagename,argv[i+1]);
          goti = 1;
       }

/*     Exposure time */
       if (!strncmp(argv[i],"-t",2)) {
          sscanf(argv[i+1],"%lf",&texposure);
          gott = 1;
       }

/*     Shutter state */
       if (!strncmp(argv[i],"-s",2)) {
          sscanf(argv[i+1],"%d",&shutter);
          gots= 1;
       }

/*     Bulk sqeuence download */
       if (!strncmp(argv[i],"-B",2)) {
          sscanf(argv[i+1],"%d",&bulkseq);
       }

/*     IP address for ALTA-E models */
       if (!strncmp(argv[i],"-a",2)) {
          sscanf(argv[i+1],"%d.%d.%d.%d",ip,ip+1,ip+2,ip+3);
          gota = 1;
       }

/*     Fast readout mode for ALTA-U models */
       unsigned int ispeed;
       if (!strncmp(argv[i],"-F",2)) {
          sscanf(argv[i+1],"%d",&ispeed);
          highspeed = (Apg::AdcSpeed)ispeed;
       }

/*     Drift readout mode - number of rows */
       if (!strncmp(argv[i],"-d",2)) {
          sscanf(argv[i+1],"%d",&tdirows);
       }

/*     Drift readout mode - TDI */
       if (!strncmp(argv[i],"-D",2)) {
          sscanf(argv[i+1],"%d",&tdimode);
       }

/*     Horizontal binning */
       if (!strncmp(argv[i],"-x",2)) {
          sscanf(argv[i+1],"%d",&xbin);
       }

/*     Vertical binning */
       if (!strncmp(argv[i],"-y",2)) {
          sscanf(argv[i+1],"%d",&ybin);
       }

/*     Firmware revision */
       if (!strncmp(argv[i],"-R",2)) {
          sscanf(argv[i+1],"%d",&firmware);
       }
/*     Model number */
       if (!strncmp(argv[i],"-M",2)) {
          sscanf(argv[i+1],"%d",&modelnum);
       }

/*     Region of interest */
       if (!strncmp(argv[i],"-r",2)) {
          sscanf(argv[i+1],"%d,%d,%d,%d",&xstart,&ystart,&xend,&yend);
       }

/*     Bias subtraction */
       if (!strncmp(argv[i],"-b",2)) {
          sscanf(argv[i+1],"%d",&biascols);
       }

/*     Fan mode */
       if (!strncmp(argv[i],"-f",2)) {
          if (!strncmp(argv[i+1],"off",3)==0) fanmode=Apg::FanMode_Off;
          if (!strncmp(argv[i+1],"slow",4)==0) fanmode=Apg::FanMode_Low;
          if (!strncmp(argv[i+1],"medium",6)==0) fanmode=Apg::FanMode_Medium;
          if (!strncmp(argv[i+1],"fast",4)==0) fanmode=Apg::FanMode_High;
       }

/*     Setpoint temperature */
       if (!strncmp(argv[i],"-c",2)) {
          sscanf(argv[i+1],"%lf",&cooling);
       }

/*     Sequence of exposures */
       if (!strncmp(argv[i],"-n",2)) {
          sscanf(argv[i+1],"%d",&numexp);
       }

/*     USB camera number */
       if (!strncmp(argv[i],"-u",2)) {
          sscanf(argv[i+1],"%d",&camnum);
       }

/*     Interval to pause between exposures */
       if (!strncmp(argv[i],"-p",2)) {
          sscanf(argv[i+1],"%d",&ipause);
       }

/*     Be more verbose */
       if (!strncmp(argv[i],"-v",2)) {
          sscanf(argv[i+1],"%d",&verbose);
       }

/*     Print usage info */
       if (!strncmp(argv[i],"-h",2)) {
          printf("Apogee image tester -  Usage: \n \
	 -i imagename    Name of image (required) \n \
	 -t time         Exposure time is seconds (required)\n \
	 -s 0/1          1 = Shutter open, 0 = Shutter closed (required)\n \
	 -a a.b.c.d      IP address of camera e.g. 192.168.0.1 (required for ALTA-E models only)\n \
	 -F 0/1          Fast readout mode (ALTA-U models only)\n \
	 -D 0/1          Drift readout mode - TDI, exposure time specifies time-per-row\n \
	 -d num          Number of rows for Drift mode readout\n \
	 -u num          Camera number (default 1 , ALTA-U only) \n \
	 -x num          Binning factor in x, default 1 \n \
	 -y num          Binning factor in y, default 1 \n \
	 -r xs,ys,xe,ye  Image subregion in the format startx,starty,endx,endy \n \
	 -b biascols     Number of Bias columns to subtract \n \
	 -f mode         Fanmode during exposure, off,slow,medium,fast (default medium) \n \
	 -c temp         Required temperature for exposure, default is current value \n \
	 -n num          Number of exposures \n \
	 -p time         Number of seconds to pause between multiple exposures \n \
         -B 0/1          Bulk sequence download mode for multiple exposures \n \
         -R num          Firmware Revision \n \
         -M num          Model number \n \
	 -v verbosity    Print more details about exposure\n");
         exit(0);
        }

/*      All options are 2 args long! */
        i = i+2;
   } 

/* Complain about missing required options, then give up */
   if ( goti == 0 ) printf("Missing argument  -i imagename\n");
   if ( gott == 0 ) printf("Missing argument  -t exposure time\n");
   if ( gots == 0 ) printf("Missing argument  -s shutter state\n");
#ifdef ALTA_NET
   if ( gota == 0 ) printf("Missing argument  -a IP address\n");
   if (goti+gots+gott+gota != 4) exit(1);
#else
   if (goti+gots+gott != 3) exit(1);
#endif

/* Print exposure details */
   if (verbose > 0) {
      printf("Apogee ALTA image test - V2.0\n");
      printf("	Image name is %s\n",imagename);
      printf("	Exposure time is %lf\n",texposure);
      if (numexp    > 1) printf("	Sequence of %d exposures requested\n",numexp);
      if (ipause    > 0.0) printf("	Pause of %d seconds between exposures\n",ipause);
      printf("	Shutter state during exposure will be %d\n",shutter);
#ifdef ALTA_NET
      if (ip[0]     != 0) printf("	ALTA-E ip address is %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
#endif
      if (xbin      > 1) printf("	X binning selected xbin=%d\n",xbin);
      if (ybin      > 1) printf("	Y binning selected ybin=%d\n",ybin);
      if (xstart    != 0) printf("	Subregion readout %d,%d,%d,%d\n",xstart,xend,ystart,yend);
      if (biascols  != 0) printf("	Bias subtraction using %d columns\n",biascols);
      if (fanmode > 0) printf("	Fan set to mode = %d\n",fanmode);
      if (cooling < 99.0) printf("	Requested ccd temperature for exposure is %lf\n",cooling);
      if (tdimode == 1) printf("	TDI mode , number of rows = %d, %lf secs per row\n",tdirows,texposure);
   }
   return(0);

}


/*  This routine provides simple FITS writer. It uses the routines
 *  provided by the fitsTcl/cfitsio libraries
 *
 *  NOTE : It will fail if the image already exists
 */

int saveimage(unsigned short *src_buffer, char *filename, int nx, int ny)
{
  

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  long  fpixel, nelements;
  unsigned short *array;
  unsigned short *simg;
  int status;
  /* initialize FITS image parameters */
  int bitpix   =  USHORT_IMG; /* 16-bit unsigned short pixel values       */
  long naxis    =   2;  /* 2-dimensional image                            */
  long naxes[2];   

    char tbuffer[8];
    strcpy(tbuffer,"stemp");
    naxes[0] = nx-bcols;
    naxes[1] = ny; 
    array = src_buffer;
    status = 0;         /* initialize status before calling fitsio routines */
    simg = (unsigned short *)CCD_locate_buffer(tbuffer,2,nx-bcols,ny,1,1);
 
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
 
    /* write the required keywords for the primary array image.     */
    /* Since bitpix = USHORT_IMG, this will cause cfitsio to create */
    /* a FITS image with BITPIX = 16 (signed short integers) with   */
    /* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
    /* FITS uses to store unsigned integers.  Note that the BSCALE  */
    /* and BZERO keywords will be automatically written by cfitsio  */
    /* in this case.                                                */          
 
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
 
 
    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */

    if (bcols > 0) {
      dobiassubtract(src_buffer,simg,naxes[0],naxes[1]);
 
      /* write the array of unsigned integers to the FITS file */
      if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, simg, &status) )
        printerror( status );
 
    } else {
      /* write the array of unsigned integers to the FITS file */
      if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, src_buffer, &status) )
        printerror( status );
 
    } 

    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );

  
    return(status);
}                                                                               


/*  This routine should do bias subtraction. At present it
 *  uses the minium pixel DN as the bias value, instead of
 *  averaging the bias columns. This is because on the 
 *  test unit I have, averaging these columns does not seem
 *  to give values consistently lower than those in the 
 *  exposed region.
 *
 *  src is the input image with bias columns
 *  dest is a smaller output image with the bias columns trimmed off
 *       and the "bias" subtracted from the image pixels.
 */

int dobiassubtract(unsigned short *src,unsigned short *dest, int nx, int ny)
{
   double biases[128000];
   double abiases;
   int ix,iy, oix;
   int ipix, opix;
   unsigned short minbias;
   minbias = 65535;
   if (bcols == 0) {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
     }
     minbias = 0;
   } else {
     for (iy=0;iy<ny;iy++) {
        biases[iy] = 0.0;
        for (ix=bias_start;ix<=bias_end;ix++) {
            ipix = (nx+bcols)*iy + ix-1;
            biases[iy] = biases[iy] + (float)src[ipix];
            if (src[ipix]<minbias) minbias = src[ipix];
        }
        biases[iy] =  biases[iy] / (float)bcols;
     }
   }
  
   for (iy=0;iy<ny;iy++) {
      oix = 0;
      for (ix=0;ix<nx+bcols;ix++) {
        if (ix < bias_start || ix > bias_end) {
          ipix = (nx+bcols)*iy + ix;
          opix = nx*iy + oix;
          if (src[ipix] < minbias) {
             dest[opix] = 0;
          } else {
            dest[opix] = src[ipix] - (int)minbias;
          }
          oix++;
        }
      }
   }
   return(0);
}

////////////////////////////
//		CHECK	STATUS
void checkStatus( const Apg::Status status )
{
	switch( status )
	{
		case Apg::Status_ConnectionError:
		{
			std::string errMsg("Status_ConnectionError");
			std::runtime_error except( errMsg );
			throw except;
		}
		break;
	
		case Apg::Status_DataError:
		{
			std::string errMsg("Status_DataError");
			std::runtime_error except( errMsg );
			throw except;
		}
		break;
	
		case Apg::Status_PatternError:
		{
			std::string errMsg("Status_PatternError");
			std::runtime_error except( errMsg );
			throw except;
		}
		break;
	
		case Apg::Status_Idle:
		{
			std::string errMsg("Status_Idle");
			std::runtime_error except( errMsg );
			throw except;
		}
		break;
	
		default:
			//no op on purpose
		break;
	}
	
}


AltaF Apogee;

#ifdef MAIN
int main (int argc, char **argv) 
{
  double temp;
  int tempstatus,n=0,nx,ny;
  std::vector<uint16_t> pImageData;
  unsigned short *pccdData;
  short *buf;
  struct timeval m_start;
  struct timeval m_end;
  int msreadout, bnum;
  unsigned short *image;

  apogee_initialize();

  nx = Apogee.GetRoiNumCols();
  ny = Apogee.GetRoiNumRows();
  printf("nx: %d ny: %d\n",nx,ny);

  // Allocate buffer array. 
  buf = (short *)malloc((nx*ny)*2);
  if (buf ==NULL) {
    fprintf(stderr,"error allocating buf\n");
  }

/*	Start an exposure */
  Apogee.StartExposure(texposure,shutter);

// Check camera status to make sure image data is ready
  Apg::Status status = Apg::Status_Flushing;
  while( Apg::Status_ImageReady !=  status ) {
		status = Apogee.GetImagingStatus();
		//make sure there isn't an error
		//throw here if there is
		checkStatus( status );
  }
  char tbuffer[8];
  strcpy(tbuffer,"tempobs");
  pccdData = (unsigned short *)CCD_locate_buffer(tbuffer, 2 , nx, ny, 1, 1 );
  if (pccdData == NULL) {
      printf("ERROR - no CCD_Buffer\n");
      exit(1);
  }

  gettimeofday( &m_start, NULL);
  Apogee.GetImage(pImageData);
  gettimeofday( &m_end, NULL);
  copy(pImageData.begin(), pImageData.end(), pccdData);
  msreadout = ( ( (m_end.tv_sec - m_start.tv_sec) * 1000) + (m_end.tv_usec - m_start.tv_usec) / 1000.0) + 0.5;

/*      Use the libccd routine to find the corresponding buffer index */
  bnum = CCD_locate_buffernum(tbuffer);

/*      Print details about the buffer for debug purposes */
  printf("Buffer %4d %s = %d bytes cols=%d rows=%d depth=%d\n",bnum,CCD_Frame[bnum].name,
        CCD_Frame[bnum].size,CCD_Frame[bnum].xdim,CCD_Frame[bnum].ydim,CCD_Frame[bnum].zdim);
        printf("CCD readout time = %d ms\n",msreadout);

/*      Obtain the memory address of the actual image data, and x,y dimensions */
  image = CCD_Frame[bnum].pixels;
  printf("%d %d %d\n",*image,*image+1,*image+2);

  while (n++<2) {
    apogee_temp(&temp,&tempstatus);
    sleep(1);
  }
  printf("close\n");
  Apogee.CloseConnection();
  printf("done close\n");
}
#endif

void apogee_initialize()
{
  int status;
  unsigned short error;
  double t_setpoint = -20.;
  static int init = 0;
  unsigned short ir, Reg;

  if ( init==1 ) return;

  std::string ioInterface("usb");
  FindDeviceUsb lookUsb;
  std::string msg = lookUsb.Find();                
  std::string addr = apgSampleCmn::GetUsbAddress( msg );
  uint16_t id = apgSampleCmn::GetID( msg );
  uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );

/* Create the camera object , this will reserve memory */
  Apogee.OpenConnection(ioInterface, addr, frmwrRev, id);

/* Do a system Init to ensure known state, flushing enabled etc */
  Apogee.Init();

/*
  for(ir=0;ir<106;ir++) {
      Reg=Apogee.ReadReg(ir);
      printf ("Register %d = %d (%x)\n",ir,Reg,Reg);
  }
*/
  CCD_buffer_init();
  init=1;
}

void apogee_close() {
  printf("close\n");
  Apogee.CloseConnection();
  printf("done close\n");
}

int apogee_status()
{
  return(0);
}
int set_apogee_temp(double cooling)
{
  Apogee.SetCooler(1);
  Apogee.SetCoolerSetPoint(cooling);
}

int apogee_temp(double *temp, int *tempstatus)
{
  double t;
  *temp = Apogee.GetTempCcd();
  printf("temperature: %f\n",*temp);
  *tempstatus = 0;
  return(0);
}

int apogee_expose(unsigned short *buf, double exptime, int use_shutter, int fast,
               int xs, int xe, int ys, int ye, int writeout)
{
  int nx, ny;
  FILE *fp;
  int alert;
  struct timeval m_start;
  struct timeval m_end;
  int msreadout, bnum;
  std::vector<uint16_t> pImageData;
  unsigned short *pccdData;

  nx = Apogee.GetRoiNumCols();
  ny = Apogee.GetRoiNumRows();
  printf("nx: %d ny: %d\n",nx,ny);

/*	Start an exposure */
  Apogee.StartExposure(exptime,use_shutter);

// Check camera status to make sure image data is ready
  Apg::Status status = Apg::Status_Flushing;
  while( Apg::Status_ImageReady !=  status ) {
		status = Apogee.GetImagingStatus();
		//make sure there isn't an error
		//throw here if there is
		checkStatus( status );
  }
  char tbuffer[8];
  strcpy(tbuffer,"tempobs");
  pccdData = (unsigned short *)CCD_locate_buffer(tbuffer, 2 , nx, ny, 1, 1 );
  if (pccdData == NULL) {
      printf("ERROR - no CCD_Buffer\n");
      exit(1);
  }

  gettimeofday( &m_start, NULL);
  Apogee.GetImage(pImageData);
  gettimeofday( &m_end, NULL);
  copy(pImageData.begin(), pImageData.end(), pccdData);
  msreadout = ( ( (m_end.tv_sec - m_start.tv_sec) * 1000) + (m_end.tv_usec - m_start.tv_usec) / 1000.0) + 0.5;

/*      Use the libccd routine to find the corresponding buffer index */
  bnum = CCD_locate_buffernum(tbuffer);

/*      Print details about the buffer for debug purposes */
  printf("Buffer %4d %s = %d bytes cols=%d rows=%d depth=%d\n",bnum,CCD_Frame[bnum].name,
        CCD_Frame[bnum].size,CCD_Frame[bnum].xdim,CCD_Frame[bnum].ydim,CCD_Frame[bnum].zdim);
        printf("CCD readout time = %d ms\n",msreadout);

/*      Obtain the memory address of the actual image data, and x,y dimensions */
  memcpy(buf,CCD_Frame[bnum].pixels,CCD_Frame[bnum].size);
  //buf = CCD_Frame[bnum].pixels;

}
