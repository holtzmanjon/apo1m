/*******************************************************************************
*   File:	astropci_defs.h
*   Author:	Marco Bonati, modified by Scott Streit
*   Abstract:	Linux device driver for the SDSU PCI Interface Board. This file
*		contains the initialization and all user defined functions for
*		the driver.
*
*
*   Revision History:     Date      Who   Version    Description
*   --------------------------------------------------------------------------
*                       06/16/00    sds     1.3      Removed misc defines no
*                                                    longer used by the driver.
*                       07/11/00    sds     1.4      Added function prototype
*                                                    for astropci_read_reply().
*                       01/26/01    sds     1.4      Just changed some values.
*				07/13/01	sds		1.6		 Removed alot of code to
*											 support version 1.6.
*
*   Development notes:
*   -----------------
*   This driver has been tested on Redhat Linux 7.1 with Kernel 2.4.2.
*******************************************************************************/
#ifndef _ASTROPCI_DEFS_H
#define _ASTROPCI_DEFS_H


#ifdef __KERNEL__
#include <linux/version.h>       /* LINUX_VERSION_CODE */
#include <linux/sched.h>         /* struct task_struct */
#include <asm/page.h>            /* PAGE_SIZE, PAGE_ALIGN */
#endif

#ifdef DEBUG_ASTROPCI
	#define PDEBUG(fmt, args...) printk (KERN_WARNING fmt, ## args)
#else
	#define PDEBUG(fmt, args...)
#endif

#define PPDEBUG(fmt, args...)
#define astropci_printf(fmt, args...) printk("<1>astropci: " fmt, ## args)

/*******************************************************************************
	General Definitions
*******************************************************************************/
#define MAX_DEV		      	4   /*max devices*/
#define IRQ_NO				9
#define SDSU_PCI_DEVICE_ID	0x1801

#define REGS_SIZE			(0x9C/sizeof(uint32_t))*sizeof(uint32_t)
#define OK				0
#define READY_FOR_COMMANDS	0x2
#define CAMREG_OFF			0x80
#define EMPTY				-1
#define TRUE				1
#define FALSE				0

/* General wait queue, used for bit test operations. */
wait_queue_head_t wait_Q;
struct timer_list wait_timer;

/*******************************************************************************
	PCI DSP Control Registers
*******************************************************************************/
struct astropci_regs {
        volatile uint32_t reserved1;    
        volatile uint32_t reserved2;    
        volatile uint32_t reserved3;    
        volatile uint32_t reserved4;   
        volatile uint32_t hctr;       	/*0x10 Host interface control register */
        volatile uint32_t hstr;       	/*0x14 Host interface status register */
        volatile uint32_t hcvr;       	/*0x18 Host command vector register */
        volatile uint32_t reply_buffer;	/* 0x1C  */
        volatile uint32_t cmd_data;    	/* 0x20 DSP command register    */
        volatile uint32_t unused1;		/*0x24 Board destination (PCI,Timing,Utility)*/
        volatile uint32_t arg1;       	/* 0x28 DSP command argument #1    */
        volatile uint32_t arg2;       	/* 0x2C DSP command argument #2   */
        volatile uint32_t arg3;       	/* 0x30 DSP command argument #3    */
        volatile uint32_t arg4;       	/* 0x34 DSP command argument #4 */
        volatile uint32_t arg5;       	/* 0x38 DSP command argument #5 */
        volatile uint32_t unused2; 		/* 0x3C Reply buffer address     */
        volatile uint32_t unused3;  	/* 0x40 DMA kernel buffer 1 address */
        volatile uint32_t unused4;  	/* 0x44 DMA kernel buffer 2 address */
};

/*************************************************************************** 
	PCI Structure - Structure that describes the device's PCI info.
			Essentially configuration register info.
***************************************************************************/
struct astropci_pci {
	/*virt. memory base address for PCI - Interface Control Registers */
	struct astropci_regs *kmem_confreg_base;

	uint16_t vendor;
	uint16_t device;
	uint8_t	dev_fn;
	uint8_t	devno;	
	uint8_t	bus;
	uint8_t interrupt_line;
	uint8_t revision_id;            /* board revision number */
	uint16_t stat_reg;              /* board revision level type */
};

/*************************************************************************** 
	State Structure - Driver state structure. All state variables related
				  to the state of the driver are kept in here.
***************************************************************************/
typedef struct astropci_state {
  	struct astropci_pci	*pci;
	short				opened;
	uint32_t			imageBufferSize;
	uint32_t			imageBufferStart;
	uint32_t			imageBufferEnd;
	uint8_t				imageBufferMapped;
	volatile int 		comm_busy;
	volatile int		int_busy;
} astropci_t;

#endif
