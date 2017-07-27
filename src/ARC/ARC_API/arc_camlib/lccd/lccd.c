#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

/*#include "DSPlibrary.h"*/
#include "DSPCommand.h"
#include "Bool.h"
#include "Deinterlace.h"
#include "ErrorString.h"
#include "LoadDspFile.h"
#include "Memory.h"
#include "Temperature.h"
#ifdef WRITE
#include "fitsio.h"
void printerror( int status);
#endif

#define PCI_DEV "/dev/astropci0"
#define NROWS   2200
#define NCOLS   2200
/*#define NROWS 500
  #define NCOLS 500*/

#define HARDWARE_DATA_MAX	1000000

/* Menu Options */

#define SET_TEMP_OPT  't'
#define GET_TEMP_OPT  'g'
#define SET_GAIN_OPT  'G'
#define RESET_ALL_OPT 'R'
#define QUIT_OPT      'q'
#define IM_NAME_OPT   'n'
#define SET_TIME_OPT  'r'
#define START_EXP_OPT 's'
#define SET_MPP_OPT   'm'
#define SET_SER_OPT   'S'
#define SET_DIM_OPT   'd'
#define SET_PATH_OPT  'p'
#define SET_SHUT_OPT  'o'
#define GET_TEC_VOLTAGE 'v'
#define TEST_SHUTTER  'y'
#define SET_TIME_SHORT 'e'
#define EXPOSE_AND_ABORT 'a'

enum serial_type {
    LEFT, RIGHT, BOTH
};


/*extern image_fd;*/
char str_reply_value[125];
/*char error_string[125];*/
extern int reply_value;

struct cam_config {
  int controller_master_value;
  int do_util;
  int temp_control;
  int gain;
  int speed;
  int expose_time;
  int deinterlace;   /* 0 for none, 2 for split-serial */
  double temp_coeff_0;
  double temp_coeff_1;
  double target_temp;
  int nrows;
  int ncols;  
  int mpp;
  int serial_mode;
  char path[100];
  char fname[100];
  int open_shutter;
    int current_shutter_position;
};
    

void close_and_free(int pci_fd, void *buffer);
void pof(int pci_fd);
#ifdef WRITE
int writeimage(void *buf, int nrows, int ncols, char *filename, double start_temp, double end_temp);
#endif
int set_gain_and_speed(int pci_fd, int gain, int speed);
int set_dimensions(int pci_fd, int nrows, int ncols);
int set_serial_mode(int pci_fd, int serial_mode, int *deinterlace);
int set_mpp_mode(int pci_fd, int mpp);
int setup_defaults(int pci_fd, struct cam_config *conf);
char get_option(struct cam_config conf);
int do_option(int pci_fd, void *buf, struct cam_config *conf, char option);
int read_gain_and_speed(struct cam_config *conf);
int read_temp(struct cam_config *conf);
int read_image_name(struct cam_config *conf);
int read_time(struct cam_config *conf, int scaling);
int read_mpp(struct cam_config *conf);
int read_serial_mode(struct cam_config *conf);
int read_dim(struct cam_config *conf);
int set_expo_time(int pci_fd, int expo_time, int controller_master_value);
int do_exposure(int pci_fd, void *buf, struct cam_config *conf, int abort);
int read_image_path(struct cam_config *conf);
int read_shutter(int pci_fd, struct cam_config *conf);
int timediff(const struct timeval *tlast, float *delta_t);
void do_range(void *buf, int *min, int *max, float *mean);
int set_shutter_position(int pci_fd, int open_shutter, int controller_master_value);
int get_temp(int pci_fd);
int set_temp(int pci_fd, double temp);

int pci_fd;
unsigned short *buf = NULL;

