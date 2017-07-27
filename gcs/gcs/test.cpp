#include "camera2.h"
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
main()
{
  WORD wPixelBuffer[512];
  WORD wNumPixels = 512;
  WORD wOffset = 0;
  BOOL bLowResMode = FALSE;
  WORD xBinFactor = 1;
  WORD yBinFactor = 1;
  WORD numLines = 512;
  ULONG exposureLength = 100;
  WORD antiBloomInterval = 0;
  register WORD loop;
  char header[2880];
  int outfile,ifile;
  char file[64];
  void fitshead(char *);
  void packfit(unsigned int *, unsigned int *, int);

  CameraInitialize (0x120);
  CameraCooler(TRUE);

  for (ifile=0; ifile<30; ifile++) {
    CameraClearCCD();
    CameraOpenShutter();
    CameraBeginIntegrate();
    CameraExpose(exposureLength,antiBloomInterval,0);
    CameraEndIntegrate();
    CameraCloseShutter();

    fitshead(header);
    sprintf(file,"e:\\spec\temp%3.3d.fits",ifile);
    outfile = open(file,O_RDWR|O_BINARY|O_CREAT,S_IWRITE|S_IREAD);
    write(outfile,header,2880);

    for (loop=0; loop<numLines; loop++)
    {
      asm cli;
      CameraDigitizeLine(&wPixelBuffer[0],wNumPixels,wOffset,bLowResMode,
                         xBinFactor, yBinFactor);
      asm sti;
      packfit(wPixelBuffer,wPixelBuffer,wNumPixels*2);
      write(outfile,wPixelBuffer,wNumPixels*2);
    }
    close(outfile);

  }
  CameraCooler(FALSE);
}

void fitshead(char *header)
{

  void inheadset(char *, int, char *);
  void fheadset(char *, double, char *);
  void lheadset(char *, int, char *);
  void cheadset(char *, char *, char *);

    memset(header,(int)' ',2880);
    sprintf(header,"END ");
    header[4] = ' ';
    header[2779] = '\0';
    lheadset("SIMPLE",TRUE,header);
    inheadset("BITPIX",16,header);
    inheadset("NAXIS",2,header);
    inheadset("NAXIS1",512,header);
    inheadset("NAXIS2",512,header);
    inheadset("CRVAL1",1,header);
    inheadset("CRVAL2",1,header);
    inheadset("CDELT1",1,header);
    inheadset("CDELT2",1,header);
}
