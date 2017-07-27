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

#include "leach/Driver.h"
#include "leach/DSPCommand.h"
#include "leach/Deinterlace.h"
#include "leach/LoadDspFile.h"
#include "leach/FitsFile.h"
#include "leach/Memory.h"
#include "leach/Temperature.h"
#include "leach/ErrorString.h"
#include "leach/ContinuousReadout.h"
#include "leach/apiTest.h"
#include "leach/WhichSystem.h"

char message[300];

#define WORD unsigned short

unsigned short *leach_initialize(int);
double get_temp(HANDLE);
void leach_close();
void update_status();
void xtv_refresh();

int prior, old_prior;

#undef MAIN
#ifdef MAIN
main()
{
  int ncol, nrow, nbytes;
  double exptime = 5.;
  int use_shutter = 1;
  int fast = 0;
  int xs, xe, ys, ye, writeout;
  unsigned char *buf;
  struct timeval starttime;
  struct timezone startzone;

  buf = leach_initialize(0);
  leach_expose(buf, exptime, use_shutter, fast, xs, xe, ys, ye, writeout,&starttime);
  leach_close();

  exit(0);
}
#else
extern int ctrlc;
extern int doalert;
#endif

HANDLE pci_fd = 0;
unsigned short *mem_fd;
int bufferSize;
double camtemp = 0;
double targettemp = 0;
struct Setup setup = {
        TRUE,                   /* do pci file                  */
        TRUE,                   /* do reset                     */
        TRUE,                   /* do hardware test             */
        TRUE,                   /* do pci hardware test         */
        TRUE,                   /* do tim hardware test         */
        TRUE,                   /* do util hardware test        */
        FALSE,                  /* do tim app                   */
        TRUE,                   /* do tim file                  */
        FALSE,                  /* do util app                  */
        TRUE,                   /* do util file                 */
        TRUE,                   /* do power on                  */
        TRUE,                   /* do dimensions                */
        16,                     /* 16 bits/pixel                */
        "/home/tcomm/1m/src/leach/pci.lod",     /* pci lod file    */
        "/home/tcomm/1m/src/leach/tim.lod",     /* timing lod file */
        "/home/tcomm/1m/src/leach/util.lod",    /* utility lod file*/
        0,                      /* tim app                      */
        0,                      /* util app                     */
        1055,                   /* number pci tests             */
        1055,                   /* number tim tests             */
        10,                     /* number util tests            */
        2200,                   /* image cols                   */
        2048,                   /* image rows                   */
        TIM_ID,                 /* controller master            */
        FALSE,                  /* hardware byte swapping (enabled if USECFITSIO defined) */
        TRUE                    /* validate timing/utility download */
};
struct ControllerParameters contParams = {
        0,                      /* configuration word           */
        FALSE,                  /* do mpp                       */
        TRUE,                   /* do readout_mode              */
        FALSE,                  /* do gain                      */
        FALSE,                  /* do temperature               */
        FALSE,                  /* do continuous readout        */
        FALSE,                  /* mpp                          */
#ifdef DUALAMP
        LR_AMP,                 /* amp                          */
#else
        L_AMP,                  /* amp                          */
#endif
        TEN,                    /* gain                         */
        FAST,                   /* speed                        */
        -20,                    /* temperature deg C            */
        "nonlinear",            /* temperature_algorithm        */
        0.0,                    /* linear_temp_coeff0           */
        0.0,                    /* linear_temp_coeff1           */
        0,                      /* Frames-per-buffer            */
        0                       /* Number of frames             */
};

