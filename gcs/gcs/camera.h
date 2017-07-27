#ifndef __CAMERA_H    // prevent multiple includes
#define __CAMERA_H

//#define OLDTEMP      // comment this out if new temperature readout

#ifdef   _MSC_VER
#define inportb inp
#define outportb  outp
#endif  // Microsoft C Compatibility

#ifdef __cplusplus
#define EXTERN_COMPAT extern "C"
#else
#define EXTERN_COMPAT extern
#endif

#ifndef __WINDOWS_H     // __WINDOWS_H is a Windows define, so if it's not 
						// defined, then WINDOWS.H has not been included.

#define NULL          0
#define FALSE       0
#define TRUE          1
#define LONG          long
#define FAR         far

typedef unsigned int        HANDLE;
typedef unsigned long       ULONG;
typedef int                 BOOL;
typedef unsigned int          WORD;
typedef char FAR*           LPSTR;


#endif

#define STAT_COOLER         0x0001
#define STAT_SHUTTER          0x0002
#define STAT_ANTIBLOOM        0x0004
#define STAT_FRAMETRANSFER    0x0008
#define STAT_NOCAMERA       0x0010
#define STAT_VIDEO            0x0020

#define PAL_TIMING            1
#define NTSC_TIMING         2

#define DEFAULT_BASE_PORT      0x140

typedef unsigned long ULONG;

typedef struct                // Camera information structure
	{
		BOOL    bCooler;          // camera has cooler?
		BOOL    bAntibloom;       // camera has anti-bloom?
		BOOL    bGain;            // TRUE if camera supports programmable gain
		BOOL    bMpp;             // TRUE if camera supports MPP
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

/***
 * Backward Compatibility
	***/
#define       CameraMPP(bsetting)   CameraGain(bsetting)

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL   bHardware;
extern WORD   wGlobalLineNumber;
extern char * szCameraName;

BOOL          CameraInitialize( WORD wPort);
void          CameraAntibloom(  BOOL bAntibloomEnable);
void          CameraFrameTransferMode(  BOOL bTransferTransferMode);
void          CameraGetEquipment( CameraType *Camera);
unsigned      CameraGetXExtent( void);
unsigned      CameraGetYExtent( void);
ULONG       CameraGetStatus(  void);
void          CameraOpenShutter(  void);
void          CameraCloseShutter( void);
void        CameraBeginIntegrate( void);
BOOL          CameraDetect( void );
BOOL          CameraExpose( LONG lmSecs, 
														WORD wAntibloomInterval, 
														WORD wLookupInterval );
void          CameraClockABG( void );
void        CameraEndIntegrate( void );
void          CameraCooler( BOOL bOn );
void          CameraGain( BOOL bSetting );
void          CameraClearCCD( void );
BOOL          CameraSetPort(  unsigned int Port );
void        CameraDigitizeLine( WORD far *Line, 
																	WORD wNumPixels, 
																	WORD wOffset, 
																	BOOL bLowResMode, 
																	unsigned xBinFactor,
																	unsigned yBinFactor );
void          CameraDigSubLineLoRes(  WORD far *Line, 
																			WORD wNumPixels );
//void          CameraDigSubLineHiRes(  WORD far *Line, 
//                                    WORD wNumPixels );

//!!! Mark removed PORT_DECLARE from definition here
//   v-  was right here.
void /*PORT_DECLARE*/ CameraDigSubLineHiResBinning(WORD far *Line,
	 WORD wNumPixels,
	 WORD wOffset,
	 WORD xBinFactor);
void /*PORT_DECLARE*/ CCDFrame(WORD far *Line,
	WORD wNumLines,
	WORD wNumPixels,
	WORD wOffset,
	WORD xBinFactor);
void        CameraCCDParallel(  WORD wCount );
void          CameraCCDParallelReverse( WORD wCount );
void        CameraFrameTransferParallel(  WORD wCount );
void          CameraSkipPixels( WORD wCount );
void          CameraWriteLatch( WORD wLatch );
WORD          CameraReadLatch(  void );
void          CameraVideoWrite( int x, 
																int y, 
																WORD FAR *lpwPixelBuffer,
																WORD wNumPixels, 
																int iBitsPerPixel );
void          CameraVideoRead(  int x, 
																int y, 
																WORD FAR *lpwPixelBuffer,
																WORD wNumPixels);
void          CameraVideoClear( void );
void          CameraVideoInitialize(  int iTiming);
void          CameraParallelSpd(  int delay );
void        CameraSetModeWord(  WORD newMode );
WORD        CameraGetModeWord(  void );
float         CameraGetTemp(void);
BOOL        CameraDelayMs(WORD wDelay);
void          CameraSetOpenDelay(int delay);
void          CameraSetCloseDelay(int delay);
void          CameraSetParallelPix(int pixels);
void         CCDResetXfer(void);
void         CCDPixelXfer(void);
void         CCDWaitForSH(void);

#ifdef __cplusplus
}
#endif


#endif
