/*******************************************************************************
*   File:	astropci_main.c
*   Author:	Marco Bonati, modified by Scott Streit
*   Abstract:	Linux device driver for the SDSU PCI Interface Board.
*
*
*   Revision History:     Date      Who   Version    Description
*   --------------------------------------------------------------------------
*                       06/09/00    sds     1.3      Initial
*                       07/11/00    sds     1.4      Changed ioctl GET_REPLY
*                                                    case to use ASTROPCI_READ_REPLY()
*                                                    function.
*				07/13/01	sds		1.6		 Removed alot of code and added
*											 mmap to support version 1.6.
*				09/18/01	sds		1.7		 Fixed unload bug.
*				12/05/01	sds		1.7		 Added image buffer code to astropci_t
*											 struct and removed mem_base[]. Fixed
*											 PCI board probing problem. Correct boards
*											 are now found, but this could still be
*											 improved. Supports multiple boards better.
*
*   Development notes:
*   -----------------
*   This driver has been tested on Redhat Linux 7.1 with Kernel 2.4.2.
*
*   This version is capable of handling multiples boards. The maximum amount
*   of boards is given by the MAX_DEV parameter. Any number of boards minor
*   or equal than MAX_DEV will be automatically handled. At this time, 
*   MAX_DEV can be no more than 4, but it is easy to increase that number
*   (just add more definitions to the "board" constant).
*
*******************************************************************************/
#include <linux/fs.h>	 	/* file_operations */
#include <linux/mm.h>	 	/* vm_area_struct */
#include <linux/wrapper.h>	/* mem_map_reserved() */
#include <linux/wait.h>	 	/* select_table */
#include <linux/timer.h>  	/* timer_list */
/*#include <linux/malloc.h> */	/* kfree */
#include <linux/slab.h>
#include <linux/string.h> 	/* memset, strcpy */
#include <linux/sched.h>  	/* task_struct */
#include <linux/string.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/module.h> 	/* if compiled as module */
#include <asm/irq.h>	 	/* request_irq / free_irq */
#include <asm/ptrace.h>	 	/* pt_regs */
#include <asm/io.h>	 	/* readw, inw ... */ 
#include <asm/param.h>	 	/* HZ */ 
#include <asm/segment.h>	/* get_fs get_base */
#include <asm/pgtable.h>	/* pgd_offset ... */
#include <asm/signal.h>	 	/* SIGUSR1 */
#include <asm/bitops.h>
#include <asm/types.h>
#include <asm/uaccess.h>
#include "astropci_defs.h"
#include "astropci_io.h"

/*******************************************************************************
	Global variables
*******************************************************************************/
static astropci_t astropci[MAX_DEV];
static u_int major[MAX_DEV];
const char *board[] = {"astropci0", "astropci1", "astropci2", "astropci3",};

/*******************************************************************************
	Prototypes for main entry points
*******************************************************************************/
static int astropci_open (struct inode *, struct file*);
static int astropci_close (struct inode *, struct file*);
static int astropci_ioctl (struct inode*, struct file*, unsigned int, unsigned long); 
static int astropci_mmap(struct file *file, struct vm_area_struct *vma);

/*******************************************************************************
	Prototypes for user defined functions
*******************************************************************************/
static int  astropci_attach (void);
static void astropci_deattach (void);
static void astropci_intr (int irq, void *dev_id, struct pt_regs *regs);
static int astropci_flush_reply_buffer(astropci_t *ast);
static int astropci_check_reply_flags(astropci_t *ast);
static int astropci_check_dsp_input_fifo(astropci_t *ast);
static int astropci_check_dsp_output_fifo(astropci_t *ast);
void astropci_init_vars(astropci_t *ast);
int astropci_pci_init(int *devs);
int astropci_check_dsp_buffer(astropci_t *ast);
int astropci_set_buffer_addresses(astropci_t *ast, int dev);
void astropci_wait_while (uint16_t slp);
void astropci_wait_timeout (unsigned long data);
int init_module(void);
void cleanup_module(void);

/*******************************************************************************
	file_operations - character/block entry points structure. Contains all
		 		  entry points for drivers that support both character
		 		  and block entry points. "NULL" specifies that the
		 		  entry point is not in use.
*******************************************************************************/
static struct file_operations astropci_fops = {
	NULL,			/* struct module *owner ??? */
        NULL,			/* seek               */
	NULL,			/* read               */
	NULL,			/* write              */
	NULL,			/* readdir            */
	NULL,			/* select             */
	astropci_ioctl, /* ioctl              */
	astropci_mmap,  /* mmap               */
        astropci_open,	/* open               */
	NULL,			/* flush              */
        astropci_close,	/* release            */
	NULL,			/* fsync              */
	NULL,			/* fasync             */
	NULL,			/* check_media_change */
	NULL,			/* revalidate         */
	NULL			/* lock               */	
};

/*******************************************************************************
 FUNCTION: INIT_MODULE()
 
 PURPOSE:  Initializes the module.
 
 NOTES:    Calls attach() and astro_init_vars(). Called by OS, not user.
*******************************************************************************/
int init_module (void)
{
  	int error;

  	PDEBUG ("astropci: entering to init_module\n"); 

   	if ((error = astropci_attach()) < 0) {
    		cleanup_module();
    		return (error);
  	}

  	return (OK);
}

