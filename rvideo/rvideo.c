#undef nohardware
#define DEBUG
//#define QUICKCAM4000PRO
/* Program to grab images from a video camera using the Linux BTTV driver
   and to display them in rapid succession on an X server using the xvista
   display tools. This allows for remote operation, although not at speeds
   approaching the rates available on a local host using direct mapping to
   the display. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev.h>
#ifdef QUICKCAM4000PRO
#include "pwc-ioctl.h"
#endif

//#ifndef nohardware
///*#include "/usr/src/linux/bttv/driver/videodev.h"
//#include "/usr/src/linux/2.4/bttv/driver/bttv.h"*/
//#include "/usr/src/linux-2.4/drivers/media/video/bttv.h"
//#endif
void yuvrgb24(unsigned char *buf, unsigned char *rgb, int RTjpeg_width, int RTjpeg_height);

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>

Display *dpy;
extern int lgtstatus[4];
extern int lgtenable;
int update = 0;
int bin = 1;
int snap = 1;
int old = 0;
int nintegrate = 1;

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#ifdef QUICKCAM4000PRO
#define NROW 480
#define NCOL 640
#define NCAM 1
#else
#define NROW 236
#define NCOL 320
#define NCAM 1
#endif

#define DISPLAY
#ifdef DISPLAY
void inittv();
extern int tvinit;
double dspan, dzero;
int nr, nc;
int zero = 0;
int one = 1;
int i;
int status;
#endif

int imtvnum_ = 0;
int jcam = 0;
int newcam = 0;

