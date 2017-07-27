/******************************************************************************
* 			 
*  FILE:	mreadout_pci.h
*  VERSION:	1.00
*  AUTHOR:	Scott Streit
*  DATE:	02/05/2001
* 			 
*  DESCRIPTION:	Header file for mreadout_pci.c.
*
*  INPUT:	None
*
* 
*  REVISION HISTORY:
*	Date		Who	Version	Description
*  ---------------------------------------------------------------------------
* 	02/05/2001	sds	1.00	Initial
* 			 
******************************************************************************/
#ifndef _MREADOUT_PCI_H_
#define _MREADOUT_PCI_H_

#include "WhichSystem.h"
#include "Bool.h"

/* Device Driver */
#ifdef WIN2K
	#define PCI_DEVICE_NAME		"astropci1\0"
#else
	#define PCI_DEVICE_NAME		"/dev/astropci0\0"
#endif

/* Max TDL Data Value */
#define HARDWARE_DATA_MAX	1000000

/********************************************************
*	CONTROLLER SETUP STRUCTURE
********************************************************/
struct Setup
{
	boolean do_pci_file;
	boolean do_reset;
	boolean do_hardware_test;
	boolean do_pci_hardware_test;
	boolean do_tim_hardware_test;
	boolean do_util_hardware_test;
	boolean do_tim_app;
	boolean do_tim_file;
	boolean do_util_app;
	boolean do_util_file;
	boolean do_power_on;
	boolean do_dimensions;
	int bits_per_pixel;
	char pci_file[80];
	char tim_file[80];
	char util_file[80];
	int tim_app;
	int util_app;
	int num_pci_tests;
	int num_tim_tests;
	int num_util_tests;
	int cols;
	int rows;
	int controllerMaster;
	boolean byteSwapping;
	boolean validate;
};

/********************************************************
*	CONTROLLER PARAMETERS STRUCTURE
********************************************************/
struct ControllerParameters
{
	int configWord;
	boolean do_mpp;
	boolean do_readout_mode;
	boolean do_gain;
	boolean do_temperature;
	boolean do_continuousReadout;
	int mpp;
	int amp;
	int gain;
	int speed;
	int temperature;
	char temperature_algorithm[20];
	double linear_temp_coeff0;
	double linear_temp_coeff1;
	int framesPerBuffer;
	int numberOfFrames;
};

/********************************************************
*	EXPOSURE STRUCTURE
********************************************************/
struct Expose
{
	boolean do_open_shutter;
	boolean do_image_save;
	boolean do_deinterlace;
	int number_of_exposures;
	int exptime;
	char image_directory[100];
	char image_filename[20];
	int deinterlace_mode;
};

/* Function prototypes */
int do_controller_setup();
int doControllerParameters();
void setControllerParameters(int configWord);
int do_exposure();
int load_setup_file(char *filename);
int shutter_position();
void to_ascii(int value, char *ascii_value);
void done(HANDLE pci_fd, unsigned short *mem_fd);
int doCoadditionExposure();
int doWriteToDiskExposure();

#endif
