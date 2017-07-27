/******************************************************************************
*
*   File:       Driver.h
*   Version:	1.00
*   Author:     Scott Streit
*   Abstract:   Defines device driver values.
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*	10/30/02			sds		1.00	Initial
*
******************************************************************************/
#ifndef __DRIVER__
#define __DRIVER__

#include "./WhichSystem.h"

#ifdef WIN2K
	#include <windows.h>
#endif

/* FUNCTION PROTOTYPES */
HANDLE openDriver(char *PCI_DEVICE_NAME);
void closeDriver(HANDLE pci_fd);

#endif