main(argc, argv)
int argc;
char *argv[];
{
  int tv[NCAM],out,iframe=0;
#ifndef nohardware
  struct video_mbuf mbuf[NCAM];
  struct video_mmap gb[NCAM];
  struct video_channel vc;
  struct video_capability vcap;
  struct video_picture vp;
#ifdef QUICKCAM4000PRO
  struct pwc_probe probe;
#endif
#endif
  unsigned char *tm[NCAM], *p, *rgb;
  int ii,i,j,msize, offset, ncol[NCAM], nrow[NCAM], ncam = NCAM;
  int icam, shut;
  float *tm2, *add;
  char string[20], *mx11gets_();
  FILE *fp;
  float *fdata, amin, amax;
  int lgtsav[4];

  if (argc<2) 
    jcam = 0;
  else
    sscanf(argv[1],"%d",&jcam);

 while (1) {
  for (icam=0; icam<ncam; icam++) {
    ncol[icam] = NCOL;
    nrow[icam] = NROW;
#ifndef nohardware
  /* Open the video device */
    sprintf(string,"/dev/video%1d",jcam);
    if ((tv[icam] = open(string,O_RDWR)) < 0) {
      perror("open error:");
      printf("Cannot open %s\n",string);
      exit(1);
    }

// Get device capabilities
    if (ioctl(tv[icam], VIDIOCGCAP, &vcap))
      perror("capability");
#ifdef DEBUG
    fprintf(stderr,"name: %s\n"
                 "type: %d\n"
                 "channels: %d\n"
                 "audios: %d\n"
                 "maxwidth: %d\n"
                 "maxheight: %d\n"
                 "minwidth: %d\n"
                 "minheight: %d\n",
         vcap.name, vcap.type, vcap.channels, vcap.audios, vcap.maxwidth, 
         vcap.maxheight, vcap.minwidth, vcap.minheight);
#endif

#ifdef QUICKCAM4000PRO
    if (ioctl(tv[icam], VIDIOCPWCPROBE, &probe) == 0) {
#ifdef DEBUG
       fprintf(stderr,"vcap.name: %s  probe.name: %s \n",vcap.name, probe.name);
#endif
    } else
       fprintf(stderr,"VIDIOCPWCPROBE failed\n");
#endif

    if (ioctl(tv[icam], VIDIOCGPICT, &vp))
      perror("capability");
#ifdef DEBUG
    fprintf(stderr,"depth: %d\n palette: %d\n",vp.depth,vp.palette);
#endif

    #ifdef QUICKCAM4000PRO
    ncol[icam] = vcap.maxwidth;
    nrow[icam] = vcap.maxheight;
    ncol[icam] = 320;
    nrow[icam] = 240;
    if (ncol[icam]!=ncol[0]||nrow[icam]!=nrow[0]) {
      fprintf(stderr,"All cameras must be the same size!\n");
      exit(1);
    }
/*
    fprintf(stderr,"Enter shutter value: ");
    scanf("%d",&shut);
    if (ioctl(tv[icam],  VIDIOCPWCSSHUTTER, &shut))
      perror("shutter");
    fprintf(stderr,"Enter AGC value: ");
    scanf("%d",&shut);
    if (ioctl(tv[icam],   VIDIOCPWCSAGC, &shut))
      perror("AGC");
*/
    #endif
    /* Allocate space for grey image and rgb image if necessary */
    if (vp.palette == VIDEO_PALETTE_YUV420P) 
       rgb = malloc(ncol[icam]*nrow[icam]*3*sizeof(char));

    /* Map memory for image */
    msize = ncol[icam]*nrow[icam]*3;
    mbuf[icam].size = msize;
    mbuf[icam].frames = 1;
    mbuf[icam].offsets[0] = 0;
    msize=462848;
    if (ioctl(tv[icam], VIDIOCGMBUF, &mbuf[icam])) perror("CGMBUF");
    tm[icam] = (unsigned char *) mmap(0,msize,PROT_READ|PROT_WRITE, MAP_SHARED,tv[icam],0);
    if (tm[icam]==(unsigned char *)-1)  {
      printf("Cannot mmap memory for snapshot\n");
      close(tv[icam]);
      exit(1);
    }

    /* Set proper channel and mode: */
  #ifdef QUICKCAM4000PRO
    vc.channel = 0;
  #else
    vc.channel = 1;
  #endif
    vc.norm = 1;
    if(ioctl(tv[icam], VIDIOCSCHAN, &vc))
        perror("change tuner");

    gb[icam].width = ncol[icam];
    gb[icam].height = nrow[icam];
    gb[icam].frame = 0;
    if (vp.palette)
      gb[icam].format=vp.palette ;
    else
      gb[icam].format=VIDEO_PALETTE_RGB24 ;
#else
    msize = ncol[icam]*nrow[icam]*3;
    tm[icam] = (unsigned char *)malloc(msize);
    p = tm[icam];
    for (j=0;j<nrow[icam];j++) {
       for (i=0;i<ncol[icam];i++) {
        *p++ = (icam/2)*127+i*127/ncol[icam];
        *p++ = (icam/2)*127+i*127/ncol[icam];
        *p++ = (icam/2)*127+i*127/ncol[icam];
       }
    }
#endif
    //lgtstatus[icam] = icam+1;
  }

/* Allocate memory for display image */

  tm2 = (float *)malloc(ncol[0]*nrow[0]*ncam*sizeof(float));
  nc = (ncam>1 ? 2*ncol[0]/bin : ncol[0]/bin);
  nr = nrow[0]/bin*(1+((ncam-1)/2));
  amin = 0;
  amax = 255*3;
  ii=1;
  newcam=0;
  while (newcam==0) {

    /* update can be toggled by hitting u in the display window */
    /* Check for event in the X window. */

    if (tvinit==1) mx11gets_(string,10);
    for (i=0;i<4;i++)
      if (lgtstatus[i] != lgtsav[i] && lgtstatus[i]>0) {
        newcam=1;
        if (i==0) jcam=0;
        if (i==1) jcam=2;
        if (i==2) jcam=1;
      }

    /* Check for command files */
    fp = fopen("/home/export/rvideo/start","r");
    if (fp != NULL) {
      fclose(fp);
      remove("/home/export/rvideo/start");
      update = 1;
    }
    fp = fopen("/home/export/rvideo/stop","r");
    if (fp != NULL) {
      fclose(fp);
      remove("/home/export/rvideo/stop");
      update = 0;
    }
    fp = fopen("/home/export/rvideo/quit","r");
    if (fp != NULL) {
      fclose(fp);
      remove("/home/export/rvideo/quit");
      for (i=0;i<ncam;i++) close(tv[icam]);
      exit(1);
    }
    fp = fopen("/home/export/rvideo/snap","r");
    if (fp != NULL) {
      fclose(fp);
      remove("/home/export/rvideo/snap");
      snap = 1;
    }

    if (update || snap) {
#ifndef nohardware
//      for (icam=0; icam<ncam; icam++) {
//        if (snap || lgtstatus[icam])
//fprintf(stderr,"CMCAPTURE %d\n",icam);
//          if (ioctl(tv[icam], VIDIOCMCAPTURE, &gb[icam])) perror("capture");
//      }
#endif

      for (icam=0; icam<ncam; icam++) {
        if (update || snap || lgtstatus[icam]>0) {
          if (ioctl(tv[icam], VIDIOCMCAPTURE, &gb[icam])) perror("capture");
#ifndef nohardware      
        if (ioctl(tv[icam], VIDIOCSYNC, &gb[icam].frame)) {
          perror("csync");
          /* Reopen the video device */
          close(tv[icam]);
          sprintf(string,"/dev/video%1d",icam);
          if ((tv[icam] = open(string,O_RDWR)) < 0) {
            perror("open error:");
            printf("Cannot open %s\n",string);
            exit(1);
          }
        } else {
#endif

          /* Load image into floating array by summing up all three colors. 
             Sample image every "bin" pixels. */
#ifndef nohardware
          if (vp.palette == VIDEO_PALETTE_YUV420P) {
            yuvrgb24(tm[icam],rgb,ncol[0],nrow[0]);
            p = rgb;
          } else
#endif
            p = tm[icam];

	  amax=0;
	  amin=1e10;
          for (j=0;j<nrow[0]/bin;j++) {
            for (i=0;i<ncol[0]/bin;i++) { 
              add = tm2+icam%2*ncol[0]+icam/2*nrow[0]*ncol[0]+j*nc+i;

              if (iframe==0) {
                *add = *p + *(p+1) + *(p+2);
              } else {
                *add += *p + *(p+1) + *(p+2);
              }
              if (j>10&&i>10&&j<nrow[0]/bin-10&&i<ncol[0]/bin-10) {
                amax = *add > amax ? *add : amax;
                amin = *add < amin ? *add : amin;
              }
              p+=3*bin;
            }
            p+=(bin-1)*ncol[0]*3;
          }
//fprintf(stderr,"iframe : %d  amax: %f amin: %f\n",iframe, amax, amin);
          
          //amax = ++iframe*255*3;
          if (++iframe >= nintegrate) iframe=0;
#ifndef nohardware
        }
#endif
        }
      }
  
      /*  Write image to image file if desired */
      /*
      out = open("test.img",O_CREAT|O_WRONLY|O_TRUNC);
      write(out,tm2,nrow*ncol*2);
      */

      /* Initialize display if not already */
      if (tvinit==0) {
         inittv();
         setmessage(1,0,"CAM 1");
         setmessage(1,1,"CAM 1");
         setmessage(3,0,"CAM 2");
         setmessage(3,1,"CAM 2");
         setmessage(2,0,"CAM 3");
         setmessage(2,1,"CAM 3");
         setmessage(4,0,"N/A");
         setmessage(4,1,"N/A");
/*
         if (ncam>1) {
           setmessage(3,0,"CAM 2");
           setmessage(3,1,"CAM 2");
         } else {
           setmessage(3,0,"N/A");
           setmessage(3,1,"N/A");
         }
         if (ncam>2) {
           setmessage(2,0,"CAM 3");
           setmessage(2,1,"CAM 3");
         } else {
           setmessage(2,0,"N/A");
           setmessage(2,1,"N/A");
         }
         if (ncam>3) {
           setmessage(4,0,"CAM 4");
           setmessage(4,1,"CAM 4");
         } else {
           setmessage(4,0,"N/A");
           setmessage(4,1,"N/A");
         }
*/
      }
  
      /* Use full scale display */
      dspan = amax-amin;
      dzero = amin;
      /* Display image on X display */
      if (tvinit==1 && iframe== 0 && (update==1 || snap==1)) {
         fdata = (float *)(tm2);
         tvload(fdata,nr,nc,nc,0,0,1,1,dspan,dzero,0,0,0);
         for (i=0;i<4;i++) {
           lgtstatus[i] = -1*(i+1);
           if (jcam==0) lgtstatus[0] = 1;
           if (jcam==1) lgtstatus[2] = 3;
           if (jcam==2) lgtstatus[1] = 2;
           lgtsav[i] = lgtstatus[i];
           if (lgtstatus[i]>0) 
             lights(i+1);
           else
             lights(-1*(i+1));
         }
         if (ncam>1) {
           imagerelocate(ncol[0],1);
	   imagedraw(ncol[0],nr,1);
           imagerelocate(1,nrow[0]);
	   imagedraw(nc,nrow[0],1);
         }
         old = 1;
         tvimnum("IMAGE",ii);
         //XSync(dpy,0);
         ii++;
      }
  
      snap = 0;
    }
  }
 close(tv[0]);
 munmap(tm[0],msize);
 free(tm2);
 lgtstatus[jcam+1] = -1;
 snap = 1;
 }
}
extern int fd_for_X;
int xtv_refresh();