unsigned short *leach_initialize(int fast)
{
  int status;
  unsigned short error;
  double t_setpoint = -125., camtemp;

#ifndef no_hardware

// Initialize Leach
  fprintf(stderr,"opening camera\n");
  pci_fd = openDriver(PCI_DEVICE_NAME);
  fprintf(stderr,"pci_fd: %d\n",pci_fd);
  bufferSize = 2200*2200*2;

  if ((mem_fd = create_memory(pci_fd, setup.rows, setup.cols, bufferSize)) == NULL) {
    fprintf(stderr,"Unable to create image buffer: 0x%X (%d)\n", mem_fd, mem_fd);
    fprintf(stderr,"\tERROR: %s\n", error_string);
    done(pci_fd, mem_fd);
    exit(EXIT_FAILURE);
  }

 /* SET (HARDWARE) BYTE SWAPPING */
  setup.byteSwapping = setHardwareByteSwapping(pci_fd);

  fprintf (stderr,"Image buffer of size %d successfully created, mem_fd: 0x%X\n", setup.rows*setup.cols, mem_fd);

  if (fast) 
    sprintf(setup.tim_file,"/home/tcomm/1m/src/leach/new/timFast.lod");
  else
    sprintf(setup.tim_file,"/home/tcomm/1m/src/leach/tim.lod");

  if (do_controller_setup() == _ERROR) {
          fprintf(stderr,"Closing driver connection\n");
          done(pci_fd, mem_fd);
          exit(EXIT_FAILURE);
  }


  /* Apply controller parameters */
  if (doControllerParameters() == _ERROR) {
          fprintf(stderr,"Closing driver connection\n");
          done(pci_fd, mem_fd);
          exit(EXIT_FAILURE);
  }

  camtemp=get_temp(pci_fd);

// Turn on cooler
  set_temp(pci_fd,t_setpoint); /* useless return value */
#endif

  return(mem_fd);
}

void leach_close()
{
    int status;
   // temperature to set CCD to when program is not running
    double t_setpoint = -150.; 

    status = set_leach_temp(t_setpoint);
    fprintf(stderr,"Closing driver connection and unmapping image buffer.\n");
    done(pci_fd, mem_fd);
    return;
}

char *holdfile = "/home/export/tocc/fochold.doc";

