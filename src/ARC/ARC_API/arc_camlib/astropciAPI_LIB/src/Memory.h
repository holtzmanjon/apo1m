/******************************************************************************
*
*   File:       Memory.h
*   Version:    1.00
*   Author:     Scott Streit
*   Abstract:   Include file for the C library for Memory usage functions.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       02/05/2001      sds     1.00    Initial
*
******************************************************************************/
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../WhichSystem.h"

/*------------------------------*/
/*	Globals						*/
/*------------------------------*/
extern char error_string[];

/*------------------------------*/
/*	Function Prototypes			*/
/*------------------------------*/
unsigned short *create_memory(HANDLE pci_fd, int rows, int cols);
void free_memory(HANDLE pci_fd, unsigned short *mem_fd);
void swap_memory(unsigned short *mem_fd);

#endif
