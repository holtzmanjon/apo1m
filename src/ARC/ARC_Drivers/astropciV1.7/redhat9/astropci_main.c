/******************************************************************************
*   File:       astropci_main.c
*   Author:     Marco Bonati, modified by Scott Streit, 
*               Brian Taylor, and Michael Ashley
*   Abstract:   Linux device driver for the SDSU PCI Interface Board.
*
*
*   Revision History:
*     
*   Date      Who Version Description
* -----------------------------------------------------------------------------
* 09-Jun-2000 sds  1.3    Initial.
* 11-Jul-2000 sds  1.4    Changed ioctl GET_REPLY
*                         case to use ASTROPCI_READ_REPLY()
*                         function.
* 13-Jul-2001 sds  1.6    Removed alot of code and added
*                         mmap to support version 1.6.
* 18-Sep-2001 sds  1.7    Fixed unload bug.
* 05-Dec-2001 sds  1.7    Added image buffer code to astropci_t
*                         struct and removed mem_base[]. Fixed
*                         PCI board probing problem. Correct boards
*                         are now found, but this could still be
*                         improved. Supports multiple boards better.
* 11-Jun-2002 sds  1.7    Improved mmap management system. Still
*                         cannot recover used memory very well, however.
* 09-Jan-2002 mcba 1.7    Using new PCI probing technique; register memory
*                         regions; spin_locks; open now enforces only
*                         one process per board; removed unnecessary includes;
*                         removed lots of code; reduced size of static structs;
*                         MAX_DEV consistency check; more use of MOD_INC/DEC_
*                         USAGE_COUNT; +ve error returns made -ve; added
*                         astropci_wait_for_condition, and made it sleep for
*                         long delays; fixed memory caching in mmap; added
*                         /proc interface; MODULE_ stuff; EXPORT_NO_SYMBOLS;
*                         LINUX_VERSION_CODE switches; removed unnecessary
*                         wait queues.
* 15-Apr-2003 sds  1.7    Re-added support for multiple boards. Fixed mmap
*			  to support 2.4.20-8 kernel.
*
*   Development notes:
*   -----------------
*   This driver has been tested on Redhat Linux 7.2 with Kernel 2.4.20.
*   It compiles under at least 2.4.2, 2.4.7, 2.4.18, and 2.4.20.
*
*   This version is capable of handling multiple boards. The maximum number
*   of boards is given by the MAX_DEV parameter. Any number of boards less than
*   or equal to MAX_DEV will be automatically handled. At this time, 
*   MAX_DEV is set to 4, but it is easy to increase that number
*   (you must also add more definitions to the "board" constant).
*
******************************************************************************/
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include "astropci_defs.h"
#include "astropci_io.h"

/******************************************************************************
        Global variables
******************************************************************************/
static char *astropci_version = "V1.7 030414a";   // Format: YYMMDDx
static astropci_t astropci[MAX_DEV];
const  char *board[] = {"astropci0", "astropci1", "astropci2", "astropci3"};
static u_int major[MAX_DEV];
static uint32_t nextValidStartAddress;
static spinlock_t astropci_lock = SPIN_LOCK_UNLOCKED;
static int ndev = 0; // Total number of boards found
static long max_elapsed_time = 0L;

/******************************************************************************
        Prototypes for main entry points
******************************************************************************/
static int astropci_open (struct inode *, struct file *);
static int astropci_close (struct inode *, struct file *);
static int astropci_ioctl (struct inode *, struct file *, unsigned int, unsigned long);
static int astropci_mmap (struct file *file, struct vm_area_struct *vma);

