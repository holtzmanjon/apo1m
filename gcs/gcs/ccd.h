//#define BIGCCD
#define NHEADER 2880

#ifdef BIGCCD
#include "camera2.h"
#define BASE 0x120
#else
#include "camera.h"
#define BASE 0x320
#endif

void ccd_initialize();
int ccd_expose(WORD **, unsigned int, BOOL, int, int, int, int, int);
int ccd_write(WORD **, unsigned int,int, int, int, int, int);
int guide(WORD **buf,double x0,double y0, int size,unsigned int exptime,
  int update, double ax, double bx, double ay, double by, 
  float *peak, float *tot);
int findcent(WORD **, float *, float *, float *, float *, int);
int printarr(WORD **, float *, float *);
int ccd_cooler(BOOL);
int ccd_shutter(BOOL);


