/******************************************************************************
*
*   File:       FitsFile.c
*   Version:    1.00
*   Author:     Jose Maria Panero Ciprian
*   Abstract:   C library for fits-file functions.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       07/10/00        jmp     1.00 	Initial
*		01/03/02		sds		1.7		Modified to be a win2k static library.
*
******************************************************************************/
#include <stdio.h>		/*  for SEEK_SET						*/
#include <time.h>		/*  get the date for the file's header	*/
#include "FitsFile.h"	/*  FitsFile module include file		*/
#include "ErrorString.h"
#include "DSPCommand.h"

/*******************************************************************************
*
*       Function:
*       ---------
*	int write_fits_file (const char *filename, int image_fd, 
*					struct fits_header_struct fits_header)
*
*       Description:
*       ------------
*	Writes in the file designed by filename, the contents of the buffer
*	pointed by image_fd that is the user's image buffer. And, adds as a
*	header of that file the fits_strcut that is a header.
*
*	Fits file are files that have a special header, that describes its 
*	contents and another characteristics.
*
*	This function open the file with name filename (if exits overwrites 
*	and, if doesn't creates it), adds the header and,  append the image
*	data that it's stored in the image_fd.
*
*	If there is some input/output problem related with the file I/O 
*	operation, the function returns _ERROR.
*
*       Parameters:
*       -----------
*       filename		The name of the file where to save the fits image.
*		image_fd		Memory buffer that contains the fits image.
*		fits_header		Header for the fits file.
*
*	Returns:
*	--------
*	The result of to write the image buffer to the file, either success
*	or fail.
*
*       Version:        1.00
*       Author:         Jose Maria Panero Ciprian
*       Date:           07/10/2000
*
*   	Revision History:
*   	-----------------
*       Date            Who   Version    Description
*   	----------------------------------------------------------------------
*       07/10/2000      jmp     1.00    Initial
*
*
*******************************************************************************/
int write_fits_file (const char *filename, unsigned short *image_fd, struct fits_header_struct fits_header)
{
	FILE *fd_out;
	int result = _NO_ERROR;
	int z;
	int fill_count = 0;
	int columns = 0;
	int rows = 0;
	int bytes = 0;
	short fill_data = 0;

	/**********
	*  columns and rows of the image.
	**********/
	columns = fits_header.naxis1;
	rows = fits_header.naxis2;
	bytes = columns*rows*2;

	/**********
	*  open the file in readwrite mode.
	*  if some, error, returns it.
	**********/
	fd_out = fopen(filename, "wb");

	if (fd_out == NULL) {
		printf("ERROR: Cannot open file \"%s\"\n", filename);
		return _ERROR;
	}

	write_fits_header (fd_out, fits_header);

	/* Write the image data. */
	fwrite(image_fd, rows*columns, 1, fd_out);

	/* Fill the rest of the buffer with zeros. */
	fill_count = 2880 - (bytes - ((bytes/2880)*2880));

	/* Fill the rest of the file with 0's for padding. */
	for (z=0; z<fill_count; z++)
		fwrite(&fill_data, sizeof(char), 1, fd_out);

	fclose(fd_out);

	result = _NO_ERROR;
	printf("Fits file \"%s\" has been written.", filename);

	/**********
	*  return result
	**********/
	return result;
}