void inittv()
{
#ifdef DISPLAY
  short r[256], g[256], b[256];
  int nrd = NROW/2;
  int ncd = NCOL/2;
  int ncold = 16, zf=0, yup=0;
  int xoff = 560;
  int yoff = 110;
  int ierr;
#ifdef QUICKCAM4000PRO
  char *resourcename="rvideo2";
  char *windowname="1m video(2)";
#else
  char *resourcename="rvideo";
  char *windowname="1m video";
#endif
#endif
  int toggle(), in(), out(), dosnap(), quit(), integrate(), camchange();

  ierr = imageinit(&ncd,&nrd,&ncold,zf,yup,resourcename,windowname,xoff,yoff);
  lgtenable=1;
  if (ierr == 0) {
    r[0] = b[0] = 255;
    g[0] = 0;
    imagepalette(1,r,g,b,1);
    for (i=0;i<256;i++)
      r[i] = g[i] = b[i] = i;
    imagepalette(255,r,g,b,0);
    imageinstallkey('q',0,quit);
    imageinstallkey('Q',0,quit);
    imageinstallkey('u',0,toggle);
    imageinstallkey('U',0,toggle);
    imageinstallkey('s',0,dosnap);
    imageinstallkey('S',0,dosnap);
    imageinstallkey('i',0,in);
    imageinstallkey('I',0,in);
    imageinstallkey('o',0,out);
    imageinstallkey('O',0,out);
    for(i=0;i<10;i++) imageinstallkey('0'+i,0,camchange);
/* Install '0' - '9' for integration update vista variables */
 //   for(i=0;i<10;i++) imageinstallkey('0'+i,0,integrate);
 
    tvinit = 1;
    mx11register_(&fd_for_X,xtv_refresh);

  } else
    fprintf(stderr,"Could not open display!!");

}