void lccd_initialize(char *ccdbuf)
{
  int usecount,size;
  FILE *fp;
  /*void *buf = NULL;*/
  char option;
  char module[100];
  
  /* for temperature */
  struct cam_config conf;

  /* Set Defaults */
  conf.temp_coeff_0 = 13.42;
  conf.temp_coeff_1 = -0.14293;
  conf.target_temp = -10.0;
  conf.gain = 10;
  conf.speed = 1;
  conf.expose_time = 10000;
  conf.deinterlace = 0;
  conf.nrows = NROWS;
  conf.ncols = NCOLS;
  conf.mpp = 1;
  conf.serial_mode = LEFT;
  conf.open_shutter = FALSE;
  strcpy(conf.path, "/home/observer/images/");
  strcpy(conf.fname, "testfit.fit");

  if (LOAD_UTIL_LOD) {
    //  conf.controller_master_value = TIM_ID;
    conf.do_util = 1;
  } else {
    //conf.controller_master_value = TIM_ID;
    conf.do_util = 0;
  }
  conf.controller_master_value = TIM_ID;

  /* open device, checking to make sure module is already loaded */
  fprintf(stdout,"about to open...\n");
  fp = fopen("/proc/modules", "r");
  usecount = -1;
  while(usecount == -1) {
    if (fscanf(fp, "%s", module) == EOF) {
      break;
    }
    if (strncmp(module, "astropci", 8) == 0) {
      fscanf(fp, "%d %d", &size, &usecount);
      break;
    }
  }
  fclose(fp);
   
  if (usecount == -1) {
    fprintf(stderr, "Module astropci is not loaded!\n");
    exit(-1);
  } else if (usecount == 1) {
    fprintf(stderr, "/dev/astropci0 is already in use!");
    exit(-1);
  } else if ((pci_fd = open(PCI_DEV, O_RDWR)) == -1) {
    fprintf(stderr,"failed to open: %m\n");
    exit(-1);
  }
  fprintf(stdout,"Driver opened!\n");
  /*sleep(1);*/

  fprintf(stdout,"Creating Memory...\n");
  if ((buf = create_memory(pci_fd, conf.nrows, conf.ncols)) == NULL) {
    fprintf(stderr,"Unable to create image buffer\n");
    free_memory(pci_fd, buf);  /* ? */
    exit(-1);
  }
  ccdbuf = (char *)buf;

  /*bzero(buf, conf.nrows*conf.ncols*2);*/

  /* create memory for use.  It goes to the extern int image_fd ptr */
  /*
  printf("Allocating memory...\n");
  if (buffer_allocate(&buf, conf.nrows * conf.ncols) == -1) {
      fprintf(stderr,"Error in buffer_allocate\n");
      close(pci_fd);
      exit(-1);
  } else { printf("Done\n"); }

  memset(buf,0,1);
  */
  /* Do setup routine */
  if (setup_defaults(pci_fd, &conf) == -1) {
      fprintf(stderr, "Error in setup.  Exiting\n");
      close_and_free(pci_fd, buf);
      exit(-1);
  }
}

  /* Main Loop */
/*
  option = 0;
  while (option != QUIT_OPT) {
      option = get_option(conf);
      if (do_option(pci_fd, buf, &conf, option) == -1) {
	  fprintf(stderr,"Error in option %c\n", option);
	  close_and_free(pci_fd, buf);
	  exit(-1);
      }
  }
*/

void lccd_close()
{
  pof(pci_fd);

  close_and_free(pci_fd, buf);

  exit(0);
}

