#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
/*#include <sys/io.h>*/
/*#include <asm/io.h>*/
#include <ctype.h>

#include "apccd.h"
#include "aplow.h"
#include "aperr.h"

#define INI_FILE "APCCD.INI"

CAMDATA pCamera;
HCCD hccd;

char message[300];

#define WORD unsigned short

void apogee_initialize();
void writeterm(char *);
#ifndef MAIN
void update_status();
#endif
void xtv_refresh();

int prior, old_prior;

#ifdef MAIN
main()
{
  PDATA buf;
  int ncol, nrow, nbytes;

  apogee_initialize();

  ncol = (int)pCamera.cols;
  nrow = (int)pCamera.rows;
  nbytes = ncol*2;

  // Allocate buffer array. 
  buf = (PDATA)malloc((ncol*nrow+1)*2);
  if (buf ==NULL) {
    fprintf(stderr,"error allocating buf\n");
  }

  close_camera(hccd);

  exit(0);
}
void writeterm(char *message)
{
  fprintf(stderr,"%s",message);
}
BOOL ctrlc = FALSE;
BOOL doalert = FALSE;
#else
extern BOOL ctrlc;
extern BOOL doalert;
#endif

void apogee_close()
{
    int status;
   // temperature to set CCD to when program is not running
    double t_setpoint = -20.; 

    setpriority(PRIO_PROCESS,0,old_prior);
    prior = getpriority(PRIO_PROCESS,0);
    sprintf(message,"Current Priority is %d\n",prior);
    writeterm(message);
    status = set_temp(hccd, t_setpoint, CCD_TMP_SET);
    //set_temp(hccd,0.,CCD_TMP_OFF);
    close_camera(hccd);
}

void apogee_initialize()
{
  int status;
  unsigned short error;
  double t_setpoint = -20.;
  static int init = 0;

  if ( init==1 ) return;

  init=1;

#ifndef no_hardware
  fprintf(stderr,"setting ioperm %d %d\n",getuid(), init);
  if (ioperm(0,1000,1) !=0) {
      fprintf(stderr ,"io permission failure %d\n",getuid());
      perror("ioperm: ");
      exit(-2);
  }

// Initialize Apogee
  writeterm("opening camera\n");
  hccd = open_camera(IGNORE, IGNORE, INI_FILE);
  if (hccd == 0) {
    writeterm("error opening camera\n");
    // fprintf(stderr,"get_openerror returns: %d\n",get_openerror());
  }

  old_prior = getpriority(PRIO_PROCESS,0);
  setpriority(PRIO_PROCESS,0,-20);
  prior = getpriority(PRIO_PROCESS,0);
  sprintf(message,"Current Priority is %d\n",prior);
  writeterm(message);
#ifdef OFFSET
  setuid(500);
#else
  setuid(1012);
#endif
  sprintf(message,"uid is: %d\n",getuid());
  writeterm(message);

// Turn on cooler
  writeterm("turning on cooler\n");
  status = set_temp(hccd, t_setpoint, CCD_TMP_SET);
  if (status != CCD_OK) {
    writeterm("error in set_temp");
    get_error(hccd,&error);
  }

// Get camera information
  status = get_camdata(hccd, &pCamera);
  if (status != CCD_OK)
    writeterm("error in get_camdata");

  sprintf(message,"cooler set point: %f\n", pCamera.target_temp);
/*
  sprintf(message,"cooler set point: %f\n"
         "binning factors: %d %d\n"
         "x size: %d\n"
         "y size: %d\n",
         pCamera.target_temp,
         pCamera.hbin, pCamera.vbin,
         pCamera.cols,
         pCamera.rows);
*/
  writeterm(message);

#endif


}
int apogee_status()
{
  int status;
// Get camera information
  status = get_camdata(hccd, &pCamera);
  if (status != CCD_OK) {
    fprintf(stderr,"error in get_camdata: %d", status);
    status = status > 0 ? status : -status;
    return(status);
  }
  sprintf(message,"cooler set point: %f\n"
         "binning factors: %d %d\n"
         "x size: %d\n"
         "y size: %d\n",
         pCamera.target_temp,
         pCamera.hbin, pCamera.vbin,
         pCamera.cols,
         pCamera.rows);
  writeterm(message);
  return(status);
}


char *holdfile = "/home/export/tocc/fochold.doc";

