#include <stdio.h>
#include <string.h>
#include <math.h>
#include "io.h"

#define R2D 180./3.14159
#define D2R 3.14159/180.

void guidepos(double, double, double, double, struct CCDSTATUS, double *, double *, double *, int);

#ifdef MAIN
FILE *infile[MAXDEPTH];
int idepth = 0;
int havepipe[MAXDEPTH];

main()
{
  double gx, gy, gpa, gmag, grad;
  void readusno();
  struct CCDSTATUS ccdinfo;

  readccdstatus("guidestatus.doc","guidestatus.fin",&ccdinfo);
  readusno("M67-1.usno",0,&ccdinfo,&gx,&gy,&gpa,&grad,&gmag);
}
#endif

void readusno(char *file, int nskip, double cx, double cy, struct CCDSTATUS *gccd)
{
  FILE *fp;
  double ra, dec, mag, gra, gdec, dra, ddec, tra, tdec;
  double x, y, pa;
  char line[80];
  char grah[6], gram[6], gras[6], gdecd[6], gdecm[6], gdecs[6];
  int ngood;

  sprintf(line,"xvista/%s",file);
  fp = fopen(line,"r");
  if (fp == NULL) {
    fprintf(stderr,"error opening file %s\n",file);
    return;
  }
  fgets(line,80,fp);
  sscanf(line+22,"%lf%lf",&ra,&dec);
  fgets(line,80,fp);
  fgets(line,80,fp);
  fgets(line,80,fp);
  fgets(line,80,fp);
  gccd->guide_mag=99;
  gccd->guide_pa=0;
  gccd->guide_x0=gccd->xc;
  gccd->guide_y0=gccd->yc;
  gra=ra;
  gdec=dec;
//fprintf(stderr,"ra: %f  dec: %f\n",ra,dec);
fprintf(stderr,"cx: %f cy: %f gccdxc: %f gccdyc: %f\n",cx,cy,gccd->xc,gccd->yc);
fprintf(stderr,"gccd: %f %f %f %f %f\n",gccd->sx,gccd->sy,gccd->cx,gccd->cy,gccd->theta);

  ngood = 0;
  while ( fgets(line,80,fp) != NULL && ngood<=nskip ) {
    sscanf(line,"%s%s%s%s%s%s%lf",grah,gram,gras,gdecd,gdecm,gdecs,&mag);
//    if (mag < gccd->guide_mag) {
      sprintf(line,"%s %s %s",grah, gram, gras);
      getcoord(line,&tra,0);
      sprintf(line,"%s %s %s",gdecd, gdecm, gdecs);
      getcoord(line,&tdec,0);
      dra=(tra-ra)*15*3600*cos(dec*D2R);
      ddec=(tdec-dec)*3600;

      guidepos(dra, ddec, cx, cy, *gccd, &pa, &x, &y, 0);
      // printf("%f %f %f %f\n",x,y,pa,mag);
//      if (x> 30 && x<160 && y>30 && y<140 ) {
//      if (x> 35 && x<157 && y>30 && y<130 ) {
      if (x> 700 && x<924 && y>200 && y<800 ) {
        ngood++;
        printf(
"guide star predicted at (x,y)=(%5.1f,%5.1f) pa=%5.1f mag=%5.1f %2d %2d\n",x,y,pa,mag,ngood,nskip);
// file is sorted by magnitude
//        if (mag < gccd->guide_mag) {
          if ( ngood == nskip+1 ) {
           gccd->guide_x0=x;
           gccd->guide_y0=y;
           gccd->guide_pa=pa;
           gccd->guide_rad=0;
           gccd->guide_mag=mag;
           gra=tra;
           gdec=tdec;
          }
//        }
      }
//    }
  }
  printf("brightest guide star at (x,y)=(%5.1f,%5.1f) pa=%5.1f mag=%5.1f\n",
  gccd->guide_x0,gccd->guide_y0,gccd->guide_pa,gccd->guide_mag);
  printf("  RA: %f\n",gra);
  printf("  DEC: %f\n",gdec);
}

void guidepos(double dra, double ddec, double cx, double cy, struct CCDSTATUS ccd, double *pa, double *x, double *y, int print)
{
  double pa0, xp, yp;

  // Position angle of target
//  *pa = atan2(-1*ddec,dra)*R2D;
//  *pa = dra>0 ? *pa+180 : *pa;
  *pa = atan2(ddec+cy,-dra+cx)*R2D;

  // Position angle of guider
  pa0 = atan2(ccd.cy,ccd.cx)*R2D;
  if (print) printf("dra: %f ddec: %f pa1: %f pa0: %f\n",dra,ddec,*pa,pa0);

  // Desired position angle
  *pa-=pa0;

  // Rotator coordinates
  xp = -dra*cos(*pa*D2R) + ddec*sin(*pa*D2R) + cx;
  yp =  dra*sin(*pa*D2R) + ddec*cos(*pa*D2R) + cy;
  if (print) printf("%f %f %f %f %f %f %f\n",dra, ddec,*pa,xp,yp,cx,cy);
  if (print) printf("%f %f %f %f %f\n",ccd.sx,ccd.sy,ccd.cx,ccd.cy,ccd.theta);

  // Guider coordinates
  *x = (xp-ccd.cx)*cos(ccd.theta*D2R) - (yp-ccd.cy)*sin(ccd.theta*D2R);
  *x = *x/ccd.sx + ccd.xc;
  if (print) printf("%f %f\n",(xp-ccd.cx)*cos(ccd.theta*D2R),(yp-ccd.cy)*sin(ccd.theta*D2R));

  *y = -(xp-ccd.cx)*sin(ccd.theta*D2R) - (yp-ccd.cy)*cos(ccd.theta*D2R);
  *y = *y/ccd.sy + ccd.yc;
  if (print) printf("%f %f\n",(xp-ccd.cx)*sin(ccd.theta*D2R), (yp-ccd.cy)*cos(ccd.theta*D2R));
  if (print) printf("x: %f y: %f xc: %f yc: %f\n",*x,*y,ccd.xc,ccd.yc);
}