int do_exposure(int pci_fd, void *buf, struct cam_config *conf, int abort)
{
  /*
  int i = 0;
  int elapsed_time = 0;
  int maxPixelCount = conf->nrows * conf->ncols;
  int currentPixelCount = 0;
  int lastPixelCount = 0;
  int exposureCounter = 0;
  int timeoutCounter = 0;
  int status = 0;
  char path[100];

  char image_name[200];
  int min,max;
  float mean;


  printf("Setting exposure time: %d sec.\n", conf->expose_time/1000);
  if (doCommand1(pci_fd, TIM_ID, SET, conf->expose_time, DON) == _ERROR) {
    printf("\tERROR: Setting exposure time failed -> 0x%X\n", getError());
    return _ERROR;
  }

  currentPixelCount = 0;
  lastPixelCount = 0;
  exposureCounter = 0;
  timeoutCounter = 0;
  elapsed_time = 0;
  status = 0;
  
  printf("Starting exposure.\n");
  if (doCommand(pci_fd, TIM_ID, SEX, DON) == _ERROR) {
    printf("\tERROR: Starting exposure failed -> 0x%X\n", getError());
    return _ERROR;
  }
  
  while (currentPixelCount < maxPixelCount) {
    
    status = (getHstr(pci_fd) & HTF_BITS) >> 3;
    
    
    if (status != READOUT && exposureCounter >= 20 && conf->expose_time > 1000) {
      
      elapsed_time = doCommand(pci_fd, TIM_ID, RET, UNDEFINED);
      
      printf("Elapsed Time: %d\n", elapsed_time);
      
      if (elapsed_time < 0)
	elapsed_time = 0;
      
      exposureCounter = 0;
      timeoutCounter = 0;
    }
    
    exposureCounter++;
    
    ioctl(pci_fd, ASTROPCI_GET_PROGRESS, &currentPixelCount);
    printf("Current pixel count: %d\n", currentPixelCount);
    
    if (currentPixelCount == lastPixelCount)
      timeoutCounter++;
    
    lastPixelCount = currentPixelCount;
    
    if (timeoutCounter >= READ_TIMEOUT) {
      printf("ERROR: Readout TIMEOUT!\n");
      return _ERROR;
    }
    
    usleep(250);
  }
  */  
    

     

  int i=0;
  int status = 0;
  struct timeval tstart;
  float ftmp;
  int num_pix_count,last_pix_count,max_pix_count;
  int timeoutctr;
  char image_name[200];
  int min,max;
  float mean;
  int ctr;
  int retval;

  max_pix_count = conf->nrows * conf->ncols;

  
  fprintf(stdout,"Setting shutter position to %d\n",conf->open_shutter);
  if (set_shutter_position(pci_fd, conf->open_shutter,conf->controller_master_value) != 0) {
    fprintf(stderr,"oops\n");
    return(-1);
  } else { fprintf(stdout,"Done\n"); }
 

  set_expo_time(pci_fd, conf->expose_time, conf->controller_master_value);

  fprintf(stdout,"Starting exposure...\n");
  if (abort) {
    fprintf(stdout, "And will try to abort...\n");
  }

  if (doCommand(pci_fd, TIM_ID, SEX, DON) == _ERROR) {
    fprintf(stderr,"oops\n");
    return(-1);
  } else { fprintf(stdout,"Done\n"); }

  if (gettimeofday(&tstart, NULL) != 0) {
    fprintf(stderr," Error in gettimeofday.\n");
    pof(pci_fd);
    return(-1);
  }
 
  ctr = 0;
  while(conf->expose_time > 0) {
    if (timediff(&tstart, &ftmp) != 0) {
      fprintf(stderr,"timediff failed\n");
      pof(pci_fd);
      return(-1);
    }

    /*  if ((ftmp > 2.0) && abort) {
      fprintf(stdout,"Trying to abort...");
      fflush(stdout);
      if (doCommand(pci_fd, TIM_ID, AEX, DON) == _ERROR) {
	fprintf(stderr,"Error aborting.\n");
	return(-1);
      }
      fprintf(stdout,"Succeeded?\n");
      return(0);
      }*/


    if (ftmp > (float)(conf->expose_time/1000.)) {
      fprintf(stdout,"ftmp: %f, conf->expose_time: %f\n",ftmp,(float)conf->expose_time/1000);
      break;
    }

    usleep(250);
  }
  
  num_pix_count = 0;
  last_pix_count = 0;
  timeoutctr=0;
  fprintf(stdout,"Starting readout...\n");
  while (num_pix_count < max_pix_count) {
    ioctl(pci_fd, ASTROPCI_GET_PROGRESS, &num_pix_count);
    /*fprintf(stdout,"Current pixel: %d\n", num_pix_count);*/
    if (num_pix_count == last_pix_count) {
      timeoutctr++;
    }

    if ((num_pix_count > 1000000) && (abort)) {
      fprintf(stdout,"Trying to abort readout...");
      if ((retval = hcvr(pci_fd, ABORT_READOUT, UNDEFINED, UNDEFINED)) == _ERROR) {
	fprintf(stderr,"Failed, with retval = %d (0x%0x)", retval, retval);
	return(-1);
      }
      fprintf(stdout,"Succeeded?\n");
      return(0);
    }
    
    last_pix_count = num_pix_count;

    if (timeoutctr >= READ_TIMEOUT) {
      fprintf(stderr,"error: read timeout!\n");
      return(-1);
    }

    
    usleep(250);
  }

  fprintf(stdout,"Running do_range\n");
  do_range(buf,&min,&max,&mean);
  fprintf(stdout,"min = %d, max = %d, mean = %f\n",min,max,mean);
  /*fprintf(stdout,"Byte swapping?\n");
  swap_memory(buf);
  do_range(buf,&min,&max,&mean);
  fprintf(stdout,"min = %d, max = %d, mean = %f\n",min,max,mean);*/

  fprintf(stdout,"Deinterlacing image with mode %d\n",conf->deinterlace);
  if (deinterlace(conf->ncols,conf->nrows,buf,conf->deinterlace) == _ERROR) {
    fprintf(stdout,"error: deinterlace failed\n");
    return(-1);
  }

  sprintf(image_name,"%s%s",conf->path, conf->fname);

#ifdef WRITE
  fprintf(stdout,"Writing image...\n");
  if (writeimage(buf, conf->nrows, conf->ncols, image_name, 0.0, 0.0) == -1) {
    fprintf(stderr,"Error writing image\n");
  } else { fprintf(stdout,"Done\n"); }
#endif
  
  return(0);

}