/******************************************************************************
        Prototypes for other functions
******************************************************************************/
static int __devinit astropci_probe (struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit astropci_remove (struct pci_dev *pdev);
static void astropci_intr (int irq, void *dev_id, struct pt_regs *regs);
static int astropci_read_proc (char *buf, char **start, off_t offset, int len);
static int astropci_flush_reply_buffer (int dev);
static int astropci_check_reply_flags (int dev);
static int astropci_check_dsp_input_fifo (int dev);
static int astropci_check_dsp_output_fifo (int dev);
static int astropci_wait_for_condition (int dev, int condition_to_wait_for);
static int astrocpi_check_for_readout (int dev);
int astropci_set_buffer_addresses (int dev);
int init_module (void);
void cleanup_module (void);

/******************************************************************************
        Structures used by the kernel PCI API
******************************************************************************/
static struct pci_device_id astropci_pci_tbl[] __devinitdata =
{
        {PCI_VENDOR_ID_MOTOROLA, SDSU_PCI_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
        {0,}
};
MODULE_DEVICE_TABLE(pci, astropci_pci_tbl);

static struct pci_driver astropci_driver = {
        name:           "astropci",
        id_table:       astropci_pci_tbl,
        probe:          astropci_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
        remove:         __devexit_p(astropci_remove),
#else
        remove:         astropci_remove,
#endif
};

/******************************************************************************
        file_operations - character/block entry points structure. Contains
                          entry points for drivers that support both character
                          and block entry points. We only specify the functions
                          that we provide.
******************************************************************************/
static struct file_operations astropci_fops = {
        owner:   THIS_MODULE,
        ioctl:   astropci_ioctl,
        mmap:    astropci_mmap,
        open:    astropci_open,
        release: astropci_close
};

/******************************************************************************
 FUNCTION: INIT_MODULE()
 
 PURPOSE:  Initializes the module.
 
 NOTES:    Called by Linux during insmod, not by the user.
******************************************************************************/
int init_module(void)
{
	int i;
	int retval;

	astropci_printf("ASTROPCI --> Start init_module\n");
        astropci_printf("version %s\n", astropci_version);

        if (MAX_DEV != (sizeof board) / sizeof (char *)) {
                astropci_printf("MAX_DEV error, please recompile\n");
                return -ENODEV;
        }

	retval = pci_module_init(&astropci_driver);

	for (i=0; i<ndev; i++) {
        	if ((major[i] = register_chrdev(0, board[i], &astropci_fops)) < 0) {
                	astropci_printf("char device allocation failed\n");
                	return major[i];
        	}

        	if (!create_proc_info_entry(board[i], 0, NULL, astropci_read_proc)) {
                	astropci_printf("can not create proc entry\n");
                	unregister_chrdev(major[i], board[i]);
                	major[i] = 0;
                	return -ENODEV;
        	}

	        astropci_printf("major device number %d\n", major[i]);
	}

        astropci_printf("Number of PCI boards found: %d\n", ndev);
	astropci_printf("ASTROPCI --> End init_module\n");

        nextValidStartAddress = 0;
        return retval;
}

/******************************************************************************
 FUNCTION: CLEANUP_MODULE()
 
 PURPOSE:  Remove all resources allocated by init_module().
 
 NOTES:    Called by Linux during rmmod, not by the user.
******************************************************************************/
void
cleanup_module(void)
{
	int i;

	for (i=0; i<ndev; i++) {
        	if (major[i] > 0) {
                	unregister_chrdev(major[i], board[i]);
                	remove_proc_entry(board[i], NULL);
        	}
	}

        pci_unregister_driver (&astropci_driver);
        astropci_printf("unloaded\n");
}

/******************************************************************************
 FUNCTION: ASTROPCI_PROBE()
 
 PURPOSE:  This function is called by pci_module_init for each PCI device
           that matches astropci_pci_tbl.
 
 RETURNS:  Returns 0 for success, or the appropriate error number.
 
 NOTES:    See Documentation/pci.txt in the Linux sources.
******************************************************************************/
static int __devinit 
astropci_probe (struct pci_dev *pdev,
                const struct pci_device_id *ent)
{
        int dev = 0;
        long memaddr;

	astropci_printf("ASTROPCI --> Start Probe\n");

        if (ndev == MAX_DEV) {
                astropci_printf("WARNING: more than %d devices found\n -> coersing to the maximum quantity of devices allowed\n -> please recompile if you need more\n", MAX_DEV);
                return 0;
        }

        dev = ndev;
        ndev++;

        if (pci_enable_device(pdev))
                goto err_out;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
        if (pci_request_region(pdev, 0, (char *)board[dev]))
                goto err_out;
#endif
        /* Set the private data field in pdev, so that we can determine
           which board is referred to. Note that we are storing the 
           current value of dev, not a pointer to it.*/ 
        pci_set_drvdata(pdev, (void *)dev);

        memset(&astropci[dev], 0x00, sizeof (struct astropci_state));
        astropci[dev].pdev = pdev;

        memaddr = pci_resource_start(pdev, 0);
        astropci[dev].ioaddr = (long) ioremap_nocache(memaddr, REGS_SIZE);
        if (!astropci[dev].ioaddr) {
                astropci_printf(
                        "ioremap failed for device %s, region 0x%X @ 0x%lX\n",
                        pdev->slot_name, REGS_SIZE, memaddr);
                goto err_out_free_res;
        }
        
        /* Request the device interrupt level; note, we use a dev_id of
           dev+1 to avoid the "Bad boy" message from arch/i386/kernel/irq.c */
        if (request_irq(pdev->irq, &astropci_intr, SA_INTERRUPT | SA_SHIRQ,
                        board[dev], (void *)(dev+1)) < 0) {
                astropci_printf("request IRQ %d failed\n", pdev->irq);
                goto err_out_unmap;
        }
        astropci[dev].have_irq = 1;

        /* Enable the device interrupts */
        disable_irq(pdev->irq);
        enable_irq(pdev->irq);
        astropci_printf(
                "successfully probed dev %d, irq %d, func %d\n", 
                dev, pdev->irq, pdev->devfn);

	astropci_printf("ASTROPCI --> End Probe\n");

        return 0;

 err_out_unmap:
        iounmap((void *)astropci[dev].ioaddr);
 err_out_free_res:
        pci_release_regions(pdev);
 err_out:
        ndev--;
        return -ENODEV;
}

/******************************************************************************
 FUNCTION: ASTROPCI_REMOVE()
 
 PURPOSE:  This function is called by pci_unregister_driver for each 
           PCI device that was successfully probed.
 
 NOTES:    See Documentation/pci.txt in the Linux sources.
******************************************************************************/
static void __devexit 
astropci_remove (struct pci_dev *pdev)
{
        int dev;
      
        dev = (int)pci_get_drvdata(pdev);

	astropci_printf("Removing device #: %d\n", dev);

        if (dev >= 0 && dev < MAX_DEV && astropci[dev].have_irq) {
                disable_irq(pdev->irq);
                free_irq(pdev->irq, (void *)(dev+1));
                astropci[dev].have_irq = 0;
                iounmap((void *)(astropci[dev].ioaddr));
        } else {
                astropci_printf("bad dev %d\n", dev);
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
        pci_release_regions(pdev);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,3)
        pci_disable_device(pdev);
#endif
        pci_set_drvdata(pdev, (void *)-1);
}

/******************************************************************************
 FUNCTION: ASTROPCI_OPEN()
 
 PURPOSE:  Entry point. Open a device for access.
 
 RETURNS:  Returns 0 for success, or the appropriate error number.
******************************************************************************/
static int
astropci_open(struct inode *inode, struct file *file)
{
        uint32_t value;
        int dev;
        int retval = 0;

        MOD_INC_USE_COUNT;

        /* We use a lock to protect global variables */
        spin_lock(&astropci_lock);

        /* Obtain the minor device number, which is used to determine
           which of the possible SDSU cards we want to open */
        dev = MINOR(inode->i_rdev);

        /* Check that this device actually exists */
        if (dev < 0 || dev >= MAX_DEV || !astropci[dev].have_irq) {
                retval = -ENXIO;
                goto exit;
        }

        /* Allow only one process to open the device at a time */
        if (astropci[dev].opened) {
                retval = -EBUSY;
                goto exit;
        }

        /* Write 0xFF to the configuration-space address PCI_LATENCY_TIMER */
        pci_write_config_byte(astropci[dev].pdev, PCI_LATENCY_TIMER, 0xFF);

        /* Set HCTR bit 8 to 1 and bit 9 to 0 for 
               32-bit PCI commands   -> 24-bit DSP data */

        /* Set HCTR bit 11 to 1 and bit 12 to 0 for 
               24-bit DSP reply data -> 32-bit PCI data */
        value = readl(astropci[dev].ioaddr + hctr);
        writel(((value & 0xCFF) | 0x900), astropci[dev].ioaddr + hctr);

        if (astropci_flush_reply_buffer(dev)) {
                astropci_printf("flush_reply_buffer failed\n");
                retval = -EACCES;
                goto exit;
        }

        /* Set the device state to opened */
        astropci[dev].opened = 1;

        /* Store the board number into the file structure, so that
           mmap() can easily determine the board */
        file->private_data = (void *)dev;

 exit:
        spin_unlock(&astropci_lock);
        if (retval) MOD_DEC_USE_COUNT;
        return retval;
}

/******************************************************************************
 FUNCTION: ASTROPCI_CLOSE()
 
 PURPOSE:  Entry point. Close a device from access.
 
 RETURNS:  Returns 0 always.
******************************************************************************/
static int
astropci_close(struct inode *inode, struct file *file)
{
        int dev;

        spin_lock(&astropci_lock);

        dev = MINOR(inode->i_rdev);

        /* Set the device state to closed */
        astropci[dev].opened = 0;
        nextValidStartAddress = astropci[dev].imageBufferStart; // DANGER
        astropci[dev].imageBufferStart = 0;

        /* Check if all devices are closed. If so, 
           reset nextValidStartAddress */
        for (dev = 0; dev < MAX_DEV; dev++) {
                if (astropci[dev].opened) break;
        }
        if (dev != MAX_DEV) {
                nextValidStartAddress = 0;
        }

        spin_unlock(&astropci_lock);
        MOD_DEC_USE_COUNT;
        return 0;
}

/******************************************************************************
 FUNCTION: ASTROPCI_IOCTL()
 
 PURPOSE:  Entry point. Control a character device.
 
 RETURNS:  Returns 0 for success, or the appropriate error number.
 
 NOTES:    Locking is used to prevent simultaneous access.
******************************************************************************/
static int
astropci_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
               unsigned long arg)
{

        uint32_t reg;
        uint32_t reply;
        int retval = 0;
        int dev;

        MOD_INC_USE_COUNT;
        spin_lock(&astropci_lock);

        dev = MINOR(inode->i_rdev);

        /* Check that this device actually exists */
        if (dev < 0 || dev >= MAX_DEV || !astropci[dev].have_irq) {
                retval = -ENXIO;
                goto exit;
        }

        switch (cmd) {
        case ASTROPCI_GET_HCTR:
                reg = readl(astropci[dev].ioaddr + hctr);
                if (put_user(reg, (uint32_t *) arg))
                        retval = -EFAULT;
                break;

        case ASTROPCI_GET_PROGRESS: {
                uint32_t progress = 0;
                uint32_t upper = 0;
                uint32_t lower = 0;

                /* Ask the PCI board for the current image address */
                writel((uint32_t) READ_PCI_IMAGE_ADDR, astropci[dev].ioaddr + hcvr);
                
                /* Read the current image address */
                if (astropci_check_dsp_output_fifo(dev)) {
                        lower = readw(astropci[dev].ioaddr + reply_buffer);
                        upper = readw(astropci[dev].ioaddr + reply_buffer);
                        progress = ((upper << 16) | lower);
                } else
                        retval = -EFAULT;
                
                if (put_user(progress, (uint32_t *) arg))
                        retval = -EFAULT;
        }
                break;

        case ASTROPCI_GET_FRAMES_READ: {
 	    	uint32_t pciFrameCount = 0;
  	    	uint32_t upper = 0;
  	    	uint32_t lower = 0;

  		/* Ask the PCI board for the current image address */
   		writel ((uint32_t)READ_NUMBER_OF_FRAMES_READ, astropci[dev].ioaddr + hcvr);

		/* Read the current image address */
		if (astropci_check_dsp_output_fifo(dev) == OUTPUT_FIFO_OK_MASK) {
	 		lower = readw (astropci[dev].ioaddr + reply_buffer);
	 		upper = readw (astropci[dev].ioaddr + reply_buffer);
	 		pciFrameCount = ((upper << 16) | lower);
		}
	 	else
	 		retval = EFAULT;

                    if (put_user (pciFrameCount, (uint32_t *)arg))
                    	retval = -EFAULT;
        }
		break;

        case ASTROPCI_GET_HSTR:
                reg = readl(astropci[dev].ioaddr + hstr);
                if (put_user(reg, (uint32_t *) arg))
                        retval = -EFAULT;
                break;

        case ASTROPCI_GET_DMA_ADDR:
                if (put_user((uint32_t) astropci[dev].imageBufferStart, (uint32_t *) arg))
                        retval = -EFAULT;
                break;

        case ASTROPCI_SET_HCTR:
                if (get_user(reg, (uint32_t *) arg)) {
                        retval = -EFAULT;
                } else {
                        writel(reg, astropci[dev].ioaddr + hctr);
                }
                break;

        case ASTROPCI_SET_HCVR: {
                uint32_t Hcvr;

                if (get_user(Hcvr, (uint32_t *) arg))
                        retval = -EFAULT;
                else {
                        /* Clear the status bits if command not ABORT_READOUT. 
                           The pci board can't handle maskable commands 
                           (0xXX) during readout. */
                        if (Hcvr != ABORT_READOUT)
                                writel((uint32_t) CLEAR_REPLY_FLAGS,
                                       astropci[dev].ioaddr + hcvr);

                        /* Pass the command to the PCI board */
                        writel(Hcvr, astropci[dev].ioaddr + hcvr);

                        /* Return reply */
                        reply = astropci_check_reply_flags(dev);
                        if (reply != RDR) {
                                if (put_user(reply, (uint32_t *) arg))
                                        retval = -EFAULT;
                        } else {
                                /* Flush the reply buffer */
                                astropci_flush_reply_buffer(dev);

                                /* Go read some data */
                                writel((uint32_t) READ_REPLY_VALUE,
                                       astropci[dev].ioaddr + hcvr);
                                if (astropci_check_dsp_output_fifo(dev)) {
                                        reply = readl(astropci[dev].ioaddr + reply_buffer);
                                        if (put_user(reply, (uint32_t *) arg))
                                                retval = -EFAULT;
                                } else
                                        retval = -EFAULT;
                        }
                }
        }
                break;

        case ASTROPCI_HCVR_DATA: {
                uint32_t Cmd_data;

                if (get_user(Cmd_data, (uint32_t *) arg))
                        retval = -EFAULT;
                else {
                        if (astropci_check_dsp_input_fifo(dev))
                                writel(Cmd_data, astropci[dev].ioaddr + cmd_data);
                        else
                                retval = -EIO;
                }
        }
                break;

        case ASTROPCI_COMMAND: {
                uint32_t Cmd_data[6];
                int i;
		int numberOfParams = 0;

                if (copy_from_user(Cmd_data, (uint32_t *) arg, sizeof (Cmd_data)))
                	retval = -EFAULT;

                else {
                        /* Clear the status bits */
                        writel((uint32_t) CLEAR_REPLY_FLAGS, astropci[dev].ioaddr + hcvr);
                        
                        /* Wait for the FIFO to be empty. */
                        if (!astropci_check_dsp_input_fifo(dev)) {
                                retval = -EIO;
                        } else {

				/* Check that the controller is not in readout */
				if (astrocpi_check_for_readout(dev)) {
					retval = -EFAULT;
					break;
				}

				/* Get the number of command parameters. */
				numberOfParams = Cmd_data[0] & 0x000000FF;
				if (numberOfParams > CMD_MAX) {
					astropci_printf("Incorrect number of command parameters!\n");
					retval = -EFAULT;
					break;
				}

                                /* All is well, so write rest of the data. */
                                for (i = 0; i < numberOfParams; i++)
                                         writel(Cmd_data[i], astropci[dev].ioaddr + cmd_data);

                                /* Tell the PCI board to do a WRITE_COMMAND vector command */
                                writel((uint32_t) WRITE_COMMAND, astropci[dev].ioaddr + hcvr);

                                /* Return reply */
                                Cmd_data[0] =  astropci_check_reply_flags(dev);
                                if (Cmd_data[0] != RDR) {
                                        if (copy_to_user((uint32_t *) arg, Cmd_data, sizeof (Cmd_data)))
                                                retval = -EFAULT;
                                } else {
                                        /* Flush the reply buffer */
                                        astropci_flush_reply_buffer(dev);

                                        /* Go read some data */
                                        writel((uint32_t) READ_REPLY_VALUE, astropci[dev].ioaddr + hcvr);
                                        if (astropci_check_dsp_output_fifo(dev)) {
                                                Cmd_data[0] = readl(astropci[dev].ioaddr + reply_buffer);
                                                if (copy_to_user((uint32_t *) arg, Cmd_data, sizeof (Cmd_data)))
                                                        retval = -EFAULT;
                                        } else {
                                                retval = -EFAULT;
                                        }
                                }
                        }
                }
        }
                break;

        case ASTROPCI_PCI_DOWNLOAD:
                /* This vector command is here because it expects NO reply. */
                writel((uint32_t) PCI_DOWNLOAD, astropci[dev].ioaddr + hcvr);
                break;

        case ASTROPCI_PCI_DOWNLOAD_WAIT:
                reply = astropci_check_reply_flags(dev);
                if (put_user(reply, (uint32_t *) arg))
                        retval = -EFAULT;
                break;

        case ASTROPCI_MUNMAP:
                astropci[dev].imageBufferStart = 0;
                astropci_printf("board[%d] unmapped!\n", dev);
                break;

#ifdef DEBUG_ON
	case 999:
		astropci_printf("----------------------------------\n");
		astropci_printf("Reading values from image buffer\n");
		astropci_printf("----------------------------------\n");
		unsigned long *buffer = (unsigned long *)astropci[dev].imageBufferStart;
		astropci_printf("ASTROPCI buffer[0]: 0x%lX\n", buffer[0]);
		astropci_printf("ASTROPCI buffer[1]: 0x%lX\n", buffer[1]);
		astropci_printf("ASTROPCI buffer[2]: 0x%lX\n", buffer[2]);
		astropci_printf("ASTROPCI buffer[3]: 0x%lX\n", buffer[3]);
		break;
#endif

        default:
                retval = -EINVAL;
                break;
        }

 exit:
        spin_unlock(&astropci_lock);
        MOD_DEC_USE_COUNT;
        return retval;
}

/******************************************************************************
 FUNCTION: ASTROPCI_INTR()
 
 PURPOSE:  Entry point. Interrupt handler.
 *****************************************************************************/
static void
astropci_intr(int irq, void *dev_id, struct pt_regs *regs)
{
        uint32_t int_flag = 0;
        int dev;

        dev = (int)dev_id;
        dev--;

        if (dev >= 0 && dev < MAX_DEV && astropci[dev].have_irq) {
                int_flag = readl(astropci[dev].ioaddr + hstr);
        }

        /* If no SDSU devices match the interrupting device, then exit */
        if (!(int_flag & DMA_INTERRUPTING)) {
 /*
               astropci_printf
                    ("couldn't find interrupt dev (%d) among the known devices\n", dev);
*/
                return;
        }

	/* Clear the interrupt, no questions asked */
	astropci_printf("Clearing Interrupts!\n");
	writel((uint32_t) CLEAR_INTERRUPT, astropci[dev].ioaddr + hcvr);

        /* OK, we found the device which is interrupting */
        PDEBUG("device %d interrupting\n", dev);
        return;
}

/******************************************************************************
 FUNCTION: ASTROPCI_SET_BUFFER_ADDRESSES
 
 PURPOSE:  Pass the DMA kernel buffer address to the DSP
 
 RETURNS:  0 on success, -ENXIO on failure.
******************************************************************************/
int
astropci_set_buffer_addresses(int dev)
{
        uint32_t phys_address = astropci[dev].imageBufferStart;

        /* Pass the DMA kernel buffer address to the DSP */
        if (astropci_check_dsp_input_fifo(dev)) {
                PDEBUG("address %ld\n", astropci[dev].ioaddr + cmd_data);
                writew((uint16_t) (phys_address & 0x0000FFFF),
                       astropci[dev].ioaddr + cmd_data);
                writew((uint16_t) ((phys_address & 0xFFFF0000) >> 16),
                       astropci[dev].ioaddr + cmd_data);
        } else {
                PDEBUG("timeout during set_buffer_addresses\n");
                return -ENXIO;
        }

        writel((uint32_t) WRITE_PCI_ADDRESS,
               astropci[dev].ioaddr + hcvr);

        /* Check the reply */
        if (astropci_check_reply_flags(dev) != DON)
                return -ENXIO;

        return 0;
}

/******************************************************************************
 FUNCTION: ASTROPCI_FLUSH_REPLY_BUFFER
 
 PURPOSE:  Utility function to clear DSP reply buffer and driver 
           internal value.
 RETURNS:  Returns 0 if successful. Non-zero otherwise.
******************************************************************************/
static int
astropci_flush_reply_buffer(int dev)
{
        uint32_t reply_status = 0;
        int reply_value = 0;
        int x;

        /* Flush the reply buffer FIFO on the PCI DSP. 6 is the number 
           of 24 bit words the FIFO can hold */
        for (x = 0; x < 6; x++) {
                reply_status = readl(astropci[dev].ioaddr + hstr);
                if ((reply_status & OUTPUT_FIFO_OK_MASK) == OUTPUT_FIFO_OK_MASK)
                        reply_value = readl(astropci[dev].ioaddr + reply_buffer);
                else
                        break;
        }

        return reply_value;
}

/******************************************************************************
 FUNCTION: ASTROPCI_CHECK_FOR_READOUT
 
 PURPOSE:  Checks if the controller has entered readout. Uses the PCI DSP 
	   status HSTR HTF bits 3,4,5.
 
 RETURNS:  Returns 1 if the controller has entered readout.
           Returns 0 otherwise.

 NOTES:    This function must be called before sending a command to the PCI
           board or controller.
******************************************************************************/
static int astrocpi_check_for_readout(int dev)
{
	uint32_t status = readl(astropci[dev].ioaddr + hstr);

	if ((status = ((status & HTF_BIT_MASK) >> 3)) == 0x5)
		return 1;
	else
		return 0;
}

/******************************************************************************
 FUNCTION: ASTROPCI_CHECK_REPLY_FLAGS
 
 PURPOSE:  Check the current PCI DSP status. Uses HSTR HTF bits 3,4,5.
 
 RETURNS:  Returns DON if HTF bits are a 1 and command successfully completed.
           Returns RDR if HTF bits are a 2 and a reply needs to be read.
           Returns ERR if HTF bits are a 3 and command failed.
           Returns SYR if HTF bits are a 4 and a system reset occurred.

 NOTES:    This function must be called after sending a command to the PCI
           board or controller.
******************************************************************************/
static int
astropci_check_reply_flags(int dev)
{
        uint32_t status;
        int retval[] = {TIMEOUT, DON, RDR, ERR, SYR, READOUT, 0, 0};

        do {
                status = astropci_wait_for_condition(dev, CHECK_REPLY);

                /* Clear the status bits if not in READOUT (0x5) */
		if (status != 5)
                	writel((uint32_t) CLEAR_REPLY_FLAGS, astropci[dev].ioaddr + hcvr);

        } while (status == 6);       // DANGER: endless loop possible
        
        return retval[status & 0x7]; // Insure against array out of bounds
}

/******************************************************************************
 FUNCTION: ASTROPCI_CHECK_DSP_INPUT_FIFO
 
 PURPOSE:  Check that the DSP input buffer (fifo) is not full.
 
 RETURNS:  Returns INPUT_FIFO_OK_MASK if HSTR bit 1 is set and buffer is
           available for input.

           Returns 0 if HSTR bit 1 is unset and buffer is unavailable 
           for input.

 NOTES:    This function must be called before writing to any register in the
           astropci_regs structure. Otherwise, data may be overwritten in the
           DSP input buffer, since the DSP cannot keep up with the input rate.
           This function will exit after "max_tries". This will help prevent
           the system from hanging in case the PCI DSP hangs.
******************************************************************************/
static int
astropci_check_dsp_input_fifo(int dev)
{
        return astropci_wait_for_condition(dev, INPUT_FIFO);
}

/******************************************************************************
 FUNCTION: ASTROPCI_CHECK_DSP_OUTPUT_FIFO
 
 PURPOSE:  Check that the DSP output buffer (fifo) has data.
 
 RETURNS:  Returns OUTPUT_FIFO_OK_MASK if HSTR bit 1 is set and buffer is
           available for output?
           
 NOTES:    Please insert documentation here... 
******************************************************************************/

static int
astropci_check_dsp_output_fifo(int dev)
{
        return astropci_wait_for_condition(dev, OUTPUT_FIFO);
}

/******************************************************************************
 FUNCTION: ASTROPCI_WAIT_FOR_CONDITION
 
 PURPOSE:  Waits for a particular state of the hstr register. The
           condition can be INPUT_FIFO, INPUT_FIFO, or CHECK_REPLY.
 
 RETURNS:  0 on timeout, else a masked copy of the value of hstr.
           
 NOTES:    The condition is first tested, and if satisfied, the routine
           returns immediately. Else, up to BUSY_MAX_WAIT microseconds
           are spent polling every BUSY_WAIT_DELAY microseconds. If
           the condition is still unsatisfied, up to SLEEP_MAX_WAIT
           microseconds total time will be spent, sleeping in intervals
           of SLEEP_WAIT_DELAY microseconds.

           So, this routine will busy wait for at most BUSY_MAX_WAIT 
           microseconds, and is guaranteed to return within SLEEP_MAX_WAIT
           microseconds (provided SLEEP_MAX_WAIT is greater than 
           BUSY_MAX_WAIT) plus or minus a jiffy.

           BUSY_WAIT_DELAY should be choosen so as not to overly
           tax the PCI card.

           BUSY_MAX_WAIT should be short enough not to cause unacceptable
           non-responsiveness of the computer, but long enough to cope
           with typical hardware delays.

           SLEEP_WAIT_DELAY should be at least a jiffy or three.

           SLEEP_MAX_WAIT should be the longest time before we clearly
           have a timeout problem.
******************************************************************************/
static int
astropci_wait_for_condition(int dev, int condition_to_wait_for)
{
        uint32_t status = 0;
        struct timeval then, now;
        long elapsed_time = 0L, sleep_timeout;

        sleep_timeout = (SLEEP_WAIT_DELAY * HZ) / 1000000L;
        if (sleep_timeout < 1) sleep_timeout = 1;
 
        do_gettimeofday (&then);
        while (elapsed_time < SLEEP_MAX_WAIT) {
                status = readl(astropci[dev].ioaddr + hstr);

                switch (condition_to_wait_for) {
                case INPUT_FIFO:
                        status &= INPUT_FIFO_OK_MASK;
                        break;

                case OUTPUT_FIFO:
                        status &= OUTPUT_FIFO_OK_MASK;
                        break;

                case CHECK_REPLY:
                        status = (status & HTF_BIT_MASK) >> 3;
                        break;
                }

                if (status) break;

                do_gettimeofday (&now);
                elapsed_time = (now.tv_sec  - then.tv_sec) * 1000000L +
                               (now.tv_usec - then.tv_usec);
                if (elapsed_time < BUSY_MAX_WAIT) {                     
                        udelay(BUSY_WAIT_DELAY);
                } else {
                        current->state = TASK_UNINTERRUPTIBLE;
                        schedule_timeout(sleep_timeout);
                }
        }

        /* The following could provide useful debugging information. */
        if (elapsed_time > max_elapsed_time) {
                astropci_printf("slept for %ld usecs\n", elapsed_time);
                max_elapsed_time = elapsed_time;
        }

        return status;
}

/******************************************************************************
 FUNCTION: ASTROPCI_MMAP
        
         - Brian Taylor contributed to the original version of this function.

 PURPOSE:  Map the image buffer from unused RAM.
 
 RETURNS:  Returns 0 if memory was mapped, else a negative error code.
 
 NOTES:    For this function to work, the user must specify enough buffer
           space by passing "mem=xxxM" to the kernel as a boot parameter. 
           If you have 128M RAM and want a 28M image buffer, then use: 
           "mem=100M".
******************************************************************************/

static int
astropci_mmap(struct file *file, struct vm_area_struct *vma)
{
        unsigned long prot;
        int dev, retval;

        MOD_INC_USE_COUNT;
        spin_lock(&astropci_lock);

        dev = (int)file->private_data;

        /* Sanity check, failure here should be impossible */
        if (dev < 0 || dev >= MAX_DEV || !astropci[dev].opened) {
                astropci_printf("serious programming error\n");
                retval = -EAGAIN;
                goto exit;
        }

        astropci_printf("memory mapping for dev %d\n", dev);

        /* Identify a suitable high memory area to map to */
        if (nextValidStartAddress == 0) {
                nextValidStartAddress = (unsigned long) __pa(high_memory);
        }
        astropci[dev].imageBufferStart = nextValidStartAddress;
        astropci[dev].imageBufferSize = (uint32_t)(vma->vm_end - vma->vm_start);
        nextValidStartAddress = astropci[dev].imageBufferStart + 
                astropci[dev].imageBufferSize + PAGE_SIZE;

        astropci_printf
            ("board %d  buffer start: 0x%X  end: 0x%X size: %u\n",
             dev, astropci[dev].imageBufferStart, 
             astropci[dev].imageBufferStart + astropci[dev].imageBufferSize,
             astropci[dev].imageBufferSize);

        /* Ensure that the memory will not be cached; see drivers/char/mem.c */
        if (boot_cpu_data.x86 > 3) {
                prot = pgprot_val(vma->vm_page_prot) | _PAGE_PCD | _PAGE_PWT;
                vma->vm_page_prot = __pgprot(prot);
        }
 
        /* Don't try to swap out physical pages */
        vma->vm_flags |= VM_RESERVED;

        /* Don't dump addresses that are not real memory to a core file */
        vma->vm_flags |= VM_IO;

        /* Remap the page range to see the high memory */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,20)
        if ((retval = remap_page_range(vma,
                                       vma->vm_start,
				       astropci[dev].imageBufferStart,
                                       (vma->vm_end - vma->vm_start),
                                       vma->vm_page_prot))) {
                astropci_printf("remap page range failed.\n");
                goto exit;
        }
#else
        if ((retval = remap_page_range(vma->vm_start, 
                                       astropci[dev].imageBufferStart, 
                                       astropci[dev].imageBufferSize, 
                                       vma->vm_page_prot))) {
                astropci_printf("remap page range failed.\n");
                goto exit;
        }
#endif

        astropci_printf
            ("%u bytes mapped: 0x%lX - 0x%lX --> 0x%X - 0x%X\n",
             astropci[dev].imageBufferSize, vma->vm_start, vma->vm_end,
             astropci[dev].imageBufferStart, 
             astropci[dev].imageBufferStart + astropci[dev].imageBufferSize);

#ifdef DEBUG_ON
	// Write some test values to the image buffer.
	astropci_printf("Writing test values to image buffer\n");
	unsigned long *buffer = (unsigned long *)astropci[dev].imageBufferStart;
	buffer[0] = 0x11111111;
	buffer[1] = 0x22222222;
	buffer[2] = 0x33333333;
	buffer[3] = 0x44444444;
	astropci_printf("ASTROPCI buffer[0]: 0x%lX\n", buffer[0]);
	astropci_printf("ASTROPCI buffer[1]: 0x%lX\n", buffer[1]);
	astropci_printf("ASTROPCI buffer[2]: 0x%lX\n", buffer[2]);
	astropci_printf("ASTROPCI buffer[3]: 0x%lX\n", buffer[3]);
#endif

        /* Assign the image buffers */
        if ((retval = astropci_set_buffer_addresses(dev) < 0)) {
                astropci_printf("set_buffer_addresses failed.\n");
                goto exit;
        }

 exit:
        /* If there was a failure, set imageBufferStart to zero as a flag */
        if (retval && dev != MAX_DEV) {
                astropci[dev].imageBufferStart = 0;
        }
        spin_unlock(&astropci_lock);
        MOD_DEC_USE_COUNT;
        return retval;
}