// Function to expose Leach and read image into buffer
int leach_expose(WORD *buf, double exptime, int use_shutter, int fast,
               int xs, int xe, int ys, int ye, int writeout, struct timeval *starttime, int port, char *server)
{
  int status, init=0, ifile;
  FILE *fp;
  time_t start, current;
  int alert;
  int nx, ny, nbias, x0, y0, b0;
  int currentStatus = 0;
  int currentPixelCount = 0;
  int lastPixelCount = 0;
  int exposureCounter = 0;
  int elapsed_time = 0;
  float elapsed_time_float = 0.;
  int maxPixelCount = setup.rows * setup.cols;
  int readout_begun = 0;
  int timeoutCounter = 0;
  struct timezone startzone;
  char newcom[80],ret[80];

#ifdef VERBOSE
  gettimeofday(starttime,&startzone);
  fprintf(stderr,"Starting leach_expose: %f\n",starttime->tv_sec+starttime->tv_usec/1.e6);
#endif
  fp = fopen(holdfile,"w");
  if (fp != NULL) fclose(fp);

  /* shutter */
  /* Ask for the current controller status. */
  fprintf(stderr,"Setting shutter position: %d...\n", use_shutter);
  if (setup.controllerMaster == TIM_ID)
    currentStatus = doCommand1(pci_fd, TIM_ID, RDM, (X | 0), UNDEFINED);
  else
    currentStatus = doCommand1(pci_fd, UTIL_ID, RDM, (X | 1), UNDEFINED);

  if (use_shutter && exptime > 0.05) {
    if (setup.controllerMaster == TIM_ID) {
      if (doCommand2(pci_fd, TIM_ID, WRM, (X | 0), (currentStatus | _OPEN_SHUTTER_), DON) == _ERROR)
       return _ERROR;
    } else {
      if (doCommand2(pci_fd, UTIL_ID, WRM, (X | 1), (currentStatus | 0x1), DON) == _ERROR)
       return _ERROR;
    }
  }
  else if (!use_shutter) {
      if (setup.controllerMaster == TIM_ID) {
        if (doCommand2(pci_fd, TIM_ID, WRM, (X | 0), (currentStatus & _CLOSE_SHUTTER_), DON) == _ERROR)
         return _ERROR;
      } else {
        if (doCommand2(pci_fd, UTIL_ID, WRM, (X | 1), (currentStatus & 0xFFFFFFFE), DON) == _ERROR)
         return _ERROR;
      }
  }

  fprintf(stderr,"Setting exposure time: %f sec.\n", exptime);
  if (setup.controllerMaster == TIM_ID) {
    if (doCommand1(pci_fd, TIM_ID, SET, (int)(exptime*1000), DON) == _ERROR) {
      fprintf(stderr,"\tERROR: Setting exposure time failed -> 0x%X\n", getError());
      return _ERROR;
    }
  } else {
    if (doCommand2(pci_fd, UTIL_ID, WRM, (Y | 0x18), (int)(exptime*1000), DON) == _ERROR) {
      fprintf(stderr,"\tERROR: Setting exposure time failed -> 0x%X\n", getError());
      return _ERROR;
    }
  }

  nbias = 20;
  nx = xe-xs+1-nbias;
  ny = ye-ys+1;
  x0 = xs;
  y0 = ys;
  b0 = 2100-xe;
  b0 = 2100;

/*
  if (nbias+nx != setup.cols || ny != setup.rows) {
    nbias=50;
    nx = xe-xs+1;
    ny = ye-ys+1;
    x0 = xs;
    y0 = ys;
    b0 = 2110;
  }
*/

  maxPixelCount = nx*ny;

  fprintf(stderr,"Setting image columns %d ...", nbias+nx);
  if (doCommand2(pci_fd, TIM_ID, WRM, (Y | 1), nbias+nx, DON) == _ERROR) {
    fprintf(stderr,"failed -> 0x%X\n", getError());
    return _ERROR;
  } else
    fprintf(stderr,"done.\n");

  fprintf(stderr,"Setting image rows %d ...", ny);
  if (doCommand2(pci_fd, TIM_ID, WRM, (Y | 2), ny, DON) == _ERROR) {
    fprintf(stderr,"failed -> 0x%X\n", getError());
    return _ERROR;
  } else
    fprintf(stderr,"done.\n");

  if (setup.controllerMaster == TIM_ID) {
    fprintf(stderr,"SSS parameters: %d %d %d  ",nbias,nx,ny);
    if (doCommand3(pci_fd, TIM_ID, SSS, nbias, nx, ny, DON) == _ERROR) {
        fprintf(stderr,"\tERROR: Setting window size failed -> 0x%X\n", getError());
        return _ERROR;
    }

    // Windowing
    if (nbias+nx != setup.cols || ny != setup.rows) {
    //nx -= (setup.cols - setup.rows);
/*
    nbias=50;
    nx -= nbias;
*/
      maxPixelCount = (nx+nbias)*ny;
/*
    x0 = 100;
    y0 = 100;
    b0 = 2100;
*/
      fprintf(stderr,"SSP parameters: %d %d %d\n",y0,x0,b0);
      if (doCommand3(pci_fd, TIM_ID, SSP, y0, x0, b0, DON) == _ERROR) {
        fprintf(stderr,"\tERROR: Setting window position failed -> 0x%X\n", getError());
        return _ERROR;
      }
    }
/*
    if (do_controller_setup() == _ERROR) {
          fprintf(stderr,"Closing driver connection\n");
          done(pci_fd, mem_fd);
          exit(EXIT_FAILURE);
    }
    if (doControllerParameters() == _ERROR) {
          fprintf(stderr,"Closing driver connection\n");
          done(pci_fd, mem_fd);
          exit(EXIT_FAILURE);
    }
*/
  }

  gettimeofday(starttime,&startzone);
  fprintf(stderr,"Starting exposure: %f\n",starttime->tv_sec+starttime->tv_usec/1.e6);
  if (doCommand(pci_fd, setup.controllerMaster, SEX, DON) == _ERROR) {
    fprintf(stderr,"\tERROR: Starting exposure failed -> 0x%X\n", getError());
    return _ERROR;
  }

  fprintf(stderr,"Elapsed Time: \n");
  alert = FALSE;
  while (!ctrlc && !alert && currentPixelCount < maxPixelCount) {

    status = (getHstr(pci_fd) & HTF_BITS) >> 3;
    /*fprintf(stderr,"status: %d\n", status);*/

    /* READ ELAPSED EXPOSURE TIME */
    if (status != READOUT && exposureCounter >= 20 && (int)(exptime*1000) > 0) {

        if (setup.controllerMaster == TIM_ID)
          elapsed_time = doCommand(pci_fd, TIM_ID, RET, UNDEFINED);
        else
          elapsed_time = doCommand1(pci_fd, UTIL_ID, RDM, (Y | 0x17), UNDEFINED);

        elapsed_time_float = (float)elapsed_time/1000.0;
        if (readout_begun == 0) {
          fprintf(stderr," %f %d %d\n", elapsed_time_float, ctrlc, alert);
        }

        /* Make sure the controller has started the timer. */
        if (elapsed_time < 0) elapsed_time = 0;

        exposureCounter = 0;
        timeoutCounter = 0;
    } else {
        exposureCounter++;
        /* READOUT */
        if ((status == READOUT)&&(readout_begun==0)) {
          fprintf(stderr,"Beginning readout...\n");
          readout_begun=1;
//          sprintf(newcom,"FILTER 6");
//fprintf(stderr,"sending: %s to %d %s\n",newcom,port,server);
//          status = sendport(port,server,newcom,ret,80);

        }
        ioctl(pci_fd, ASTROPCI_GET_PROGRESS, &currentPixelCount);
        /*fprintf(stderr,"Current pixel count: %d\n", currentPixelCount);*/

        /* Increment the timeout counter. */
        if (currentPixelCount == lastPixelCount) timeoutCounter++;

        lastPixelCount = currentPixelCount;

        if (timeoutCounter >= READ_TIMEOUT) {
          fprintf(stderr,"ERROR: Readout TIMEOUT: %d %d!\n",currentPixelCount, maxPixelCount);
          leach_close();
          ccd_initialize();
          return _ERROR;
        }

        usleep(25000);
    }
    fp = fopen("/home/tcomm/alert/alert.int","r");
    if (fp != NULL) {
      if (doalert) alert = TRUE;
      fclose(fp);
      remove("/home/tcomm/alert/alert.int");
    }
    update_status(1);
  }
  if (ctrlc || alert) {
    leach_close();
    ccd_initialize();
    update_status(-1);
    return(2);
  }
  remove(holdfile);
  if (!fast) update_status(4);
  status=0;

  return(status);
}