/*******************************************************************************
*
*       Function:
*       ---------
*	int write_fits_header(int fd_out, struct fits_header_struct fits_header)
*
*       Description:
*       ------------
*	Writes in the file designed by fd_out the corresponding header.
*	The header is a parameter. To fill the header see the functions 
*	below:
*
*	Fits file are files that have a special header, that describes its 
*	contents and another characteristics.
*
*       Parameters:
*       -----------
*       fd_out 		file descriptor to the file where to write to.	
*	fits_header	header for the fits file.
*
*	Returns:
*	--------
*	The result of to write the header to the file.
*
*       Version:        1.00
*       Author:         Jose Maria Panero Ciprian
*       Date:           07/10/2000
*
*   	Revision History:
*   	-----------------
*       Date            Who   Version    Description
*   	----------------------------------------------------------------------
*       07/10/2000      jmp     1.00    Initial
*
*
*******************************************************************************/
int write_fits_header (FILE *fd_out, struct fits_header_struct fits_header)
{
	char header[2880];	/* Array is 81 elements since sprintf appends a \0 */
	char *headerPtr = &header[0];

	memset(header, 0x20, 2880);

	/**********
	*  Gets the actual date and time.
	**********/
	set_fits_header_time (&fits_header);

	/**********
	*  fill the fits file header array.
	**********/
	sprintf(headerPtr, "SIMPLE  =           %10d                                                 ", fits_header.simple);
	headerPtr += 80;
    sprintf(headerPtr, "BITPIX  =           %10d                                                 ", fits_header.bitpix);
    headerPtr += 80;
    sprintf(headerPtr, "NAXIS   =           %10d                                                 ", fits_header.naxis);
    headerPtr += 80;
    sprintf(headerPtr, "NAXIS1  =           %10d                                                 ", fits_header.naxis1);
    headerPtr += 80;
    sprintf(headerPtr, "NAXIS2  =           %10d                                                 ", fits_header.naxis2);
    headerPtr += 80;
    sprintf(headerPtr, "DATE    = %20s                                                 ", fits_header.date);
    headerPtr += 80;
    sprintf(headerPtr, "UTSTART = %20s                                                 ", fits_header.time);
    headerPtr += 80;
    sprintf(headerPtr, "LOCATION= %20s                                                 ", fits_header.location);
    headerPtr += 80;
    sprintf(headerPtr, "SIDETIME= %20s                                                 ", fits_header.sidetime);
    headerPtr += 80;
    sprintf(headerPtr, "EPOCH   = %20s                                                 ", fits_header.epoch);
    headerPtr += 80;
    sprintf(headerPtr, "AIRMASS = %20s                                                 ", fits_header.airmass);
    headerPtr += 80;
    sprintf(headerPtr, "EXP_TIME=           %10d                                       ", fits_header.exp_time);
    headerPtr += 80;
    sprintf(headerPtr, "IMG_TYPE= %20s                                                 ", fits_header.img_type);
    headerPtr += 80;
    sprintf(headerPtr, "TELESCOP= %20s                                                 ", fits_header.telescope);
    headerPtr += 80;
    sprintf(headerPtr, "INSTRMEN= %20s                                                 ", fits_header.instrment);
    headerPtr += 80;
    sprintf(headerPtr, "FILTER  = %20s                                                 ", fits_header.filter);
    headerPtr += 80;
    sprintf(headerPtr, "OBJECT  = %20s                                                 ", fits_header.object);
    headerPtr += 80;
    sprintf(headerPtr, "RA      = %20s                                                 ", fits_header.ra);
    headerPtr += 80;
    sprintf(headerPtr, "DEC     = %20s                                                 ", fits_header.dec);
    headerPtr += 80;
    sprintf(headerPtr, "OBSERVER= %20s                                                 ", fits_header.observer);
    headerPtr += 80;
    sprintf(headerPtr, "COMMENT1= %20s                                                 ", fits_header.comment1);
    headerPtr += 80;
    sprintf(headerPtr, "COMMENT2= %20s                                                 ", fits_header.comment2);
    headerPtr += 80;
    sprintf(headerPtr, "COMMENT3= %20s                                                 ", fits_header.comment3);
    headerPtr += 80;
    sprintf(headerPtr, "COMMENT4= %20s                                                 ", fits_header.comment4);
    headerPtr += 80;
    sprintf(headerPtr, "END                                                                             ");

	fwrite(header, 2880, 1, fd_out);

	return 0;
}


