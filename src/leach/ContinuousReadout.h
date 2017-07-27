/******************************************************************************
*
*   File:       ContinuousReadout.h
*   Version:    1.00
*   Author:     Scott Streit
*   Abstract:   Include file for Continuous Readout functions.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       03/04/2003      sds     1.00    Initial
*
******************************************************************************/
#ifndef _CONTINUOUS_READOUT_H_
#define _CONTINUOUS_READOUT_H_

#include "WhichSystem.h"

#ifdef WIN2K
	#include <windows.h>
	#include <winioctl.h>
#endif

/*------------------------------*/
/*	Globals						*/
/*------------------------------*/
extern char error_string[];

/*------------------------------*/
/*	Function Prototypes			*/
/*------------------------------*/
void coaddition(HANDLE pci_fd, int framesPerBuffer, int numberOfFrames, unsigned short *pixels, unsigned int *totalBuffer, int pixelCount, int byteSwapping);
void add(HANDLE pci_fd, int src_fd, int dest_fd, int n, int m);
void writeToDisk(HANDLE pci_fd, unsigned short *mem_fd, char *image_directory, char *image_filename, int rows, int cols, int numberOfFrames, int framesPerBuffer, int exptime, int byteSwapping, int do_deinterlace, int deinterlace_mode);
void incrementFilename(char *filename);

#endif