toggle()
{
  update = !update;
}
dosnap()
{
  snap = !snap;
}
quit()
{
  exit(1);
}
in()
{
  bin = (bin*2 > 8 ? 8 : bin*2);
  old = 0;
}
out()
{
  bin = (bin/2 < 1 ? 1 : bin/2);
  old = 0;
}
camchange(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  jcam = key - '1';
  newcam=1;
//fprintf(stderr,"integrate: %d\n",nintegrate);
}

integrate(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  nintegrate = key - '0';
  nintegrate *= 3;
//fprintf(stderr,"integrate: %d\n",nintegrate);
}


#define KcrR 76284
#define KcrG 53281
#define KcbG 25625
#define KcbB 132252
#define Ky 76284


void yuvrgb24(unsigned char *buf, unsigned char *rgb, int RTjpeg_width, int RTjpeg_height)
{
 int tmp;
 int i, j;
 int y, crR, crG, cbG, cbB;
 unsigned char *bufcr, *bufcb, *bufy, *bufoute, *bufouto;
 int oskip, yskip;
 
 oskip=RTjpeg_width*3;
 yskip=RTjpeg_width;
 
 bufcb=&buf[RTjpeg_width*RTjpeg_height];
 bufcr=&buf[RTjpeg_width*RTjpeg_height+(RTjpeg_width*RTjpeg_height)/4];
 bufy=&buf[0];
 bufoute=rgb;
 bufouto=rgb+oskip;
 
 for(i=0; i<(RTjpeg_height>>1); i++)
 {
  for(j=0; j<RTjpeg_width; j+=2)
  {
   crR=(*bufcr-128)*KcrR;
   crG=(*(bufcr++)-128)*KcrG;
   cbG=(*bufcb-128)*KcbG;
   cbB=(*(bufcb++)-128)*KcbB;
  
   y=(bufy[j]-16)*Ky;
   
   tmp=(y+cbB)>>16;
   *(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y-crG-cbG)>>16;
   *(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y+crR)>>16;
   *(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

   y=(bufy[j+1]-16)*Ky;

   tmp=(y+cbB)>>16;
   *(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y-crG-cbG)>>16;
   *(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y+crR)>>16;
   *(bufoute++)=(tmp>255)?255:((tmp<0)?0:tmp);

   y=(bufy[j+yskip]-16)*Ky;

   tmp=(y+cbB)>>16;
   *(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y-crG-cbG)>>16;
   *(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y+crR)>>16;
   *(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);

   y=(bufy[j+1+yskip]-16)*Ky;

   tmp=(y+cbB)>>16;
   *(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y-crG-cbG)>>16;
   *(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);
   tmp=(y+crR)>>16;
   *(bufouto++)=(tmp>255)?255:((tmp<0)?0:tmp);
   
  }
  bufoute+=oskip;
  bufouto+=oskip;
  bufy+=yskip<<1;
 }
}