/******************************************************************************
 FUNCTION: ASTROPCI_READ_PROC

 PURPOSE:  This function is called when the user reads /proc/astropci.
 
 RETURNS:  Returns the number of bytes available to be read.
******************************************************************************/
static int 
astropci_read_proc(char *buf, char **start, off_t offset, int len)
{
        int dev;

        if (offset > 0) {
                return offset;
        }
        
        len = 0;

        for (dev = 0; dev < MAX_DEV; dev++) {
        	len += sprintf(buf + len,
                       "astropciDriverName=\"%s\"\nastropciDriverVersion=\"%s\"\n",
                       board[dev], astropci_version);

                if (astropci[dev].have_irq) {
                        len += sprintf(buf + len,
                                       "astropci%dIRQ=%d\n",
                                       dev, astropci[dev].pdev->irq);
                }
                if (astropci[dev].imageBufferStart) {
                        len += sprintf(buf + len,
                                       "astropci%dStart=0x%X\nastropci%dSize=%u\n",
                                       dev, astropci[dev].imageBufferStart, 
                                       dev, astropci[dev].imageBufferSize);
                }
                len += sprintf(buf + len, "astropci%dMaxElapsed=%ld\n",
                               dev, max_elapsed_time);
        }
        return len;
}

MODULE_AUTHOR("Marco Bonati, Scott Streit, Brian Taylor, Michael Ashley");
MODULE_DESCRIPTION("SDSU PCI interface driver");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10)
MODULE_LICENSE("GPL"); // NOTE: should check with SDSU that GPL is OK.
#endif

EXPORT_NO_SYMBOLS;