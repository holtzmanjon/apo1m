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

#ifdef DEBUG_ASTROPCI
        #define PDEBUG(fmt, args...) printk (KERN_WARNING fmt, ## args)
#else
        #define PDEBUG(fmt, args...)
#endif

#define PPDEBUG(fmt, args...)
#define astropci_printf(fmt, args...) printk("<1>astropci: " fmt, ## args)

/******************************************************************************
        General Definitions
******************************************************************************/
#define MAX_DEV                 4   /*max devices*/
#define SDSU_PCI_DEVICE_ID      0x1801
#define REGS_SIZE              (0x9C/sizeof(uint32_t))*sizeof(uint32_t)

#define INPUT_FIFO  0  /* For astropci_wait_for_condition() */
#define OUTPUT_FIFO 1
#define CHECK_REPLY 2

/******************************************************************************
        PCI DSP Control Registers
******************************************************************************/

#if 0
const int reserved1    = 0x00;    
const int reserved2    = 0x04;    
const int reserved3    = 0x08;    
const int reserved4    = 0x0C;   
#endif 

const int hctr         = 0x10; /* Host interface control register       */
const int hstr         = 0x14; /* Host interface status register        */
const int hcvr         = 0x18; /* Host command vector register          */
const int reply_buffer = 0x1C;
const int cmd_data     = 0x20; /* DSP command register                  */

#if 0
const int unused1      = 0x24; /* Board destination (PCI,Timing,Utility)*/
const int arg1         = 0x28; /* DSP command argument #1               */
const int arg2         = 0x2C; /* DSP command argument #2               */
const int arg3         = 0x30; /* DSP command argument #3               */
const int arg4         = 0x34; /* DSP command argument #4               */
const int arg5         = 0x38; /* DSP command argument #5               */
const int unused2      = 0x3C; /* Reply buffer address                  */
const int unused3      = 0x40; /* DMA kernel buffer 1 address           */
const int unused4      = 0x44; /* DMA kernel buffer 2 address           */
#endif 

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
