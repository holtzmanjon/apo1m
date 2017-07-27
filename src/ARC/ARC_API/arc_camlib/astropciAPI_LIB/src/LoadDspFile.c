/********************************************************************************
*   File:       LoadDspFile.c
*   Version:    1.00
*   Author:     Jose Maria Panero Ciprian
*
*   Abstract:   C library function for load DSP files.
*
*		The LoadDspFile adds the funcionality to the DSPCommand.c 
*		to load files into the boards, pci board, util board,  or 
*		timing board.
*
*		Two C functions,  load_file() used to load a  file either 
*		to the util board or to the timing board. 
*		And, load_pci_file() to load a file to the pci board.
*
*		Note: that this module may be  used with the DSPCommand.c
*		      module.	
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       07/10/00        jmp     1.00    Initial. The module is the result of
*										merge the modules LoadDspFile.c, and
*										PCIBoot.c, author  Scott Streit. The 
*										functions are updated to support the
*										global vars: reply_value and error_string
*										(see functions for details).
*
*		01/02/02		sds		1.7		Modified to be win2k static library.
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "DSPCommand.h"
#include "LoadDspFile.h"
#include "ErrorString.h"
#include "../WhichSystem.h"

#ifdef WIN2K
	#include <windows.h>
	#include <winioctl.h>
#endif

/*******************************************************************************
*
*	Function:
*	---------
*		int load_file(int pci_fd, const char *filename, 
*						const char *expected_file_type)
*
* 	Description:	
*	------------
*	This function loads a file into the DSP of the specified controller 
*	board. The controller board can be the  timing board or the utility
*	board as defined by the parameter expected_file_type.
*
*
*       Parameters:
*       -----------
*       pci_fd 			File descriptor for the pci device.
*	filename 		The name of the file to load.
*	expected_file_type 	The controller board to load the file into.
*				May be "timing" or "utility".
* 				This parameter is not case sensitive.
*
*	Returns:
*	--------
*	If successfull returns _NO_ERROR, and if fail returns as _ERROR the
*	source of the fail.
*
*	Version: 	1.00
*	Author:  	Scott Streit
* 	Date:		11/22/1999
*
*   Revision History:
*   -----------------
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       11/22/1999	sds     1.00    Initial
*
*	07/10/2000	jmp		Add support for the error variables
*					reply_value, error_string
*					Take out comments a print outs and
*					resend them to the error variables.
*					Fits the funcionality of the function
*					accordingly the DSPCommand module.
*
*******************************************************************************/
int loadFile(HANDLE pci_fd, const char *filename, const char *expected_file_type)
{
	int result = _NO_ERROR;

	const char * input_value = " ";
	char input_line[81];
	const char * type_string = "__";
	const char * addr_string = " ";
	const char *data_string = " ";
	const char * file_type = " ";
	const char * path = "./";
	const char * name = " ";
	int end_of_file = 0;
	int done_reading_data = 0;
	int type = 0;
	int addr = 0;
	int data = 0;
	int board_id = 0;
	int n = -1;
	long file_position = 0;
	FILE *inFile;
	int count = 0;
	int i = 0;
	const char *tokens[20];
	
	/*---------------------------
	  Open the file for reading.
	  ---------------------------*/
	if ((inFile = fopen(filename, "r")) == NULL) 
	{
		fprintf (stdout,"ERROR: Cannot open file: %s \n", filename);
		return _ERROR;
	}
		
	/*--------------------------------------
	  Read in the file one line at a time.
	  --------------------------------------*/
	while (end_of_file == 0) 
	{
	     fgets(input_line, 80, inFile);

	     input_value = strtok(input_line, " ");

	     /* Check for the start of valid data */
	     if (strstr(input_value, "DATA") != NULL) {
		 type_string = strtok(NULL, " ");
		 addr_string = strtok(NULL, " ");

		 /* Convert the "address" string to the correct data type */
		 sscanf(addr_string, "%X", &addr);

		 /* A valid start address must be less than MAX_DSP_START_LOAD_ADDR */
		 if (addr < MAX_DSP_START_LOAD_ADDR) {
		      /* Set the "type" string to the correct ASCII sequence */
		      if (strcmp(type_string, "X") == 0)
			      type = X;
		      else if (strcmp(type_string, "Y") == 0)
			      type = Y;
		      else if (strcmp(type_string, "P") == 0)
			      type = P;
		      else if (strcmp(type_string, "R") == 0)
			      type = R;
					
		      /* Read in the data block */ 
		      while (!done_reading_data) 
			     {
			       fgets(input_line, 80, inFile);
			       count = 0;
			       tokens[count] = strtok(input_line, " ");
						
			       if (strstr(tokens[0], "_") != NULL) 
			           {
				     done_reading_data = 1;
				     fseek (inFile, -15, SEEK_CUR);
				   }
						
				   else 
				   {
				     while (tokens[count] != NULL) 
				      {
					 count++;
					 tokens[count] = 
					 strtok(NULL, " ");
				       }
										
				       for (i=0; i<count-1; i++) 
				       {
					 sscanf(tokens[i], "%X", &data);

					 if (doCommand2(pci_fd, board_id, WRM, (type | addr), data, DON) == _ERROR) {
					       fprintf(stdout,"\nERROR: Could not do write memory command (0x%X).\n", getError());
					       return _ERROR;
					 }
							
					 addr++;
				       }
				}
			}
		    }
		}
			
		/* Check the file validity and set the board destination */ 
		if (strcmp(input_value, "_START") == 0) {
			file_type = strtok(NULL, " ");
				
			if ((strspn(file_type, "TIM") > 0) &&
				(strcmp(expected_file_type, "timing") == 0)) 
				board_id = TIM_ID;
			
			else if ((strspn(file_type, "UTIL") > 0) && 
					 (strcmp(expected_file_type, "utility") == 0)) 
				board_id = UTIL_ID;
			
			else {
				fprintf(stdout,"ERROR: Unexpected file type.");
				return _ERROR;
			}
		}
			
		/* Check for the end of file */
		else if (strcmp(input_value, "_END") == 0) 
			end_of_file = 1;
			
		/* Check for the end of file */
		else if (strcmp(input_value, "_SYMBOL") == 0) 
			end_of_file = 1;
						
		/* Re-initialize variables */
		done_reading_data = 0;
		type_string = "__";
		input_value = NULL;
	}
		
	/* Close the file */
	fclose(inFile);

	return _NO_ERROR;
}

