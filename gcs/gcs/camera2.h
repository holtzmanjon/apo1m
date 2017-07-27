#ifndef __CAMERA_H    // prevent multiple includes
#define __CAMERA_H

//#define OLDTEMP      // comment this out if new temperature readout

#ifdef   _MSC_VER
#define inportb inp
#define outportb  outp
#define	asm		__asm
#endif  // Microsoft C Compatibility

#ifdef __cplusplus
#define EXTERN_COMPAT extern "C"
#else
#define EXTERN_COMPAT extern
#endif

#ifdef _INC_WINDOWS		// Microsoft Windows header define
#define	__WINDOWS_H
#endif

#ifndef __WINDOWS_H     // __WINDOWS_H is a Windows define, so if it's not 
						// defined, then WINDOWS.H has not been included.

#define NULL  0
#define FALSE  0
#define TRUE  1
#define LONG  long
#define FAR   far

typedef unsigned int  HANDLE;
typedef unsigned long  ULONG;
typedef int     BOOL;
typedef unsigned int  WORD;
typedef char FAR*   LPSTR;

#endif

#define STAT_COOLER   0x0001
#define STAT_SHUTTER  0x0002
#define STAT_ANTIBLOOM  0x0004
#define STAT_FRAMETRANSFER 0x0008
#define STAT_NOCAMERA  0x0010
#define STAT_VIDEO   0x0020

#define PAL_TIMING   1
#define NTSC_TIMING   2

#define DEFAULT_BASE_PORT  0x140

typedef unsigned long ULONG;

typedef struct                // Camera information structure
	{
		BOOL    bCooler;          // camera has cooler?
		BOOL    bAntibloom;       // camera has anti-bloom?
		BOOL    bGain;            // TRUE if camera supports programmable gain
		BOOL    bMpp;             // TRUE if camera supports MPP
		BOOL    bSpeed;           // TRUE if camera supports Speed selection
		BOOL    bBinning;         // TRUE if camera supports binning
		BOOL    bReverseParallel; // TRUE if reverse parallel ops supported
		BOOL    bFrameTransfer;   // TRUE if a Frame Transfer Camera

		unsigned  CCDReadTime;    // time (in ms) to read CCD
		unsigned  BitsPerColor;   // bits per color
		unsigned  NumberOfColors; // number of colors
		unsigned  XSize;          // X extent of CCD
		unsigned  YSize;          // Y extent of CCD
		unsigned  HiResBPP;       // Hi-res bits=per-pixel
		unsigned  LoResBPP;       // Lo-res bits=per-pixel

		int     iVideoWidth;      // Camera video width
		int     iVideoHeight;     // Camera video height

		BOOL    bTempReadout;     // Is there a temperature readout?
		int     driverVer;        // Version Number of Driver
	} CameraType;

#ifdef  DLL
#define PORT_DECLARE  _export FAR PASCAL
#else
#define PORT_DECLARE
#endif

/***
	* Backward Compatibility
	***/
#define  CameraMPP(bsetting)   CameraGain(bsetting)

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL   bHardware;
extern WORD   wGlobalLineNumber;
extern char * szCameraName;

BOOL     PORT_DECLARE CameraInitialize( WORD wPort);
void     PORT_DECLARE CameraAntibloom(  BOOL bAntibloomEnable);
void     PORT_DECLARE CameraFrameTransferMode(  BOOL bTransferTransferMode);
void     PORT_DECLARE CameraGetEquipment( CameraType *Camera);
unsigned PORT_DECLARE CameraGetXExtent( void);
unsigned PORT_DECLARE CameraGetYExtent( void);
ULONG    PORT_DECLARE CameraGetStatus(  void);
void     PORT_DECLARE CameraOpenShutter(  void);
void     PORT_DECLARE CameraCloseShutter( void);
void     PORT_DECLARE CameraBeginIntegrate( void);
BOOL     PORT_DECLARE CameraDetect( void );
BOOL     PORT_DECLARE CameraExpose( LONG lmSecs, WORD wAntibloomInterval, WORD wLookupInterval );
void     PORT_DECLARE CameraClockABG( void );
void     PORT_DECLARE CameraEndIntegrate( void );
void     PORT_DECLARE CameraCooler( BOOL bOn );
void     PORT_DECLARE CameraGain( BOOL bSetting );
void     PORT_DECLARE CameraSpeed( BOOL bSetting );
void     PORT_DECLARE CameraClearCCD( void );
BOOL     PORT_DECLARE CameraSetPort(  unsigned int Port );
void     PORT_DECLARE CameraDigitizeLine( WORD far *Line, WORD wNumPixels, WORD wOffset, 
					BOOL bLowResMode, unsigned xBinFactor, unsigned yBinFactor );
//void   PORT_DECLARE CameraDigSubLineLoRes(  WORD far *Line, WORD wNumPixels );
//void   PORT_DECLARE CameraDigSubLineHiRes(  WORD far *Line, WORD wNumPixels );
void     PORT_DECLARE CameraResolution( WORD wRes );

void     PORT_DECLARE CameraDigSubLineHiResBinning(WORD far *Line, WORD wNumPixels, 
					WORD wOffset, WORD xBinFactor);
void     PORT_DECLARE CCDFrame(WORD far *Line, WORD wNumLines, WORD wNumPixels, WORD wOffset, WORD xBinFactor);
void     PORT_DECLARE CameraCCDParallel(  WORD wCount );
void     PORT_DECLARE CameraCCDParallelReverse( WORD wCount );
void     PORT_DECLARE CameraFrameTransferParallel(  WORD wCount );
void     PORT_DECLARE CameraSkipPixels( WORD wCount );
void     PORT_DECLARE CameraWriteLatch( WORD wLatch );
WORD     PORT_DECLARE CameraReadLatch(  void );
void     PORT_DECLARE CameraVideoWrite( int x, int y, WORD FAR *lpwPixelBuffer, WORD wNumPixels, int iBitsPerPixel );
void     PORT_DECLARE CameraVideoRead(  int x, int y, WORD FAR *lpwPixelBuffer, WORD wNumPixels);
void     PORT_DECLARE CameraVideoClear( void );
void     PORT_DECLARE CameraVideoInitialize(  int iTiming);
void     PORT_DECLARE CameraParallelSpd(  int delay );
void     PORT_DECLARE CameraSetModeWord(  WORD newMode );
WORD     PORT_DECLARE CameraGetModeWord(  void );
float    PORT_DECLARE CameraGetTemp(void);
BOOL     PORT_DECLARE CameraDelayMs(WORD wDelay);
void     PORT_DECLARE CameraSetOpenDelay(int delay);
void     PORT_DECLARE CameraSetCloseDelay(int delay);
void     PORT_DECLARE CameraSetParallelPix(int pixels);
void     PORT_DECLARE CCDResetXfer(void);
void     PORT_DECLARE CCDPixelXfer(void);
void     PORT_DECLARE CCDWaitForSH(void);

#ifdef __cplusplus
}
#endif

#endif
