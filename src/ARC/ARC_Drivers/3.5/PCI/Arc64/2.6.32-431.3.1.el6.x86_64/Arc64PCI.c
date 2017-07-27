/*
+-------------------------------------------------------------------------------------------------------+
|   File:       Arc64PCI.c                                                    							|
|                                                                              							|
|   Author:     Scott Streit                                                   							|
|                                                                              							|
|   Abstract:   Linux device driver for the ARC-63/64 PCI Interface Board.    							|
|                                                                              							|
|                                                                             							|
|   Revision History:                                                          							|
+-------------------------------------------------------------------------------------------------------+
|   Date        Who Version Description                                        							|
|   13-Dec-2012 sds  1.0    Initial                                            							|
|   28-Mar-2014 sds  3.5	The following changes were made to support kernels >= 3.13.73:				|
|                           Removed __devinit / __init on Arc64_probe() and Arc64_cdevInit().			|
|                           Changed __devinit to __init on Arc66_mapBars().   							|
|                           Changed __devexit_p() to __exit_p() on remove().   							|
|							Changed vma->vm_flags |= (VM_IO | VM_LOCKED | VM_RESERVED);					|
|                           to vma->vm_flags |= (VM_IO | VM_LOCKED | (VM_DONTEXPAND | VM_DONTDUMP));	|
|                                                                              							|
|   Development notes:                                                         							|
+-------------------------------------------------------------------------------------------------------+
|   This driver has been tested on CentOS 6.3 x64, Kernel 2.6.32-279.14.1.el6  							|
|                                                                              							|
+-------------------------------------------------------------------------------------------------------+
*/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>

/* Kernel Change - 3.x */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	#include <linux/spinlock_types.h>
	#include <linux/semaphore.h>
	#include <linux/sched.h>
#else
	#include <linux/smp_lock.h>
#endif

#include "Arc64PCI.h"
#include "Arc64Ioctl.h"


/*
+------------------------------------------------------------------------------+
|       Module command line parameters                                         |
+------------------------------------------------------------------------------+
*/
static ulong g_ulSize = 2 * 4200 * 4400;
module_param( g_ulSize, ulong, S_IRUGO );


/*
+------------------------------------------------------------------------------+
|       Global variables                                                       |
+------------------------------------------------------------------------------+
*/

/* Kernel Change - 3.x */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	static DEFINE_SPINLOCK( g_tArc64Lock );
#else
	static spinlock_t g_tArc64Lock = SPIN_LOCK_UNLOCKED;
#endif


static struct class*	g_pArc64Class		= NULL;
static unsigned long	g_ulNextStartAddr	= 0;
static int				g_dArc64Major		= 0;
static int				g_dArc64Minor		= 0;
static int				g_dUsageCount		= 0;


/*
+------------------------------------------------------------------------------+
|       Prototypes for main entry points                                       |
+------------------------------------------------------------------------------+
*/
static int	Arc64_probe( struct pci_dev* dev, const struct pci_device_id* id );
static void	Arc64_remove( struct pci_dev* dev );

static int  Arc64_open( struct inode* inode, struct file* filp );
static int  Arc64_close( struct inode* inode, struct file* filp );

static long Arc64_ioctl( struct file* filp, unsigned int cmd, unsigned long arg );

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
	static irqreturn_t Arc64_isr( int irq, void* dev_id, struct pt_regs* regs );
#else
	static irqreturn_t Arc64_isr( int irq, void* dev_id );
#endif

static int  Arc64_mmap( struct file* filp, struct vm_area_struct* vma );


/*
+------------------------------------------------------------------------------+
|       Prototypes for device specific functions                               |
+------------------------------------------------------------------------------+
*/
static int  Arc64_mapCommonBuffer( ArcDevExt* pDevExt, struct vm_area_struct* vma );

static int	__init Arc64_mapBars( ArcDevExt *pDevExt, struct pci_dev *dev );
static void Arc64_unMapBars( ArcDevExt *pDevExt, struct pci_dev *dev );

static int	Arc64_cdevInit( ArcDevExt *pDevExt );
static void	Arc64_cdevCleanUp( ArcDevExt *pDevExt );

static int		Arc64_flushReplyBuffer( ArcDevExt* pDevExt );
static int		Arc64_checkReplyFlags( ArcDevExt* pDevExt );
static int		Arc64_checkDspInputFifo( ArcDevExt* pDevExt );
static int		Arc64_checkDspOutputFifo( ArcDevExt* pDevExt );
static uint32_t	Arc64_waitForCondition( ArcDevExt* pDevExt, int dCondition2Wait4 );
static int		Arc64_setBufferAddresses( ArcDevExt* pDevExt );

static uint32_t	Arc64_readRegister_32( void* __iomem pAddr );
static uint16_t	Arc64_readRegister_16( void* __iomem pAddr );
static void		Arc64_writeRegister_32( uint32_t u32Value, void* __iomem pAddrr );
static void		Arc64_writeRegister_16( uint16_t u16Value, void* __iomem pAddr );
static uint32_t	Arc64_readHCTR( ArcDevExt* pDevExt );
static uint32_t	Arc64_readHSTR( ArcDevExt* pDevExt );
static uint32_t	Arc64_readREPLYBUFFER_32( ArcDevExt* pDevExt );
static uint16_t	Arc64_readREPLYBUFFER_16( ArcDevExt* pDevExt );
static void		Arc64_writeHCTR( ArcDevExt* pDevExt, uint32_t u32Value );
static void		Arc64_writeCMDDATA_32( ArcDevExt* pDevExt, uint32_t u32Value );
static void		Arc64_writeCMDDATA_16( ArcDevExt* pDevExt, uint16_t u16Value );
static int		Arc64_writeHCVR( ArcDevExt* pDevExt, uint32_t u32Value );


/*
+------------------------------------------------------------------------------+
|       Prototypes for large memory allocation functions                       |
+------------------------------------------------------------------------------+
*/
static struct page*	Arc64_bigBufAlloc( unsigned int flags, ulong size );
static void			Arc64_bigBufFree( struct page* start, ulong size );


/*
+------------------------------------------------------------------------------+
|   As of kernel 2.4, you can rename the init and cleanup functions. They      |
|   no longer need to be called init_module and cleanup_module. The macros     |
|   module_init and module_exit, found in linux/init.h, can be used to         |
|   define your own functions. BUT, these macros MUST be called after defining |
|   init and cleanup functions.                                                |
+------------------------------------------------------------------------------+
*/
static int  __init Arc64_init( void );
static void __exit Arc64_exit( void );

module_init( Arc64_init );
module_exit( Arc64_exit );


/*
+------------------------------------------------------------------------------+
|       Structures used by the kernel PCI API                                  |
+------------------------------------------------------------------------------+
*/
static struct pci_device_id Arc64_ids[] = {
	{ PCI_DEVICE( PCI_VENDOR_ID_MOTOROLA, ARC_PCI_DEVICE_ID ), },
	{ 0, }
};
MODULE_DEVICE_TABLE( pci, Arc64_ids );


/*
+------------------------------------------------------------------------------+
|  Used to register the driver with the PCI kernel sub system                  |
+------------------------------------------------------------------------------+
*/
static struct pci_driver Arc64_driver = {
	.name		= DEVICE_NAME,
	.id_table	= Arc64_ids,
	.probe		= Arc64_probe,
	.remove		= __exit_p( Arc64_remove ),
};


/*
+------------------------------------------------------------------------------+
|  Character device file operations                                            |
+------------------------------------------------------------------------------+
*/
static struct file_operations Arc64_fops = {
		.owner			= THIS_MODULE,
		.unlocked_ioctl	= Arc64_ioctl,
		.mmap			= Arc64_mmap,
		.open			= Arc64_open,
		.release		= Arc64_close
};


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_init() - Init Module                                        |
+------------------------------------------------------------------------------+
|  PURPOSE:  Initializes the module.                                           |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
|                                                                              |
|  NOTES:    Called by Linux during insmod, not by the user.                   |
+------------------------------------------------------------------------------+
*/
static int __init Arc64_init( void )
{
	int dResult = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,11 )
	ArcPrintf( "INIT - Invalid kernel version, must be 2.6.11 or greater!\n" );
	dResult = -1;
