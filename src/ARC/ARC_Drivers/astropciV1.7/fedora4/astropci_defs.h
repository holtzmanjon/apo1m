/******************************************************************************
*   File:       astropci_defs.h
*   Author:     Marco Bonati, modified by Scott Streit and Michael Ashley
*   Abstract:   Linux device driver for the SDSU PCI Interface Board. This file
*               contains the initialization and all user defined functions for
*               the driver.
*
*
*   Revision History:     Date      Who   Version    Description
*   --------------------------------------------------------------------------
*                       06/16/00    sds     1.3      Removed misc defines no
*                                                    longer used by the driver.
*                       07/11/00    sds     1.4      Added function prototype
*                                                    for astropci_read_reply().
*                       01/26/01    sds     1.4      Just changed some values.
*                       07/13/01    sds     1.6      Removed a lot of code to
*                                                    support version 1.6.
*                       09-Jan-2003 mcba    1.7B     Removed lots more code.
*   Development notes:
*   -----------------
*   This driver has been tested on Redhat Linux 7.2 with Kernel 2.4.20.
******************************************************************************/
#ifndef _ASTROPCI_DEFS_H
#define _ASTROPCI_DEFS_H

/******************************************************************************
        General Definitions
******************************************************************************/
#define DRIVER_NAME		(char *)"astropci"
#define DRIVER_VERSION		(char *)"1.7 Jan 13, 2004"
#define SUPPORTED_KERNELS	(char *)"2.4.x (Redhat 7.2, 9), 2.6.5-1.358 (Fedora Core2)"
#define MAX_DEV                 4   /* Max number of devices */
#define ARC_PCI_DEVICE_ID      0x1801
#define REGS_SIZE              (0x9C/sizeof(uint32_t))*sizeof(uint32_t)
#define DEBUG_OFF
#define CMD_MAX			6

#define INPUT_FIFO  0  /* For astropci_wait_for_condition() */
#define OUTPUT_FIFO 1
#define CHECK_REPLY 2

/******************************************************************************
        Debug Print Definitions
******************************************************************************/
#ifdef DEBUG_ASTROPCI
        #define PDEBUG(fmt, args...) printk (KERN_WARNING fmt, ## args)
#else
        #define PDEBUG(fmt, args...)
#endif

#define PPDEBUG(fmt, args...)
#define astropci_printf(fmt, args...) printk("<1>astropci: " fmt, ## args)

/******************************************************************************
        PCI DSP Control Registers
******************************************************************************/
#define HCTR		0x10	/* Host interface control register      */
#define HSTR		0x14	/* Host interface status register       */
#define HCVR		0x18	/* Host command vector register         */
#define REPLY_BUFFER	0x1C	/* Reply Buffer				*/
#define CMD_DATA	0x20	/* DSP command register                 */

/*************************************************************************** 
        State Structure - Driver state structure. All state variables related
                                  to the state of the driver are kept in here.
***************************************************************************/
typedef struct astropci_state {
        struct pci_dev *pdev;
        long           ioaddr;
        uint8_t        have_irq;
        uint32_t       imageBufferSize;
        uint32_t       imageBufferStart;
        short          opened;
} astropci_t;

#endif
