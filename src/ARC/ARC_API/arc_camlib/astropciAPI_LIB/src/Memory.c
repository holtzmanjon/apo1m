/******************************************************************************
*
*   File:       Memory.c
*   Version:    1.00
*   Author:     Scott Streit
*   Abstract:   C library functions for memory allocation and de-allocation.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*       02/05/2001      sds     1.00    Initial
*
******************************************************************************/
#include <stdio.h>
#include "ErrorString.h"
#include "DSPCommand.h"
#include "Memory.h"
#include "../WhichSystem.h"

#ifdef WIN2K
	#include <windows.h>
	#include <winioctl.h>
#else
	#include <errno.h>
	#include <fcntl.h>
	#include <sys/types.h>
	#include <sys/mman.h>
#endif

#define BUFFER_SIZE	2200*2200*2


/*******************************************************************************
*
*	Function:
*	---------
*	int create_memory(int pci_fd, int rows, int cols)
*
*	Description:
*	------------
*       This function is used to allocate space to an image buffer.
*	The function returns a pointer to a block of at least 'image_size'
*	bytes suitably aligned for any use.
*
*	Parameters:     
*	-----------
*	pci_fd  	The reference to the PCI driver returned by open().
*	rows	  	The number of image rows (pixels).
*	cols	  	The number of image columns (pixels).
*
*	Returns:        
*	--------
*	NULL if a not successfull, Pointer to image buffer otherwise.
*
*
*       Version:        1.00
*       Author:         Scott Streit
*       Date:           02/05/2001
*
*	Revision History:
*	-----------------
*	Date            Who   Version    Description
*	----------------------------------------------------------------------
*	02/05/2001      sds     1.00    Initial
*
*
*******************************************************************************/
unsigned short *create_memory(HANDLE pci_fd, int rows, int cols)
{
	unsigned short *mem_fd;
#ifdef WIN2K
	ULONG outBuffer = 0;
	ULONG bytesReturned = 0;

	if (!DeviceIoControl((HANDLE)pci_fd, ASTROPCI_MEM_MAP, NULL, 0, &outBuffer,
		sizeof(outBuffer), &bytesReturned, NULL)) {
		sprintf(error_string, "Could not map image buffer. (errno = %d)\n", GetLastError());
		return NULL;
	}

	mem_fd = (unsigned short *)outBuffer;

	return mem_fd;

#else
	printf("\nCalling mmap ...\n");

	/* Map the kernel buffer */
	mem_fd = (unsigned short *)mmap(0, BUFFER_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, (int)pci_fd, 0);

	printf("\nmmap fd = 0x%X\n", mem_fd);

	/* Check for an out of physical memory error. */
	if (mem_fd == NULL && errno == ENOMEM) {
       		sprintf(error_string, "Error Allocating Image Buffers. The Systems \n Physical Limits Have Been Exceeded. \n");
       		return NULL;
       	}

	/* Check for an out of memory error. */
	else if (mem_fd == NULL && errno == EAGAIN) {
       		sprintf(error_string, "Error Allocating Image Buffers. \n Not Enough Memory CURRENTLY Available. \n");
       		return NULL;
        }

	/* Check for any other errors. */
	else if (mem_fd == NULL) {
       		sprintf(error_string, "Error Allocating Memory (errno = %d) \n", errno);
       		return NULL;
        }

	return mem_fd;
#endif
}

/*******************************************************************************
*
*	Function:
*	---------
*	void free_memory()
*
*	Description:
*	------------
*       This function is used to de-allocate the space of an image buffer.
*	The argument to the free_memory function is a pointer to a memory block 
*	previously allocated.
*	This space is made available for further allocation after the function
*	is executed. If  mem_fd is a null pointer, no action occurs.
*	Undefined results will occur if some  random number is passed to the
*	function as a parameter.
*
*	Parameters:     
*	-----------
*	pci_fd  The reference to the PCI driver returned by open().
*	mem_fd	The reference to the image buffer returned by create_memory().
*
*   Returns:        
*	--------
*	None.
*
*       Version:        1.00
*       Author:         Scott Streit
*       Date:           08/16/2001
*******************************************************************************/
void free_memory(HANDLE pci_fd, unsigned short *mem_fd)
{
#ifdef WIN2K
	ULONG inBuffer = 0;
	ULONG bytesReturned = 0;

	if (mem_fd != NULL) {
		if (!DeviceIoControl((HANDLE)pci_fd, ASTROPCI_MEM_UNMAP, &inBuffer,
			sizeof(inBuffer), NULL, 0, &bytesReturned, NULL))
			printf("Could not free image buffer: %d\n", GetLastError());
	}
#else
        if (mem_fd != NULL) 
		munmap((char *)mem_fd, BUFFER_SIZE);
#endif
}

/*******************************************************************************
*
*	Function:
*	---------
*	void swap_memory()
*
*	Description:
*	------------
*       used to byte swap the words within the data Dwords.
*
*	Parameters:     
*	-----------
*	None.
*
*       Returns:        
*	--------
*	None.
*
*       Version:        1.00
*       Author:         Scott Streit
*       Date:           08/16/2001
*******************************************************************************/
void swap_memory(unsigned short *mem_fd)
{
	int x;
	unsigned int temp1;
	unsigned int temp2;
	unsigned int temp3;
	unsigned int temp4;
	unsigned short *fd = mem_fd;

	/* Check for a NULL image buffer. */
	if (mem_fd == NULL) {
       		sprintf(error_string, "Error! Attempting To Access A NULL Image Buffer. \n");
       		return;
       	}

	/* Swap the image data bytes within the words. */
	for (x=0; x<(BUFFER_SIZE/2); x++) {
		temp1 = fd[x];
		temp2 = fd[x];
		
		temp3 = ((temp1 & 0x00ff) << 8);
		temp4 = ((temp2 & 0xff00) >> 8);
		
		fd[x] = (temp4 | temp3);
	}
}