#endif

	ArcPrintf( "INIT - +------------------------------------+\n" );
	ArcPrintf( "INIT - |     ARC-63/64 - Initialization     |\n" );
	ArcPrintf( "INIT - +------------------------------------+\n" );

	if ( !IS_ERR_VALUE( dResult ) )
	{
		/* Allocate a single dynamically allocated char device major
		  +--------------------------------------------------------+ */
		if ( g_dArc64Major == 0 )
		{
			dev_t tTempDev;

			dResult = alloc_chrdev_region( &tTempDev,
										   0,
										   ARC64_MAX_DEV,
										   DEVICE_NAME );

			/* Allocation failed?
			  +--------------------------------------------------------+ */
			if ( dResult < 0 )
			{
				ArcPrintf( "INIT - Alloc_chrdev_region() failed = %d\n",
							dResult );
			}

			else
			{
				g_dArc64Major = MAJOR( tTempDev );
				g_dArc64Minor = 0;
			}
		}
	}

	/*  Create the driver class
	   +---------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) )
	{
		g_pArc64Class = class_create( THIS_MODULE, DEVICE_NAME );

		if ( IS_ERR( g_pArc64Class ) )
		{
			dResult = PTR_ERR( g_pArc64Class );

			ArcPrintf( "INIT - Failure creating class, error %d\n", dResult );
		}
	}

	/*  Register the PCI driver
	   +---------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) )
	{
		dResult = pci_register_driver( &Arc64_driver );

		if ( IS_ERR_VALUE( dResult ) )
		{
			ArcPrintf( "INIT - Failed to register driver!\n" );
		}
	}

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_exit() - Cleanup Module                                     |
+------------------------------------------------------------------------------+
|  PURPOSE:  Module cleanup, unregisters devices and allocated resources.      |
|                                                                              |
|  NOTES:    Called by Linux during rmmod, not by the user.                    |
+------------------------------------------------------------------------------+
*/
static void __exit Arc64_exit( void )
{
	int dDev = 0;

	ArcPrintf( "EXIT - +------------------------------------+\n" );
	ArcPrintf( "EXIT - |      ARC-63/64 - EXIT CALLED       |\n" );
	ArcPrintf( "EXIT - +------------------------------------+\n" );

	/* Free the dynamically allocated character device node
	  +---------------------------------------------------+ */
	unregister_chrdev_region( MKDEV( g_dArc64Major, 0 ), 1 );

	/* Remove /dev entry
	 * NOTE: This MUST be called before class_destroy!
	 *       So it's called in EXIT, not REMOVE.
	  +------------------------------------------------+ */
	for ( dDev = 0; dDev < ARC64_MAX_DEV; dDev++ )
	{
		device_destroy( g_pArc64Class, MKDEV( g_dArc64Major, dDev ) );
	}

	/* Class destroy
	  +------------------------------------------------+ */
	if ( g_pArc64Class != NULL )
	{
		class_destroy( g_pArc64Class );
	}

	/* Unregister PCI driver
	  +------------------------------------------------+ */
	pci_unregister_driver( &Arc64_driver );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_probe()                                                     |
+------------------------------------------------------------------------------+
|  PURPOSE:  Called when the PCI sub system thinks we can control the given    |
|            device. Inspect if we can support the device and if so take       |
|            control of it.                                                    |
|                                                                              |
|            - allocate board specific device extension                        |
|            - enable the board                                                |
|            - request regions                                                 |
|            - query DMA mask                                                  |
|            - obtain and request irq                                          |
|            - map regions into kernel address space                           |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
|                                                                              |
|  NOTES:    See Documentation/pci.txt in the Linux sources.                   |
+------------------------------------------------------------------------------+
*/
static int Arc64_probe( struct pci_dev* dev, const struct pci_device_id* id )
{
	ArcDevExt*	pDevExt = NULL;
	int			dResult = 0;

	/* Immediately try to allocate the image buffer
	  +--------------------------------------------------------+ */
	struct page* pMemBlock =
					Arc64_bigBufAlloc( ( GFP_KERNEL | __GFP_DMA32 ), g_ulSize );

	if ( pMemBlock )
	{
		ArcPrintf( "PROBE - Allocated memory block at 0x%lX of size %lu\n",
					( ulong )page_to_phys( pMemBlock ),
					g_ulSize );
	}
	else
	{
		ArcPrintf( "PROBE - Allocation of size %lu failed.\n", g_ulSize );

		dResult = -ENOMEM;
	}

	if ( !IS_ERR_VALUE( dResult ) )
	{
		ArcPrintf( "PROBE - Looking for ARC-64 ( Gen IV ) boards ...\n" );

		/*  Allocate memory for per-board book keeping
		   +----------------------------------------------------+ */
		pDevExt = kzalloc( sizeof( ArcDevExt ), GFP_KERNEL );

		if ( !pDevExt )
		{
			ArcPrintf(
				"PROBE - Could not kzalloc()ate DEVICE EXTENSION memory.\n" );

			dResult = -ENOMEM;
		}
	}

	if ( !IS_ERR_VALUE( dResult ) )
	{
		/* Save the device reference in the device extension
		  +----------------------------------------------------+ */
		pDevExt->pPCIDev = dev;

		/*  Initialize device extension stuff
		  +----------------------------------------------------+ */
		pDevExt->pMemBlock			= pMemBlock;
		pDevExt->dOpen				= 0;
		pDevExt->ulCommonBufferVA	= 0;
		pDevExt->ulCommonBufferPA	= 0;
		pDevExt->ulCommonBufferSize	= 0;

		/* Save the device extension for later use
		  +----------------------------------------------------+ */
		dev_set_drvdata( &dev->dev, pDevExt );

		/* Initialize device before it's used by the driver
		  +----------------------------------------------------+ */
		dResult = pci_enable_device( dev );

		if ( IS_ERR_VALUE( dResult ) )
		{
			ArcPrintf( "PROBE - Failed to enable device!\n" );
		}
	}

	/* Request all PCI I/O regions associated with the device
	  +------------------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) )
	{
		dResult = pci_request_regions( dev, DEVICE_NAME );

		if ( dResult )
		{
			ArcPrintf( "PROBE - Failed to retrieve PCI I/O regions!\n" );
		}
	}

	/* Map BARs
	  +------------------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) )
	{
		dResult = Arc64_mapBars( pDevExt, dev );

		if ( dResult )
		{
			ArcPrintf( "PROBE - Failed to map PCI BAR's!\n" );
		}
	}

	/* Initialize character device
	  +----------------------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) )
	{
		dResult = Arc64_cdevInit( pDevExt );

		if ( dResult )
		{
			ArcPrintf( "PROBE - Failed to initialize char device!\n" );
		}
	}

	/* Request ISQ
	  +----------------------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) )
	{
		// Install the interrupt service routine. The pci_dev structure
		// that's passed in already contains the IRQ read from the PCI
		// board. Uses shared interrupts.
		//
		// NOTE: the last parameter ( void *dev_id ) CANNOT be NULL for
		// shared interrupts, which is what is used by the PCI boards.
		dResult = request_irq( dev->irq,
							   &Arc64_isr,
						       IRQF_SHARED,
						       pDevExt->szName,
						       pDevExt );

		if ( !IS_ERR_VALUE( dResult ) )
		{
			pDevExt->dHasIRQ = 1;
		}

		// IMPORTANT - DO NOT mess with the enable_irq and disable_irq
		// functions when using shared interrupts, because these will
		// disable the interrupt for all devices using the IRQ, which
		// may cause problems for other devices, such as the NIC.
	}

	/* Create the device in /dev
	  +----------------------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) )
	{
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
		pDevExt->pDevice = device_create( g_pArc64Class,
										  NULL,
										  pDevExt->tCDevno,
										  "Arc64PCI%d",
										  MINOR( pDevExt->tCDevno ) );
		#else
		pDevExt->pDevice = device_create( g_pArc64Class,
										  NULL,
										  pDevExt->tCDevno,
										  pDevExt,
										  "Arc64PCI%d",
										  MINOR( pDevExt->tCDevno ) );
		#endif

		if ( IS_ERR( pDevExt->pDevice ) )
		{
			ArcPrintf( "PROBE - Failed to create /dev entry, error: %d\n",
					   ( int )PTR_ERR( pDevExt->pDevice ) );

			dResult = ( int )PTR_ERR( pDevExt->pDevice );
		}
	}

	/*  Undo everything if we failed
	   +----------------------------------------------------------+ */
	if ( IS_ERR_VALUE( dResult ) )
	{
		if ( pDevExt->dHasIRQ )
		{
			free_irq( dev->irq, pDevExt );
		}

		Arc64_unMapBars( pDevExt, dev );

		pci_release_regions( dev );

		if ( pDevExt )
		{
			kfree( pDevExt );
		}
	}

//	else
//	{
//		ArcPrintf( "PROBE - DEV_EXT->pPCIDev: %p!\n", pDevExt->pPCIDev );
//		ArcPrintf( "PROBE - DEV_EXT->tCDev: %p!\n", &pDevExt->tCDev );
//		ArcPrintf( "PROBE - DEV_EXT->pBar: %p!\n", pDevExt->pBar );
//		ArcPrintf( "PROBE - DEV_EXT->tCDevno: %d!\n", pDevExt->tCDevno );
//		ArcPrintf( "PROBE - DEV_EXT->pDevice: %p!\n", pDevExt->pDevice );
//		ArcPrintf( "PROBE - DEV_EXT->szName: %s!\n", pDevExt->szName );
//		ArcPrintf( "PROBE - DEV_EXT->dOpen: %d!\n", pDevExt->dOpen );
//		ArcPrintf( "PROBE - DEV_EXT->dHasIRQ: %d IRQ: %d!\n", pDevExt->dHasIRQ, dev->irq );
//	}

	ArcPrintf( "PROBE - Finished looking for ARC-64 boards\n" );

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_remove()                                                    |
+------------------------------------------------------------------------------+
|  PURPOSE:  This function is called by pci_unregister_driver for each PCI     |
|            device that was successfully probed.                              |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
|                                                                              |
|  NOTES:    See Documentation/pci.txt in the Linux sources.                   |
+------------------------------------------------------------------------------+
*/
static void Arc64_remove( struct pci_dev* dev )
{
	ArcDevExt *pDevExt = dev_get_drvdata( &dev->dev );

	if ( pDevExt )
	{
		/* Free the image buffer memory block
		  +---------------------------------------------------+ */
		if ( pDevExt->pMemBlock )
		{
			ArcPrintf( "REMOVE - Freeing memory block at 0x%lX.\n",
					   ( ulong )page_to_phys( pDevExt->pMemBlock ) );

			Arc64_bigBufFree( pDevExt->pMemBlock, g_ulSize );
		}

		ArcPrintf( "REMOVE - Removing device #: %d [ Major: %d Minor: %d ]\n",
					pDevExt->tCDevno,
					MAJOR( pDevExt->tCDevno ),
					MINOR( pDevExt->tCDevno ) );

		/* Free IRQ
		  +------------------------------------------------+ */
		if ( pDevExt->dHasIRQ )
		{
			free_irq( dev->irq, pDevExt );
		}

		/* Remove character device
		  +------------------------------------------------+ */
		Arc64_cdevCleanUp( pDevExt );

		/* UnMap the BARS
		  +------------------------------------------------+ */
		Arc64_unMapBars( pDevExt, dev );

		/* Free device extension
		  +------------------------------------------------+ */
		kfree( pDevExt );
	}

	/* Disable the device
	  +----------------------------------------------------+ */
	pci_disable_device( dev );

	/* Release the device regions
	  +----------------------------------------------------+ */
	pci_release_regions( dev );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_open                                                        |
+------------------------------------------------------------------------------+
|  PURPOSE:  Entry point. Open a device for access.                            |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
+------------------------------------------------------------------------------+
*/
static int Arc64_open( struct inode* inode, struct file* filp )
{
	uint32_t u32Value = 0;
	int      dResult  = 0;

	/* Pointer to containing data structure of the character device inode
	  +-------------------------------------------------------------------+ */
	ArcDevExt* pDevExt = container_of( inode->i_cdev, ArcDevExt, tCDev );

	if ( !pDevExt )
	{
		ArcPrintf( "OPEN - Failed to find device extension!\n" );

		dResult = -ENODEV;
	}

	/* Save the device extension to the file pointer for easier future use
	  +-------------------------------------------------------------------+ */
	filp->private_data = pDevExt;

	/* We use a lock to protect global variables
	  +----------------------------------------------------------+ */
	spin_lock( &g_tArc64Lock );

	/* Allow only one process to open the device at a time
	  +----------------------------------------------------------+ */
	if ( !IS_ERR_VALUE( dResult ) && pDevExt->dOpen )
	{
		ArcPrintf( "OPEN - Device %d already opened!\n",
					MINOR( pDevExt->tCDevno ) );

		dResult = -EBUSY;
	}

	if ( !IS_ERR_VALUE( dResult ) )
	{
		/* Increase the module usage count. Prevents accidental
		   unloading of the device while it's in use.
		  +----------------------------------------------------------+ */
		try_module_get( THIS_MODULE );

		ArcPrintf( "OPEN - Device MAJOR: %d  MINOR:%d  MAX_DEV: %d  OPEN: %d COUNT: %d\n",
					MAJOR( pDevExt->tCDevno ),
					MINOR( pDevExt->tCDevno ),
					ARC64_MAX_DEV,
					pDevExt->dOpen,
					g_dUsageCount );

		/* Set the device state to opened
		  +----------------------------------------------------------+ */
		pDevExt->dOpen = 1;

		/*  Initialize the semaphore
		 *  Kernel Change - 2.6.37
		  +----------------------------------------------------------+ */
		#ifndef init_MUTEX
			sema_init( &pDevExt->tSem, 1 );
		#else
			init_MUTEX( &pDevExt->tSem );
		#endif

		/*  Increase the driver usage count. This is for resetting
		    the image buffer "next available" address.
		  +----------------------------------------------------------+ */
		g_dUsageCount++;

		/* Write 0xFF to the configuration-space address PCI_LATENCY_TIMER
		  +------------------------------------------------------------------+ */
		pci_write_config_byte( pDevExt->pPCIDev, PCI_LATENCY_TIMER, 0xFF );

		/* Set HCTR bit 8 to 1 and bit 9 to 0 for 32-bit PCI commands -> 24-bit DSP data
		   Set HCTR bit 11 to 1 and bit 12 to 0 for 24-bit DSP reply data -> 32-bit PCI data
		  +---------------------------------------------------------------------------------+ */
		u32Value = Arc64_readHCTR( pDevExt );

		Arc64_writeHCTR( pDevExt, ( ( u32Value & 0xCFF ) | 0x900 ) );

 		if ( Arc64_flushReplyBuffer( pDevExt ) )
		{
			ArcPrintf( "OPEN - Flush reply buffer failed!\n" );

			dResult = -EACCES;
		}

		ArcPrintf( "OPEN - Device MAJOR: %d  MINOR:%d  MAX_DEV: %d  OPEN: %d COUNT: %d\n",
					MAJOR( pDevExt->tCDevno ),
					MINOR( pDevExt->tCDevno ),
					ARC64_MAX_DEV,
					pDevExt->dOpen,
					g_dUsageCount );
	}

	spin_unlock( &g_tArc64Lock );

	return dResult;
}

/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_close                                                       |
+------------------------------------------------------------------------------+
|  PURPOSE:  Entry point. Close a device for access.                           |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
+------------------------------------------------------------------------------+
*/
static int Arc64_close( struct inode* inode, struct file* filp )
{
	int dResult = 0;

	/* Pointer to containing data structure of the character device inode
	  +-------------------------------------------------------------------+ */
	ArcDevExt* pDevExt = ( ArcDevExt * )filp->private_data;

	if ( !pDevExt )
	{
		ArcPrintf( "CLOSE - Failed to find device extension!\n" );

		dResult = -ENODEV;
	}

	if ( !IS_ERR_VALUE( dResult ) && pDevExt->dOpen )
	{
		spin_lock( &g_tArc64Lock );

		/* Set the device state to closed
		  +---------------------------------------------------------------+ */
		pDevExt->dOpen				= 0;
		pDevExt->ulCommonBufferPA	= 0;
		pDevExt->ulCommonBufferVA	= 0;

		/*  Decrease and check the driver usage count.  Reset the
		    counter if no devices are currently open.
		  +----------------------------------------------------------+ */
		g_dUsageCount--;

		if ( g_dUsageCount <= 0 )
		{
			g_ulNextStartAddr = 0;
		}

		spin_unlock( &g_tArc64Lock );
	}

	if ( pDevExt )
	{
		ArcPrintf( "CLOSE - Device: %d:%d closed! USAGE COUNT: %d\n",
					MAJOR( pDevExt->tCDevno ),
					MINOR( pDevExt->tCDevno ),
					g_dUsageCount );
	}

	filp->private_data = NULL;

	/* Decrease the module usage count. This is the "Used" column shown
	   via lsmod or the third parameter in /proc/modules. If the module
	   use count doesn't get decremented properly rmmod will not work, an
	   application may get "Device busy" error and a reboot is required.
	  +-----------------------------------------------------------------+ */
	module_put( THIS_MODULE );

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_ioctl                                                       |
+------------------------------------------------------------------------------+
|  PURPOSE:  Entry point. Command control for device.                          |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
+------------------------------------------------------------------------------+
*/
static long Arc64_ioctl( struct file* filp, unsigned int cmd, unsigned long arg )
{
	int dCtrlCode	= 0;
	int dResult		= 0;

	/* Pointer to containing data structure of the character device inode
	  +-------------------------------------------------------------------+ */
	ArcDevExt* pDevExt = ( ArcDevExt * )filp->private_data;

	/* Check that this device actually exists
	  +-------------------------------------------------------------------+ */
	if ( !pDevExt )
	{
		dResult = -ENXIO;
	}

	else
	{
		if ( down_interruptible( &pDevExt->tSem ) )
		{
			ArcPrintf( "IOCTL - Failed to obtain SEMA4!\n" );

			dResult = -EINTR;
		}
	}

	if ( !IS_ERR_VALUE( dResult ) )
	{
		dCtrlCode = EXCMD( cmd );

        switch ( dCtrlCode )
		{
			/*
			+----------------------------------------------------+
			|  GET HCTR                                          |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_GET_HCTR:
			{
				uint32_t u32Value = Arc64_readHCTR( pDevExt );

				dResult = put_user( u32Value, ( uint32_t * )arg );
			}
			break;

			/*
			+----------------------------------------------------+
			|  GET PROGRESS                                      |
			|  GET FRAMES READ                                   |
			+----------------------------------------------------+
        	*/
        	case ASTROPCI_GET_PROGRESS:
			case ASTROPCI_GET_FRAMES_READ:
			{
				uint32_t u32Progress = 0;
				uint32_t u32Upper    = 0;
				uint32_t u32Lower    = 0;

				/*  Ask the PCI board for the current value
				  +-----------------------------------------------+ */
				if ( dCtrlCode == ASTROPCI_GET_PROGRESS )
				{
					Arc64_writeHCVR( pDevExt, READ_PCI_IMAGE_ADDR );
				}

				else if ( dCtrlCode == ASTROPCI_GET_FRAMES_READ )
				{
					Arc64_writeHCVR( pDevExt, READ_NUMBER_OF_FRAMES_READ );
				}

				else
				{
					dResult = -EINVAL;
				}

                /*  Read the current image address
				  +-----------------------------------------------+ */
				if ( dResult == 0 )
				{
                	if ( Arc64_checkDspOutputFifo( pDevExt ) == OUTPUT_FIFO_OK_MASK )
					{
                		u32Lower = Arc64_readREPLYBUFFER_16( pDevExt );
                   		u32Upper = Arc64_readREPLYBUFFER_16( pDevExt );

                		u32Progress = ( ( u32Upper << 16 ) | u32Lower );
 					}

                	else
                	{
                 		dResult = -EFAULT;
					}
				}

				if ( !IS_ERR_VALUE( dResult ) )
				{
       		       	dResult =
       		       		put_user( u32Progress, ( uint32_t * )arg );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  GET HSTR                                          |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_GET_HSTR:
			{
				uint32_t u32Value = Arc64_readHSTR( pDevExt );

				dResult = put_user( u32Value, ( uint32_t * )arg );
			}
			break;

			/*
			+----------------------------------------------------+
			|  GET DMA ADDR                                      |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_GET_DMA_ADDR:
			{
				ulong ulDmaAddr = pDevExt->ulCommonBufferPA;

				dResult =
						put_user( ulDmaAddr, ( ulong * ) arg );
			}
			break;

			/*
			+----------------------------------------------------+
			|  GET DMA SIZE                                      |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_GET_DMA_SIZE:
			{
				ulong ulBufSize = pDevExt->ulCommonBufferSize;

				dResult = put_user( ulBufSize, ( ulong * )arg );
			}
			break;

			/*
			+----------------------------------------------------+
			|  SET HCTR                                          |
			+----------------------------------------------------+
			*/
			case ASTROPCI_SET_HCTR:
			{
				uint32_t u32Value = 0;

				dResult = get_user( u32Value, ( uint32_t * ) arg );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					Arc64_writeHCTR( pDevExt, u32Value );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  SET HCVR                                          |
			+----------------------------------------------------+
			*/
			case ASTROPCI_SET_HCVR:
			{
				uint32_t u32Value = 0;
				uint32_t u32Reply = 0;

				dResult = get_user( u32Value, ( uint32_t * ) arg );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					/* Clear the status bits if command not ABORT_READOUT.
					   The pci board can't handle maskable commands
					   (0xXX) during readout.
					  +-----------------------------------------------+ */
					if ( u32Value != ABORT_READOUT )
					{
						Arc64_writeHCVR( pDevExt, CLEAR_REPLY_FLAGS );
					}

					/* Pass the command to the PCI board
					  +-----------------------------------------------+ */
					Arc64_writeHCVR( pDevExt, u32Value );

					/* Return reply
					  +-----------------------------------------------+ */
					u32Reply = Arc64_checkReplyFlags( pDevExt );

					if ( u32Reply == RDR )
					{
						/* Flush the reply buffer
						  +--------------------------------------------+ */
						Arc64_flushReplyBuffer( pDevExt );

						/* Go read some data
						  +--------------------------------------------+ */
						Arc64_writeHCVR( pDevExt, READ_REPLY_VALUE );

						if ( Arc64_checkDspOutputFifo( pDevExt ) )
						{
							u32Reply = Arc64_readREPLYBUFFER_32( pDevExt );
						}
					}
				}

				/* A value must be returned to the user, so don't protect it.
				  +------------------------------------------------------+ */
				dResult = put_user( u32Reply, ( uint32_t * ) arg );
			}
			break;

			/*
			+----------------------------------------------------+
			|  SET HCVR DATA                                     |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_HCVR_DATA:
			{
				uint32_t u32Param = 0;

				dResult = get_user( u32Param, ( uint32_t * ) arg );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					if ( Arc64_checkDspInputFifo( pDevExt ) )
					{
						Arc64_writeCMDDATA_32( pDevExt, u32Param );
					}
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  SEND COMMAND                                      |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_COMMAND:
			{
				uint32_t u32CmdData[ CMD_MAX ]	= { 0, 0, 0, 0, 0, 0 };
				uint32_t u32CurrStatus			= 0;
				uint32_t u32Reply				= 0;
				int      dParamCount			= 0;
				int      i						= 0;

        		dResult = copy_from_user( u32CmdData,
        								  ( uint32_t * )arg,
        								  sizeof( u32CmdData ) );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					/* Check that the command isn't maskable and that
					   we're currently not in readout.
					  +-----------------------------------------------------+ */
					u32CurrStatus = ( Arc64_readHSTR( pDevExt ) & HTF_BIT_MASK ) >> 3;

					if ( ( u32CmdData[ 1 ] & 0x8000 ) == 0 && u32CurrStatus == READOUT_STATUS )
					{
						dResult = -EIO;
					}

					/* Clear the status bits
					  +-----------------------------------------------------+ */
					if ( !IS_ERR_VALUE( dResult ) )
					{
						dResult = Arc64_writeHCVR( pDevExt, CLEAR_REPLY_FLAGS );
					}

					/* Wait for the FIFO to be empty.
					  +-----------------------------------------------------+ */
					if ( !IS_ERR_VALUE( dResult ) )
					{
						if ( !Arc64_checkDspInputFifo( pDevExt ) )
						{
							dResult = -EIO;
						}
					}

					if ( !IS_ERR_VALUE( dResult ) )
					{
						/* Get the number of command parameters
						  +-------------------------------------------------------+ */
						dParamCount = u32CmdData[ 0 ] & 0x000000FF;

						if ( dParamCount > CMD_MAX )
						{
							ArcPrintf( "IOCTL - Incorrect number of command parameters!\n" );

							dResult = -EFAULT;
						}
						else
						{
							/* All is well, so write rest of the data.
							  +-------------------------------------------------------+ */
							for ( i = 0; i < dParamCount; i++ )
							{
								Arc64_writeCMDDATA_32( pDevExt, u32CmdData[ i ] );
							}

							/* Tell the PCI board to do a WRITE_COMMAND vector command
							  +-------------------------------------------------------+ */
							dResult = Arc64_writeHCVR( pDevExt, WRITE_COMMAND );
						}
					}

					if ( !IS_ERR_VALUE( dResult ) )
					{
						// Set the reply
						u32Reply =  Arc64_checkReplyFlags( pDevExt );

						if ( u32Reply == RDR )
						{
							// Flush the reply buffer
							Arc64_flushReplyBuffer( pDevExt );

							// Go read some data
							dResult = Arc64_writeHCVR( pDevExt, READ_REPLY_VALUE );

							if ( !IS_ERR_VALUE( dResult ) )
							{
								if ( Arc64_checkDspOutputFifo( pDevExt ) )
								{
									u32Reply = Arc64_readREPLYBUFFER_32( pDevExt );
								}
								else
								{
									dResult = -EFAULT;
								}
							}
						}
					}
				}

				/* Return reply
				  +------------------------+ */
				u32CmdData[ 0 ] = u32Reply;

				if ( copy_to_user( ( uint32_t * ) arg,
									u32CmdData,
									sizeof( u32CmdData ) ) )
				{
					dResult = -EFAULT;
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  PCI DOWNLOAD                                      |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_PCI_DOWNLOAD:
			{
				/* This vector command is here because it
				   expects NO reply.
				  +-----------------------------------------+ */
				dResult =
						Arc64_writeHCVR( pDevExt, PCI_DOWNLOAD );
			}
			break;

			/*
			+----------------------------------------------------+
			|  PCI DOWNLOAD WAIT                                 |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_PCI_DOWNLOAD_WAIT:
			{
				uint32_t u32Reply =
							Arc64_checkReplyFlags( pDevExt );

				if ( put_user( u32Reply, ( uint32_t * )arg ) )
				{
					dResult = -EFAULT;
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  GET CONFIG BYTE                                   |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_GET_CONFIG_BYTE:
			{
				uint32_t u32CfgReg = 0;
				uint8_t  u8Value   = 0;

        		dResult = get_user( u32CfgReg, ( uint32_t * )arg );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult =
						pci_read_config_byte( pDevExt->pPCIDev,
											  u32CfgReg,
											  &u8Value );
				}

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult = put_user( ( uint32_t )u8Value,
									    ( uint32_t * )arg );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  GET CONFIG WORD                                   |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_GET_CONFIG_WORD:
			{
				uint32_t u32CfgReg = 0;
				uint16_t u16Value  = 0;

        		dResult = get_user( u32CfgReg, ( uint32_t * )arg );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult =
						pci_read_config_word( pDevExt->pPCIDev,
											  u32CfgReg,
											  &u16Value );
				}

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult = put_user( ( uint32_t )u16Value,
									    ( uint32_t * )arg );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  GET CONFIG DWORD                                  |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_GET_CONFIG_DWORD:
			{
				uint32_t u32CfgReg = 0;
				uint32_t u32Value  = 0;

        		dResult = get_user( u32CfgReg, ( uint32_t * )arg );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult =
						pci_read_config_dword( pDevExt->pPCIDev,
											   u32CfgReg,
											   &u32Value );
				}

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult = put_user( ( uint32_t )u32Value,
									    ( uint32_t * )arg );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  SET CONFIG BYTE                                   |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_SET_CONFIG_BYTE:
			{
				uint32_t u32Params[ 2 ] = { 0, 0 };

				dResult = copy_from_user( u32Params,
										  ( uint32_t * )arg,
										  sizeof( u32Params ) );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult =
						pci_write_config_byte(
								pDevExt->pPCIDev,
								u32Params[ CFG_OFFSET ],
								( uint8_t )u32Params[ CFG_VALUE ] );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  SET CONFIG WORD                                   |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_SET_CONFIG_WORD:
			{
				uint32_t u32Params[ 2 ] = { 0, 0 };

				dResult = copy_from_user( u32Params,
										  ( uint32_t * )arg,
										  sizeof( u32Params ) );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult =
						pci_write_config_word(
								pDevExt->pPCIDev,
								u32Params[ CFG_OFFSET ],
								( uint16_t )u32Params[ CFG_VALUE ] );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  SET CONFIG DWORD                                  |
			+----------------------------------------------------+
        	*/
			case ASTROPCI_SET_CONFIG_DWORD:
			{
				uint32_t u32Params[ 2 ] = { 0, 0 };

				dResult = copy_from_user( u32Params,
										  ( uint32_t * )arg,
										  sizeof( u32Params ) );

				if ( !IS_ERR_VALUE( dResult ) )
				{
					dResult =
						pci_write_config_dword(
										pDevExt->pPCIDev,
										u32Params[ CFG_OFFSET ],
										u32Params[ CFG_VALUE ] );
				}
			}
			break;

			/*
			+----------------------------------------------------+
			|  DEFAULT                                           |
			+----------------------------------------------------+
        	*/
			default:
			{
				dResult = -EINVAL;
			}
			break;

		}	// end switch

 		up( &pDevExt->tSem );

	}  // end else

	return ( long )dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_isr                                                         |
+------------------------------------------------------------------------------+
|  PURPOSE:  Entry point. Interrupt handler.                                   |
+------------------------------------------------------------------------------+
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
static irqreturn_t Arc64_isr( int irq, void* dev_id, struct pt_regs* regs )
#else
static irqreturn_t Arc64_isr( int irq, void* dev_id )
#endif
{
	uint32_t u32IntFlag = 0;

	ArcDevExt* pDevExt = ( ArcDevExt * )dev_id;

	if ( pDevExt->dHasIRQ )
	{
		u32IntFlag = Arc64_readHSTR( pDevExt );
	}

	/* If no devices match the interrupting device, then exit
	  +--------------------------------------------------------+ */
	if ( !( u32IntFlag & DMA_INTERRUPTING ) )
	{
		return -IRQ_NONE;
	}

	/* Clear the interrupt, no questions asked
	  +--------------------------------------------------------+ */

//	#ifdef DEBUG_ON
//	ArcPrintf( "ISR - Clearing Interrupts!\n" );
//	#endif

	Arc64_writeHCVR( pDevExt, CLEAR_INTERRUPT );

	return 0;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_mmap                                                        |
+------------------------------------------------------------------------------+
|  PURPOSE:  Maps the kernel image buffer to user space.                       |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
|                                                                              |
|  NOTES:    For this function to work, the user must specify enough buffer    |
|            space by passing "mem=xxxM" to the kernel as a boot parameter.    |
|            If you have 128M RAM and want a 28M image buffer, then use:       |
|            "mem=100M".                                                       |
|                                                                              |
|		From: "Eli Rykoff" <erykoff@umich.edu>:                                |
|                                                                              |
|		The bug is in the astropci_mmap() function, and applies to all         |
|		versions of the astropci driver (RH9, FC2, FC4), when using at least   |
|		1GB of system ram.  It has to do with how the Linux kernel defines     |
|		"low memory" and "high memory." (Which I didn't know before, but I     |
|		sure do now). The Linux kernel can only directly address up to 915MB   |
|		of memory (with usual stock compile options), and the rest of the      |
|		system ram is the "high memory".  The problem comes in determining     |
|		where the top of the system ram is.  In the current version of the     |
|		driver, this is set as "__pa(high_memory)", which ( as it turns out )  |
|		is only equal to the top of the system ram if you have <915MB, and     |
|		maxes out at 0x38000000.  If you have >= 1GB of RAM then you can       |
|		allocate this memory, but as soon as you try to use it you get         |
|		problems. Unfortunately, I have found that the "right" way to do this  |
|		is not very well documented (or documented at all, for that matter),   |
|		but poking into mm.c and page.h in the Linux source reveals that the   |
|		top of the memory is at the address num_physpages*PAGE_SIZE.           |
+------------------------------------------------------------------------------+
*/
static int Arc64_mmap( struct file* filp, struct vm_area_struct* vma )
{
	int dResult = 0;

	/* Pointer to containing data structure of the character device inode
	  +-------------------------------------------------------------------+ */
	ArcDevExt* pDevExt = ( ArcDevExt * )filp->private_data;

	ArcPrintf( "MMAP - called!\n" );

	/* Check that this device actually exists
	  +-------------------------------------------------------------------+ */
	if ( !pDevExt )
	{
		dResult = -ENXIO;
	}

	else
	{
		spin_lock( &g_tArc64Lock );

		/*  Sanity check, failure here should be impossible
		  +---------------------------------------------------------------+ */
		if ( !pDevExt->dOpen )
		{
			ArcPrintf( "MMAP - Invalid device, not open\n" );

			dResult = -EAGAIN;
		}

		if ( !IS_ERR_VALUE( dResult ) )
		{
			ArcPrintf( "MMAP - Mapping memory for device %s\n", pDevExt->szName );

			dResult = Arc64_mapCommonBuffer( pDevExt, vma );
		}

		spin_unlock( &g_tArc64Lock );
	}

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_mapCommonBuffer                                             |
+------------------------------------------------------------------------------+
|  PURPOSE:  Maps the kernel image buffer to user space.                       |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
+------------------------------------------------------------------------------+
*/
static int Arc64_mapCommonBuffer( ArcDevExt* pDevExt, struct vm_area_struct* vma )
{
	uint uProt		= 0;
	int  dResult	= 0;
	int  i			= 0;

	unsigned long* pBuffer;


	if ( pDevExt == NULL )
	{
		ArcPrintf( "MMAP - NULL device extension pointer!\n" );

		dResult = -ENODEV;
	}

	if ( !IS_ERR_VALUE( dResult ) )
	{
		if ( ( vma->vm_end - vma->vm_start ) > g_ulSize )
		{
			ArcPrintf(
				"MMAP - Requested buffer size ( %lu ) too large! Common buffer size: %lu.\n",
				( vma->vm_end - vma->vm_start ),
				g_ulSize );

			dResult = -EINVAL;
		}
	}

	if ( !IS_ERR_VALUE( dResult ) )
	{
		/*  Save the image buffer physical address and size
		  +---------------------------------------------------------------------+ */
		pDevExt->ulCommonBufferPA	= ( unsigned long )page_to_phys( pDevExt->pMemBlock );
		pDevExt->ulCommonBufferSize	= ( vma->vm_end - vma->vm_start );

		ArcPrintf( "MMAP - board %s  buffer PA start: 0x%lX  end: 0x%lX size: %lu\n",
				    pDevExt->szName,
				    pDevExt->ulCommonBufferPA,
				    ( pDevExt->ulCommonBufferPA + pDevExt->ulCommonBufferSize ),
				    pDevExt->ulCommonBufferSize );

		/*  Ensure that the memory will not be cached; see drivers/char/mem.c
		  +---------------------------------------------------------------------+ */
		if ( boot_cpu_data.x86 > 3 )
		{
			uProt = pgprot_val( vma->vm_page_prot ) | _PAGE_PCD | _PAGE_PWT;

			vma->vm_page_prot = __pgprot( uProt );
		}

		/*  Set flag for I/O resource AND don't try to swap out physical pages
		  +---------------------------------------------------------------------+ */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
		vma->vm_flags |= ( VM_IO | VM_LOCKED | ( VM_DONTEXPAND | VM_DONTDUMP ) );
#else
		vma->vm_flags |= ( VM_IO | VM_RESERVED );
#endif

		/*  Remap the page range to see the high memory
		  +---------------------------------------------------------------------+ */
		dResult = remap_pfn_range( vma,
								   vma->vm_start,
								   page_to_pfn( pDevExt->pMemBlock ),
								   ( vma->vm_end - vma->vm_start ),
								   vma->vm_page_prot );

		if ( IS_ERR_VALUE( dResult ) )
		{
			ArcPrintf( "MMAP - Remap page range failed.\n" );
		}
		else
		{
			ArcPrintf( "MMAP - %lu bytes mapped: 0x%lX - 0x%lX --> 0x%lX - 0x%lX\n",
						 pDevExt->ulCommonBufferSize,
						 vma->vm_start,
						 vma->vm_end,
						 pDevExt->ulCommonBufferPA,
						 ( pDevExt->ulCommonBufferPA + pDevExt->ulCommonBufferSize ) );

			/*  Save the virtual address, this seems to be what the driver needs in
			    order to access the image buffer.
			  +---------------------------------------------------------------------+ */
			pDevExt->ulCommonBufferVA = vma->vm_start;

			/*  Write some test values to the image buffer.
			  +---------------------------------------------------------------------+ */
			ArcPrintf( "MMAP - Writing test values to image buffer\n" );

			pBuffer = ( unsigned long * )pDevExt->ulCommonBufferVA;

			if ( pBuffer != NULL )
			{
				for ( i = 0; i < pDevExt->ulCommonBufferSize/sizeof( unsigned long ); i++ )
				{
					#if defined( __x86_64 ) || defined( __x86_64__ )
						pBuffer[ i ] = 0xDEADDEADDEADDEAD;
					#else
						pBuffer[ i ] = 0xDEADDEAD;
					#endif
				}

//				ArcPrintf( "MMAP - buffer[ 0 ]: 0x%lX\n", pBuffer[ 0 ] );
//				ArcPrintf( "MMAP - buffer[ 1 ]: 0x%lX\n", pBuffer[ 1 ] );
//				ArcPrintf( "MMAP - buffer[ 2 ]: 0x%lX\n", pBuffer[ 2 ] );
//				ArcPrintf( "MMAP - buffer[ 3 ]: 0x%lX\n", pBuffer[ 3 ] );
			}
			else
			{
				ArcPrintf( "MMAP - Failed to access image buffer!\n" );
			}

			/*  Assign the image buffers
			  +---------------------------------------------------------------+ */
			Arc64_setBufferAddresses( pDevExt );
		}
	}

	/*  If there was a failure, set imageBufferPhysAddr to zero as a flag
	  +-----------------------------------------------------------------------+ */
	if ( IS_ERR_VALUE( dResult ) )
	{
		pDevExt->ulCommonBufferPA	= 0;
		pDevExt->ulCommonBufferVA	= 0;
		pDevExt->ulCommonBufferSize	= 0;
	}

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_mapBars                                                     |
+------------------------------------------------------------------------------+
|  PURPOSE:  Map the device memory regions into kernel virtual address space.  |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
+------------------------------------------------------------------------------+
*/
static int __init Arc64_mapBars( ArcDevExt *pDevExt, struct pci_dev *dev )
{
	int dResult	= 0;
	int i		= 0;

	for ( i=0; i<BAR_COUNT; i++ )
	{
		/* Get the BAR start address
		  +----------------------------------------------------+ */
		unsigned long ulBarStart  = pci_resource_start( dev, i );
		unsigned long ulBarEnd    = pci_resource_end( dev, i );
		unsigned long ulBarLength = ulBarEnd - ulBarStart + 1;

		pDevExt->pBar[ i ] = NULL;

		/* Only map non-zero memory BARs
		  +----------------------------------------------------+ */
		if ( ulBarStart && ulBarEnd )
		{
			/* Map I/O region into kernel virtual address space
			  +-------------------------------------------------+ */
			pDevExt->pBar[ i ] = pci_iomap( dev, i, ulBarLength );

			/* Move on if we successfully got the I/O address
			  +--------------------------------------------------+ */
			if ( !pDevExt->pBar[ i ] )
			{
				ArcPrintf( "MAP BARS - Failed to map BAR #%d!\n", i );

				dResult = -1;

				break;
			}

			ArcPrintf( "MAP BARS - Successfully mapped BAR #%d [ size: 0x%lX addr: 0x%lX ]\n",
						i, ulBarLength, ulBarStart );
		}
	}

	if ( dResult == -1 )
	{
		Arc64_unMapBars( pDevExt, dev );
	}

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_unMapBars                                                   |
+------------------------------------------------------------------------------+
|  PURPOSE:  Unmap the BAR regions that had been mapped earlier using          |
|            map_bars().                                                       |
|                                                                              |
|  RETURNS:  N/A                                                               |
+------------------------------------------------------------------------------+
*/
static void Arc64_unMapBars( ArcDevExt *pDevExt, struct pci_dev *dev )
{
	int i = 0;

	ArcPrintf( "UNMAP BARS - Unmapping ALL PCI BARs!\n" );

	for ( i=0; i<BAR_COUNT; i++ )
	{
		if ( pDevExt->pBar[ i ] )
		{
			pci_iounmap( dev, pDevExt->pBar[ i ] );

			pDevExt->pBar[ i ] = NULL;
		}
	}
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_cdevInit                                                    |
+------------------------------------------------------------------------------+
|  PURPOSE:  Initialize character device                                       |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
+------------------------------------------------------------------------------+
*/
static int Arc64_cdevInit( ArcDevExt *pDevExt )
{
	int dResult	= 0;

	/* Create a new device using the ARC-64 major and the next available minor
	  +-------------------------------------------------------------------------+ */
	if ( g_dArc64Minor >= ARC64_MAX_DEV )
	{
		dResult = -ENODEV;
	}

	else
	{
		pDevExt->tCDevno = MKDEV( g_dArc64Major, g_dArc64Minor++ );

		/* Couple the device file operations to the character device
		  +----------------------------------------------------------+ */
		cdev_init( &pDevExt->tCDev, &Arc64_fops );

		pDevExt->tCDev.owner = THIS_MODULE;
		pDevExt->tCDev.ops	= &Arc64_fops;

		/* Bring character device live
		  +----------------------------------------------------------+ */
		dResult = cdev_add( &pDevExt->tCDev, pDevExt->tCDevno, 1 );

		if ( dResult < 0 )
		{
			ArcPrintf( "CDEV INIT - cdev_add() = %d\n", dResult );

			/* Free the dynamically allocated character device node
			  +-------------------------------------------------------+ */
			unregister_chrdev_region( pDevExt->tCDevno, 1 );
		}

		else
		{
			/* Set the device name in the device extension
			  +-------------------------------------------------------+ */
			sprintf( pDevExt->szName, "%s%d", DEVICE_NAME, MINOR( pDevExt->tCDevno ) );

			ArcPrintf( "CDEV INIT - Device: %s Major:Minor = %d:%d\n",
						pDevExt->szName,
						MAJOR( pDevExt->tCDevno ),
						MINOR( pDevExt->tCDevno ) );
		}
	}

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_cdevCleanUp                                                 |
+------------------------------------------------------------------------------+
|  PURPOSE:  Initialize character device                                       |
|                                                                              |
|  RETURNS:  Returns 0 for success, or the appropriate error number.           |
+------------------------------------------------------------------------------+
*/
static void Arc64_cdevCleanUp( ArcDevExt *pDevExt )
{
	/* Remove the character device
	  +---------------------------------------------------+ */
	cdev_del( &pDevExt->tCDev );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_setBufferAddresses                                          |
+------------------------------------------------------------------------------+
|  PURPOSE:  Pass the DMA kernel buffer address to the DSP                     |
|                                                                              |
|  RETURNS:  0 on success, -ENXIO on failure.                                  |
+------------------------------------------------------------------------------+
*/
static int Arc64_setBufferAddresses( ArcDevExt* pDevExt )
{
	uint32_t u32PhysAddr	= 0;
	int      dResult		= 0;
	int      dReply			= 0;

	/* Clear the reply flags - Oct 23, 2008
	  +-----------------------------------------------+ */
	Arc64_writeHCVR( pDevExt, CLEAR_REPLY_FLAGS );

	/* Pass the DMA kernel buffer address to the DSP
	  +-----------------------------------------------+ */
	if ( Arc64_checkDspInputFifo( pDevExt ) )
	{
		u32PhysAddr = pDevExt->ulCommonBufferPA;

		Arc64_writeCMDDATA_16(
					pDevExt,
					( uint16_t )( u32PhysAddr & 0x0000FFFF ) );

		Arc64_writeCMDDATA_16(
					pDevExt,
					( uint16_t )( ( u32PhysAddr & 0xFFFF0000 ) >> 16 ) );

		ArcPrintf(
			"SET BUFFER ADDR - DMA buffer address ( 0x%X )set on PCI board\n",
			 u32PhysAddr );
 	}
	else
	{
		ArcPrintf( "SET BUFFER ADDR - Timeout while	setting DMA buffer address\n" );

		dResult = -ENXIO;
	}

	if ( !IS_ERR_VALUE( dResult ) )
	{
		dResult = Arc64_writeHCVR( pDevExt, WRITE_PCI_ADDRESS );

 		/* Check the reply
		  +-----------------------------------------------+ */
		if ( !IS_ERR_VALUE( dResult ) )
		{
			dReply = Arc64_checkReplyFlags( pDevExt );

			if ( dReply != DON )
			{
				ArcPrintf(
					"SET BUFFER ADDR - WRITE_PCI_ADDRESS failed! Device: %d:%d Reply: 0x%X\n",
					 MAJOR( pDevExt->tCDevno ),
					 MINOR( pDevExt->tCDevno ),
					 dReply );

				dResult = -ENXIO;
			}
		}
	}

	return dResult;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_flushReplyBuffer                                            |
+------------------------------------------------------------------------------+
|  PURPOSE:  Check that the DSP input buffer (fifo) is not full.               |
|                                                                              |
|  RETURNS:  Returns 0 if successful. Non-zero otherwise.                      |
+------------------------------------------------------------------------------+
*/
static int Arc64_flushReplyBuffer( ArcDevExt* pDevExt )
{
	uint32_t u32Status	= 0;
	int      dReply		= 0;
	int      i			= 0;

	/* Flush the reply buffer FIFO on the PCI DSP.
	   6 is the number of 24 bit words the FIFO can hold.
	  +-----------------------------------------------------+ */
	for ( i = 0; i < 6; i++ )
	{
		u32Status = Arc64_readHSTR( pDevExt );

		if ( ( u32Status & OUTPUT_FIFO_OK_MASK ) == OUTPUT_FIFO_OK_MASK )
		{
			dReply = Arc64_readREPLYBUFFER_32( pDevExt );
		}
		else
		{
			break;
		}
	}

	return dReply;
}


/*
+-------------------------------------------------------------------------------+
|  FUNCTION: Arc64_checkReplyFlags                                              |
+-------------------------------------------------------------------------------+
|  PURPOSE:  Check the current PCI DSP status. Uses HSTR HTF bits 3,4,5.        |
|                                                                               |
|  RETURNS:  Returns DON if HTF bits are a 1 and command successfully completed |
|            Returns RDR if HTF bits are a 2 and a reply needs to be read.      |
|            Returns ERR if HTF bits are a 3 and command failed.                |
|            Returns SYR if HTF bits are a 4 and a system reset occurred.       |
|                                                                               |
| NOTES:    This function must be called after sending a command to the PCI     |
|           board or controller.                                                |
+-------------------------------------------------------------------------------+
*/
static int Arc64_checkReplyFlags( ArcDevExt* pDevExt )
{
	uint32_t u32Status	= 0;
	int      dReply		= TIMEOUT;


	do {
		u32Status = Arc64_waitForCondition( pDevExt, CHECK_REPLY );

		if ( u32Status == DONE_STATUS )
		{
			dReply = DON;
		}

		else if ( u32Status == READ_REPLY_STATUS )
		{
			dReply = RDR;
		}

		else if ( u32Status == ERROR_STATUS )
		{
			dReply = ERR;
		}

		else if ( u32Status == SYSTEM_RESET_STATUS )
		{
			dReply = SYR;
		}

		else if ( u32Status == READOUT_STATUS )
		{
			dReply = READOUT;
		}

		/* Clear the status bits if not in READOUT
		  +--------------------------------------------------+ */
		if ( dReply != READOUT )
		{
 			Arc64_writeHCVR( pDevExt, CLEAR_REPLY_FLAGS );
		}

	} while ( u32Status == BUSY_STATUS );

	return dReply;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_checkDspInputFifo                                           |
+------------------------------------------------------------------------------+
|  PURPOSE:  Check that the DSP input buffer (fifo) is not full.               |
|                                                                              |
|  RETURNS:  Returns INPUT_FIFO_OK_MASK if HSTR bit 1 is set and buffer is     |
|            available for input.                                              |
|                                                                              |
|            Returns 0 if HSTR bit 1 is unset and buffer is unavailable for    |
|            input.                                                            |
|                                                                              |
|  NOTES:    This function must be called before writing to any register in    |
|            the astropci_regs structure. Otherwise, data may be overwritten   |
|            in the DSP input buffer, since the DSP cannot keep up with the    |
|            input rate. This function will exit after "max_tries". This will  |
|            help prevent the system from hanging in case the PCI DSP hangs.   |
+------------------------------------------------------------------------------+
*/
static int Arc64_checkDspInputFifo( ArcDevExt* pDevExt )
{
	return Arc64_waitForCondition( pDevExt, INPUT_FIFO );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_checkDspOutputFifo                                          |
+------------------------------------------------------------------------------+
|  PURPOSE:  Check that the DSP output buffer (fifo) has data.                 |
|                                                                              |
|  RETURNS:  Returns OUTPUT_FIFO_OK_MASK if HSTR bit 1 is set and buffer is    |
|            available for output                                              |
+------------------------------------------------------------------------------+
*/
static int Arc64_checkDspOutputFifo( ArcDevExt* pDevExt )
{
	return Arc64_waitForCondition( pDevExt, OUTPUT_FIFO );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_waitForCondition                                            |
+------------------------------------------------------------------------------+
|  PURPOSE: Waits for a particular state of the HSTR register. The condition   |
|           can be INPUT_FIFO, OUTPUT_FIFO, or CHECK_REPLY.                    |
|                                                                              |
|  RETURNS:  0 on timeout, else a masked copy of the value of HSTR.            |
|                                                                              |
| NOTES:    The condition is first tested, and if satisfied, the routine       |
|           returns immediately. Else, up to BUSY_MAX_WAIT microseconds        |
|           are spent polling every BUSY_WAIT_DELAY microseconds. If           |
|           the condition is still unsatisfied, up to SLEEP_MAX_WAIT           |
|           microseconds total time will be spent, sleeping in intervals       |
|           of SLEEP_WAIT_DELAY microseconds.                                  |
|                                                                              |
|           So, this routine will busy wait for at most BUSY_MAX_WAIT          |
|           microseconds, and is guaranteed to return within SLEEP_MAX_WAIT    |
|           microseconds (provided SLEEP_MAX_WAIT is greater than              |
|           BUSY_MAX_WAIT) plus or minus a jiffy.                              |
|                                                                              |
|           BUSY_WAIT_DELAY should be choosen so as not to overly              |
|           tax the PCI card.                                                  |
|                                                                              |
|           BUSY_MAX_WAIT should be short enough not to cause unacceptable     |
|           non-responsiveness of the computer, but long enough to cope        |
|           with typical hardware delays.                                      |
|                                                                              |
|           SLEEP_WAIT_DELAY should be at least a jiffy or three.              |
|                                                                              |
|           SLEEP_MAX_WAIT should be the longest time before we clearly        |
|           have a timeout problem.                                            |
+------------------------------------------------------------------------------+
*/
static uint32_t Arc64_waitForCondition( ArcDevExt* pDevExt, int dCondition2Wait4 )
{
	struct timeval	tNow;
	struct timeval	tInitial;
	uint32_t		u32Status      = 0;
	uint32_t		u32ElapsedTime = 0L;
	uint32_t		u32SleepDelay  = 0L;

	/* Set the sleep delay ( in jiffies )
	  +------------------------------------------------+ */
	u32SleepDelay = ( SLEEP_WAIT_DELAY * HZ ) / 1000000L;

	if ( u32SleepDelay < 1 )
	{
		u32SleepDelay = 1;
	}

	/* Get the current time
	  +------------------------------------------------+ */
	do_gettimeofday( &tInitial );

	/* Loop for the specified number of usec
	  +------------------------------------------------+ */
	while ( u32ElapsedTime < 10 * SLEEP_MAX_WAIT )
	{
		u32Status = Arc64_readHSTR( pDevExt );

		switch ( dCondition2Wait4 )
		{
			case INPUT_FIFO:
			{
				u32Status &= INPUT_FIFO_OK_MASK;
			}
			break;

			case OUTPUT_FIFO:
			{
				u32Status &= OUTPUT_FIFO_OK_MASK;
			}
			break;

			case CHECK_REPLY:
			{
				u32Status = ( u32Status & HTF_BIT_MASK ) >> 3;
			}
			break;
		}

		if ( u32Status > 0 )
		{
			break;
		}

		/* Get the current time and the elapsed time (usec)
		  +------------------------------------------------+ */
		do_gettimeofday( &tNow );

		u32ElapsedTime = ( tNow.tv_sec  - tInitial.tv_sec ) * 1000000L +
						 ( tNow.tv_usec - tInitial.tv_usec );

		if ( u32ElapsedTime < BUSY_MAX_WAIT )
		{
			/* Busy wait delay
			  +--------------------------------------------+ */
			udelay( BUSY_WAIT_DELAY );
 		}
		else
		{
			/* Sleep delay
			  +--------------------------------------------+ */
			current->state = TASK_UNINTERRUPTIBLE;

			schedule_timeout( u32SleepDelay );
		}
	}

	return u32Status;
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_readRegister_32                                             |
+------------------------------------------------------------------------------+
|  PURPOSE: Reads a 32-bit value from a BAR 0 address.                         |
|                                                                              |
|  RETURNS:  Returns the 32-bit value read from "addr".                        |
+------------------------------------------------------------------------------+
*/
static uint32_t Arc64_readRegister_32( void* __iomem pAddr )
{
	ndelay( REGISTER_ACCESS_DELAY );

	return ioread32( pAddr );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_readRegister_16                                             |
+------------------------------------------------------------------------------+
|  PURPOSE: Reads a 16-bit value from a BAR 0 address.                         |
|                                                                              |
|  RETURNS:  Returns the 16-bit value read from "addr".                        |
+------------------------------------------------------------------------------+
*/
static uint16_t Arc64_readRegister_16( void* __iomem pAddr )
{
	ndelay( REGISTER_ACCESS_DELAY );

	return ioread16( pAddr );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_writeRegister_32                                            |
+------------------------------------------------------------------------------+
|  PURPOSE: Writes a 32-bit value to a BAR 0 address.                          |
|                                                                              |
|  RETURNS:  N/A                                                               |
+------------------------------------------------------------------------------+
*/
static void
Arc64_writeRegister_32( uint32_t u32Value, void* __iomem pAddr )
{
	ndelay( REGISTER_ACCESS_DELAY );

	iowrite32( u32Value, pAddr );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_writeRegister_16                                            |
+------------------------------------------------------------------------------+
|  PURPOSE: Writes a 16-bit value to a BAR 0 address.                          |
|                                                                              |
|  RETURNS:  N/A                                                               |
+------------------------------------------------------------------------------+
*/
static void
Arc64_writeRegister_16( uint16_t u16Value, void* __iomem pAddr )
{
	ndelay( REGISTER_ACCESS_DELAY );

	iowrite16( u16Value, pAddr );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_readHCTR                                                    |
+------------------------------------------------------------------------------+
|  PURPOSE: Reads a 32-bit value from the HCTR. Calls Arc64_readRegister_32.   |
|                                                                              |
|  RETURNS:  Returns the value read.                                           |
+------------------------------------------------------------------------------+
*/
static uint32_t Arc64_readHCTR( ArcDevExt* pDevExt )
{
	return Arc64_readRegister_32( pDevExt->pBar[ 0 ] + HCTR );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_readHSTR                                                    |
+------------------------------------------------------------------------------+
|  PURPOSE: Reads a 32-bit value from the HSTR. Calls Arc64_readRegister_32.   |
|                                                                              |
|  RETURNS:  Returns the value read.                                           |
+------------------------------------------------------------------------------+
*/
static uint32_t Arc64_readHSTR( ArcDevExt* pDevExt )
{
	return Arc64_readRegister_32( pDevExt->pBar[ 0 ] + HSTR );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_readREPLYBUFFER_32                                          |
+------------------------------------------------------------------------------+
|  PURPOSE: Reads a 32-bit value from the REPLY_BUFFER.                        |
|           Calls Arc64_readRegister_32.                                       |
|                                                                              |
|  RETURNS:  Returns the value read.                                           |
+------------------------------------------------------------------------------+
*/
static uint32_t Arc64_readREPLYBUFFER_32( ArcDevExt* pDevExt )
{
	return Arc64_readRegister_32( pDevExt->pBar[ 0 ] + REPLY_BUFFER );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_readREPLYBUFFER_16                                          |
+------------------------------------------------------------------------------+
|  PURPOSE: Reads a 16-bit value from the REPLY_BUFFER.                        |
|           Calls Arc64_readRegister_16.                                       |
|                                                                              |
|  RETURNS:  Returns the value read.                                           |
+------------------------------------------------------------------------------+
*/
static uint16_t Arc64_readREPLYBUFFER_16( ArcDevExt* pDevExt )
{
	return Arc64_readRegister_16( pDevExt->pBar[ 0 ] + REPLY_BUFFER );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_writeHCTR                                                   |
+------------------------------------------------------------------------------+
|  PURPOSE: Writes a 32-bit value to the HCTR. Calls Arc64_writeRegister_32.   |
|                                                                              |
|  RETURNS:  N/A                                                               |
+------------------------------------------------------------------------------+
*/
static void Arc64_writeHCTR( ArcDevExt* pDevExt, uint32_t u32RegVal )
{
	Arc64_writeRegister_32( u32RegVal, pDevExt->pBar[ 0 ] + HCTR );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_writeCMDDATA_32                                             |
+------------------------------------------------------------------------------+
|  PURPOSE: Writes a 32-bit value to the CMD_DATA.                             |
|           Calls Arc64_writeRegister_32.                                      |
|                                                                              |
|  RETURNS:  N/A                                                               |
+------------------------------------------------------------------------------+
*/
static void Arc64_writeCMDDATA_32( ArcDevExt* pDevExt, uint32_t u32RegVal )
{
	Arc64_writeRegister_32( u32RegVal, pDevExt->pBar[ 0 ] + CMD_DATA );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_writeCMDDATA_16                                             |
+------------------------------------------------------------------------------+
|  PURPOSE: Writes a 16-bit value to the CMD_DATA.                             |
|           Calls Arc64_writeRegister_16.                                      |
|                                                                              |
|  RETURNS:  N/A                                                               |
+------------------------------------------------------------------------------+
*/
static void Arc64_writeCMDDATA_16( ArcDevExt* pDevExt, uint16_t u16RegVal )
{
	Arc64_writeRegister_16( u16RegVal, pDevExt->pBar[ 0 ] + CMD_DATA );
}


/*
+------------------------------------------------------------------------------+
|  FUNCTION: Arc64_writeHCVR                                                   |
+------------------------------------------------------------------------------+
|  PURPOSE: Writes a 32-bit value to the HCVR. Checks that the HCVR register   |
|	        bit 1 is not set, otherwise a command is still in the register.    |
|	        Calls WriteRegister_32.                                            |
|                                                                              |
|  RETURNS:  N/A                                                               |
+------------------------------------------------------------------------------+
*/
static int Arc64_writeHCVR( ArcDevExt* pDevExt, uint32_t u32RegVal )
{
	uint32_t	u32CurrentValue	= 0;
	int			i				= 0;
	int			dStatus			= -EIO;

	for ( i=0; i<100; i++ )
	{
		u32CurrentValue = Arc64_readRegister_32( pDevExt->pBar[ 0 ] + HCVR );

		if ( ( u32CurrentValue & ( uint32_t )0x1 ) == 0 )
		{
			dStatus = 0;

			break;
		}

		ArcPrintf(
			"Write_HCVR - HCVR not ready. Count: %d Value: 0x%X\n",
			 i,
			 u32CurrentValue );
	}

	if ( dStatus == 0 )
	{
		Arc64_writeRegister_32( u32RegVal, pDevExt->pBar[ 0 ] + HCVR );
	}

	return dStatus;
}


/*
+=====================================================================================================================+
||                                                                                                                   ||
||                                      Large Memory Allocation Section                                              ||
||                                                                                                                   ||
+=====================================================================================================================+
*/

/*
+------------------------------------------------------------------------------+
|  Un-Comment to allow debug printf statements                                 |
+------------------------------------------------------------------------------+
*/
//#define ARC_BIGBUF_DEBUG


/*
+------------------------------------------------------------------------------+
|  Chapter: basic allocation unit retrieved via the buddy allocator            |
+------------------------------------------------------------------------------+
*/
#define CHAPTER_ORDER	( MAX_ORDER - 1 )				/* page order of chapter */
#define CHAPTER_PAGES	( 1 << CHAPTER_ORDER )			/* pages in a chapter    */
#define CHAPTER_SIZE	( PAGE_SIZE * CHAPTER_PAGES )	/* chapter size in bytes */


/*
+------------------------------------------------------------------------------+
|  Cluster definition                                                          |
+------------------------------------------------------------------------------+
|  We join adjacent chapters into clusters, keeping track of allocations as an |
|  ordered set of clusters.                                                    |
|                                                                              |
|  Note that the physical page frame number (pfn) is stored in the hopes that  |
|  continuous pfn's represent continuous memory. Should we merge clusters via  |
|  dma_addr_t physical addresses?                                              |
+------------------------------------------------------------------------------+
*/
struct cluster
{
	struct list_head head;	/* ordered list of clusters */
	ulong page_first;		/* first page in cluster    */
	ulong page_count;		/* number of pages          */
};


/*
+------------------------------------------------------------------------------+
|  Cluster set definition                                                      |
+------------------------------------------------------------------------------+
*/
struct cluster_set
{
	struct list_head clusters;	/* allocated clusters */
};


/*
+------------------------------------------------------------------------------+
|  Declare and initialize a cluster set                                        |
+------------------------------------------------------------------------------+
*/
#define CLUSTER_SET( name ) \
		struct cluster_set name = { clusters: LIST_HEAD_INIT( name.clusters ) }


/*
+------------------------------------------------------------------------------+
|  Retrieve the cluster from it's list head                                    |
+------------------------------------------------------------------------------+
*/
static struct cluster *get_cluster( struct list_head *node )
{
	return list_entry( node, struct cluster, head );
}


/*
+------------------------------------------------------------------------------+
|  Retrieve the physical address of the first page in the cluster              |
+------------------------------------------------------------------------------+
*/
static inline dma_addr_t phys_start( const struct cluster *cl )
{
	return page_to_phys( pfn_to_page( cl->page_first ) );
}


/*
+------------------------------------------------------------------------------+
|  Retrieve the physical address of the last page in the cluster               |
+------------------------------------------------------------------------------+
*/
static inline dma_addr_t phys_end( const struct cluster *cl )
{
	return page_to_phys(nth_page(pfn_to_page(cl->page_first), cl->page_count));
}


/*
+------------------------------------------------------------------------------+
|  Return node after target location for new chapter ( passed as pfn )         |
+------------------------------------------------------------------------------+
*/
static struct list_head *find_insert_location( struct cluster_set* set, ulong chapter_start )
{
	struct list_head *p;

	list_for_each( p, &set->clusters )
	{
		if ( get_cluster( p )->page_first > chapter_start )
		{
			return p;
		}
	}

	return &set->clusters;
}


/*
+------------------------------------------------------------------------------+
|  Try to merge a new chapter by prepending it to the cluster at pos.          |
|  Return true on success, false if unable to merge.                           |
+------------------------------------------------------------------------------+
*/
static bool try_prepend( struct cluster_set *set, struct list_head *pos, ulong chapter_start )
{
	if ( pos != &set->clusters )
	{
		struct cluster *cl = get_cluster( pos );

		if ( chapter_start + CHAPTER_PAGES == cl->page_first )
		{
			cl->page_first = chapter_start;
			cl->page_count += CHAPTER_PAGES;

			return true;
		}
	}

	return false;
}


/*
+------------------------------------------------------------------------------+
|  Try to merge a new chapter by appending it to cluster at pos.               |
|  Return true on success, false if unable to merge.                           |
+------------------------------------------------------------------------------+
*/
static bool try_append( struct cluster_set* set, struct list_head* pos, ulong chapter_start )
{
	if (pos != &set->clusters)
	{
		struct cluster *cl = get_cluster( pos );

		if ( cl->page_first + cl->page_count == chapter_start )
		{
			cl->page_count += CHAPTER_PAGES;

			return true;
		}
	}

	return false;
}


/*
+------------------------------------------------------------------------------+
|  Tries to merge the "pos" cluster with the cluster previous to it.           |
|  Returns true on success (invalidates previous cluster, pos stays valid).    |
+------------------------------------------------------------------------------+
*/
static void try_merge_prev( struct cluster_set* set, struct cluster* pos )
{
	struct list_head *prev_head = pos->head.prev;

	if (prev_head != &set->clusters)
	{
		struct cluster *prev = get_cluster(prev_head);

		if (prev->page_first + prev->page_count == pos->page_first)
		{
			pos->page_first = prev->page_first;
			pos->page_count += prev->page_count;
			list_del(prev_head);
			kfree(prev);
		}
	}
}


/*
+------------------------------------------------------------------------------+
|  Account for another chapter allocation, returning the cluster it became     |
|  part of. Returns NULL on error ( out of memory ).                           |
+------------------------------------------------------------------------------+
*/
static struct cluster* add_alloc( struct cluster_set* set, struct page* new_chapter )
{
	ulong chapter_start = page_to_pfn( new_chapter );

	struct list_head *insert_loc = find_insert_location( set, chapter_start );

#ifdef ARC_BIGBUF_DEBUG
	ArcPrintf( "add_alloc - chapter_start: %lu\n", chapter_start );
#endif

	if ( try_prepend( set, insert_loc, chapter_start ) )
	{
		struct cluster *cl = get_cluster( insert_loc );

#ifdef ARC_BIGBUF_DEBUG
		ArcPrintf( "add_alloc - PREPEND\n" );
#endif

		try_merge_prev( set, cl );

		return cl;
	}

	else if ( try_append( set, insert_loc->prev, chapter_start ) )
	{
#ifdef ARC_BIGBUF_DEBUG
		ArcPrintf( "add_alloc - APPEND\n" );
#endif

		return get_cluster( insert_loc->prev );
	}

	else
	{
		struct cluster *new_cluster = kmalloc( GFP_KERNEL, sizeof( *new_cluster ) );

#ifdef ARC_BIGBUF_DEBUG
		ArcPrintf( "add_alloc - NEW CLUSTER\n" );
#endif

		if ( new_cluster )
		{
			new_cluster->page_first = chapter_start;
			new_cluster->page_count = CHAPTER_PAGES;
			list_add_tail( &new_cluster->head, insert_loc );
		}

		return new_cluster;
	}
}


/*
+------------------------------------------------------------------------------+
|  Give up count chapters starting at start.                                   |
+------------------------------------------------------------------------------+
*/
static void free_chapters( struct page* start, unsigned long count )
{
	unsigned long i;

#ifdef ARC_BIGBUF_DEBUG
	ArcPrintf( "Freeing %lu chapters @ 0x%lx.\n", count, ( ulong )page_to_phys( start ) );
#endif

	for ( i = 0; i < count; i++, start = nth_page( start, CHAPTER_PAGES ) )
	{
		__free_pages( start, CHAPTER_ORDER );
	}
}


/*
+------------------------------------------------------------------------------+
|  Free the set and all clusters allocated to it.                              |
+------------------------------------------------------------------------------+
*/
static void free_set( struct cluster_set* set )
{
	struct cluster *pos, *t;

#ifdef ARC_BIGBUF_DEBUG
	ArcPrintf( "Freeing clusters.\n" );
#endif

	list_for_each_entry_safe( pos, t, &set->clusters, head )
	{
		free_chapters( pfn_to_page( pos->page_first ), ( pos->page_count / CHAPTER_PAGES ) );

		kfree( pos );
	}
}


/*
+------------------------------------------------------------------------------+
|  Lists the allocations in the given cluster set.                             |
+------------------------------------------------------------------------------+
*/
#ifdef ARC_BIGBUF_DEBUG
static void list_allocs( struct cluster_set* set )
{
	struct cluster* cluster;

	ArcPrintf( "list_allocs - Allocations in ascending order:\n" );

	list_for_each_entry( cluster, &set->clusters, head )
	{
		ArcPrintf( "list_allocs - Cluster from 0x%08lx .. 0x%08lx ( %lu pages ).\n",
					( ulong )phys_start( cluster ),
					( ulong )phys_end( cluster ),
					cluster->page_count );
	}
}
#endif


/*
+------------------------------------------------------------------------------+
|  Remove the given cluster from the set, keeping the allocation.              |
+------------------------------------------------------------------------------+
*/
static void unlink_cluster( struct cluster_set* set, struct cluster* cl )
{
	list_del_init( &cl->head );
}


/*
+------------------------------------------------------------------------------+
|  Allocate a big buffer of given size [bytes]. Flags as in alloc_pages.       |
+------------------------------------------------------------------------------+
*/
static struct page* Arc64_bigBufAlloc( unsigned int flags, ulong size )
{
	int order = size ? get_order( size ) : 0;

#ifdef ARC_BIGBUF_DEBUG
	ArcPrintf( "size: %ld, order: %d, CHAPTER_ORDER: %d, CHAPTER_SIZE: %ld, CHAPTER_PAGES: %d\n",
			 	size, order, CHAPTER_ORDER, CHAPTER_SIZE, CHAPTER_PAGES );
#endif

	if ( order <= CHAPTER_ORDER )
	{
		return alloc_pages( flags, order );
	}
	else
	{
		struct page* result;

		int chapters = ( size + CHAPTER_SIZE - 1 ) / CHAPTER_SIZE;

		CLUSTER_SET( allocation_set );

		struct cluster* merged;

#ifdef ARC_BIGBUF_DEBUG
		ArcPrintf( "Allocate huge block of size %lu (%i chapters).\n", size, chapters );
#endif

		do
		{
			struct page* chapter = alloc_pages( flags, CHAPTER_ORDER );

			if ( !chapter )
			{
				goto fail;
			}

#ifdef ARC_BIGBUF_DEBUG
			ArcPrintf( "Allocated chapter @ %lx.\n", ( ulong )page_to_phys( chapter ) );
#endif

			merged = add_alloc( &allocation_set, chapter );

			if ( !merged )
			{
				goto fail;
			}

#ifdef ARC_BIGBUF_DEBUG
			list_allocs( &allocation_set );
#endif

		} while ( merged->page_count < chapters * CHAPTER_PAGES );

		unlink_cluster( &allocation_set, merged );

#ifdef ARC_BIGBUF_DEBUG
		ArcPrintf( "After taking result:\n" );

		list_allocs( &allocation_set );
#endif

		free_set( &allocation_set );

		result = pfn_to_page( merged->page_first );

		kfree( merged );

		return result;

fail:
		free_set( &allocation_set );

		ArcPrintf( "Allocation failed.\n" );

		return NULL;
	}
}


/*
+------------------------------------------------------------------------------+
|  Free a buffer allocates by <module>_bigBufAlloc.                            |
+------------------------------------------------------------------------------+
*/
static void Arc64_bigBufFree( struct page* start, ulong size )
{
	int size_order = size ? get_order( size ) : 0;

	if ( size_order < CHAPTER_ORDER )
	{
		__free_pages( start, size_order );
	}
	else
	{
		free_chapters( start, ( ( size + CHAPTER_SIZE - 1 ) / CHAPTER_SIZE ) );
	}
}


MODULE_AUTHOR( "Scott Streit" );
MODULE_DESCRIPTION( "ARC-63/64 PCI Interface Driver" );
MODULE_SUPPORTED_DEVICE( DEVICE_NAME );

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
	MODULE_LICENSE("GPL");
#endif
