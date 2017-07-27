#include "camera.h"
#include <stdio.h>
main()
{
        WORD wPixelBuffer[1024];
        WORD wNumPixels = 192;
        WORD wOffset = 0;
        BOOL bLowResMode = FALSE;
        WORD xBinFactor = 1;
        WORD yBinFactor = 1;
        WORD numLines = 165;
        ULONG exposureLength = 100;
        WORD antiBloomInterval = 0;
        register WORD loop;

        //Initialization of Camera System

  printf("CameraInitialize\n");
        CameraInitialize( 0x320 );

  printf("CameraCooler\n");
        CameraCooler( TRUE );

  printf("CameraClearCCD\n");
        CameraClearCCD();

        //Expose Camera

  printf("CameraOpenShutter\n");
        CameraOpenShutter();
  printf("CameraBeginIntegrate\n");
        CameraBeginIntegrate();
  printf("CameraExpose\n");
        CameraExpose( exposureLength, antiBloomInterval, 0);
  printf("CameraEndIntegrate\n");
        CameraEndIntegrate();
  printf("CameraCloseShutter\n");
        CameraCloseShutter();

        //Digitize Frame
        for(loop=0;loop<numLines;loop++)
        {
  printf("CameraDigitizeLine: %d\n", loop, wNumPixels);
                CameraDigitizeLine(     &wPixelBuffer[0],
                                        wNumPixels,wOffset,
                                        bLowResMode,
                                        xBinFactor,yBinFactor   );
        }
        CameraCooler(FALSE);
}
