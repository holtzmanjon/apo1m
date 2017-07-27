/***********************************************************************************************
*	FUNCTION: 	Deinterlace.c
*	AUTHOR:	  	Scott Streit
*	DATE:     	02-05-2001
*	VERSION:  	1.0
*
*	PARAMETERS:	mem_fd   File descriptor that references the image.
*			rows	 The number of rows in the image.
*			cols	 The number of columns in the image.
*
*	RETURNS:	_NO_ERROR if all ok, else returns _ERROR.
*                                                                           			
*	DESCRIPTION:	The deinterlacing algorithms work on the principle
*			that the ccd/ir array will read out the data in a
*			predetermined order depending on the type of readout
*			being implemented.  Here's how they look:     			
*                                                                           			
*   split-parallel          split-serial              quad CCD                quad IR          	
*  ----------------       ----------------        ----------------        ----------------     	
* |     1  ------->|     |        |------>|      |<-----  |  ---->|      | -----> | ----> |    	
* |                |     |        |   1   |      |   3    |   2   |      |   0    |   1   |    	
* |                |     |        |       |      |        |       |      |        |       |    	
* |_______________ |     |        |       |      |________|_______|      |________|_______|    	
* |                |     |        |       |      |        |       |      |        |       |    	
* |                |     |        |       |      |        |       |      |        |       |    	
* |                |     |   0    |       |      |   0    |   1   |      |   3    |   2   |    	
* |<--------  0    |     |<------ |       |      |<-----  |  ---->|      | -----> | ----> |    	
*  ----------------       ----------------        ----------------        ----------------  
*
*    CDS quad IR          	
*  ----------------     	
* | -----> | ----> |    	
* |   0    |   1   |    	
* |        |       |    	
* |________|_______|    	
* |        |       |    	
* |        |       |    	
* |   3    |   2   |    	
* | -----> | ----> |    	
*  ----------------    
* | -----> | ----> |    	
* |   0    |   1   |    	
* |        |       |    	
* |________|_______|    	
* |        |       |    	
* |        |       |    	
* |   3    |   2   |    	
* | -----> | ----> |    	
*  ----------------  
*
***********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Deinterlace.h"
#include "DSPCommand.h"
#include "ErrorString.h"

int deinterlace(int cols, int rows, unsigned short *image_fd, int algorithm)
{
	unsigned short *old_iptr;
	unsigned short *new_iptr;

	old_iptr = image_fd;
	
	/* Allocate a new buffer to hold the deinterlaced image. */
	if((new_iptr = (unsigned short *) malloc(cols*rows*sizeof(unsigned short))) == NULL) {
       		sprintf(error_string, "Error in allocating memory for deinterlacing. \nSaving interlaced image to disk.");
		free (new_iptr);
		return _ERROR;
        }

	switch(algorithm) {
		case 1:
		/*-------------- SPLIT PARALLEL READOUT ---------------------------------------*/
		{
			int i;
			
			if (((float)rows/2) != (int)rows/2) {
       				sprintf(error_string, "Number of rows must be EVEN for split-parallel readout. \nSkipping deinterlace.");
				free (new_iptr);
				return _ERROR;
        		}

			for (i=0;i<(cols*rows)/2;i++) {
        			*(new_iptr+i) = *(old_iptr+(2*i));
        			*(new_iptr+(cols*rows)-i-1) = *(old_iptr+(2*i)+1);
        		}

			memcpy(old_iptr, new_iptr, cols*rows*sizeof(unsigned short));
			free(new_iptr);
			return _NO_ERROR;
		}

		case 2:
		/*-------------- SPLIT SERIAL READOUT -----------------------------------------*/
		{
			int i;
			int j;
			int p1;
			int p2;
			int begin;
			int end;

			if ((float)cols/2 != (int)cols/2) {
       				sprintf(error_string, "Number of cols must be EVEN for split-serial readout. \nSkipping deinterlace.");
				free (new_iptr);
				return _ERROR;
        		}
			
			for (i=0;i<rows;i++) {
				/* leave in +0 for clarity */
        			p1      = i*cols+0; /*position in raw image */
        			p2      = i*cols+1;
        			begin   = i*cols+0; /*position in interlaced image */
        			end     = i*cols+cols-1;
        			for (j=0;j<cols;j+=2) {
                			*(new_iptr+begin) = *(old_iptr+p1);
                			*(new_iptr+end) = *(old_iptr+p2);
                			++begin; --end;
                			p1+=2; p2+=2;
                		}
        		}

			memcpy(old_iptr, new_iptr, cols*rows*sizeof(unsigned short));
			free(new_iptr);
			return _NO_ERROR;
		}

		case 3:
		/*--------------- SPLIT QUAD READOUT -----------------------------------------*/
		{
			int i = 0;
			int j = 0;
			int counter = 0;
			int end = 0;
			int begin = 0;

			if ( (float)cols/2 != (int)cols/2 || (float)rows/2 != (int)rows/2) {
       				sprintf(error_string, "Number of cols AND rows must be EVEN for quad readout. \nSkipping deinterlace.");
				free (new_iptr);
				return _ERROR;
        		}
			
			while(i<cols*rows) {
        			if (counter%(cols/2) == 0) {
                			end = (cols*rows)-(cols*j)-1;
                			begin = (cols*j)+0; 	/* left in 0 for clarity */
                			j++; 					/* number of completed rows */
                			counter=0; 				/* reset for next convergece */
                		}
        			*(new_iptr+begin+counter)       = *(old_iptr+i++);		/* front_row--->  */
        			*(new_iptr+begin+cols-1-counter)   = *(old_iptr+i++);	/* front_row<--   */
        			*(new_iptr+end-counter)         = *(old_iptr+i++);		/* end_row<----   */
        			*(new_iptr+end-cols+1+counter)     = *(old_iptr+i++);	/* end_row---->   */
        			counter++;
        		}

			memcpy(old_iptr, new_iptr, cols*rows*sizeof(unsigned short));
			free(new_iptr);
			return _NO_ERROR;
		}

		case 4:
		/*--------------- IR SPLIT QUAD READOUT --------------------------------------*/
		{
			int i = 0;
			int j = rows-1;
			int counter = 0;
			int end = 0;
			int begin = 0;

			if ( (float)cols/2 != (int)cols/2 || (float)rows/2 != (int)rows/2) {
       				sprintf(error_string, "Number of cols AND rows must be EVEN for quad readout. \nSkipping deinterlace.");
				free (new_iptr);
				return _ERROR;
        		}
			
			while(i<cols*rows) {
        			if (counter%(cols/2) == 0) {
                			end = (j-(rows/2))*cols;
                			begin = j*cols;
                			j--; 			/*number of completed rows*/
                			counter=0; 		/*reset for next convergece*/
                		}

        			*(new_iptr+begin+counter)       = *(old_iptr+i++);		/* front_row--->  */
        			*(new_iptr+begin+(cols/2)+counter) = *(old_iptr+i++);	/* front_row<--   */
        			*(new_iptr+end+(cols/2)+counter)   = *(old_iptr+i++);	/* end_row<----   */
        			*(new_iptr+end+counter)         = *(old_iptr+i++);		/* end_row---->   */
        			counter++;
        		}

			memcpy(old_iptr, new_iptr, cols*rows*sizeof(unsigned short));
			free(new_iptr);
			return _NO_ERROR;
		}

		case 5:
		/*----------- CORRELATED DOUBLE SAMPLING IR SPLIT QUAD READOUT ---------------*/
		{
			int i;
			int j;
			int imageSection;
			int counter;
			int end;
			int begin;
			int oldRows;
			unsigned short *orig_old_iptr = old_iptr;
			unsigned short *orig_new_iptr = new_iptr;

			if ( (float)cols/2 != (int)cols/2 || (float)rows/2 != (int)rows/2) {
       				sprintf(error_string, "Number of cols AND rows must be EVEN for quad readout. \nSkipping deinterlace.");
				free (new_iptr);
				return _ERROR;
       			}

			/* Set the the number of rows to half the image size. */
			oldRows = rows;
			rows = rows/2;

			/* Deinterlace the two image halves separately. */
			for (imageSection = 0; imageSection <= 1; imageSection++) {
				i = 0;
				j = rows-1;
				counter = 0;
				end = 0;
				begin = 0;

				while(i<cols*rows) {
        				if (counter%(cols/2) == 0) {
                				end = (j-(rows/2))*cols;
                				begin = j*cols;
                				j--; 			/*number of completed rows*/
                				counter=0; 		/*reset for next convergece*/
                			}

        				*(new_iptr+begin+counter)       = *(old_iptr+i++);		/* front_row--->  */
        				*(new_iptr+begin+(cols/2)+counter) = *(old_iptr+i++);	/* front_row<--   */
        				*(new_iptr+end+(cols/2)+counter)   = *(old_iptr+i++);	/* end_row<----   */
        				*(new_iptr+end+counter)         = *(old_iptr+i++);		/* end_row---->   */
        				counter++;
        			}
        			old_iptr += rows*cols;
        			new_iptr += rows*cols;
			}
			old_iptr = orig_old_iptr;
			new_iptr = orig_new_iptr;

			memcpy(old_iptr, new_iptr, cols*oldRows*sizeof(unsigned short));
			free(new_iptr);

			return _NO_ERROR;
		}

		case 6:
		/*--------------- USER DEFINED READOUT -----------------------------------------*/
		{
      			sprintf(error_string, "User Defined Readout... UNDER CONSTRUCTION.");
			free(new_iptr);
			return _ERROR;
		}
	}

	return _NO_ERROR;
}
