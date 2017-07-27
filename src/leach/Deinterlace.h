/******************************************************************************
*
*   File:       Deinterlace.h
*   Version:	1.00
*   Author:     Scott Streit
*   Abstract:   Header file for Deinterlace.c
*		Defines deinterlace constants.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*	12/20/2001	sds	1.00	Initial
*
******************************************************************************/
#ifndef _DEINTERLACE_H
#define _DEINTERLACE_H

/* ----------------------------------
	Define Globals
 ------------------------------------*/
extern char error_string[];

/* ----------------------------------
	Define Deinterlace Modes
 ------------------------------------*/
#define NONE			0
#define SPLIT_PARALLEL	1
#define SPLIT_SERIAL	2
#define QUAD_CCD		3
#define QUAD_IR			4
#define CDS_QUAD_IR		5

/* ----------------------------------
	Function Prototypes
 ------------------------------------*/
int deinterlace(int cols, int rows, unsigned short *image_fd, int algorithm);
int deinterlace32(int cols, int rows, unsigned int *image_fd, int algorithm);

#endif