int do_option(int pci_fd, void *buf, struct cam_config *conf, char option)
{
    struct cam_config local_conf;
    int result = 0;
    float temp;
    
    switch (option)
	{
	case QUIT_OPT:
	    /* Do nothing and quit */
	    result = 0;
	    break;
	case RESET_ALL_OPT:
	    /* Run the setup routine */
	    local_conf = *conf;
	    result = setup_defaults(pci_fd, conf);
	    break;
	case SET_GAIN_OPT:
	    result = read_gain_and_speed(conf);
	    result = set_gain_and_speed(pci_fd, conf->gain, conf->speed);
	    break;
	case IM_NAME_OPT:
	    result = read_image_name(conf);
	    break;
	case SET_PATH_OPT:
	    result = read_image_path(conf);
	    break;
	case SET_TIME_OPT:
	    result = read_time(conf,1000);
	    result = set_expo_time(pci_fd, conf->expose_time, conf->controller_master_value);
	    break;
	case SET_TIME_SHORT:
	  result = read_time(conf,1);
	  result = set_expo_time(pci_fd,conf->expose_time,conf->controller_master_value);
	  break;
	case SET_MPP_OPT:
	    result = read_mpp(conf);
	    result = set_mpp_mode(pci_fd, conf->mpp);
	    break;
	case SET_SER_OPT:
	    result = read_serial_mode(conf);
	    result = set_serial_mode(pci_fd, conf->serial_mode, &(conf->deinterlace));
	    break;
	case SET_DIM_OPT:
	  /*result = read_dim(conf);
	    result = set_dimensions(pci_fd, conf->nrows, conf->ncols);
	    if (result != -1) {
		result = buffer_allocate(&buf, conf->nrows * conf->ncols);
		}*/
	    break;
	case START_EXP_OPT:
	    result = do_exposure(pci_fd, buf, conf, 0);
	    break;
	case EXPOSE_AND_ABORT:
	  result = do_exposure(pci_fd, buf, conf, 1);
	  break;
	case SET_SHUT_OPT:
	  result = read_shutter(pci_fd, conf);
	  break;
	case GET_TEMP_OPT:
	  fprintf(stdout,"Getting temperature...\n");
	  result = get_temp(pci_fd);
	  break;
	case SET_TEMP_OPT:
	  result = read_temp(conf);
	  result = set_temp(pci_fd, conf->target_temp);
	default:
	    /* Invalid Option */
	    fprintf(stdout,"%c. is an invalid option\n", option);
	}

    return(result);
}

char get_option(struct cam_config conf)
{
    char option[10];

    fprintf(stdout,"\n\t%c.: Reset Everything\n", RESET_ALL_OPT);
    fprintf(stdout,"\t%c.: Set File Path\n", SET_PATH_OPT);
    fprintf(stdout,"\t%c.: Set File Name\n", IM_NAME_OPT);
    fprintf(stdout,"\t%c.: Set Gain and Speed\n", SET_GAIN_OPT);
    fprintf(stdout,"\t%c.: Get TEC Voltage\n", GET_TEC_VOLTAGE);
    fprintf(stdout,"\t%c.: Set Dimensions\n", SET_DIM_OPT);
    fprintf(stdout,"\t%c.: Set MPP Mode\n", SET_MPP_OPT);
    fprintf(stdout,"\t%c.: Set Shutter Open/Closed\n", SET_SHUT_OPT);
    fprintf(stdout,"\t%c.: Test Shutter\n", TEST_SHUTTER);
    fprintf(stdout,"\t%c.: Set Temperature\n", SET_TEMP_OPT);
    fprintf(stdout,"\t%c.: Get Temperature\n", GET_TEMP_OPT);
    fprintf(stdout,"\t%c.: Set Serial Mode\n", SET_SER_OPT);
    fprintf(stdout,"\t%c.: Set Exposure Time (sec)\n", SET_TIME_OPT);
    fprintf(stdout,"\t%c.: Set Short Exposure Time (msec)\n", SET_TIME_SHORT);
    fprintf(stdout,"\t%c.: Start Exposure\n", START_EXP_OPT);
    fprintf(stdout,"\t%c.: Start Exposure then Abort\n", EXPOSE_AND_ABORT);
    fprintf(stdout,"\t%c.: Quit\n", QUIT_OPT);
    fprintf(stdout,"\nFilename: %s%s  Dimensions %dx%d\n", conf.path, conf.fname, conf.ncols, conf.nrows);
    fprintf(stdout,"MPP mode: %d  Serial Mode: %d  Deinterlace Mode: %d\n", conf.mpp, conf.serial_mode, conf.deinterlace);
    fprintf(stdout,"Gain: %d  Speed: %d  Exposure Time: %d  Target Temp: %f\n", conf.gain, conf.speed, conf.expose_time, conf.target_temp);
    fprintf(stdout,"Shutter: %d\n", conf.open_shutter);
    fprintf(stdout,"\n\t\tEnter Option: ");

    fflush(stdin);
    
    fgets(option, 3, stdin);
    fprintf(stdout,"\n");
    fflush(stdin);
    
    return(option[0]);
}

