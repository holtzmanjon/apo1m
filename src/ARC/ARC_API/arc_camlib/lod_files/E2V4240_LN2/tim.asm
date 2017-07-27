       COMMENT *

This file is used to generate boot DSP code for the 250 MHz fiber optic
	timing board using a DSP56303 as its main processor. It supports 
	split serial and frame transfer, but not split parallel nor binning.
	*
	PAGE    132     ; Printronix page width - 132 columns

; Include the boot and header files so addressing is easy
	INCLUDE "timhdr.asm"
	INCLUDE	"timboot.asm"

	ORG	P:,P:

CC	EQU	CCDVIDREV5+TIMREV5+UTILREV3+TEMP_POLY+SPLIT_SERIAL+SUBARRAY+SHUTTER_CC

; Put number of words of application in P: for loading application from EEPROM
	DC	TIMBOOT_X_MEMORY-@LCV(L)-1

; Set software to IDLE mode
START_IDLE_CLOCKING
	MOVE	#IDLE,R0		; Exercise clocks when idling
	MOVE	R0,X:<IDL_ADR
	BSET	#IDLMODE,X:<STATUS	; Idle after readout
	JMP     <FINISH			; Need to send header and 'DON'

; Keep the CCD idling when not reading out
IDLE	DO      Y:<NSR,IDL1     	; Loop over number of pixels per line
	MOVE    #SERIAL_IDLE,R0 	; Serial transfer on pixel
	JSR     <CLOCK  		; Go to it
	MOVE	#COM_BUF,R3
	JSR	<GET_RCV		; Check for FO or SSI commands
	JCC	<NO_COM			; Continue IDLE if no commands received
	ENDDO
	JMP     <PRC_RCV		; Go process header and command
NO_COM	NOP
IDL1
	MOVE    #PARALLEL_CLEAR,R0	; Address of parallel clocking waveform
	JSR     <CLOCK  		; Go clock out the CCD charge
	JMP     <IDLE

;  *****************  Exposure and readout routines  *****************

; Overall loop - transfer and read NPR lines
RDCCD	BSET	#ST_RDC,X:<STATUS 	; Set status to reading out
	JSR	<PCI_READ_IMAGE		; Get the PCI board reading the image
	JSET	#TST_IMG,X:STATUS,SYNTHETIC_IMAGE

; Delay readout until the shutter has fully closed
	MOVE	Y:<SHDEL,A
	TST	A
	JLE	<S_DEL2
	MOVE	#100000,X0
	DO	A,S_DEL0		; Delay by Y:SHDEL milliseconds
	DO	X0,S_DEL1
	NOP
S_DEL1	NOP
S_DEL0	NOP

; Calculate some readout parameters
S_DEL2	MOVE	Y:<NBOXES,A		; NBOXES = 0 => full image readout
	TST	A
	JNE	<SUB_IMG
	MOVE	A1,Y:<NP_SKIP		; Zero these all out
	MOVE	A1,Y:<NS_SKP1
	MOVE	A1,Y:<NS_SKP2
	MOVE	Y:<NSR,A		; NSERIALS_READ = NSR
	JCLR	#SPLIT_S,X:STATUS,*+3
	ASR	A			; Split serials requires / 2
	NOP
	MOVE	A,Y:<NSERIALS_READ	; Number of columns in each subimage
	JMP	<WT_CLK

; Loop over the required number of subimage boxes
SUB_IMG	MOVE	#READ_TABLE,R7		; Parameter table for subimage readout
	DO	Y:<NBOXES,L_NBOXES	; Loop over number of boxes
	MOVE	Y:(R7)+,X0
	MOVE	X0,Y:<NP_SKIP
	MOVE	Y:(R7)+,X0
	MOVE	X0,Y:<NS_SKP1
	MOVE	Y:(R7)+,X0
	MOVE	X0,Y:<NS_SKP2
	MOVE	Y:<NS_READ,A
	JCLR	#SPLIT_S,X:STATUS,*+3	; Split serials require / 2
	ASR	A
	NOP
	MOVE	A,Y:<NSERIALS_READ	; Number of columns in each subimage

; Start the loop for parallel shifting desired number of lines
WT_CLK	JSR	<WAIT_TO_FINISH_CLOCKING

; Skip over the required number of rows for subimage readout
	MOVE	Y:<NP_SKIP,A		; Number of rows to skip
	TST	A
	JEQ	<CLR_SR
	DO      Y:<NP_SKIP,*+5	
	MOVE    #<PARALLEL,R0
	JSR     <CLOCK
	NOP