int leach_temp(double *temp, int *tempstatus)
{
  *temp = get_temp(pci_fd);
  *tempstatus = 0;
  return(0); 
}
double get_temp(HANDLE pci_fd)
{
  double temp;
  int adu;
  adu = doCommand1(pci_fd, UTIL_ID, RDM, (Y | 0xC), UNDEFINED);
  adu = adu & 0x00000FFF;
  temp = (double) calculate_temperature(adu);
  //fprintf(stderr,"The temperature is %f C, ADU: %d %f\n", temp,adu,calculate_temperature(adu));
  return(temp);
}
int set_leach_temp(double temp)
{
  int status;
  status = set_temp(pci_fd, temp);
  return(status);
}
int set_temp(HANDLE pci_fd, double temperature)
{
  int adu;
  adu = calculate_adu(temperature) & 0x00000FFF;
  fprintf(stderr,"Setting temperature to %f C, ADU: %d\n", temperature,adu);
  doCommand2(pci_fd, UTIL_ID, WRM, (Y | 0x1C), adu, UNDEFINED);
  return(0);
}


/***********************************************************************
*	DO CONTROLLER SETUP
***********************************************************************/
int do_controller_setup()
{
	int data = 1;
	int i = 0;

	/* -----------------------------------
	   LOAD PCI FILE
	   ----------------------------------- */
	if (setup.do_pci_file) {
		fprintf(stderr,"Loading PCI file %s...", setup.pci_file);
		if (loadPciFile(pci_fd, setup.pci_file) == _ERROR) {
			fprintf(stderr,"failed. -> Could not load pci file.\n");
			return _ERROR;
		}
		else {
			fprintf(stderr,"done.\n");

			/* -----------------------------------
	   		   SET (HARDWARE) BYTE SWAPPING
	   		----------------------------------- */
			setup.byteSwapping = setHardwareByteSwapping(pci_fd);
		}
	}

	/* -----------------------------------
	   RESET CONTROLLER
	   ----------------------------------- */
	if (setup.do_reset) {
		fprintf(stderr,"Resetting controller...");
		if (hcvr(pci_fd, RESET_CONTROLLER, UNDEFINED, SYR) == _ERROR) {
			fprintf(stderr,"failed. -> Could not reset controller.\n");
			return _ERROR;
		}
		else
			fprintf(stderr,"done.\n");
	}
	
	/* -----------------------------------
	   HARDWARE TESTS
	   ----------------------------------- */
	if (setup.do_hardware_test) {
		/* PCI TESTS */
		if (setup.do_pci_hardware_test) {
			fprintf(stderr,"Doing %d PCI hardware tests...\n", setup.num_pci_tests);
			for (i=0; i<setup.num_pci_tests; i++) {
				if (doCommand1(pci_fd, PCI_ID, TDL, data, data) == _ERROR) {
					fprintf(stderr,"\tERROR: 0x%X\n", getError());
				}
				data += HARDWARE_DATA_MAX/setup.num_pci_tests;
			}
		}

		/* TIMING TESTS */
		if (setup.do_tim_hardware_test) {
			fprintf(stderr,"Doing %d timing hardware tests...\n", setup.num_tim_tests);
			for (i=0; i<setup.num_tim_tests; i++) {
				if (doCommand1(pci_fd, TIM_ID, TDL, data, data) == _ERROR) {
					fprintf(stderr,"\tERROR: 0x%X\n", getError());
				}
				data += HARDWARE_DATA_MAX/setup.num_tim_tests;
			}
		}

		/* UTILITY TESTS */
		if (setup.do_util_hardware_test) {
			fprintf(stderr,"Doing %d utility hardware tests...\n", setup.num_util_tests);
			for (i=0; i<setup.num_util_tests; i++) {
				if (doCommand1(pci_fd, UTIL_ID, TDL, data, data) == _ERROR) {
					fprintf(stderr,"\tERROR: 0x%X\n", getError());
				}
				data += HARDWARE_DATA_MAX/setup.num_util_tests;
			}
		}
	}
		
	/* -----------------------------------
	   LOAD TIMING FILE/APPLICATION
	   ----------------------------------- */
	if (setup.do_tim_file)  {
		fprintf(stderr,"Loading timing file %s...", setup.tim_file);
		loadFile(pci_fd, setup.tim_file, setup.validate);
		fprintf(stderr,"done.\n");
	}

	else if (setup.do_tim_app) {
		fprintf(stderr,"Loading timing application %d...", setup.tim_app);
		if (doCommand1(pci_fd, TIM_ID, LDA, setup.tim_app, DON) == _ERROR) {
			fprintf(stderr,"failed -> 0x%X\n", getError());
			return _ERROR;
		}
		else
			fprintf(stderr,"done.\n");
	}

	/* -----------------------------------
	   LOAD UTILITY FILE/APPLICATION
	   ----------------------------------- */
	if (setup.do_util_file)  {
		fprintf(stderr,"Loading utility file %s...", setup.util_file);
		loadFile(pci_fd, setup.util_file, setup.validate);
	}
	else if (setup.do_util_app) {
		fprintf(stderr,"Loading utility application %d...\n", setup.util_app);
		if (doCommand1(pci_fd, UTIL_ID, LDA, setup.util_app, DON) == _ERROR) {
			fprintf(stderr,"failed -> 0x%X\n", getError());
			return _ERROR;
		}
		else
			fprintf(stderr,"done.\n");
	}

	/* -----------------------------------
	   POWER ON
	   ----------------------------------- */
	if (setup.do_power_on) {
		fprintf(stderr,"Doing power on...");
		if (doCommand(pci_fd, setup.controllerMaster, PON, DON) == _ERROR) {
			fprintf(stderr,"failed -> 0x%X\n", getError());
			return _ERROR;
		}
		else
			fprintf(stderr,"done.\n");
	}

	/* -----------------------------------
	   SET DIMENSIONS
	   ----------------------------------- */
	if (setup.do_dimensions) {
		fprintf(stderr,"Setting image columns %d ...", setup.cols);
		if (doCommand2(pci_fd, TIM_ID, WRM, (Y | 1), setup.cols, DON) == _ERROR) {
			fprintf(stderr,"failed -> 0x%X\n", getError());
			return _ERROR;
		}
		else
			fprintf(stderr,"done.\n");

		fprintf(stderr,"Setting image rows %d ...", setup.rows);
		if (doCommand2(pci_fd, TIM_ID, WRM, (Y | 2), setup.rows, DON) == _ERROR) {
			fprintf(stderr,"failed -> 0x%X\n", getError());
			return _ERROR;
		}
		else
			fprintf(stderr,"done.\n");
	}

	/* --------------------------------------
	   READ THE CONTROLLER PARAMETERS INFO
	   -------------------------------------- */
	contParams.configWord = doCommand(pci_fd, TIM_ID, RCC, UNDEFINED);
	setControllerParameters(contParams.configWord);

	return _NO_ERROR;
}