int setup_defaults(int pci_fd, struct cam_config *conf)
{
    int result;
    char lodfile[100];
    int retval;
    int configword;
    int i;
    int data = 0;

    fprintf(stdout,"In setup defaults.\n");

    /* Load PCI boot application */
    /*  This is not necessary if the rom has burned */
    if (LOAD_PCI_LOD == 1) {
      sprintf(lodfile,"%s/pci.lod",LOD_FILE_DIR);
      fprintf(stdout,"Loading PCI application %s...\n",lodfile);
      result = loadPciFile(pci_fd, lodfile);
      if (result == _ERROR) {
	fprintf(stderr,"error loading pci file.\n");
	fprintf(stderr,"%s\n",str_reply_value);
	return(-1);
      } else { fprintf(stdout,"Done\n"); }
    }

    /*fprintf(stdout,"Resetting controller...\n");
    if (hcvr(pci_fd, RESET_CONTROLLER, UNDEFINED, SYR) == _ERROR) {
      fprintf(stdout,"failed!\n");
      return(-1);
    } else fprintf(stdout,"done.\n");
    */

    /* Load Timing Application */
    
    fprintf(stdout,"Loading Timing File...\n");
    sprintf(lodfile,"%s/tim.lod", LOD_FILE_DIR);
    fprintf(stdout,"Loading Timing application %s...\n", lodfile);
    result = loadFile(pci_fd, lodfile, "timing");
    if (result == _ERROR) {
	fprintf(stderr,"error loading timing File.%X\n",getError());
	fprintf(stderr,"%s\n",str_reply_value);
	return(-1);
    } else { fprintf(stdout,"Done\n"); }
    
    /* Load Utility Application */ /* perhaps */
    if (conf->do_util) {
      fprintf(stdout,"Loading Utility file...\n");
      sprintf(lodfile,"%s/util.lod", LOD_FILE_DIR);
      fprintf(stdout,"Loading Utility application %s...\n",lodfile);
      result = loadFile(pci_fd, lodfile, "utility");
      if (result == _ERROR) {
	fprintf(stderr, "error loading utility file.%X\n",getError());
	fprintf(stderr,"%s\n",str_reply_value);
	return(-1);
      } else { fprintf(stdout,"Done\n"); }
    }
    
    /* do some tests (?) */
    fprintf(stdout,"Doing 10 PCI tests\n");
    for (i=0;i<10;i++) {
      if (doCommand1(pci_fd, PCI_ID, TDL, data, data) == _ERROR) {
	printf("error\n");
      }
      data += HARDWARE_DATA_MAX/10;
    }
    fprintf(stdout,"Done\n");

    /*
    printf("Doing 10 Tim tests\n");
    for (i=0;i<10;i++) {
      if (doCommand1(pci_fd, TIM_ID, TDL, data, data) == _ERROR) {
	printf("error\n");
      }
      data += HARDWARE_DATA_MAX/10;
    }
    */

    /* Turn Power On */
    fprintf(stdout,"Turning Power On...\n");
    if (doCommand(pci_fd, conf->controller_master_value, PON, DON) == _ERROR) {
      fprintf(stderr,"Error turning power on.%X\n",getError());
      return(-1);
    } else { fprintf(stdout,"Done\n"); }
    
    /* Run dimension procedure */
    if (set_dimensions(pci_fd, conf->nrows, conf->ncols) == -1) {
	return(-1);
    }

   /* we interrupt here... */
    configword = doCommand(pci_fd, TIM_ID, RCC, UNDEFINED);
    if ((configword & MPP_MASK) == MPP_CAPABLE)
      fprintf(stdout,"Can do mpp\n");
    if ((configword & READOUT_MASK) == SERIAL)
      fprintf(stdout,"Serial ok\n");
    if ((configword & READOUT_MASK) == PARALLEL)
      fprintf(stdout,"Parallel ok\n");
    if ((configword & READOUT_MASK) == BOTH_READOUTS)
      fprintf(stdout,"Both ok\n");

    if ((configword & VIDEO_PROCESSOR_MASK) == CCD_REV3B)
      fprintf(stdout,"do_gain ok\n");

    if ((configword & TEMPERATURE_READOUT_MASK) == NO_TEMPERTURE_CONTROL) {
      fprintf(stdout,"no temp control\n");
      conf->temp_control = 0;
    } else {
      conf->temp_control = 1;
    }
    
    if (conf->temp_control) {
      fprintf(stdout,"Setting temp algorithm %f, %f\n", conf->temp_coeff_0, conf->temp_coeff_1);
      if (set_algorithm(LINEAR) == _ERROR) {
	fprintf(stderr,"Error setting algorithm to LINEAR\n");
	return(-1);
      }
      if (set_linear_coefficients(conf->temp_coeff_0, conf->temp_coeff_1) == _ERROR) {
	fprintf(stderr,"Error setting coefficients\n");
	return(-1);
      }

      if (set_temp(pci_fd, conf->target_temp) != 0) {
	fprintf(stderr,"Error setting temperature\n");
	return(-1);
      }
    }

    /* Set serial mode */
    if (set_serial_mode(pci_fd, conf->serial_mode, &(conf->deinterlace)) == -1) {
	return(-1);
    }


    /* Run gain/speed procedure */
    if (set_gain_and_speed(pci_fd, conf->gain, conf->speed) == -1) {
	return(-1);
    }
    
    /* Set MPP mode */
    /*
    if (set_mpp_mode(pci_fd, conf->mpp) == -1) {
      return(-1);
    }
    */

    return(0);

}

