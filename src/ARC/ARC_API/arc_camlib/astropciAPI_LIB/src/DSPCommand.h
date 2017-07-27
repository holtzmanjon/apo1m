/******************************************************************************
*
*   File:       DSPCommand.h
*   Version:	1.00
*   Author:     Scott Streit
*   Abstract:   C library for DSP functions.
*		I/O control commands for Solaris device driver for the SDSU PCI
*               Interface Card, and DSP Commands.
*               These commands are for use with ioctl().
*
*
*   Revision History:
*       Date            Who   Version    Description
*   --------------------------------------------------------------------------
*	11/22/99	sds	1.00	Initial
*
*       06/20/00        jmp     1.00 	A bunch of values are added.
*
* 	03/22/2001	sds	1.60    Upgraded commands to version 1.6.
*
******************************************************************************/
#ifndef DSPCOMMAND_H
#define DSPCOMMAND_H

#include "../WhichSystem.h"

#ifdef WIN2K
	#include <windows.h>
#endif

/*************************************************
*        Define General Constants
*************************************************/
#define UNDEFINED		-1

/*************************************************
*        Define Errors
*************************************************/
#define _ERROR			-1
#define _NO_ERROR		 0

/******************************************************************************
*	PCI Vector Commands
******************************************************************************/
#define READ_PIXEL_COUNT			0x8075
#define PCI_PC_RESET				0x8077
#define ABORT_READOUT				0x8079
#define BOOT_EEPROM					0x807B
#define READ_HEADER					0x81
#define RESET_CONTROLLER			0x87
#define WRITE_NUM_IMAGE_BYTES		0x8F
#define INITIALIZE_IMAGE_ADDRESS	0x91
#define WRITE_COMMAND				0xB1

/*************************************************
*        Define ASCII Commands
*************************************************/
#define TDL	0x0054444C	/* Test Data Link					*/
#define RDM	0x0052444D	/* Read Memory						*/
#define WRM	0x0057524D	/* Write Memory						*/
#define WSI	0x00575349	/* Write Synthetic Image			*/
#define SEX	0x00534558	/* Start Exposure					*/
#define SET	0x00534554	/* Set Exposure Time				*/
#define PEX	0x00504558	/* Pause Exposure					*/
#define REX	0x00524558	/* Resume Exposure					*/
#define RET	0x00524554	/* Read Elapsed Time				*/
#define AEX	0x00414558	/* Abort Exposure					*/
#define PON	0x00504F4E	/* Power On							*/
#define POF	0x00504F46	/* Power Off						*/
#define RDI	0x00524449	/* Read Image						*/
#define SOS	0x00534F53	/* Select Output Source				*/
#define MPP	0x004D5050	/* Multi-Pinned Phase Mode			*/
#define DCA	0x00444341	/* Download Coadder					*/
#define SNC	0x00534E43	/* Set Number of Coadds				*/
#define VID	0x00564944	/* mnemonic that means video board	*/
#define SBN	0x0053424E	/* Set Bias Number					*/
#define SBV	0x00534256	/* Set Bias Voltage					*/
#define SGN	0x0053474E	/* Set Gain							*/
#define RST	0x00525354	/* Reset							*/
#define SMX	0x00534D58	/* Select Multiplexer				*/
#define CLK	0x00434C4B	/* mnemonic means clock driver board*/
#define SSS	0x00535353	/* Set Subarray Sizes				*/
#define SSP	0x00535350	/* Set Subarray Positions			*/
#define LGN	0x004C474E	/* Set Low Gain						*/
#define HGN	0x0048474E	/* Set High Gain					*/
#define SRM	0x0053524D	/* Set Readout Mode - CDS or single	*/
#define CDS	0x00434453	/* Correlated Double Sampling		*/
#define SFS	0x00534653	/* Send Fowler Sample				*/
#define SPT	0x00535054	/* Set Pass Through Mode			*/
#define LDA	0x004C4441	/* Load Application					*/
#define RCC	0x00524343	/* Read Controller Configuration	*/
#define CLR	0x00434C52	/* Clear Array						*/
#define IDL	0x0049444C	/* Idle								*/
#define STP	0x00535450	/* Stop Idle						*/
#define CSH	0x00435348	/* Close Shutter					*/
#define OSH	0x004F5348	/* Open Shutter						*/
#define SUR	0x00535552	/* Set Up The Ramp Mode				*/

