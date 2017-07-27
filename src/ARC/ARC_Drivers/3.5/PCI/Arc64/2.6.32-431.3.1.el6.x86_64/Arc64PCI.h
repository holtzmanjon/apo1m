#ifndef _ARC64PCI_H_
#define _ARC64PCI_H_


/*
+------------------------------------------------------------------------------+
|       General Definitions                                                    |
+------------------------------------------------------------------------------+
*/
#define DEVICE_NAME			( char * )"Arc64PCI"

#define ARC64_MAX_DEV		9

#define BAR_COUNT			6
#define BAR_NUM				0

#define REGS_SIZE           ( 0x9C/sizeof( uint32_t ) )*sizeof( uint32_t )
#define CMD_MAX				6
//#define DEBUG_ON

#define INPUT_FIFO  		0  		// For astropci_wait_for_condition()
#define OUTPUT_FIFO 		1
#define CHECK_REPLY 		2

#define CFG_OFFSET			0
#define CFG_VALUE			1


/*
+------------------------------------------------------------------------------+
|       Print Definition                                                       |
+------------------------------------------------------------------------------+
*/
#define ArcPrintf( fmt, args... ) printk( "[ Arc64 ]: " fmt, ## args )


/*
+------------------------------------------------------------------------------+
|  Define PCI/e Base Address Register ( BAR ) Macros                           |
+------------------------------------------------------------------------------+
*/
#define GET_BAR_TYPE( ulFlags )														\
	( ( ( ulFlags & IORESOURCE_IO ) == IORESOURCE_IO ) ? IORESOURCE_IO : 			\
	( ( ( ulFlags & IORESOURCE_MEM ) == IORESOURCE_MEM ) ? IORESOURCE_MEM : 0 ) )


/*
+------------------------------------------------------------------------------+
|  PCI DSP Control Registers                                                   |
+------------------------------------------------------------------------------+
*/
#define HCTR			0x10	// Host interface control register
#define HSTR			0x14	// Host interface status register
#define HCVR			0x18	// Host command vector register
#define REPLY_BUFFER	0x1C	// Reply Buffer
#define CMD_DATA		0x20	// DSP command register


/*
+------------------------------------------------------------------------------+
|       State Structure                                                        |
+------------------------------------------------------------------------------+
|  Driver state structure. All state variables related to the state of the     |
|  driver are kept in here.                                                    |
+------------------------------------------------------------------------------+
*/
typedef struct ArcDevExt_t
{
	/* PCI device structure */
	struct pci_dev* pPCIDev;

	/* PCI I/O start address (HSTR, etc) */
	void* __iomem pBar[ BAR_COUNT ];

	/* Char device structure */
	struct cdev tCDev;

	/* Char device count */
	dev_t tCDevno;

	/* Device name for probing purposes */
	char szName[ 20 ];

	/* Device registered with sysfs ( /dev entry ) */
	struct device* pDevice;

	/* Device in use = 1; available = 0 */
	int dOpen;

	/* Device IRQ has been requested = 1; otherwise 0 */
	int dHasIRQ;

	/* Semaphore to protect non-global access. i.e. ioctl commands */
	struct semaphore tSem;

	/* Big contiguous memory block for image buffer */
	struct page* pMemBlock;

	/* Virtual start address of image buffer */
	unsigned long ulCommonBufferVA;

	/* Physical start address of image buffer */
	unsigned long ulCommonBufferPA;

	/* Image buffer size (bytes) */
	unsigned long ulCommonBufferSize;

} ArcDevExt;


#endif	/* _ARC64PCI_H_ */
