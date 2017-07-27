#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <sched.h>
#include <sys/time.h>
#include <sys/resource.h>
/*#include <sys/io.h>*/
/*#include <asm/io.h>*/
#include <ctype.h>

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifdef FLI132
#include "libfli-1.32/libfli.h"
#include "libfli-1.32/myerr.h"
#else
// cant get following to work
#include "fli-dist-1.71/libfli/libfli.h"
#include "fli-dist-1.71/libfli/myerr.h"
#endif

#undef WARNC
#define WARNC(c, format, args...)                       \
  warnx(__ERR_PREFIX format ": %s",                     \
        __FILE__, __LINE__ , ## args, strerror(c))

#define TRYFUNC(f, ...)                         \
  do {                                          \
    if ((r = f(__VA_ARGS__)))                   \
      WARNC(-r, #f "() failed");                \
  } while (0)

#define LIBVERSIZ 1024
typedef struct {
  flidomain_t domain;
  char *dname;
  char *name;
} cam_t;

int numcams = 0;

void findcams(flidomain_t domain, cam_t **cam);
flidev_t dev;

char message[300];

#define WORD unsigned short

void fli_initialize();
void fli_close();
void fli_openshutter();
void fli_closeshutter();
#ifndef MAIN
void update_status();
void xtv_refresh();
#endif

int prior, old_prior;

#ifdef MAIN
main()
{
  int ncol, nrow, nbytes;
  double exptime = 3.;
  int use_shutter = 1;
  int fast = 0;
  int xs, xe, ys, ye, writeout;
  unsigned char *buf;

  fli_initialize();
  xs = 8;
  ys = 0;
  xe = ye = 1024;
  fli_expose(buf, exptime, use_shutter, fast, xs, xe, ys, ye, writeout);
  fli_close();

  exit(0);
}
int ctrlc = FALSE;
int doalert = FALSE;
#else
extern int ctrlc;
extern int doalert;
#endif

void fli_initialize()
{
  int status, i;
  unsigned short error;
  double t_setpoint = -20., camtemp;
  long r;
  char libver[LIBVERSIZ];
  static uid_t idorig;
  static cam_t *cam = NULL;
  static first = 1;


  if (first==1)  {
    idorig = getuid();
fprintf(stderr,"idorig: %d\n",idorig);
    if (ioperm(0,1000,1) !=0) {
      fprintf(stderr ,"io permission failure %d\n",getuid());
      perror("ioperm: ");
      exit(-2);
    }
  } else {
    fli_close();
    cam= NULL;
    numcams= 0;
fprintf(stderr,"sleep...");
    sleep(1);
//    fprintf(stderr,"setuid: %d\n",setuid(idorig));
  }

fprintf(stderr,"using uid: %d\n",getuid());

#ifndef no_hardware


  TRYFUNC(FLISetDebugLevel, "NO HOST", FLIDEBUG_FAIL);
  TRYFUNC(FLIGetLibVersion, libver, LIBVERSIZ);
  INFO("Library version `%s'", libver);
  /* Parallel port */
  //findcams(FLIDOMAIN_PARALLEL_PORT, &cam);
  /* USB */
  findcams(FLIDOMAIN_USB, &cam);
  /* Serial */
  //findcams(FLIDOMAIN_SERIAL, &cam);
  /* Inet */
  //findcams(FLIDOMAIN_INET, &cam);

  i = 0;

    long tmp1, tmp2, tmp3, tmp4, row, img_rows, row_width;
    double d1, d2;
#define BUFF_SIZ 4096
    char buff[BUFF_SIZ];
    u_int16_t *img;
    int fd, img_size;
fprintf(stderr,"trying: %d %s %s\n",i,cam[i].name,cam[i].dname);
    INFO("Trying camera `%s' from %s domain", cam[i].name, cam[i].dname);
fprintf(stderr,"done trying\n");
    TRYFUNC(FLIOpen, &dev, cam[i].name, FLIDEVICE_CAMERA | cam[i].domain);
fprintf(stderr,"done open\n");
    TRYFUNC(FLIGetModel, dev, buff, BUFF_SIZ);
    INFO("Model: %s", buff);
    TRYFUNC(FLIGetHWRevision, dev, &tmp1);
    INFO("Hardware Rev: %ld %x", tmp1,tmp1);
    TRYFUNC(FLIGetFWRevision, dev, &tmp1);
    INFO("Firmware Rev: %ld %x", tmp1,tmp1);
    TRYFUNC(FLIGetPixelSize, dev, &d1, &d2);
    INFO("Pixel Size: %f x %f\n", d1, d2);
    TRYFUNC(FLIGetVisibleArea, dev, &tmp1, &tmp2, &tmp3, &tmp4);
    INFO("Visible area: (%ld, %ld)(%ld, %ld)\n", tmp1, tmp2, tmp3, tmp4);
    TRYFUNC(FLIGetArrayArea, dev, &tmp1, &tmp2, &tmp3, &tmp4);
    INFO("Array area: (%ld, %ld)(%ld, %ld)", tmp1, tmp2, tmp3, tmp4);
    row_width = tmp3 - tmp1;
    img_rows = tmp4 - tmp2;
    TRYFUNC(FLISetImageArea, dev, tmp1, tmp2, tmp3, tmp4);
    TRYFUNC(FLISetNFlushes, dev, 0);
    TRYFUNC(FLISetTemperature, dev, t_setpoint);
    TRYFUNC(FLIGetTemperature, dev, &d1);
    INFO("Temperature: %f", d1);
    TRYFUNC(FLISetExposureTime, dev, 500);
    TRYFUNC(FLISetFrameType, dev, FLI_FRAME_TYPE_NORMAL);
    flibitdepth_t depth=FLI_MODE_16BIT;
    TRYFUNC(FLISetBitDepth, dev, depth);
    TRYFUNC(FLISetHBin, dev, 1);
    TRYFUNC(FLISetVBin, dev, 1);
#endif
fprintf(stderr,"getuid before set: %d\n",getuid());
//  setuid(getuid());
fprintf(stderr,"getuid after set: %d\n",getuid());
  first=0;

  return;
}

void fli_close()
{
    long r;
   // temperature to set CCD to when program is not running
    double t_setpoint = -20.; 

#ifndef no_hardware
    TRYFUNC(FLISetTemperature, dev, t_setpoint);
fprintf(stderr,"calling close");
    TRYFUNC(FLIClose, dev);
#endif

    return;
}

//char *holdfile = "/home/export/tocc/fochold.doc";

void fli_openshutter()
{
  long r;
  TRYFUNC(FLIControlShutter, dev, FLI_SHUTTER_OPEN);
}
void fli_closeshutter()
{
  long r;
  TRYFUNC(FLIControlShutter, dev, FLI_SHUTTER_CLOSE);
}

// Function to expose Leach and read image into buffer
int fli_expose(WORD *buf, double exptime, int use_shutter, int fast,
               int xs, int xe, int ys, int ye, int writeout)
{
  int status, init=0, ifile;
  FILE *fp;
  time_t start, current;
  int alert, row, col;
  int nx, ny, nbias, x0, y0, b0;
  long r;

//  fp = fopen(holdfile,"w");
//  if (fp != NULL) fclose(fp);

  /* shutter */
  /* Ask for the current controller status. */
/*
  printf("Setting shutter position: %d...\n", use_shutter);
  if (use_shutter && exptime > 0.05) {
  }
  else if (!use_shutter) {
  }
*/

#ifndef no_hardware
  TRYFUNC(FLISetExposureTime, dev, (long)(exptime*1000));

  TRYFUNC(FLISetImageArea, dev, xs, ys, xe+1, ye+1);
#endif
  nx = xe-xs+1;
  ny = ye-ys+1;

#ifndef no_hardware
  TRYFUNC(FLIExposeFrame, dev);
#endif

  alert = FALSE;
  long tmp1 = (long)(exptime*1000);
  while (!ctrlc && !alert && tmp1) {

#ifndef no_hardware
    TRYFUNC(FLIGetExposureStatus, dev, &tmp1);
#endif
    if (r)
        break;
    usleep(25000);
/*
    fp = fopen("/home/tcomm/alert/alert.int","r");
    if (doalert && fp != NULL) {
        alert = TRUE;
        fclose(fp);
        remove("/home/tcomm/alert/alert.int");
    }
*/
#ifndef MAIN
    update_status(1);
#endif
  }
  if (ctrlc || alert) {
#ifndef no_hardware
    TRYFUNC(FLICancelExposure, dev);
#endif
#ifndef MAIN
    update_status(-1);
#endif
    return(2);
  }

#ifndef no_hardware
  for (row = 0; row < ny; row++)
      TRYFUNC(FLIGrabRow, dev, buf+row*nx, nx);
#else
  for (row = 0; row < ny; row++)
    for (col=0; col < nx; col++)
      *(buf+row*nx+col) = col;
#endif

//  remove(holdfile);
#ifndef MAIN
  if (!fast) update_status(4);
#endif
  status=0;

  return(status);
}

int fli_flush(unsigned short *buf, int nx, int ny)
{
  int row;
  long r;
  long tmp1, tmp2, tmp3, tmp4;
  TRYFUNC(FLIControlBackgroundFlush, dev, FLI_BGFLUSH_START);
/*
  TRYFUNC(FLIGetArrayArea, dev, &tmp1, &tmp2, &tmp3, &tmp4);
  //INFO("Array area: (%ld, %ld)(%ld, %ld)", tmp1, tmp2, tmp3, tmp4);
  TRYFUNC(FLISetImageArea, dev, tmp1, tmp2, tmp3, tmp4);
  for (row = 0; row < ny; row++)
      TRYFUNC(FLIGrabRow, dev, buf+row*nx, nx-1);
*/
}


int fli_temp(double *temp, int *tempstatus)
{
  long r;
#ifndef no_hardware
  TRYFUNC(FLIGetTemperature, dev, temp);
#endif
  *tempstatus = 0;
  return(0); 
}
int set_fli_temp(double temp)
{
  int status;
  long r;
#ifndef no_hardware
  TRYFUNC(FLISetTemperature, dev, temp);
#endif
  return(0);
}

void findcams(flidomain_t domain, cam_t **cam)
{
  long r;
  char **tmplist;

 TRYFUNC(FLICreateList, domain | FLIDEVICE_CAMERA);
  TRYFUNC(FLIList, domain | FLIDEVICE_CAMERA, &tmplist);

fprintf(stderr,"tmplist[0]: %s\n",tmplist[0]);

  if (tmplist != NULL && tmplist[0] != NULL)
  {
    int i, cams = 0;

    for (i = 0; tmplist[i] != NULL; i++)
      cams++;

    if ((*cam = realloc(*cam, (numcams + cams) * sizeof(cam_t))) == NULL)
      ERR(1, "realloc() failed");

    for (i = 0; tmplist[i] != NULL; i++)
    {
      int j;

      for (j = 0; tmplist[i][j] != '\0'; j++)
        if (tmplist[i][j] == ';')
        {
          tmplist[i][j] = '\0';
          break;
        }
      cam[numcams + i]->domain = domain;
      switch (domain)
      {
      case FLIDOMAIN_PARALLEL_PORT:
        cam[numcams + i]->dname = "parallel port";
        break;

      case FLIDOMAIN_USB:
        cam[numcams + i]->dname = "USB";
        break;

      case FLIDOMAIN_SERIAL:
        cam[numcams + i]->dname = "serial";
        break;

      case FLIDOMAIN_INET:
        cam[numcams + i]->dname = "inet";
        break;

      default:
        cam[numcams + i]->dname = "Unknown domain";
      }
      cam[numcams + i]->name = strdup(tmplist[i]);
    }

    numcams += cams;
  }

  TRYFUNC(FLIFreeList, tmplist);

  return;
}