#define SAT     0x00534154      /* Set Array Temperature (3a only) */
#define RAT     0x00524154      /* Read Array Temperature (3a only) */

/*************************************************
*        Controller configuration bit masks.
*************************************************/
#define VIDEO_PROCESSOR_MASK		0x000007
#define TIMING_BOARD_MASK		0x000018
#define UTILITY_BOARD_MASK		0x000060
#define SHUTTER_MASK			0x000080
#define TEMPERATURE_READOUT_MASK	0x000300
#define SUBARRAY_READOUT_MASK		0x000400
#define BINNING_MASK			0x000800
#define READOUT_MASK			0x003000
#define MPP_MASK			0x004000
#define CLOCK_DRIVER_MASK		0x018000
#define SPECIAL_MASK			0x0E0000

/*************************************************
*        Define Controller Configuration Bits
*************************************************/
#define CCD_REV3B		0x000000
#define CCD_GENI		0x000001
#define CCD				0x000000
#define IR_REV4C		0x000002
#define IR_COADDER		0x000003
#define CLOCK_DRIVER	0x000000
#define TIM_REV4B		0x000000
#define TIM_GENI		0x000008
#define NO_UTIL_BOARD	0x000000
#define UTILITY_REV3	0x000020
#define SHUTTER			0x000080
#define NO_TEMPERTURE_CONTROL	0x000000
#define NONLINEAR_TEMP_CONV		0x000100
#define LINEAR_TEMP_CONV		0x000200
#define SUBARRAY		0x000400
#define BINNING			0x000800
#define SERIAL			0x001000
#define PARALLEL		0x002000
#define BOTH_READOUTS	0x003000
#define MPP_CAPABLE		0x004000
#define SDSU_MLO		0x020000
#define NGST			0x040000
#define NIRIM			(IR_REV4C | TIM_GENI)
#define DEFAULT_CONFIG_WORD	(NONLINEAR_TEMP_CONV | TIM_REV4B | UTILITY_REV3 | SHUTTER);  /* 0x0001A0 */

/*************************************************
*	Readout modes
*************************************************/
#define A_AMP	0x5F5F41	/* Ascii __A amp A.					*/
#define B_AMP	0x5F5F42	/* Ascii __B amp B.					*/
#define C_AMP	0x5F5F43	/* Ascii __C amp C.					*/
#define D_AMP	0x5F5F44	/* Ascii __D amp D.					*/
#define L_AMP	0x5F5F4C	/* Ascii __L left amp.				*/
#define R_AMP	0x5F5F52	/* Ascii __R left amp.				*/
#define LR_AMP	0x5F4C52	/* Ascii _LR right two amps.		*/
#define AB_AMP	0x5F4142	/* Ascii _AB top two amps A & B.	*/
#define CD_AMP	0x5F4344	/* Ascii _CD bottom two amps C & D. */
#define ALL_AMP	0x414C4C	/* Ascii ALL four amps (quad).		*/

/*************************************************
*	Define gain and speed constants
*************************************************/
#define ONE		1
#define TWO		2
#define FIVE	5
#define TEN		10
#define SLOW	0
#define FAST	1

/*************************************************
*	Define shutter positions
*************************************************/
#define  _OPEN_SHUTTER_ 	(1 << 11)
#define  _CLOSE_SHUTTER_ 	~(1 << 11)

/*************************************************
*        Define command replies
*************************************************/
#define TOUT	0x544F5554
#define DON     0x00444F4E
#define ERR		0x00455252
#define SYR     0x00535952
#define RST		0x00525354

/*************************************************
*	Define Readout Constants
*************************************************/
#define READOUT			5
#define READ_TIMEOUT	200

/******************************************************************************
*	Board Id Constants
******************************************************************************/
#define PCI_ID		1
#define TIM_ID		2
#define UTIL_ID		3

/******************************************************************************
*	Memory Location Id Constants
*		R	(Bit 20)  ROM
*		P	(Bit 21)  DSP program memory space
*		X	(Bit 22)  DSP X memory space
*		Y	(Bit 23)  DSP Y memory space
******************************************************************************/
#define P	0x100000
#define X 	0x200000
#define Y	0x400000
#define R	0x800000