; Clear out the accumulated charge from the serial shift register 
CLR_SR	DO      Y:<NSCLR,*+5		; Loop over number of pixels to skip
	MOVE    #<SERIALS_CLEAR,R0      ; Address of serial skipping waveforms
	JSR     <CLOCK          	; Go clock out the CCD charge
	NOP                     	; Do loop restriction  

; Parallel shift the image into the serial shift register
	MOVE	Y:<NPR,A		; Number of rows set by host computer
	MOVE	Y:<NBOXES,B		; NBOXES = 0 => full image readout
	TST	B
	JEQ	*+2
	MOVE	Y:<NP_READ,A		; If NBOXES .NE. 0 use subimage table
	NOP

; This is the main loop over each line to be read out
	DO      A1,LPR			; Number of rows to read out

; Exercise the parallel clocks, including binning if needed
	MOVE    #<PARALLEL,R0
	JSR     <CLOCK

; Check for a command once per line. Only the ABORT command should be issued.
	MOVE	#COM_BUF,R3
	JSR	<GET_RCV		; Was a command received?
	JCC	<CONT_READING		; If no, continue reading out
	JMP	<PRC_RCV		; If yes, go process it

; Abort the readout currently underway
ABR_RDC	JCLR	#ST_RDC,X:<STATUS,ABORT_EXPOSURE
	ENDDO				; Properly terminate readout loop
	MOVE	Y:<NBOXES,A		; NBOXES = 0 => full image readout
	TST	A
	JEQ	*+2
	ENDDO				; Properly terminate readout loop
	JMP	<RDCCD_END_ABORT

; Skip over NS_SKP1 columns for subimage readout
CONT_READING
	MOVE	Y:<NS_SKP1,A		; Number of columns to skip
	TST	A
	JLE	<L_READ
	DO	Y:<NS_SKP1,*+5		; Number of waveform entries total
	MOVE	Y:<SERIAL_SKIP,R0	; Waveform table starting address
	JSR     <CLOCK  		; Go clock out the CCD charge
	NOP

; Finally read some real pixels
L_READ	DO	Y:<NSERIALS_READ,*+5
	MOVE	Y:<SERIAL_READ,R0	; Waveform table starting address
	JSR     <CLOCK  		; Go clock out the CCD charge
	NOP

; Skip over NS_SKP2 columns if needed for subimage readout
	MOVE	Y:<NS_SKP2,A		; Number of columns to skip
	TST	A
	JLE	<L_BIAS
	DO	Y:<NS_SKP2,*+5
	MOVE	Y:<SERIAL_SKIP,R0	; Waveform table starting address
	JSR     <CLOCK  		; Go clock out the CCD charge
	NOP

; And read the bias pixels if in subimage readout mode
L_BIAS	MOVE	Y:<NBOXES,A		; NBOXES = 0 => full image readout
	TST	A
	JEQ	<END_ROW
	MOVE	Y:<NR_BIAS,A		; NR_BIAS = 0 => no bias pixels
	TST	A
	JEQ	<END_ROW
	JCLR	#SPLIT_S,X:STATUS,*+3
	ASR	A			; Split serials require / 2
	NOP
	DO      A1,*+5			; Number of pixels to read out
	MOVE	Y:<SERIAL_READ,R0	; Waveform table starting address
	JSR     <CLOCK  		; Go clock out the CCD charg
	NOP
END_ROW	NOP
LPR	NOP				; End of parallel loop
L_NBOXES NOP				; End of subimage boxes loop

; Restore the controller to non-image data transfer and idling if necessary
RDC_END	JCLR	#IDLMODE,X:<STATUS,NO_IDL ; Don't idle after readout
	MOVE	#IDLE,R0
	MOVE	R0,X:<IDL_ADR
	JMP	<RDC_E
NO_IDL	MOVE	#TST_RCV,R0
	MOVE	R0,X:<IDL_ADR
RDC_E	JSR	<WAIT_TO_FINISH_CLOCKING
	BCLR	#ST_RDC,X:<STATUS	; Set status to not reading out
        JMP     <START

; ******  Include many routines not directly needed for readout  *******
	INCLUDE "timCCDmisc.asm"


TIMBOOT_X_MEMORY	EQU	@LCV(L)

;  ****************  Setup memory tables in X: space ********************