int set_serial_mode(int pci_fd, int serial_mode, int *deinterlace)
{
    int mode_code;

    switch(serial_mode) {
    case LEFT:
	fprintf(stdout,"Setting serial mode to L (Left Amp)\n");
	/*mode_code = SOS_L;*/
	mode_code = L_AMP;
	*deinterlace = 0;
	break;
    case RIGHT:
	fprintf(stdout,"Setting serial mode to R (Right Amp)\n");
	/*mode_code = SOS_R;*/
	mode_code = R_AMP;
	*deinterlace = 0;
	break;
    case BOTH:
	fprintf(stdout,"Setting serial mode to LR (Both amps)\n");
	/*mode_code = SOS_LR;*/
	mode_code = LR_AMP;
	*deinterlace = 2;
	break;
    default:
	fprintf(stderr,"error: invalid serial mode\n");
	return(-1);
	break;
    }

    if (doCommand1(pci_fd, TIM_ID, SOS, mode_code, DON) == _ERROR) {
      fprintf(stdout,"Nope!\n");
      return(-1);
    }

    return(0);
}

int set_mpp_mode(int pci_fd, int mpp)
{
    fprintf(stdout,"Setting MPP mode to %d\n", mpp);

    if (doCommand1(pci_fd, TIM_ID, MPP, mpp, DON) == _ERROR) {
      fprintf(stderr,"Error!\n");
      return(-1);
    }

    return(0);
}

int set_dimensions(int pci_fd, int nrows, int ncols)
{
    fprintf(stdout,"Setting dimensions to %dx%d\n", ncols, nrows);

    if (doCommand2(pci_fd, TIM_ID, WRM, (Y | 1), ncols, DON) == _ERROR) {
      fprintf(stderr,"Error\n");
      return(-1);
    }
    if (doCommand2(pci_fd, TIM_ID, WRM, (Y | 2), nrows, DON) == _ERROR) {
      fprintf(stderr,"Error\n");
      return(-1);
    }

    return(0);
}

int set_gain_and_speed(int pci_fd, int gain, int speed)
{
    fprintf(stdout,"Setting gain and speed...\n");
 
    if (doCommand2(pci_fd, TIM_ID, SGN, gain, speed, DON) == _ERROR) {
      fprintf(stderr,"Error\n");
      return(-1);
    }

    return(0);
}

int set_expo_time(int pci_fd, int expo_time, int controller_master_value)
{
  /* setting exposure time */
  fprintf(stdout,"Setting exposure time to %d seconds...\n",expo_time/1000);

  if (controller_master_value == TIM_ID) {
    if (doCommand1(pci_fd, TIM_ID, SET, expo_time, DON) == _ERROR) {
      fprintf(stdout,"Failed to set\n");
      return(-1);
    }
  } else {
    if (doCommand2(pci_fd, UTIL_ID, WRM, (Y | 0x18), expo_time, DON) == _ERROR) {
      fprintf(stdout,"Error setting exposure time\n");
    }
  }


  return(0);
}

/*int set_shutter_position(int pci_fd, int open_shutter)
{
  int current_status = 0;

  current_status = doCommand1(pci_fd, TIM_ID, RDM, (X | 0), UNDEFINED);

  fprintf(stdout,"currently=%d\n",current_status);

  if (open_shutter) {
    fprintf(stdout,"open\n");
    if (doCommand2(pci_fd, TIM_ID, WRM, (X | 0), 
		   (current_status | _OPEN_SHUTTER_), DON) == _ERROR) {
      return(-1);
    }
  } else if (!open_shutter) {
    fprintf(stdout,"!open\n");
    if (doCommand2(pci_fd, TIM_ID, WRM, (X | 0),
		   (current_status & _CLOSE_SHUTTER_), DON) == _ERROR) {
      return(-1);
    }
  }
  return(0);
}*/

int set_shutter_position(int axfd, int open_shutter, int controller_master_value)
{
  int current_status = 0;

  if (controller_master_value == TIM_ID)
    current_status = doCommand1(axfd, TIM_ID, RDM, (X | 0), UNDEFINED);
  else 
    current_status = doCommand1(axfd, UTIL_ID, RDM, (X | 1), UNDEFINED);

  if (open_shutter) {
    if (controller_master_value == TIM_ID) {
      if (doCommand2(axfd, TIM_ID, WRM, (X | 0), 
		     (current_status | _OPEN_SHUTTER_), DON) == _ERROR) {
	return(-1);
      }
    } else {
      if (doCommand2(axfd, UTIL_ID, WRM, (X | 1), (current_status | 0x1), DON) == _ERROR) {
	return(-1);
      }
    }
  } else if (!open_shutter) {
    if (controller_master_value == TIM_ID) {
      if (doCommand2(axfd, TIM_ID, WRM, (X | 0),
		     (current_status & _CLOSE_SHUTTER_), DON) == _ERROR) {
	return(-1);
      }
    } else {
      if (doCommand2(axfd, UTIL_ID, WRM, (X | 1), (current_status & 0xFFFFFFFE), DON) == _ERROR) {
	return(-1);
      }
    }
  }
  return(0);
}