/***********************************************************************
*	SET CONTROLLER PARAMETERS
*
*	See the Voodoo Programmers Manual available at www.astr-cam.com
*	for a complete listing of available controller parameters.
***********************************************************************/
void setControllerParameters(int configWord)
{
	int x;
	unsigned short *imageEndAddr;

	/* -----------------------------
	   Set MPP mode
	   ----------------------------- */
	if ((configWord & MPP_MASK) == MPP_CAPABLE)
		contParams.do_mpp = TRUE;
	else
		contParams.do_mpp = FALSE;

	/* -----------------------------
	   Set serial readout mode
	   ----------------------------- */
	if  (((configWord & READOUT_MASK) == SERIAL) ||
	     ((configWord & READOUT_MASK) == PARALLEL) ||
	     ((configWord & READOUT_MASK) == BOTH_READOUTS))
		contParams.do_readout_mode = TRUE;
	else
		contParams.do_readout_mode = FALSE;

	/* -----------------------------------
	   Set the integrator gain and speed
	   ----------------------------------- */
	if ((configWord & VIDEO_PROCESSOR_MASK) == CCD_REV3B)
		contParams.do_gain = TRUE;
	else
		contParams.do_gain = FALSE;

	/* -----------------------------
	   Set the array temperature
	   ----------------------------- */
	if ((configWord & TEMPERATURE_READOUT_MASK) != NO_TEMPERTURE_CONTROL) {
		contParams.do_temperature = TRUE;

		/* Set temperature conversion algorithm. */
		if ((configWord & TEMPERATURE_READOUT_MASK) == NONLINEAR_TEMP_CONV)
			strcpy(contParams.temperature_algorithm, "nonlinear");

		else if ((configWord & TEMPERATURE_READOUT_MASK) == LINEAR_TEMP_CONV)
			strcpy(contParams.temperature_algorithm, "linear");
	}
	else
		contParams.do_temperature = FALSE;

	/* -----------------------------
	   Set continuous readout
	   ----------------------------- */
	/*if ((configWord & CONTINUOUS_READOUT) == CONTINUOUS_READOUT) {*/
	if ((configWord & 0x000005) == 0x000005) {
		contParams.do_continuousReadout = TRUE;

		/* Determine the FPB, where each new image starts on a 1024 address boundary */
		x = 1;
		imageEndAddr = mem_fd + (setup.rows*setup.cols*(setup.bits_per_pixel/8));
		while (((int)(imageEndAddr + x) % 1024) != 0)
			x++;

		contParams.framesPerBuffer = (int)floor((double)(bufferSize/(imageEndAddr - mem_fd + x)));

		/* The framesPerBuffer must be evenly divisible by 4 for this mode, which requires
		   the buffer be sub-divided in quarters. */
		while (contParams.framesPerBuffer % 4 != 0)
			contParams.framesPerBuffer--;

		fprintf(stderr,"Frames-Per-Buffer set to: %d\n", contParams.framesPerBuffer);
	}
	else
		contParams.do_continuousReadout = FALSE;

	/* -----------------------------------
	   Summary
	   ----------------------------------- */
	fprintf(stderr,"\n");
	fprintf(stderr,"-----------------------------\n");
	fprintf(stderr,"Controller Parameters Summary\n");
	fprintf(stderr,"-----------------------------\n");
	fprintf(stderr,"Configuration Word: 0x%X\n", configWord);
	fprintf(stderr,"MPP AVAILABLE: %s\n", (contParams.do_mpp ? "TRUE" : "FALSE"));
	fprintf(stderr,"READOUT MODES AVAILABLE: %s\n", (contParams.do_readout_mode ? "TRUE" : "FALSE"));
	fprintf(stderr,"GAIN MODES AVAILABLE: %s\n", (contParams.do_gain ? "TRUE" : "FALSE"));
	fprintf(stderr,"TEMERATURE MODES AVAILABLE: %s\n", (contParams.do_temperature ? "TRUE" : "FALSE"));
}