; Define the address in P: space where the table of constants begins

	IF	@SCP("DOWNLOAD","HOST")
	ORG     X:END_COMMAND_TABLE,X:END_COMMAND_TABLE
	ENDIF

	IF	@SCP("DOWNLOAD","ROM")
	ORG     X:END_COMMAND_TABLE,P:
	ENDIF

; Application commands
	DC	'PON',POWER_ON
	DC	'POF',POWER_OFF
	DC	'SBV',SET_BIAS_VOLTAGES
	DC	'IDL',START_IDLE_CLOCKING
	DC	'OSH',OPEN_SHUTTER
	DC	'CSH',CLOSE_SHUTTER
	DC	'RDC',RDCCD 			; Begin CCD readout    
	DC	'CLR',CLEAR  			; Fast clear the CCD   

; Exposure and readout control routines
	DC	'SET',SET_EXPOSURE_TIME
	DC	'RET',READ_EXPOSURE_TIME
	DC	'SEX',START_EXPOSURE
	DC	'PEX',PAUSE_EXPOSURE
	DC	'REX',RESUME_EXPOSURE
	DC	'AEX',ABORT_EXPOSURE
	DC	'ABR',ABR_RDC
	DC	'CRD',CONTINUE_READING

; Support routines
	DC	'SGN',ST_GAIN      
	DC	'SDC',SET_DC
	DC	'SBN',SET_BIAS_NUMBER
	DC	'SMX',SET_MUX
	DC	'CSW',CLR_SWS
	DC	'SOS',SEL_OS
	DC	'SSS',SET_SUBARRAY_SIZES
	DC	'SSP',SET_SUBARRAY_POSITIONS
	DC	'RCC',READ_CONTROLLER_CONFIGURATION 

END_APPLICATON_COMMAND_TABLE	EQU	@LCV(L)

	IF	@SCP("DOWNLOAD","HOST")
NUM_COM			EQU	(@LCV(R)-COM_TBL_R)/2	; Number of boot + 
							;  application commands
EXPOSING		EQU	CHK_TIM			; Address if exposing
CONTINUE_READING	EQU	CONT_READING 		; Address if reading out
	ENDIF

	IF	@SCP("DOWNLOAD","ROM")
	ORG     Y:0,P:
	ENDIF

; Now let's go for the timing waveform tables
	IF	@SCP("DOWNLOAD","HOST")
        ORG     Y:0,Y:0
	ENDIF

GAIN	DC	END_APPLICATON_Y_MEMORY-@LCV(L)-1

NSR     DC      1200   	 	; Number Serial Read, prescan + image + bias
NPR     DC      0	     	; Number Parallel Read
NSCLR	DC      NS_CLR  	; To clear the serial register
NPCLR   DC      NP_CLR    	; To clear the parallel register 
NSBIN   DC      1       	; Serial binning parameter
NPBIN   DC      1       	; Parallel binning parameter
TST_DAT	DC	0		; Temporary definition for test images
SHDEL	DC	SH_DEL		; Delay in milliseconds between shutter closing 
				;   and image readout
CONFIG	DC	CC		; Controller configuration
NSERIALS_READ	DC	0	; Number of serials to read

; Readout peculiarity parameters. Default values are selected here.
SERIAL_SKIP 	DC	SERIAL_SKIP_SPLIT	; Serial skipping waveforms
SERIAL_READ	DC	SERIAL_READ_SPLIT	; Serial reading table

; These three parameters are read from the READ_TABLE when needed by the
;   RDCCD routine as it loops through the required number of boxes
NP_SKIP		DC	0	; Number of rows to skip
NS_SKP1		DC	0	; Number of serials to clear before read
NS_SKP2		DC	0	; Number of serials to clear after read

; Subimage readout parameters. Ten subimage boxes maximum.
NBOXES	DC	0		; Number of boxes to read
NR_BIAS	DC	0		; Number of bias pixels to read
NS_READ	DC	0		; Number of columns in subimage read
NP_READ	DC	0		; Number of rows in subimage read
READ_TABLE DC	0,0,0		; #1 = Number of rows to clear 
	DC	0,0,0		; #2 = Number of columns to skip before 
	DC	0,0,0		;   subimage read 
	DC	0,0,0		; #3 = Number of rows to clear after 
	DC	0,0,0		;   subimage clear
	DC	0,0,0
	DC	0,0,0
	DC	0,0,0
	DC	0,0,0
	DC	0,0,0

; Include the waveform table for the designated type of CCD
	INCLUDE "WAVEFORM_FILE" ; Readout and clocking waveform file

END_APPLICATON_Y_MEMORY	EQU	@LCV(L)

; End of program
	END