int read_gain_and_speed(struct cam_config *conf)
{
    int gain = -1;
    int speed = -1;
    char result[50];

    fprintf(stdout,"\nSet Gain and Speed\n");
    while (gain == -1) {
	fprintf(stdout,"\tEnter New Gain (1,2,5,10): ");
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	gain = (int) strtol(result, NULL, 0);
	if (gain != 1 && gain != 2 && gain != 5 && gain != 10) {
	    fprintf(stdout,"%d is an Illegal Value\n", gain);
	    gain = -1;
	}
    }
    while (speed == -1) {
	fprintf(stdout,"\tEnter New Speed (0,1): ");
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	speed = (int) strtol(result, NULL, 0);
	if (speed != 0 && speed != 1) {
	    fprintf(stdout,"%d is an Illegal Value\n", speed);
	    speed = -1;
	}
    }
    conf->gain = gain;
    conf->speed = speed;
    return(0);
}

int read_temp(struct cam_config *conf)
{
    double temp = -999.0;
    char result[50]="0.0";

    fprintf(stdout,"\nSet Target Temperature\n");
    while (temp < -30) {
	fprintf(stdout,"\tEnter new target temp (-30 - 30): ");
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	temp = atof(result);
	if (temp < -30 || temp > 30) {
	    fprintf(stdout,"%f is an illegal value\n", temp);
	    temp = -999;
	}
    }
    conf->target_temp = temp;
    return(0);
}

int read_image_name(struct cam_config *conf)
{
    char name[100];
    
    fprintf(stdout,"\nSet Image Name: ");
    fgets(name,100,stdin);
    fflush(stdin);
    fprintf(stdout,"\n");
    name[strlen(name)-1]='\0';
    strcpy(conf->fname,name);
    return(0);
}

int read_image_path(struct cam_config *conf)
{
    char path[100];
    
    fprintf(stdout,"\n\tSet Image Path: ");
    fgets(path,100,stdin);
    fflush(stdin);
    fprintf(stdout,"\n");
    path[strlen(path)-1]='\0';
    strcpy(conf->path,path);
    return(0);
}

int read_time(struct cam_config *conf, int scaling)
{
    int time = -1;
    char result[50];
 
    fprintf(stdout,"\nSet Exposure time.\n");
    while (time == -1) {
	fprintf(stdout,"\tEnter Exposure time (millisec/%d) (0-500s):",scaling);
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	time = (int) strtol(result, NULL, 0);
	if (time < 0 || time*scaling > 500*1000) {
	    fprintf(stdout,"%d is an illegal value\n", time);
	    time = -1;
	}
    }
    conf->expose_time = scaling * time;
    return(0);
}

int read_mpp(struct cam_config *conf)
{
    int mpp = -1;
    char result[50];

    fprintf(stdout,"\nSet MPP mode\n");
    while (mpp == -1) {
	fprintf(stdout,"\tEnter 1 for MPP on, 0 for MPP off:");
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	mpp = (int) strtol(result, NULL, 0);
	if (mpp != 0 && mpp != 1) {
	    fprintf(stdout,"%d is an illegal value\n", mpp);
	    mpp = -1;
	}
    }
    conf->mpp = mpp;
    return(0);
}


int read_shutter(int pci_fd, struct cam_config *conf)
{
    int shutter = -1;
    char result[50];

    fprintf(stdout,"\nSet Shutter mode\n");
    while (shutter == -1) {
	fprintf(stdout,"\tEnter 1 for Shutter open, 0 for shutter closed:");
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	shutter = (int) strtol(result, NULL, 0);
	if (shutter != 0 && shutter != 1) {
	    fprintf(stdout,"%d is an illegal value\n", shutter);
	    shutter = -1;
	}
    }
    conf->open_shutter = shutter;
  
    return(0);
}


int read_serial_mode(struct cam_config *conf)
{
    int serial_mode = -1;
    char result[50];

    fprintf(stdout,"\nSet serial mode\n");
    while (serial_mode == -1) {
	fprintf(stdout,"\tEnter %d for left amp, %d for right amp, %d for both amps:", 
	       LEFT, RIGHT, BOTH);
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	serial_mode = (int) strtol(result, NULL, 0);
	if (serial_mode != LEFT && serial_mode != RIGHT && serial_mode != BOTH) {
	    fprintf(stdout,"%d is an illegal value\n", serial_mode);
	    serial_mode = -1;
	}
    }
    conf->serial_mode = serial_mode;
    return(0);
}

