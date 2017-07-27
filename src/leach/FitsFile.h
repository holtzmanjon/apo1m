/******************************************************************************
*
*   File:       FitsFile.h
*   Version:    1.00
*   Author:     Jose Maria Panero Ciprian
*   Abstract:   include file for the C library for fits-file functions.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       07/10/00        jmp     1.00    Initial
*
******************************************************************************/
#ifndef FITSFILE_H
#define FITSFILE_H

#define MAX_FITS_CARD_LENGTH	40
#define MAX_FITS_HEADER_CARDS	36
#define BITS_PER_BYTE			8
#define FITS_BLOCK_SIZE			2880

#define HEADER_DEFAULT_SIMPLE	'T'
#define HEADER_DEFAULT_BITPIX	(int)16
#define HEADER_DEFAULT_NAXIS	(int)2
#define FILL_DATA				(unsigned short)0

/* ----------------------------------
	Define Globals
 ------------------------------------*/
extern char error_string[];

/******************************************************************************
*       definition of fits header struct
******************************************************************************/
struct fits_header_struct
{
	char simple;
	int  bitpix;
	int  naxis;
	int  naxis1;
	int  naxis2;
	char date[MAX_FITS_CARD_LENGTH];
	char time[MAX_FITS_CARD_LENGTH];
	char location[MAX_FITS_CARD_LENGTH];
	char sidetime[MAX_FITS_CARD_LENGTH];
	char epoch[MAX_FITS_CARD_LENGTH];
	char airmass[MAX_FITS_CARD_LENGTH];
	int  exp_time;
	char img_type[MAX_FITS_CARD_LENGTH];
	char telescope[MAX_FITS_CARD_LENGTH];
	char instrment[MAX_FITS_CARD_LENGTH];
	char filter[MAX_FITS_CARD_LENGTH];
	char object[MAX_FITS_CARD_LENGTH];
	char ra[MAX_FITS_CARD_LENGTH];
	char dec[MAX_FITS_CARD_LENGTH];
	char observer[MAX_FITS_CARD_LENGTH];
	char comment1[MAX_FITS_CARD_LENGTH];
	char comment2[MAX_FITS_CARD_LENGTH];
	char comment3[MAX_FITS_CARD_LENGTH];
	char comment4[MAX_FITS_CARD_LENGTH];
};

/******************************************************************************
*       Function Prototypes
******************************************************************************/
int write_fits_file (const char *filename, unsigned short *image_fd, struct fits_header_struct fits_header);
int writeFitsFile32(const char *filename, unsigned int *image_fd, struct fits_header_struct fits_header);
int write_fits_header (FILE *fd_out, struct fits_header_struct fits_header); 
int set_fits_header (struct fits_header_struct *fits_header);
int set_fits_header_time (struct fits_header_struct *fits_header);
int edit_fits_exposure_dimensions (struct fits_header_struct *fits_header, int rows, int cols, int exp_time);
FILE *createFitsFile (const char *filename, struct fits_header_struct fits_header);
void writeToFitsFile(FILE *fd, unsigned int image_fd, int skipBytes, int bytes);
void closeFitsFile(FILE *fd, int totalBytes);

#endif

