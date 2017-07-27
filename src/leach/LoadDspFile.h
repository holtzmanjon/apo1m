/******************************************************************************
*
*   File:       LoadDspFile.h
*   Version:    1.00
*   Author:     Jose Maria Panero (see references).
*   Abstract:   C library functions for load files to the pci, util or timing
*		boards.
*               I/O control commands for Solaris device driver for the SDSU PCI
*               Interface Card, and DSP Commands.
*
*   References: This include file is fully based and is the result to merge 
*		the include files LoadDspFile.h and PCIBoot.h both wrote by 
*		Scott Streit.
*
*
*   Revision History:
*       Date            Who   Version 	Description
*   --------------------------------------------------------------------------
*
*       06/29/00        jmp     1.00	initial
*
*******************************************************************************/

#ifndef LOAD_DSP_FILE_H
#define LOAD_DSP_FILE_H

/* ------------------------------
	Define Globals
 --------------------------------*/
extern char error_string[];

/*******************************************************************************
*  Wildcards in the pci file.
*******************************************************************************/
#define SEARCH_STRING   "_DATA P"
#define END_STRING      "_END"

/*******************************************************************************
*  A valid start address must be less than 0x4000 for the load DSP file in
*  timming or utility boards.
*******************************************************************************/
#define MAX_DSP_START_LOAD_ADDR	0x4000

/*******************************************************************************
*  For check number of words is less 4096 (0x1000), for the pci boot file.
*******************************************************************************/
#define MAX_WORD_NUM_PCI_FILE	0x1000

/*******************************************************************************
*  The valid start address to load a into the pci board.
*******************************************************************************/
#define LOAD_PCI_START_ADDR	0x0000

/*******************************************************************************
* C prototype functions.
*******************************************************************************/
int loadFile(HANDLE pci_fd, char *filename, int validate);
int loadPciFile(HANDLE pci_fd, const char *filename);
int getTokens(char *textLine, char *tokens[]);

#endif
