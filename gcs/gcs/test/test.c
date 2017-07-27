#include <stdio.h>
#include "camera.h"
#ifdef _MSC_VER
#define asm       _asm
#define  inport   _inpw
#define inportb   _inp
#define outport   _outpw
#define outportb  _outp
#endif


WORD PixelArray[1024];

int YOffset, YSize;
int XOffset, XSize;
long ExposureTime;
int AntibloomInterval;

FILE *f;

void SampleDigitize(void);

void SampleDigitize(void)
{
	register int y;
	register int d;
	register char *p;

	printf("\nExposing...");
	CameraClearCCD();
	CameraOpenShutter();
	CameraBeginIntegrate();
	CameraExpose(ExposureTime, AntibloomInterval, 0);
	CameraCloseShutter();
	CameraEndIntegrate();

	printf("\nExposure done!\n");

	CameraCCDParallel(YOffset);                                                                     // Skip lines

	printf("\nDigitizing...\n");

	for (y=0; y < YSize; y++)

	{
		CameraDigitizeLine(     (WORD far *)PixelArray, 
					XSize,
					XOffset,
					FALSE,
					0,0    );
							// grab a line
	

	// convention is non-standard; FITS is reversed byte order       6/15/93 wb
	p = (char *)&PixelArray[0];                      // point at buffer
	for( d=0; d< XSize; d++ )
		{
			fwrite( p+1, sizeof(char), 1, f );   // hi-byte
			fwrite( p, sizeof(char), 1, f );     // low-byte
			p += 2;                                  // next int 
		}
	printf("\nLine %d:\r", y);

	}
}


void main(void)
{
	printf("Initializing camera...");

	CameraInitialize(0x120);
	CameraCooler(FALSE);      // TRUE here for cooler ON

	printf("done!\n");

	YOffset = 0;        // >0 = skip YOffset lines
	XOffset = 0;        // >0 = skip XOffset columns
	YSize = 165;        // 165 lines, as an example
						// < 1024 = subFrame of YSize lines
	XSize = 204;        // 204 pixels/line as an example
						// < 1024 = subFrame of XSize columns

	printf("\nExposure Length: "); 
	scanf("%i", &ExposureTime );
	AntibloomInterval = 0;      // do not clock ABG gate

	bHardware = TRUE;
	
	if( !( f = fopen("DATA.raw", "wb" )) )
		{
			printf("\n\nFILE OPEN ERROR. EXITING.\n");
			return;
		}
	
	SampleDigitize();
//  fclose( f );
}

/* at this point, the file DATA.RAW contains the pixel data, in the correct
	 byte order, to be used.  By adding a FITS header, the data can be read
	 into the standard SpectraSource Application, viewed and manipulated. */