int read_dim(struct cam_config *conf)
{
    int nrows = -1;
    int ncols = -1;
    char result[50];

    fprintf(stdout,"\nSet Image Dimensions\n");
    while (ncols == -1) {
	fprintf(stdout,"\tEnter New Ncols (x range 0 - 2500): ");
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	ncols = (int) strtol(result, NULL, 0);
	if (ncols < 0 || ncols > 2500) {
	    fprintf(stdout,"%d is an Illegal Value\n", ncols);
	    ncols = -1;
	}
    }
    while (nrows == -1) {
	fprintf(stdout,"\tEnter New Nrows (y range 0 - 2500): ");
	fgets(result,50,stdin);
	fflush(stdin);
	fprintf(stdout,"\n");
	nrows = (int) strtol(result, NULL, 0);
	if (nrows < 0 || nrows > 2500) {
	    fprintf(stdout,"%d is an Illegal Value\n", nrows);
	    nrows = -1;
	}
    }
    conf->nrows = nrows;
    conf->ncols = ncols;
    return(0);
}

    
void close_and_free(int pci_fd, void *buffer)
{
  fprintf(stdout,"Closing device and freeing memory\n");
  /* close(pci_fd);
     free(buffer);*/
  free_memory(pci_fd, buffer);
}

void pof(int pci_fd)
{
    /* Turn Power Off */
  fprintf(stdout,"Turning power off...\n");
  
  


}
#ifdef WRITE
int writeimage(void *buf, int nrows, int ncols, char *filename, double start_temp, double end_temp)
{
  fitsfile *fptr;
  int status, i;
  long fpixel, nelements;

  int bitpix = USHORT_IMG;
  unsigned short *u_pix;

  long naxis = 2;
  long naxes[2] = {(long) ncols, (long) nrows};

  remove(filename);

  status = 0;
  u_pix = (unsigned short *) buf;

  if (fits_create_file(&fptr, filename, &status)) {
    printerror(status);
    return(-1);
  }

  if (fits_create_img(fptr, bitpix, naxis, naxes, &status)) {
    printerror(status);
    return(-1);
  }
 
  fprintf(stdout,"Created Image...\n");

  fpixel = 1;
  nelements = naxes[0] * naxes[1];
  fprintf(stdout,"nelements: %d\n", (int) nelements);

  for (i=0;i<4;i++) {
    if ( fits_delete_key(fptr, "COMMENT", &status) ) {
      /*printerror(status);*/
      status = 0;
    }
  }

  if (fits_update_key(fptr,TDOUBLE,"TEMP", &start_temp, "Start temperature", &status)) {
    printerror(status);
    return(-1);
  }
  if (fits_update_key(fptr,TDOUBLE,"TEMP_END", &end_temp, "End Temperature", &status)) {
    printerror(status);
    return(-1);
  }

  if (fits_write_img(fptr, TUSHORT, fpixel, nelements, u_pix, &status)) {
    printerror(status);
    return(-1);
  }


  fprintf(stdout,"wrote it out\n");

  if (fits_close_file(fptr,&status)) {
    printerror(status);
    return(-1);
  }

  return(0);

}
void printerror( int status)
{
  if (status) {
    fits_report_error(stderr,status);
    exit(status);
  }
  return;
}

#endif




int timediff(const struct timeval *tlast, float *delta_t)
{
   struct timeval tmptv;

   if (gettimeofday(&tmptv, NULL) == 0) {
      *delta_t = (float) (tmptv.tv_sec - tlast->tv_sec) +
          0.000001 * (tmptv.tv_usec - tlast->tv_usec);
   } else {
      *delta_t = 0.0;
      return (-1);
   }
   return (0);
}

void do_range(void *buf, int *min, int *max, float *mean)
{
  unsigned short *u_buf;

  int i,j;
  int count;
  float temp = 0.0;
  int ncols = 2200;
  int this_val;

  *min = 100000;
  *max = -100000;

  u_buf = (unsigned short *)buf;

  count = 0;
  for (i=1000;i<1100;i++) {
    for (j=1000;j<1100;j++) {
      this_val = (int) u_buf[i+j*ncols];
      if (this_val < *min) {
	*min = this_val;
      }
      if (this_val > *max) {
	*max = this_val;
      }
      temp = temp + (float) this_val;
      count++;
    }
  }
  *mean = temp / (float) count;

}


int set_temp(int pci_fd, double temp)
{
  int adu;

  fprintf(stdout,"Setting temperature to %f C\n", temp);

  adu = calculate_adu(temp) & 0x00000FFF;
  doCommand2(pci_fd, UTIL_ID, WRM, (Y | 0x1C), adu, UNDEFINED);

  return(0);
} 

int get_temp(int pci_fd)
{
  double temp;
  int adu;

  adu = doCommand1(pci_fd, UTIL_ID, RDM, (Y | 0xC), UNDEFINED);
  adu = adu & 0x00000FFF;

  temp = (double) calculate_temperature(adu);

  fprintf(stdout,"The temperature is %f C\n", temp);
  return(0);
}