/******************************************************************************
*  Masks to set the Host Control Register HCTR.
*
*       Only three bits of this register are used. Two are control bits to set
*  the mode of the PCI board (bits 8 and 9)  and  the  other (bit 3) is a flag
*  indicating the progress of image data transfer to the user's application.
*
*       Bit 3   = 1     Image buffer busy transferring to user space.
*               = 0     Image buffer not  transferring to user space.
*
*       Bit 8= 0 & Bit 9= 1   PCI board set to slave mode for PCI file download.
*       Bit 8= 0 & Bit 9= 0   PCI board set to normal processing.
*
*       Note that the HTF_MASK, sets the HTF bits 8 and 9 to transfer mode.
*
******************************************************************************/
#define HTF_MASK        0x200
#define HTF_CLEAR_MASK  0xFFFFFCFF
#define BIT3_CLEAR_MASK 0xFFFFFFF7
#define BIT3_SET_MASK   0x00000008
#define HTF_BITS		0x00000038

/******************************************************************************
*        Device Driver Commands
******************************************************************************/
#ifdef WIN2K
	#define ASTROPCI_DEVICE		33000	// Device Type - arbitrary # in range: 32768 to 65535.

	#define ASTROPCI_GET_HCTR \
		CTL_CODE(ASTROPCI_DEVICE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_GET_PROGRESS \
		CTL_CODE(ASTROPCI_DEVICE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_GET_DMA_ADDR \
		CTL_CODE(ASTROPCI_DEVICE, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_GET_HSTR \
		CTL_CODE(ASTROPCI_DEVICE, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_MEM_MAP \
		CTL_CODE(ASTROPCI_DEVICE, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_HCVR_DATA \
		CTL_CODE(ASTROPCI_DEVICE, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_SET_HCTR \
		CTL_CODE(ASTROPCI_DEVICE, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_SET_HCVR \
		CTL_CODE(ASTROPCI_DEVICE, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_PCI_DOWNLOAD \
		CTL_CODE(ASTROPCI_DEVICE, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_PCI_DOWNLOAD_WAIT \
		CTL_CODE(ASTROPCI_DEVICE, 0x814, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_COMMAND \
		CTL_CODE(ASTROPCI_DEVICE, 0x815, METHOD_BUFFERED, FILE_ANY_ACCESS)

	#define ASTROPCI_MEM_UNMAP \
		CTL_CODE(ASTROPCI_DEVICE, 0x816, METHOD_BUFFERED, FILE_ANY_ACCESS)
#else
	#define ASTROPCI_GET_HCTR			0x1
	#define ASTROPCI_GET_PROGRESS		0x2
	#define ASTROPCI_GET_DMA_ADDR		0x3
	#define ASTROPCI_GET_HSTR			0x4
	#define ASTROPCI_HCVR_DATA			0x10
	#define ASTROPCI_SET_HCTR			0x11
	#define ASTROPCI_SET_HCVR			0x12
	#define ASTROPCI_PCI_DOWNLOAD		0x13
	#define ASTROPCI_PCI_DOWNLOAD_WAIT	0x14
	#define ASTROPCI_COMMAND			0x15
#endif

/******************************************************************************
*	Function Prototypes
******************************************************************************/
int getHstr(HANDLE pci_fd);
int hcvr(HANDLE pci_fd, int command, int data, int expected_reply);
int hcvr2(HANDLE pci_fd, int command, int data1, int data2, int expected_reply);
int doCommand(HANDLE pci_fd, int board_id, int command, int expected_reply);
int doCommand1(HANDLE pci_fd, int board_id, int command, int arg1, int expected_reply);
int doCommand2(HANDLE pci_fd, int board_id, int command, int arg1, int arg2, int expected_reply);
int doCommand3(HANDLE pci_fd, int board_id, int command, int arg1, int arg2, int arg3, int expected_reply);
int doCommand4(HANDLE pci_fd, int board_id, int command, int arg1, int arg2, int arg3, int arg4, int expected_reply);
int getError();
int check_expected_reply(int reply, int expected_reply);
int check_standard_reply(int reply);

#endif