/*******************************************************************************
*
*	Function:
*	---------
*		int load_pci_file(int pci_fd, const char *filename)
*
* 	Description:	
*	------------
*	This function downloads PCI boot code from a file to the PCI board
* 	DSP program memory space (P). The file download must  be performed
* 	before any  other  controller  actions occur,  otherwise, the  PCI
* 	board's actions are undefined.
*			
*
*       Parameters:
*       -----------
*       pci_fd 		File descriptor for the pci device.
*	filename 	The name of the file to load, e.g. "pci.lod"
*
*	Returns:
*	--------
*	If successfull returns _NO_ERROR, and if fail returns as ERROR the
*	source of the fail.
*
*       Version:	1.00
*       Author:	Scott Streit
* 	Date:		02/01/2000
*
*   Revision History:
*   -----------------
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
* 	02/01/2000			sds		1.00	Initial
*
* 	02/11/2000			sds				Updated to check that the total number
*										of words to transfer is less than MAX_ 
* 										and that the start addr equals 0.
*
*	07/10/2000			jmp				Add support for the error variables
*										reply_value, error_string. 
*										Change the order of checks. First of 
*										all check if the pci load file can be
*										open, before send any command to the
*										HTF register, otherwise the pci device
*										state can became undefinied.
*										Take out comments a print outs and
*										resend them to the error variables.
*										Fits the funcionality of the function
*										accordingly the DSPCommand module.
*
*	01/03/2002			sds		1.7		Modified to be a win2k dll.
*
******************************************************************************/
int loadPciFile(HANDLE pci_fd, const char *filename)
{
	char input_line[81];
	const char *words_string = " ";
	const char *addr_string = " ";
	const char *data_string = " ";
	const char *fileType = " ";
	const char *tokens[20];
	int data = 0;
	int addr = 0;
	int end_of_file = 0;
	int done_reading_data = 0;
	int reply = 0;
	int word_total = 0;
	int word_count = 0;
	int count = 0;
	int i = 0;
	int j = 0;
	long file_position = 0;
	FILE *inFile;
		
	/* Set the PCI board to slave mode. */
#ifdef WIN2K
	ULONG bytesReturned = 0;
	DeviceIoControl(pci_fd, ASTROPCI_GET_HCTR, NULL, 0, &reply, sizeof(reply),
						&bytesReturned, NULL);

	DeviceIoControl(pci_fd, ASTROPCI_SET_HCTR, &reply, sizeof(reply), NULL, 0,
						&bytesReturned, NULL);

	/* Clear the HTF bits and bit 3. */
	reply = reply & HTF_CLEAR_MASK & BIT3_CLEAR_MASK;
		
	/* Set the HTF bits. */
	reply = reply | HTF_MASK;

	/* Inform the DSP that new pci boot code will be downloaded. */
	DeviceIoControl(pci_fd, ASTROPCI_PCI_DOWNLOAD, NULL, 0, NULL, 0, &bytesReturned, NULL);

	/* Set the magic value that says this is a PCI download. */
	reply = 0x00555AAA;
	DeviceIoControl(pci_fd, ASTROPCI_HCVR_DATA, &reply, sizeof(reply), NULL, 0,
						&bytesReturned, NULL);

#else
	ioctl(pci_fd, ASTROPCI_GET_HCTR, &reply);

	/* Clear the HTF bits and bit 3. */
	reply = reply & HTF_CLEAR_MASK & BIT3_CLEAR_MASK;
		
	/* Set the HTF bits. */
	reply = reply | HTF_MASK;

	ioctl(pci_fd, ASTROPCI_SET_HCTR, &reply);

	/* Inform the DSP that new pci boot code will be downloaded. */
	ioctl(pci_fd, ASTROPCI_PCI_DOWNLOAD, 0);

	/* Set the magic value that says this is a PCI download. */
	reply = 0x00555AAA;
	ioctl(pci_fd, ASTROPCI_HCVR_DATA, &reply);
#endif

	/* Open the file for reading. */
	if ((inFile = fopen(filename, "r")) == NULL) {
		sprintf(error_string, "\nError: Cannot open file: %s \n", filename);
		return _ERROR;
	}
		
	while(!done_reading_data) {
		fgets(input_line, 80, inFile);

		if (strstr(input_line, SEARCH_STRING) != NULL) {
			/* Get the next line. */
			fgets(input_line, 80, inFile);

			/* Get the number of words and starting address. */
			words_string = strtok(input_line, " ");
			addr_string = strtok(NULL, " ");

			sscanf(words_string, "%X", &word_total);
			sscanf(addr_string, "%X", &addr);

			/* Check that the number of words is less that 4096 (0x1000). */
			if (word_total > 0x1000) {
				sprintf(error_string, "\nError: Number of words to write exceeds DSP memory range.");
				return _ERROR;
			}
			else {
#ifdef WIN2K
				DeviceIoControl(pci_fd, ASTROPCI_HCVR_DATA, &word_total, sizeof(word_total),
									NULL, 0, &bytesReturned, NULL);
#else
				ioctl(pci_fd, ASTROPCI_HCVR_DATA, &word_total);
#endif
			}

			/* Check that the address is equal to 0. */
			if (addr != 0) {
				printf("\nError: Address not equal to zero.");
				exit(1);
			}
			else {
#ifdef WIN2K
				DeviceIoControl(pci_fd, ASTROPCI_HCVR_DATA, &addr, sizeof(addr), NULL, 0,
									&bytesReturned, NULL);
#else
				ioctl(pci_fd, ASTROPCI_HCVR_DATA, &addr);
#endif
				addr = 0;
			}

			/* Throw away the next line (example: _DATA P 000002). */
			fgets(input_line, 80, inFile);
				
			/* Load the data. */
			while (word_count < (word_total - 2)) {
				/* Get the next line, this is the data start. */
				fgets(input_line, 80, inFile);

				/* Check for "_DATA" strings and discard them by     */
				/* reading the next data line, which should be data. */
				if (strstr(input_line, SEARCH_STRING) != NULL)
					fgets(input_line, 80, inFile);
				
				count = 0;
				tokens[count] = strtok(input_line, " ");
					
				if (strstr(tokens[0], "_") != NULL) {
					done_reading_data = 1;
					fseek (inFile, -15, SEEK_CUR);
				}
						
				else {
					while (tokens[count] != NULL) {
						count++;
						tokens[count] = strtok(NULL, " ");
					}
					
					for (i=0; i<count-1; i++) {
						sscanf(tokens[i], "%X", &data);
#ifdef WIN2K
						DeviceIoControl(pci_fd, ASTROPCI_HCVR_DATA, &data, sizeof(data), NULL, 0,
									&bytesReturned, NULL);
#else
						ioctl(pci_fd, ASTROPCI_HCVR_DATA, &data);
#endif
						addr++;
						word_count++;
					}
				}
			}
			done_reading_data = 1;
		}
		
		/* Check the file validity. */
		else if (strspn(input_line, "_START") > 0) {
			fileType = strtok(input_line, " ");
			fileType = strtok(NULL, " ");

			if (strspn(fileType, "PCI") > 0)
				printf("PCI file ... OK.\n");
			else {
				printf("Error: Invalid file type.\n");
				return _ERROR;
			}
		}
	}
	/* Set the PCI board data transfer format (Set HTF bits to 00). */
#ifdef WIN2K
	DeviceIoControl(pci_fd, ASTROPCI_GET_HCTR, NULL, 0, &reply, sizeof(reply),
						&bytesReturned, NULL);
	reply = (reply & HTF_CLEAR_MASK) | 0x900;
	DeviceIoControl(pci_fd, ASTROPCI_SET_HCTR, &reply, sizeof(reply), NULL, 0,
						&bytesReturned, NULL);

	fclose(inFile);

	/* Wait for the PCI DSP to finish initialization. */
	DeviceIoControl(pci_fd, ASTROPCI_PCI_DOWNLOAD_WAIT, NULL, 0, &reply, sizeof(reply),
						&bytesReturned, NULL);
#else
	ioctl(pci_fd, ASTROPCI_GET_HCTR, &reply);
	reply = (reply & HTF_CLEAR_MASK) | 0x900;
	ioctl(pci_fd, ASTROPCI_SET_HCTR, &reply);

	fclose(inFile);

	/* Wait for the PCI DSP to finish initialization. */
	ioctl(pci_fd, ASTROPCI_PCI_DOWNLOAD_WAIT, &reply);
#endif
	
	/* Make sure a DON is received. */
	if (reply != DON)
		sprintf(error_string, "ERROR: Bad reply, expected: DON (0x444F4E), got: 0x%X\n", reply);

	return _NO_ERROR;
}