/***********************************************************************
*	DO CONTROLLER PARAMETERS
***********************************************************************/
int doControllerParameters()
{
	char ascii_value[4];

	/* -----------------------------
	   Set MPP mode
	   ----------------------------- */
	if (contParams.do_mpp) {
		fprintf(stderr,"Setting MPP mode: %d...\n", contParams.mpp);
		if (doCommand1(pci_fd, TIM_ID, MPP, contParams.mpp, DON) == _ERROR) {
			fprintf(stderr,"\tERROR: 0x%X\n", getError());
			return _ERROR;
		}
	}
	
	/* -----------------------------
	   Set serial readout mode
	   ----------------------------- */
	if (contParams.do_readout_mode) {
		to_ascii(contParams.amp, ascii_value);
		fprintf(stderr,"Setting readout mode: %s...\n", ascii_value);
		if (doCommand1(pci_fd, TIM_ID, SOS, contParams.amp, DON) == _ERROR) {
			fprintf(stderr,"\tERROR: 0x%X\n", getError());
			return _ERROR;
		}
	}

	/* -----------------------------------
	   Set the integrator gain and speed
	   ----------------------------------- */
	if (contParams.do_gain) {
		fprintf(stderr,"Setting gain: %d, speed: %d...\n", contParams.gain, contParams.speed);
		if (doCommand2(pci_fd, TIM_ID, SGN, contParams.gain, contParams.speed, DON) == _ERROR) {
			fprintf(stderr,"\tERROR: 0x%X\n", getError());
			return _ERROR;
		}
	}

	/* -----------------------------
	   Set the array temperature
	   ----------------------------- */
	if (contParams.do_temperature) {
		/* Set the array temperature algorithm */
		fprintf(stderr,"Setting temperature algorithm to: %s...\n", contParams.temperature_algorithm);
		if (set_algorithm(contParams.temperature_algorithm) == _ERROR) {
			fprintf(stderr,"\tERROR: 0x%X\n", getError());
			return _ERROR;
		}

		/* By default, the temperature algorithm is set to "nonlinear". If linear algorithm
		   is required, set the algorithm to "linear" to set the coefficients. */
		if (strcmp(contParams.temperature_algorithm, "linear") == 0) {
			fprintf(stderr,"Setting linear temperature coeff0: %lf, coeff1: %lf...\n", contParams.linear_temp_coeff0, contParams.linear_temp_coeff1);
			if (set_linear_coefficients(contParams.linear_temp_coeff0, contParams.linear_temp_coeff1) == _ERROR) {
				fprintf(stderr,"\tERROR: 0x%X\n", getError());
				return _ERROR;
			}
		}

		fprintf(stderr,"Setting temperature: %d deg C...\n", contParams.temperature);
		if (doCommand2(pci_fd, UTIL_ID, WRM, (Y | 0x1C), calculate_adu(contParams.temperature), UNDEFINED) == _ERROR) {
			fprintf(stderr,"\tERROR: 0x%X\n", getError());
			return _ERROR;
		}
	}

	return _NO_ERROR;
}

void done(HANDLE pci_fd, unsigned short *mem_fd)
{
        closeDriver(pci_fd);
        free_memory(pci_fd, mem_fd);
        return;
}
/***********************************************************************
*       TO ASCII
*
*       Converts a 24-bit integer into it's 3 character ascii equivalent
*       for a command. Used to output more descriptive DSP command
*       values. The input value is checked to be between ascii 0x20 and
*       0x7E.
***********************************************************************/
void to_ascii(int value, char *ascii_value)
{
        ascii_value[0] = (char)((value & 0xff0000) >> 16);
        ascii_value[1] = (char)((value & 0x00ff00) >> 8);
        ascii_value[2] = (char)(value & 0x0000ff);
        ascii_value[3] = '\0';

        if ((int)ascii_value[0] < 0x20 || (int)ascii_value[0] > 0x7E)
                ascii_value[0] = (char)'?';

        if ((int)ascii_value[1] < 0x20 || (int)ascii_value[1] > 0x7E)
                ascii_value[1] = (char)'?';

        if ((int)ascii_value[2] < 0x20 || (int)ascii_value[2] > 0x7E)
                ascii_value[2] = (char)'?';
}