/*******************************************************************************
*
*       Function:
*       ---------
*	int set_fits_header (struct fits_header_struct *fits_header)
*
*       Description:
*       ------------
*	Sets the values of the header to default values.
*
*       Parameters:
*       -----------
*	fits_header	header to be set.
*
*	Returns:
*	--------
*	The result of to set the header to default values.
*
*       Version:        1.00
*       Author:         Jose Maria Panero Ciprian
*       Date:           07/10/2000
*
*   	Revision History:
*   	-----------------
*       Date            Who   Version    Description
*   	----------------------------------------------------------------------
*       07/10/2000      jmp     1.00    Initial
*
*
*******************************************************************************/
int set_fits_header (struct fits_header_struct *fits_header)
{
	int result = _NO_ERROR;

	sprintf (fits_header->simple, HEADER_DEFAULT_SIMPLE);
	fits_header->bitpix = HEADER_DEFAULT_BITPIX;
	fits_header->naxis = HEADER_DEFAULT_NAXIS;
	fits_header->naxis1 = 0;
	fits_header->naxis2 = 0;
	sprintf (fits_header->date, "mm/dd/yyyy");
	sprintf (fits_header->time, "hh:mm:ss");
	sprintf (fits_header->location, "Observatory");
	sprintf (fits_header->sidetime, "hh:mm:ss.ss");
	sprintf (fits_header->epoch, "cccc");
	sprintf (fits_header->airmass, "99.99");
	fits_header->exp_time = 0;
	sprintf (fits_header->img_type, "Image");
	sprintf (fits_header->telescope, "Telescope");
	sprintf (fits_header->instrment, "Instrument");
	sprintf (fits_header->filter, "None");
	sprintf (fits_header->object, "None");
	sprintf (fits_header->ra, "hh:mm:ss.ss");
	sprintf (fits_header->dec, "hh:mm:ss.ss");
	sprintf (fits_header->observer, "Observer");
	sprintf (fits_header->comment1, "None");
	sprintf (fits_header->comment2, "None");
	sprintf (fits_header->comment3, "None");
	sprintf (fits_header->comment4, "None");

	result = _NO_ERROR;
	sprintf(error_string,"Fits file header have been set.");

	/**********
	*  any case, result is returned
	**********/
	return result;
}


/*******************************************************************************
*
*       Function:
*       ---------
*	int edit_fits_exposure_dimensions(
*	struct fits_header_struct *fits_header, int cols,int rows, int exp_time)
*
*       Description:
*       ------------
*	Writes in the designed header, the values corresponding to the
*	image's number of rows, number of columns, and exposure time.
*
*
*       Parameters:
*       -----------
*	fits_header	header for the fits file, and where the propierties
*			are going to be written.
*	rows		number of rows.
*	cols		number of columns.
*	exp_time	exposure time.
*
*	Returns:
*	--------
*	The result of to write to the header the specified propierties. 
*
*       Version:        1.00
*       Author:         Jose Maria Panero Ciprian
*       Date:           07/10/2000
*
*   	Revision History:
*   	-----------------
*       Date            Who   Version    Description
*   	----------------------------------------------------------------------
*       07/10/2000      jmp     1.00    Initial
*
*
*******************************************************************************/
int edit_fits_exposure_dimensions (struct fits_header_struct *fits_header, 
					int rows, int cols, int exp_time)
{
	int result = _NO_ERROR;

	fits_header->naxis1 = cols;
	fits_header->naxis2 = rows;
	fits_header->exp_time = exp_time;

	result = _NO_ERROR;
	sprintf(error_string,
			"Fits file header have been updated (%dx%d, %dms).",
        	fits_header->naxis1,
        	fits_header->naxis2,
        	fits_header->exp_time);

	/**********
	*  any case, result is returned
	**********/
	return result;
}


/*******************************************************************************
*
*       Function:
*       ---------
*	int set_fits_header_time (struct fits_header_struct *fits_header)
*
*       Description:
*       ------------
*	Writes in the designed header, the values corresponding to the
*	date, time and epoch.
*
*
*       Parameters:
*       -----------
*	fits_header	header for the fits file, and where the propierties
*			are going to be written.
*
*	Returns:
*	--------
*	The result of to write to the header the specified propierties. 
*
*       Version:        1.00
*       Author:         Jose Maria Panero Ciprian
*       Date:           07/10/2000
*
*   	Revision History:
*   	-----------------
*       Date            Who   Version    Description
*   	----------------------------------------------------------------------
*       07/10/2000      jmp     1.00    Initial
*		01/03/2002		sds		1.7		Modified to be a win2k dll.
*
*******************************************************************************/
int set_fits_header_time (struct fits_header_struct *fits_header)
{
	int result = _NO_ERROR;
	time_t time_now;
	struct tm *now;

	time_now = time (NULL);
	now = localtime(&time_now);

	sprintf (fits_header->date, "%02d/%02d/%04d", now->tm_mon + 1,
							now->tm_mday,
							now->tm_year + 1900);
	sprintf (fits_header->time, "%02d:%02d:%02d.00", now->tm_hour,
							now->tm_min,
							now->tm_sec);
	sprintf (fits_header->epoch, "%04d", now->tm_year + 1900);

	result = _NO_ERROR;
	sprintf(error_string,
		"Fits file header date and time have been updated (%s %s).",
		fits_header->date,
		fits_header->time);

	/**********
	*  any case, result is returned
	**********/
	return result;
}