/*******************************************************************************
 FUNCTION: CLEANUP_MODULE()
 
 PURPOSE:  Remove all module information created by init_module().
 
 NOTES:    Calls detach(). Called by OS, not user.
*******************************************************************************/
void cleanup_module (void)
{  
	short dev;
                              
	PDEBUG("astropci: Unregistering chrdev\n");
	
	/* Unregister the devices */
	for (dev=0; dev<MAX_DEV; dev++) {
		if (astropci[dev].pci != NULL) {
			PDEBUG ("unregistering %d (%s)\n", major[dev], board[dev]);
  			unregister_chrdev(major[dev], board[dev]);
		}
  	}
  	
  	astropci_deattach();
  	PDEBUG("astropci: cleaned\n");
}

/*******************************************************************************
 FUNCTION: ASTROPCI_OPEN()
 
 PURPOSE:  Entry point. Open a device for access. The image buffers are
 	   allocated here.
 
 RETURNS:  Returns 0 for success, or the appropriate error number.
 
 NOTES:    
*******************************************************************************/
static int astropci_open (struct inode *inode, struct file *file)
{
	uint32_t value = 0;
	astropci_t *ast;

	ast = &astropci[MINOR(inode->i_rdev)];

	/* Initialize variables */
	astropci_init_vars(ast);
        
	/* Write 0xFF to the configurations space address 0xD (latency timer) */
	pcibios_write_config_byte(astropci->pci->bus, astropci->pci->dev_fn, 0xD, 0xFF);

	/* Set HCTR bit 8 to 1 and bit 9 to 0 for 32-bit PCI commands -> 24-bit DSP data */
	/* Set HCTR bit 11 to 1 and bit 12 to 0 for 24-bit DSP reply data -> 32-bit PCI data */
	value = readl (&ast->pci->kmem_confreg_base->hctr);
	writel (((value & 0xCFF) | 0x900), &ast->pci->kmem_confreg_base->hctr);

	astropci_flush_reply_buffer(ast);

	MOD_INC_USE_COUNT;

	astropci_printf("Exiting astropci_open; everything ok?\n");

	return (OK);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_CLOSE()
 
 PURPOSE:  Entry point. Close a device from access.
 
 RETURNS:  Returns 0 for success, or the appropriate error number.
 
 NOTES:    
*******************************************************************************/
static int astropci_close (struct inode *inode, struct file *file)
{
	int dev;

	MOD_DEC_USE_COUNT;

	/* Set image buffers to EMPTY */
	for (dev=0; dev<MAX_DEV; dev++) {
		astropci[dev].imageBufferStart = 0;
		astropci[dev].imageBufferEnd = 0;
		astropci[dev].imageBufferMapped = FALSE;
		free_pages((uint32_t)astropci[dev].imageBufferStart, astropci[dev].imageBufferSize);
	}

	return (OK);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_ATTACH()
 
 PURPOSE:  Entry point. Attach a device to the system, or resume it.
 
 RETURNS:  Returns 0 on success.
 
 NOTES:    For a character device (memory-mapped), attach(9e) needs to:
           1. Allocate a state structure and initialize it
           2. Map the device's registers
           3. Add the interrupt handler
           4. Initialize mutexes and condition variables
           5. Create the device's minor node
*******************************************************************************/
static int astropci_attach (void)
{
	astropci_t *ast;
	int dev = 0;
	int devs = 0;
	int error;

  	PDEBUG ("astropci: attaching device\n");
	
	/* Set image buffers to EMPTY */
	for (dev=0; dev<MAX_DEV; dev++) {
		astropci[dev].imageBufferStart = 0;
		astropci[dev].imageBufferEnd = 0;
		astropci[dev].imageBufferMapped = FALSE;
	}

	/* Initialize PCI and IMAGE buffer structures */
	for (dev=0; dev<MAX_DEV; dev++) {
		major[dev] = 0;
		ast = &astropci[dev];
		ast->pci = NULL;
	}
	
	dev = 0;

	/* check PCI - configuration space for possible devices */
 	if ((error = astropci_pci_init (&devs)) < 0)
    		return (error);

  	/* Assign the major node numbers and assign interrupt levels */
  	for (dev=0; dev<devs; dev++) {
    		ast = &astropci[dev];

	    	if (ast == NULL)
	    		return -ENOMEM;
    		
    		if ((major[dev] > 255) || (major[dev] < 0)) {
			astropci_printf ("value %d is not valid for a major number (1-255). I will try to get an automatic one\n",
							 major[dev]);
			major[dev] = 0;      	/* this shouldn't happen, but ... :) */
    		}

     		/* Auto detect major numbers */
       		if (major[dev] == 0) {
        		if ((major[dev] = register_chrdev (major[dev], board[dev], &astropci_fops)) < 0) {
            			astropci_printf("automatic register_chrdev (character device) failed.\n");
            			return (major[dev]);
        		}
        		PDEBUG ("astropci: Automatic reg. major = %d\n", major[dev]);
        		astropci_printf("Automatic reg. major = %d\n", major[dev]);
        	}

		/* Can't auto-detect, so assign at load time*/	    	
	    	else {
        		if ((error = register_chrdev (major[dev], board[dev], &astropci_fops)) < 0) {
            			astropci_printf("manual register_chrdev (%d) (character device) failed.\n", major[dev]);
            			return (error);
        		}
        		PDEBUG("astrocpi: Manual reg. Maj= %d\n", major[dev]);
        		astropci_printf("Manual reg. Maj= %d\n", major[dev]);
        	}

	    	/* Auto assign the device interrupt level */
    		error = request_irq (ast->pci->interrupt_line, &astropci_intr, SA_INTERRUPT | SA_SHIRQ, "astropci",
							 &ast->pci->dev_fn);

    		/* Auto assign failed ... */
    		if (error < 0) {
    			/* ... So try to manually assign interrupt level */
    			ast->pci->interrupt_line = IRQ_NO;
    			if ((error = request_irq (ast->pci->interrupt_line, &astropci_intr, SA_INTERRUPT | SA_SHIRQ, "astropci",
				&ast->pci->dev_fn)) < 0) {
    				astropci_printf("[%d] request_irq (interrupt %d) failed.\n", dev, ast->pci->interrupt_line);
      				return (error);
      			}
     		}
		astropci_printf("[%d] request_irq (interrupt %d) successfully set.\n", dev, ast->pci->interrupt_line);
    	
	    	/* Enable the device interrupts */
    		disable_irq (ast->pci->interrupt_line);
	    	enable_irq (ast->pci->interrupt_line);
    		PDEBUG("astropci: dev %d, irq %d, func %d\n", dev, ast->pci->interrupt_line, ast->pci->dev_fn);
		astropci_printf("dev %d, irq %d, func %d\n", dev, ast->pci->interrupt_line, ast->pci->dev_fn);
	}

	astropci_printf("Exiting astropci_attach with OK status.\n");  

	return (OK);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_DETACH()
 
 PURPOSE:  Entry point. Detach a device.
 
 NOTES:    Essentially, detach() should undo all the resource allocations
           that attach() successfully completes.
*******************************************************************************/
static void astropci_deattach (void)
{
	uint8_t dev;
	astropci_t *ast;

  	astropci_printf("Freeing pointers and interrupt\n");
 	for (dev=0; dev<MAX_DEV; dev++) {
		ast = &astropci[dev];
        	if (ast->pci != NULL) {
			free_irq (ast->pci->interrupt_line, &ast->pci->dev_fn);
			ast->imageBufferStart = 0;
			ast->imageBufferEnd = 0;
			ast->imageBufferMapped = FALSE;
			free_pages((uint32_t)ast->imageBufferStart, ast->imageBufferSize);
                	kfree (ast->pci);
		}
  	}
}

/*******************************************************************************
 FUNCTION: ASTROPCI_IOCTL()
 
 PURPOSE:  Entry point. Control a character device.
 
 RETURNS:  Returns 0 for success, or the appropriate error number.
 
 NOTES:    Locking should be used on a case by case basis, since it is not
	   always necessary. Calls to allocation routines that might sleep
	   should never be locked.
*******************************************************************************/
static int astropci_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	astropci_t *ast;
	uint32_t reg;
	uint32_t reply;
	int retval = 0;

	ast = &astropci[MINOR (inode->i_rdev)];

 	while (test_and_set_bit (0, &ast->comm_busy) != 0)
                astropci_wait_while (5);;
       
	switch (cmd) {
		case ASTROPCI_GET_HCTR: {
			reg = readl(&ast->pci->kmem_confreg_base->hctr);
			
			if (put_user (reg, (uint32_t *)arg))
				retval = -EFAULT;
			} break;

                case ASTROPCI_GET_PROGRESS: {
       		    	uint32_t progress = 0;
       		    	uint32_t upper = 0;
       		    	uint32_t lower = 0;

   			/* Ask the PCI board for the current image address */
   			writel ((uint32_t)READ_PCI_IMAGE_ADDR, &ast->pci->kmem_confreg_base->hcvr);

			/* Read the current image address */
			if (astropci_check_dsp_output_fifo(ast) == OUTPUT_FIFO_OK_MASK) {
	    			lower = readw (&ast->pci->kmem_confreg_base->reply_buffer);
	    			upper = readw (&ast->pci->kmem_confreg_base->reply_buffer);
	    			progress = ((upper << 16) | lower);
			}
	    		else
	    			retval = EFAULT;

                        if (put_user (progress, (uint32_t *)arg))
                                retval = -EFAULT;
                        } break;

		case ASTROPCI_GET_HSTR: {
			reg = readl(&ast->pci->kmem_confreg_base->hstr);
			
			if (put_user (reg, (uint32_t *)arg))
				retval = -EFAULT;
			} break;

      		case ASTROPCI_GET_DMA_ADDR: {
      			if (put_user ((uint32_t)ast->imageBufferStart, (uint32_t *)arg))
	   			retval = EFAULT;
      		    }
      		    break;

                case ASTROPCI_SET_HCTR: {
                        if (get_user (reg, (uint32_t *)arg))
                                retval = -EFAULT;
                        
                        writel(reg, &ast->pci->kmem_confreg_base->hctr);
			} break;

                case ASTROPCI_SET_HCVR: {
			uint32_t hcvr;

			if (get_user (hcvr, (uint32_t *)arg))
	   			retval = -EFAULT;
			else {
				/* Clear the status bits if command not ABORT_READOUT. The pci board can't
				   handle maskable commands (0xXX) during readout.                         */
				if (hcvr != ABORT_READOUT)
					writel ((uint32_t)CLEAR_REPLY_FLAGS, &ast->pci->kmem_confreg_base->hcvr);

	   			/* Pass the command to the PCI board */
	   			writel ((uint32_t)hcvr, (uint32_t *)&ast->pci->kmem_confreg_base->hcvr);
				
				/* Return reply */
				reply = astropci_check_reply_flags(ast);
				if (reply != RDR) {
					if (put_user (reply, (uint32_t *)arg))
						retval = EFAULT;
				}
				else {
					/* Flush the reply buffer */
					astropci_flush_reply_buffer(ast);

					/* Go read some data */
	   				writel ((uint32_t)READ_REPLY_VALUE, (uint32_t *)&ast->pci->kmem_confreg_base->hcvr);
					if (astropci_check_dsp_output_fifo(ast) == OUTPUT_FIFO_OK_MASK) {
						reply = readl (&ast->pci->kmem_confreg_base->reply_buffer);
	   		       			if (put_user (reply, (uint32_t *)arg))
	   						retval = EFAULT;
	   				}
	   				else
	   					retval = EFAULT;
				}
			}

                        } break;

   		case ASTROPCI_HCVR_DATA: {
        		uint32_t cmd_data;

        		if (get_user(cmd_data, (uint32_t *)arg))
	   			retval = EFAULT;
			else {
	   			if (astropci_check_dsp_input_fifo(ast) == INPUT_FIFO_OK_MASK)
	   				writel (cmd_data, (uint32_t *)&ast->pci->kmem_confreg_base->cmd_data);
	   			else
	   				retval = EIO;
	   		}
      		    }
      		    break;

 		case ASTROPCI_COMMAND: {
        		uint32_t cmd_data[6];
        		int i;

        		if (copy_from_user(cmd_data, (uint32_t *)arg, sizeof(cmd_data)))
	   			retval = EFAULT;
			else {
				/* Clear the status bits */
				writel ((uint32_t)CLEAR_REPLY_FLAGS, &ast->pci->kmem_confreg_base->hcvr);

				/* Wait for the FIFO to be empty. */
				if (astropci_check_dsp_input_fifo(ast) != INPUT_FIFO_OK_MASK)
					return EIO;
					
				/* All is well, so write rest of the data. */
				for (i=0; i<6; i++) {
					if (cmd_data[i] == -1)
						break;
					writel (cmd_data[i], (uint32_t *)&ast->pci->kmem_confreg_base->cmd_data);
				}

	   			/* Tell the PCI board to do a WRITE_COMMAND vector command */
	   			writel ((uint32_t)WRITE_COMMAND, &ast->pci->kmem_confreg_base->hcvr);

				/* Return reply */
				cmd_data[0] = astropci_check_reply_flags(ast);
				if (cmd_data[0] != RDR) {
					if (copy_to_user ((uint32_t *) arg, cmd_data, sizeof(cmd_data)))
						retval = EFAULT;
				}
				else {
					/* Flush the reply buffer */
					astropci_flush_reply_buffer(ast);

					/* Go read some data */
	   				writel ((uint32_t)READ_REPLY_VALUE, &ast->pci->kmem_confreg_base->hcvr);
					if (astropci_check_dsp_output_fifo(ast) == OUTPUT_FIFO_OK_MASK) {
						cmd_data[0] = readl (&ast->pci->kmem_confreg_base->reply_buffer);
	   		       			if (copy_to_user ((uint32_t *) arg, cmd_data, sizeof(cmd_data)))
	   						retval = EFAULT;
	   				}
	   				else
	   					retval = EFAULT;
				}
  			}
     		    }
 		    break;
      
    		case ASTROPCI_PCI_DOWNLOAD: {
   			/* This vector command is here because it expects NO reply. */
   			writel ((uint32_t)PCI_DOWNLOAD, &ast->pci->kmem_confreg_base->hcvr);
      		    }  		
      		    break;

    		case ASTROPCI_PCI_DOWNLOAD_WAIT: {
			reply = astropci_check_reply_flags(ast);
			
			if (put_user(reply, (uint32_t *) arg))
				retval = EFAULT;
		    }  		
      		    break;

		case ASTROPCI_MUNMAP: {
			ast->imageBufferStart = 0;
			ast->imageBufferEnd = 0;
			ast->imageBufferMapped = FALSE;
			free_pages((uint32_t)ast->imageBufferStart, ast->imageBufferSize);
			astropci_printf("ASTROPCI_UMNAP: Board[%d] unmapped!\n", ast->pci->devno);
			}
			break;

		default:
			retval = -EINVAL;
			break;
	}
	
	if (test_and_clear_bit (0, &ast->comm_busy) == 0)
                return (-EDEADLK);
        
	return (retval);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_INTR()
 
 PURPOSE:  Entry point. Interrupt handler.
 
 RETURNS:  None.
 
 NOTES:    
*******************************************************************************/
static void astropci_intr (int irq, void *dev_id, struct pt_regs *regs)
{ 
	astropci_t *ast = NULL;
	uint8_t dev = 0;
	uint32_t int_flag = 0;
	uint8_t found = 0;

	PPDEBUG ("astropci: In Interrupt routine\n");

	/* Determine which device (board) is interrupting */
	for (dev=0; dev<MAX_DEV; dev++){
		ast = &astropci[dev];
		if (&ast->pci->dev_fn == (uint8_t *)dev_id) {
			int_flag = readl(&ast->pci->kmem_confreg_base->hstr);

			/* Check that the PCI board is interrupting */
			if (!(int_flag & DMA_INTERRUPTING)) {
				PDEBUG ("astropci: DSP shows no interrupt pending...\n");
				return;
			}
			else {
				found = 1;
				break;
			}
		}
	}	

	/* If no SDSU devices match the interrupting device, then exit */
	if (!found) {
		astropci_printf ("astropci: couldn't found interrupt dev_id (%d) among the known devices\n", (int)dev_id);
		return;
	}

	/* Clear the interrupt. */
	writel ((uint32_t)CLEAR_INTERRUPT, &ast->pci->kmem_confreg_base->hcvr);

	/* OK, we found the device which is interrupting */
	PPDEBUG ("astropci: device %d interrupting\n", dev);

	/* Allow only one access at a time */
        while (test_and_set_bit (0, &ast->int_busy) != 0)
                astropci_wait_while (5);

	clear_bit (0, &ast->int_busy);

	return;
}
 
/*******************************************************************************
 FUNCTION: ASTROPCI_INIT_VARS()
 
 PURPOSE:  Initializes all driver variables.
 
 RETURNS:  None.
 
 NOTES:
*******************************************************************************/
void astropci_init_vars (astropci_t *ast)
{
        ast->comm_busy = 0;
	ast->int_busy = 0;
	init_waitqueue_head(&wait_Q);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_PCI_INIT()
 
 PURPOSE:  This function is called by init module to initialize and create all
	   the necessary stuff. 
 
 RETURNS:  Returns "OK" or the appropriate error message.
 
 NOTES:
*******************************************************************************/
int astropci_pci_init (int *devs)
{
	struct pci_dev *astropci_dev = NULL;
	struct astropci_pci *astropci_pci_ptr = NULL;
	void *v_confreg_base = NULL;
	uint16_t dev;
	uint8_t	irq_line;
	uint8_t astropci_bus;
	uint8_t astropci_dev_fn;
	u_long p_confreg_base;
	u_long align_div;
	u_long align_mod;

  	PPDEBUG ("astropci: initializing PCI\n");
  	dev = *devs = 0;
  
	while ((astropci_dev = pci_find_device (PCI_VENDOR_ID_MOTOROLA, SDSU_PCI_DEVICE_ID, astropci_dev))) {
  		/* Check for different device function number. This prevents the same board from
		   being detected twice. */
		if ((dev > 0) && (astropci[dev-1].pci->dev_fn == astropci_dev->devfn))
			continue;

		astropci_pci_ptr = (struct astropci_pci*)kmalloc (sizeof (struct astropci_pci), GFP_KERNEL);
      
      		if (astropci_pci_ptr == NULL) {
			PDEBUG ("astropci: No memory for pci config\n");
        		return (-ENOMEM);
      		}
      		
      		memset (astropci_pci_ptr, 0x00, sizeof(struct astropci_pci));
   
   		astropci_bus = astropci_dev->bus->number;
    		astropci_dev_fn = astropci_dev->devfn;
    		irq_line = astropci_dev->irq;
    		p_confreg_base =  (u_long)astropci_dev->resource[0].start;	/*base_address[0];*/
 
		/* Check that the PCI control registers are page aligned. */
		if ((align_mod = p_confreg_base % PAGE_SIZE))
          		PDEBUG("astropci[%d]: control regs not aligned.\n", dev);
      		
      		align_div = (p_confreg_base / PAGE_SIZE) * PAGE_SIZE;
      
      		/* Remap the PCI control registers to the driver. */
      		if ((v_confreg_base = ioremap (align_div, REGS_SIZE)) == NULL) {
        		astropci_printf("[%d] unable to ioremap conf regs.\n", dev);
			kfree(astropci_pci_ptr);
        		return -EFAULT;
      		}
    
    		/* Fill in the PCI structure */
    		astropci_pci_ptr->vendor = astropci_dev->vendor;
    		astropci_pci_ptr->device = astropci_dev->device;
    		astropci_pci_ptr->bus = astropci_bus;
    		astropci_pci_ptr->interrupt_line = irq_line;
    		astropci_pci_ptr->dev_fn = astropci_dev_fn;
    		astropci_pci_ptr->devno = dev;
    		astropci_pci_ptr->kmem_confreg_base = (struct astropci_regs*)(v_confreg_base + align_mod);
		astropci[dev].imageBufferMapped = FALSE;

    		PDEBUG ("address of pci %#x\n", (uint32_t)astropci_pci_ptr);
    		astropci_printf("astropci: -------->Initializing devno: %d\n", astropci_pci_ptr->devno);

    		pci_set_master (astropci_dev);
		astropci[dev].pci = astropci_pci_ptr;
    		dev++;
	}
  
	*devs = dev;

	/* Exit and return error if no SDSU PCI boards are found. */
	if (*devs == 0) {
      		astropci_printf("No SDSU PCI devices found.\n");
      		return -ENODEV;
    	}
    	else
      		astropci_printf("%d SDSU PCI device(s) found.\n",  (int)*devs);
    	
	if (*devs > MAX_DEV) {
		astropci_printf("WARNING: %d devices found, but %d devices allowed to handle\n -> coersing to the maximum quantity of devices allowed\n", (int)*devs, MAX_DEV);
		astropci_printf("if you want %d devices to be handled, you will need to recompile the module with MAX_DEV = %d\n", (int)*devs, (int)*devs);
		*devs = MAX_DEV;
   	}

	return (OK);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_CHECK_DSP_BUFFER()
 
 PURPOSE:  Check that the DSP input buffer (FIFO) is not full. 
 
 RETURNS:  Returns 1 if bit 1 (status buf) is set and buffer is available for input.
	   Returns 0 if bit 1 (status buf) is unset and buffer is unavailable for input.

 NOTES:	   This function must be called before writing to any register on the PCI
	   DSP. Otherwise, data may be overwritten in the DSP input buffer since
	   the DSP cannot keep up with the input rate. This function will exit 
	   after "max tries". This will help prevent the system from hanging
	   in case the PCI DSP hangs.
*******************************************************************************/
int astropci_check_dsp_buffer (astropci_t *ast)
{
	uint32_t status = 0;
	uint32_t buffer_status = 0;
	short count = 0;
	const int buffer_ok_mask = READY_FOR_COMMANDS;

        status = readl (&ast->pci->kmem_confreg_base->hstr);
        buffer_status = (status & buffer_ok_mask);

	PPDEBUG ("astropci: checking buffer status %#x\n", status);

        while ((buffer_status != READY_FOR_COMMANDS) && (count < 500)){
                status = readl (&ast->pci->kmem_confreg_base->hstr);
                buffer_status = (status & buffer_ok_mask);
		count++;
		udelay(20);
        }

        return (buffer_status);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_WAIT_WHILE()
 
 PURPOSE:  Sleep for slp clock ticks -- HZ defined on linux/param.h
 
 RETURNS:  None.
 
 NOTES:
*******************************************************************************/
void astropci_wait_while (uint16_t slp)
{
	PDEBUG ("astropci: astropci_wait_while() going to sleep\n");

	del_timer(&wait_timer);
	init_timer (&wait_timer);
        wait_timer.data = (uint32_t) 0;
        wait_timer.function = astropci_wait_timeout;
        wait_timer.expires = jiffies + slp;
	add_timer (&wait_timer);
	sleep_on(&wait_Q);
	del_timer(&wait_timer);

	PDEBUG ("astropci: astropci_wait_while() woke up\n");
}

/*******************************************************************************
 FUNCTION: ASTROPCI_WAIT_TIMEOUT()
 
 PURPOSE:  This function is called from astropci_wait_while(). When the timer
           expires (this is when "jiffies" equals "jiffies + slp") this function
           is called and the astropci_wait_while() function is awakened. 
 
 RETURNS:  None.
 
 NOTES:
*******************************************************************************/
void astropci_wait_timeout (unsigned long data)
{
	wake_up(&wait_Q);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_SET_BUFFER_ADDRESSES
 
 PURPOSE:  Check that the DSP input buffer (FIFO) is not full. 
 
 RETURNS:  Returns 1 if bit 1 (status buf) is set and buffer is available for input.
	   Returns 0 if bit 1 (status buf) is unset and buffer is unavailable for input.

 NOTES:	   This function must be called before writing to any register on the PCI
	   DSP. Otherwise, data may be overwritten in the DSP input buffer since
	   the DSP cannot keep up with the input rate. This function will exit 
	   after "max tries". This will help prevent the system from hanging
	   in case the PCI DSP hangs.
*******************************************************************************/
int astropci_set_buffer_addresses (astropci_t *ast, int dev)
{
	uint32_t phys_address = ast->imageBufferStart;

	/* Pass the DMA kernel buffer address to the DSP */
  	if (astropci_check_dsp_input_fifo(ast) == INPUT_FIFO_OK_MASK) {
	  astropci_printf("In writew stuff\n");
  		writew ((uint16_t)(phys_address & 0x0000FFFF), &ast->pci->kmem_confreg_base->cmd_data);
  		writew ((uint16_t)((phys_address & 0xFFFF0000) >> 16), &ast->pci->kmem_confreg_base->cmd_data);
	}
	
	writel ((uint32_t)WRITE_PCI_ADDRESS, &ast->pci->kmem_confreg_base->hcvr);
	astropci_printf("about to check reply...\n");
	/* Check the reply */
	if (astropci_check_reply_flags(ast) != DON)
		return -ENXIO;

	return (OK);
}

/*******************************************************************************
 FUNCTION: ASTROPCI_FLUSH_REPLY_BUFFER
 
 PURPOSE:  Utility function to clear DSP reply buffer and driver internal value.
 
 RETURNS:  Returns 0 if successful. Non-zero otherwise.
 
 NOTES:
*******************************************************************************/
static int astropci_flush_reply_buffer(astropci_t *ast)
{
	uint32_t reply_status = 0;
	int reply_value = 0;
	int x;
			
	/* Flush the reply buffer FIFO on the PCI DSP. 6 is the number of 24 bit words the FIFO can hold */
   	for (x=0; x<6; x++) {
		reply_status = readl (&ast->pci->kmem_confreg_base->hstr);
   		if ((reply_status & OUTPUT_FIFO_OK_MASK) == OUTPUT_FIFO_OK_MASK)
  			reply_value = readl (&ast->pci->kmem_confreg_base->reply_buffer);
  		else
  			break;
	}

	return reply_value;
}

/*******************************************************************************
 FUNCTION: ASTROPCI_CHECK_STATUS
 
 PURPOSE:  Check the current PCI DSP status. Uses HSTR HTF bits 3,4,5.
 
 RETURNS:  Returns DON if HTF bits are a 1 and command successfully completed.
           Returns RDR if HTF bits are a 2 and a reply needs to be read.
           Returns ERR if HTF bits are a 3 and command failed.
           Returns SYR if HTF bits are a 4 and a system reset occured.

 NOTES:    This function must be called after sending a command to the PCI board
 	   or controller.
*******************************************************************************/
static int astropci_check_reply_flags(astropci_t *ast)
{
	uint32_t status = 0;
	int retval = 0;
   	int try = 0;

	do {
		/* Wait until status is received or max_tries is reached */
		for (try=0; try<BUSY_MAX_TRIES; try++) {
			status = readl (&ast->pci->kmem_confreg_base->hstr);
			if ((status = (status & HTF_BIT_MASK) >> 3) > 0)
				break;

			/* Busy wait BUSY_WAIT_DELAY usec */
			udelay(BUSY_WAIT_DELAY);
		}

		/* If busy waiting failed, sleep and wait around for a longer time */
		if (try >= BUSY_MAX_TRIES) {
			/* Wait until status is received or max_tries is reached */
			for (try=0; try<SLEEP_MAX_TRIES; try++) {
				status = readl (&ast->pci->kmem_confreg_base->hstr);
				if ((status = (status & HTF_BIT_MASK) >> 3) > 0)
					break;

				udelay(SLEEP_WAIT_DELAY);
			}

			if (try >= SLEEP_MAX_TRIES)
				retval = TIMEOUT;
		}

		if (status == 1) {
			retval = DON;
		}
		
		else if (status == 2) {
			retval = RDR;
		}
			
		else if (status == 3) {
			retval = ERR;
		}
			
		else if (status == 4) {
			retval = SYR;
		}
	
		else if (status == 5) {
			retval = READOUT;
		}

		/* Clear the status bits */
		writel ((uint32_t)CLEAR_REPLY_FLAGS, &ast->pci->kmem_confreg_base->hcvr);

	} while (status == 6);

	return retval;
}

/*******************************************************************************
 FUNCTION: ASTROPCI_CHECK_DSP_INPUT_FIFO
 
 PURPOSE:  Check that the DSP input buffer (fifo) is not full.
 
 RETURNS:  Returns INPUT_FIFO_OK_MASK if HSTR bit 1 is set and buffer is
           available for input.

 	   Returns 0 if HSTR bit 1 is unset and buffer is unavailable for input.
 
 NOTES:    This function must be called before writing to any resister in the
 	   astropci_regs structure. Otherwise, data may be overwritten in the
 	   DSP input buffer, since the DSP cannot keep up with the input rate.
 	   This function will exit after "max_tries". This will help prevent
 	   the system from hanging in case the PCI DSP hangs.
*******************************************************************************/
static int astropci_check_dsp_input_fifo(astropci_t *ast)
{
	uint32_t status = 0;
   	int try = 0;

	/* Wait until fifo is ready to receive data */
	for (try=0; try<BUSY_MAX_TRIES; try++) {
		status = readl (&ast->pci->kmem_confreg_base->hstr);
		if ((status = (status & INPUT_FIFO_OK_MASK)) == INPUT_FIFO_OK_MASK)
			break;

		/* Busy wait BUSY_WAIT_DELAY usec */
		udelay(BUSY_WAIT_DELAY);
	}

	/* If busy waiting failed, sleep and wait around for a longer time */
	if (try >= BUSY_MAX_TRIES) {
		/* Wait until fifo is ready to receive data */
		for (try=0; try<SLEEP_MAX_TRIES; try++) {
			status = readl (&ast->pci->kmem_confreg_base->hstr);
			if ((status = (status & INPUT_FIFO_OK_MASK)) == INPUT_FIFO_OK_MASK)
				break;

			udelay(SLEEP_WAIT_DELAY);
		}
	}

	return status;
}

/*******************************************************************************
 FUNCTION: ASTROPCI_CHECK_DSP_OUTPUT_FIFO
 
 PURPOSE:  Check that the DSP output buffer (fifo) has data.
 
 RETURNS:  Returns OUTPUT_FIFO_OK_MASK if HSTR bit 1 is set and buffer is
 	   available for input.
 	   
 	   Returns 0 if HSTR bit 1 is unset and buffer is unavailable for input.
 
 NOTES:    This function must be called before writing to any resister in the
 	   astropci_regs structure. Otherwise, data may be overwritten in the
 	   DSP input buffer, since the DSP cannot keep up with the input rate.
 	   This function will exit after "max_tries". This will help prevent
 	   the system from hanging in case the PCI DSP hangs.
*******************************************************************************/
static int astropci_check_dsp_output_fifo(astropci_t *ast)
{
	uint32_t status = 0;
   	int try = 0;

	/* Wait until fifo has data */
	for (try=0; try<BUSY_MAX_TRIES; try++) {
		status = readl (&ast->pci->kmem_confreg_base->hstr);
		if ((status = (status & OUTPUT_FIFO_OK_MASK)) == OUTPUT_FIFO_OK_MASK)
			break;

		/* Busy wait BUSY_WAIT_DELAY usec */
		udelay(BUSY_WAIT_DELAY);
	}

	/* If busy waiting failed, sleep and wait around for a longer time */
	if (try >= BUSY_MAX_TRIES) {
		/* Wait until fifo has data */
		for (try=0; try<SLEEP_MAX_TRIES; try++) {
			status = readl (&ast->pci->kmem_confreg_base->hstr);
			if ((status = (status & OUTPUT_FIFO_OK_MASK)) == OUTPUT_FIFO_OK_MASK)
				break;

			udelay(SLEEP_WAIT_DELAY);
		}
	}

	return status;
}

/*******************************************************************************
 FUNCTION: ASTROPCI_MMAP
	
	   - Many Thanks To Brian Taylor For Supplying This Code.
 
 PURPOSE:  Map the image buffer from unused RAM.
 
 RETURNS:  Returns 0 if memory was mapped, -ENOMEM otherwise.
 
 NOTES:    For this function to work, the user must specify enough buffer
	   space using the lilo command: append="mem=xxxM". If you have 128M
	   RAM and want a 28M image buffer, then use: append="mem=100M".
*******************************************************************************/
static int astropci_mmap(struct file *file, struct vm_area_struct *vma)
{
	astropci_t *ast = NULL;
	unsigned long prot = pgprot_val(vma->vm_page_prot);
	int dev;

	astropci_printf("Entering astropci_mmp\n");
	
	for (dev=0; dev<MAX_DEV; dev++) {
		if (!astropci[dev].imageBufferMapped) {
			ast = &astropci[dev];
			break;
		}
		else if (dev >= MAX_DEV)
			return -ENOMEM;
	}

	ast->imageBufferMapped = TRUE;
	astropci_printf("Grabbing RAM for board[%d]\n", dev);
	/* all okay to here */

	if (dev == 0) {
		astropci_printf("astropciMMAP: =====> Mapping for dev 0\n");
		ast->imageBufferStart = (unsigned long)__pa(high_memory);
		ast->imageBufferSize = (uint32_t)(vma->vm_end-vma->vm_start);
		ast->imageBufferEnd = ast->imageBufferStart + ast->imageBufferSize;
	}

	astropci_printf("astropciMMAP: Board %d  Buffer start: 0x%X  end: 0x%X size: %d\n", dev, ast->imageBufferStart, ast->imageBufferEnd, ast->imageBufferSize);

	/* all okay to here */

	
	if (boot_cpu_data.x86 > 3) {
		prot |= _PAGE_PWT | _PAGE_PCD;
		vma->vm_page_prot = __pgprot(prot);
	}
	vma->vm_page_prot = PAGE_SHARED;

	/* okay to here */

	
	/* Don't try to swap out physical pages */
	vma->vm_flags |= VM_RESERVED;

	/* Don't dump addresses that are not real memory to a core file */
	vma->vm_flags |=  VM_IO;

	/* okay to here */

	
	/* Remap the page range to see the high memory */
	if (remap_page_range(vma->vm_start, ast->imageBufferStart, vma->vm_end-vma->vm_start, vma->vm_page_prot)) {
		PDEBUG ("astropci MMAP: Remap page range failed.\n");
		return -ENOMEM;
	}

	astropci_printf ("astropci MMAP: High memory mapped, 0x%lX - 0x%lX for %d bytes\n", vma->vm_start, vma->vm_end,
				 (int)ast->imageBufferSize);

	/* okay to here */

	
        /* Assign the image buffers */
	if (astropci_set_buffer_addresses(&astropci[dev], dev) < 0) {
	  astropci_printf("failed on astropci_set_buffer_addresses\n");
	  return -ENOMEM;
	}


	astropci_printf("Leaving astropci_mmap\n");

	return 0;
}