// Function to expose Apogee and read image into buffer
int apogee_expose(WORD *buf, double exptime, BOOL use_shutter, BOOL fast,
               int xs, int xe, int ys, int ye, int writeout)
{
  int status, init=0;
  ULONG timer;
  PUSHORT error;
  USHORT nx, ny, x0, y0;
  FILE *fp;
  time_t start, current;
  BOOL alert;

  timer = (ULONG)(exptime*100.);
  timer = (timer < 2) ? 2 : timer ;
  nx = xe-xs+1;
  ny = ye-ys+1;
  x0=12+xs-1;
  y0=3+ys-1;
  //status = config_camera(hccd, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE,
#ifndef no_hardware
  status=CCD_OK;
 // if (init==0){
   status = config_camera(hccd, x0, y0, nx, ny, IGNORE, IGNORE,
            timer, IGNORE, FALSE);
 //  init=1;
 // }
  if (status != CCD_OK) {
    fprintf(stderr,"error in config_camera: %d", status);
    status = status > 0 ? status : -status;
    return(status);
  }

  status = get_camdata(hccd, &pCamera);
  if (status != CCD_OK) {
    fprintf(stderr,"error in get_camdata: %d", status);
    status = status > 0 ? status : -status;
    return(status);
  }

  fp = fopen(holdfile,"w");
  fclose(fp);

  if (timer <= 2)
    status = start_exposure(hccd, FALSE, FALSE, TRUE, !fast);
  else
    status = start_exposure(hccd, use_shutter, FALSE, fast, !fast);
  if (status != CCD_OK) {
    fprintf(stderr,"error in start_exposure: %d",status);
    status = status > 0 ? status : -status;
    return(status);
  }

  // wait for exposure to end and image to be ready
  alert = FALSE;
  if (!fast) {
    time(&start);
    time(&current);
    while (!ctrlc && !alert &&
           (check_exposure(hccd) != CCD_OK || check_image(hccd) != CCD_OK) &&
           (current-start)<(exptime+60)) {
      fp = fopen("/home/tcomm/alert/alert.int","r");
      if (fp != NULL) {
        if (doalert) alert = TRUE;
        fclose(fp);
        remove("/home/tcomm/alert/alert.int");
      }

      if (!fast) {
#ifndef MAIN
        update_status(1);
        xtv_refresh(0);
#endif
      }
      time(&current);
    }
  }
  if (ctrlc || alert) {
    close_camera(hccd);
    hccd = open_camera(IGNORE, IGNORE, INI_FILE);
    status = config_camera(hccd, x0, y0, nx, ny, IGNORE, IGNORE,
            timer, IGNORE, FALSE);
#ifndef MAIN
    update_status(-1);
#endif
    return(2);
  }

  remove(holdfile);
#ifndef MAIN
  if (!fast) update_status(4);
#endif
  status  = acquire_image(hccd, pCamera.caching, buf, !fast);
  if (status != CCD_OK) {
    fprintf(stderr,"error in acquire_image: %d", status);
    status = get_error(hccd, error);
    fprintf(stderr,"error: %d\n", *error);
    status = status > 0 ? status : -status;
  }
#else
  status=0;
#endif

  return(status);
}

int apogee_temp(double *temp, int *tempstatus)
{
  int status;
  short temp_status;
  double temp_read;
#ifndef no_hardware
  status = get_temp(hccd, &temp_status, &temp_read);
//fprintf(stderr,"status %d  temp: %f\n",temp_status,temp_read);
#else
  temp_read = -30;
  temp_status = CCD_TMP_OK;
  status=0;
#endif
  *temp = temp_read;
  if (temp_status == CCD_TMP_OK) 
     *tempstatus=0;
  else if (temp_status == CCD_TMP_RUP || temp_status == CCD_TMP_RDN)
     *tempstatus=1;
  else if (temp_status ==CCD_TMP_STUCK || temp_status == CCD_TMP_MAX)
     *tempstatus=2;
  else
     *tempstatus=-1;
  return(status); 
}

int set_apogee_temp(double temp)
{
  int status;

  status = set_temp(hccd, temp, CCD_TMP_SET);

// Get camera information
  status = get_camdata(hccd, &pCamera);
  if (status != CCD_OK) {
    fprintf(stderr,"error in get_camdata");
    status = status > 0 ? status : -status;
  }
  sprintf(message,"cooler set point: %f\n"
         "binning factors: %d %d\n"
         "x size: %d\n"
         "y size: %d\n",
         pCamera.target_temp,
         pCamera.hbin, pCamera.vbin,
         pCamera.cols,
         pCamera.rows);
  writeterm(message);

  return(status);
}
