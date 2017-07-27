#include "mytype.h"
#include "filter.h"
#include <stdio.h>
#include <curses.h>
#include <math.h>
#include "slamac.h"
#include "slalib.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define PLOT
#define ALTSCREEN

#define LFILE

void update_display(struct STATUS *);
void update_engineering(struct STATUS *);
void update_ccd_display(struct CCDSTATUS *,struct STATUS *,int offset,char *);
void update_ccd_engineering(struct CCDSTATUS *,int offset,char *);
void vcopy(chtype *vbuf, char *buf, chtype attrib);
void gethms(double in,int *h, int *m, double *s,int *sign);
void getdms(double in,int *h, int *m, double *s,int *sign);

void plot(float *, float *, float *, float *, int);
void iconify_window();
float max(float, float);
float min(float, float);

FILE *infile;
int idepth;
int havepipe=0;

/* char filtname[7] = {'?',' ','B','V','R','I',' '}; */
double **ifoc;

#define NPTS 100
main(argc, argv)
int argc;
char *argv[];
{
  struct STATUS statusinfo;
  struct CCDSTATUS ccdinfo, gccdinfo;
  char *tstatusfile = "/home/export/tocc/statr.doc";
  char *tstatusreadyfile = "/home/export/tocc/statr.fin";
  char cstatusfile[64];
  char cstatusreadyfile[64];
  char *gstatusfile = "guidestatus.doc";
  char *gstatusreadyfile = "guidestatus.fin";
  char command[80], title[64];
  int i, npts, term=0, ready, n;
  int status, ccdstatus;
  float time[NPTS], xerror[NPTS], yerror[NPTS], zerror[NPTS];
  FILE *lfile;
  fd_set readfds;
  struct timeval timeout;
  WINDOW *win;
  int update = 1;
  BOOL science = TRUE;
  
  double **foc;
  double **junk;
  double vec[3];
  double out[3];
  int sock, port;
  char server[16];

  signal(SIGINT,SIG_IGN);

/* Invert the focus matrix */
  foc = (double **)malloc(3*sizeof(double *));
  ifoc = (double **)malloc(3*sizeof(double *));
  junk = (double **)malloc(3*sizeof(double *));
  setup_focus(foc);
  setup_focus(ifoc);
  setup_focus(junk);
  gaussj(ifoc,3,junk,3);

#ifdef LFILE
  lfile = fopen("error.log","a");
#endif

/* Initialize telescope status information */
  initstatus(&statusinfo);
#ifdef SOCKET
  port = 1051;  
  sprintf(server,"192.41.211.19"); //ccd1m
  sprintf(server,"ccd1m.apo.nmsu.edu");
  //sprintf(server,"192.41.211.24"); //control1m
  //sprintf(server,"command1m.apo.nmsu.edu");
  setup_server(&sock,port,server);
#endif

/* Initialize filter information */
  status = initfilt();
  if (status != 0) {
    fprintf(stderr,"error initialing filter data!\n");
    exit(-1);
  }

  win = initscr();
  nodelay(win, TRUE);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  i = 0;
  npts = 0;
  ready = 1;
  while(1) {

    usleep(100000);
    n=0;
    if (update)
      n=getch();
    else {
      gets(command);
      n=1;
    }
    if (n>0) {
#ifdef ALTSCREEN

      science = !science;
      clear();
      refresh();

#else
      if (update) 
        clear();
        refresh();
        endwin();
      update = !update;
      if (update) {
        win = initscr();
        nodelay(win, TRUE);
      }
      else {
        fprintf(stderr,"Hit <CR> to restart updating status");
      }
#endif
    }
  
#ifdef PLOT
    if (!science && term==0) {
/* Open a plotting window for error signals */
      term = 11;
      device_(&term);
      tsetup_();
      erase_();
    }
   if (science && term>0) {
 /*   iconify_window(); */
    term = -term;
   }
#endif

    if (update) {

#ifndef RSTATUS
    if (n>0) ready=1;
#endif

  /* Get CCD information first (need to get filter focus before displaying
     telescope focus) */
   status=readstatus(sock,port,server,&statusinfo);
   if (statusinfo.tertiary_port == 2) {
     sprintf(cstatusfile,"accdstatus.doc");
     sprintf(cstatusreadyfile,"accdstatus.fin");
   } else {
     sprintf(cstatusfile,"ccdstatus.doc");
     sprintf(cstatusreadyfile,"ccdstatus.fin");
   }
   ccdstatus = readccdstatus(cstatusfile,cstatusreadyfile,&ccdinfo);
   statusinfo.filtfoc = ccdinfo.filtfoc;
  /* Get any new telescope status information */
#ifdef SOCKET
   if (status == 0) {
#else
   if (readstatus(tstatusfile,tstatusreadyfile,&statusinfo) == 0) {
#endif

#ifdef ALTSCREEN
      if (science)
        update_display(&statusinfo);
      else
        update_engineering(&statusinfo);
#else
      update_display(&statusinfo);
#endif
      statusinfo.last_az_error *= 3600;
      statusinfo.last_alt_error *= 3600;
      statusinfo.last_rot_error *= 3600;

#ifndef APOGEE
      ccdinfo.ccd_temp = statusinfo.ccd_temp;
#endif


#ifdef LFILE
      fprintf(lfile,"%f %f %f %f %f %f %f %d %d %d\n",statusinfo.current_utc,
 statusinfo.last_az_error, statusinfo.last_alt_error, statusinfo.last_rot_error,
 statusinfo.last_x_rate, statusinfo.last_y_rate, statusinfo.last_z_rate,
 statusinfo.x_encoder_tracking,statusinfo.y_encoder_tracking,statusinfo.z_encoder_tracking);
      fflush(lfile);
#endif
      npts = (++npts >100 ? 100 : npts);
      xerror[i] = statusinfo.last_az_error;
      yerror[i] = statusinfo.last_alt_error;
      zerror[i] = statusinfo.last_rot_error;
      time[i++] = (statusinfo.current_utc - (int)statusinfo.current_utc)*24.;
#ifdef PLOT
      if (!science) plot(xerror,yerror,zerror,time,npts);
#endif
      i %= NPTS;
    }
  /* Display ccd status information */
    if (ccdstatus == 0) {
      if (statusinfo.tertiary_port == 2) 
        sprintf(title,"Photometer acquisition CCD (APOGEE)");
      else
        sprintf(title,"Science imaging CCD (Leach)");
#ifdef ALTSCREEN
      if (science)
        update_ccd_display(&ccdinfo,&statusinfo,-1,title);
      else
        update_ccd_engineering(&ccdinfo,0,title);
#else
      update_ccd_display(&ccdinfo,&statusinfo,-1,title);
#endif
    }
  /* Get any new ccd status information */
    if (readccdstatus(gstatusfile,gstatusreadyfile,&gccdinfo) == 0) {
#ifdef ALTSCREEN
      if (science)
        update_ccd_display(&gccdinfo,&statusinfo,5,"Guide CCD (FLI)");
      else
        update_ccd_engineering(&gccdinfo,5,"Guide CCD (FLI)");
#else
      update_ccd_display(&gccdinfo,&statusinfo,5,"Guide CCD (FLI)");
#endif
    }

    refresh();
    }

  }
  endwin();
}

void puttext(int x1,int y1,int x2,int y2,chtype *buf)
{
  int i;
  chtype c;

  move(y1,x1);
  for (i=0;i<x2-x1+1;i++) {
    addch(buf[i]);
  }
}

#define WHITE A_NORMAL

void update_ccd_display(struct CCDSTATUS *ccdinfo, struct STATUS *G, int offset, char *title)
{
  char buf[80], color[32];
  chtype vbuf[80];
  int h,m,sign,i,start,l;
  double s, xs, ys, timeleft;
  FILE *fp;

#ifdef RSTATUS
  fp = fopen("/home/export/tocc/tmp2.html","w");
#else
  fp = fopen("/home/export/tocc/tmp.html","w");
#endif
  fprintf(fp,"<html><body><table cellspacing=3 width=100\%>\n<meta http-equiv=Refresh content=2>\n");
  fprintf(fp,"<TR>\n");
  for (i=0;i<23;i++) buf[i] = '-';
  sprintf(buf+23,"%s",title);
  start = strlen(buf);
  for (i=start;i<79;i++) buf[i] = '-';
  buf[79] = 0;
  vcopy(vbuf,buf,WHITE);
  puttext(1,9+offset,79,9+offset,vbuf);

  // next filename
  sprintf(buf,"Next file: %s.%03d.fits",ccdinfo->filename,ccdinfo->incval);
  vcopy(vbuf,buf,WHITE);
  puttext(1,10+offset,strlen(buf),10+offset,vbuf);
  fprintf(fp,"<td>%s</td>\n",buf);

  // filetype
  if (ccdinfo->filetype==1) 
    sprintf(buf,"Filetype: FITS      ");
  else if (ccdinfo->filetype==2)
    sprintf(buf,"Filetype: SDAS/IRAF ");
  else
    sprintf(buf,"Filetype: Not Saving");
  vcopy(vbuf,buf,WHITE);
  puttext(1,13+offset,20,13+offset,vbuf);
  fprintf(fp,"<td>%s</td>\n",buf);

  fprintf(fp,"<tr>\n");
  // Exptime
  sprintf(buf,"Exptime: %8.1f",ccdinfo->exposure);
  vcopy(vbuf,buf,WHITE);
  puttext(1,11+offset,17,11+offset,vbuf);
  fprintf(fp,"<td>%s</td>\n",buf);

  if (ccdinfo->expstatus == 1)
    sprintf(buf,"Status: EXPOSING                                            ");
  else if (ccdinfo->expstatus == 2)
    sprintf(buf,"Status: WRITING                                             ");
  else if (ccdinfo->expstatus == 3)
    sprintf(buf,"Status: DISPLAYING                                          ");
  else 
    sprintf(buf,"Status: WAITING                                             ");
  vcopy(vbuf,buf,WHITE);
  puttext(25,11+offset,79,11+offset,vbuf);
  fprintf(fp,"<td>%s",buf);

  if (ccdinfo->expstatus == 1) {
    timeleft = (ccdinfo->end_time - G->current_utc)*24*3600;
    sprintf(buf,"Approximate time left: %4d\0",(int)timeleft);
    vcopy(vbuf,buf,WHITE);
    puttext(46,11+offset,72,11+offset,vbuf);
    fprintf(fp,"%s",buf);
/*
    gethms((ccdinfo->end_time-(long)ccdinfo->end_time)*24,&h,&m,&s,&sign);
    sprintf(buf,"APPROXIMATE END TIME: %2d %2d %2d\0",h,m,(int)s);
    vcopy(vbuf,buf,WHITE);
    puttext(46,11+offset,79,11+offset,vbuf);
  } else {
    sprintf(buf,"                           \0",(int)timeleft);
    vcopy(vbuf,buf,WHITE);
    puttext(46,10+offset,72,10+offset,vbuf);
*/
  }
  fprintf(fp,"</td>\n");
  if (ccdinfo->guiding ==1) {
    sprintf(buf,"GUIDING x:%5.1f y:%5.1f",
                 ccdinfo->guide_x0,ccdinfo->guide_y0);
    vcopy(vbuf,buf,WHITE);
    puttext(41,11+offset,63,11+offset,vbuf);
  } else if (ccdinfo->guiding ==2) {
    sprintf(buf,"Pending (%5.1f %5.1f %5.1f %5.0f %4.1f)",
       ccdinfo->guide_x0,ccdinfo->guide_y0,ccdinfo->guide_pa,ccdinfo->guide_rad,       ccdinfo->guide_mag);
    vcopy(vbuf,buf,WHITE);
    puttext(41,11+offset,79,11+offset,vbuf);
  } 

  fprintf(fp,"<TR>\n");
  // Filter
  //sprintf(buf,"Filter: %d %s(%s)",ccdinfo->filter,filtname[ccdinfo->filter],longfiltname[ccdinfo->filter]);
  if (G->tertiary_port == 2) 
    sprintf(buf,"Filter: NONE",G->guider_filtpos,filtname[G->guider_filtpos],longfiltname[ccdinfo->filter]);
  else 
    sprintf(buf,"Filter: %d %s(%s)",G->guider_filtpos,filtname[G->guider_filtpos],longfiltname[ccdinfo->filter]);
  fprintf(fp,"<td>%s</td>\n",buf);
  l = strlen(buf);
  for (i=l; i< 80; i++) buf[i] = ' ';
  vcopy(vbuf,buf,WHITE);
  puttext(1,12+offset,28,12+offset,vbuf);

  sprintf(buf,"Cleans: %3d",ccdinfo->cleans);
  vcopy(vbuf,buf,WHITE);
  puttext(29,12+offset,39,12+offset,vbuf);

  sprintf(buf,"size:%3d  update:%3d",
                 ccdinfo->guide_size, ccdinfo->guide_update);
  vcopy(vbuf,buf,WHITE);
  puttext(41,12+offset,60,12+offset,vbuf);

  // CCD TEMP
  sprintf(buf,"CCD Temp: %4d",(int)ccdinfo->ccd_temp);
  float targtemp;
  if (G->tertiary_port == 1) 
    targtemp=-100.;
  else
    targtemp=-10.;
  if (ccdinfo->ccd_temp < targtemp) {
    vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
  }else {
    vcopy(vbuf,buf,A_REVERSE);
    sprintf(color,"<font color=red>");
  }
  puttext(66,12+offset,79,12+offset,vbuf);
  fprintf(fp,"<td>%s%s</font></td>\n",color,buf);

  fprintf(fp,"<TR>\n");
  if (ccdinfo->autodisplay)
    sprintf(buf,"Autodisplay: ON ");
  else
    sprintf(buf,"Autodisplay: OFF");
  vcopy(vbuf,buf,WHITE);
  puttext(25,13+offset,40,13+offset,vbuf);
  fprintf(fp,"<td>%s</td>\n",buf);

  if (ccdinfo->autoxfer)
    sprintf(buf,"Autoxfer: ON ");
  else
    sprintf(buf,"Autoxfer: OFF");
  vcopy(vbuf,buf,WHITE);
  puttext(45,13+offset,57,13+offset,vbuf);
  fprintf(fp,"<td>%s</td>\n",buf);

  fprintf(fp,"</table></body></html>\n");
  fclose(fp);
#ifndef RSTATUS
  if (offset <=0) 
    system("mv /home/export/tocc/tmp.html /home/export/tocc/ccdstatus.html");
  else
    system("mv /home/export/tocc/tmp.html /home/export/tocc/gccdstatus.html");
#endif

#ifndef ALTSCREEN
  sprintf(buf,"Coord xform: %6.3f %6.3f %6.3f %6.3f",
        ccdinfo->ax, ccdinfo->bx, ccdinfo->ay, ccdinfo->by);
  vcopy(vbuf,buf,WHITE);
  puttext(1,13+offset,40,13+offset,vbuf);

  xs = sqrt(ccdinfo->ax*ccdinfo->ax+ccdinfo->ay*ccdinfo->ay);
  ys = sqrt(ccdinfo->bx*ccdinfo->bx+ccdinfo->by*ccdinfo->by);
  sprintf(buf,"Scales/rot: %5.2f %5.2f %7.2f %7.2f",
        xs, ys, acos(ccdinfo->ax/xs)*180./DPI, acos(ccdinfo->by/ys)*180./DPI);
  vcopy(vbuf,buf,WHITE);
  puttext(41,13+offset,79,13+offset,vbuf);
#endif
}
void update_ccd_engineering(struct CCDSTATUS *ccdinfo, int offset, char *title)
{
  char buf[80];
  chtype vbuf[80];
  int h,m,sign,i,start;
  double s, xs, ys;

  for (i=0;i<23;i++) buf[i] = '-';
  sprintf(buf+23,"%s",title);
  start = strlen(buf);
  for (i=start;i<79;i++) buf[i] = '-';
  buf[79] = 0;
  vcopy(vbuf,buf,WHITE);
  puttext(1,9+offset,79,9+offset,vbuf);

  sprintf(buf,"Coord xform: %6.3f %6.3f %6.3f %6.3f",
        ccdinfo->ax, ccdinfo->bx, ccdinfo->ay, ccdinfo->by);
  vcopy(vbuf,buf,WHITE);
  puttext(1,13+offset,40,13+offset,vbuf);

  xs = sqrt(ccdinfo->ax*ccdinfo->ax+ccdinfo->ay*ccdinfo->ay);
  ys = sqrt(ccdinfo->bx*ccdinfo->bx+ccdinfo->by*ccdinfo->by);
  sprintf(buf,"Scales/rot: %5.2f %5.2f %7.2f %7.2f",
        xs, ys, acos(ccdinfo->ax/xs)*180./DPI, acos(ccdinfo->by/ys)*180./DPI);
  vcopy(vbuf,buf,WHITE);
  puttext(41,13+offset,79,13+offset,vbuf);

}

void update_display(struct STATUS *G)
{
   int h,m,sign;
   double s, fd, ha;
   double az, azd, azdd, el, eld, eldd, pa, pad, padd, airmas, secz;
   double latitude = 32.7867*DD2R;
   FILE *fp;

   char buf[80], color[80];
   chtype vbuf[160];

#ifdef RSTATUS
   fp = fopen("/home/export/tocc/tmp2.html","w");
#else
   fp = fopen("/home/export/tocc/tmp.html","w");
#endif
   fprintf(fp,"<html><body><table cellspacing=3 width=100\%>\n<meta http-equiv=Refresh content=2>\n");
   fprintf(fp,"<TR>\n");

   // RA
   gethms(G->current_obs_ra*DR2H,&h,&m,&s,&sign);
   sprintf(buf,"RA:  %2d %2d %4.1f\0",h,m,s);
   vcopy(vbuf,buf,WHITE);
   puttext(1,1,15,1,vbuf);
   fprintf(fp,"<TD NOWRAP>%s</TD>\n",buf);

   // AZ
   if (G->x_encoder_installed) {
     sprintf(buf,"Encoder AZ:   %7.3f\0",G->current_obs_az);
   } else {
     sprintf(buf,"Motor AZ:     %7.3f\0",G->current_obs_az);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(21,1,40,1,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   // UT
   gethms((G->current_utc - (long)G->current_utc)*24,&h,&m,&s,&sign);
   sprintf(buf,"UT:  %2d %2d %2d\0",h,m,(int)s);
   vcopy(vbuf,buf,WHITE);
   puttext(49,1,61,1,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   fprintf(fp,"<TR>\n");
   // DEC
   getdms(fabs(G->current_obs_dec*DR2D),&h,&m,&s,&sign);
   if (G->current_obs_dec >= 0)
     sprintf(buf,"DEC: %2d %2d %4.1f\0",h,m,s);
   else
     sprintf(buf,"DEC:-%2d %2d %4.1f\0",h,m,s);
   vcopy(vbuf,buf,WHITE);
   puttext(1,2,15,2,vbuf);
   fprintf(fp,"<TD NOWRAP>%s</TD>\n",buf);

   // Alt
   if (G->y_encoder_installed) {
     sprintf(buf,"Encoder ALT:  %7.3f\0",G->current_obs_alt);
   } else {
     sprintf(buf,"Motor ALT:    %7.3f\0",G->current_obs_alt);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(21,2,40,2,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   // LST
   gethms(G->current_lasth,&h,&m,&s,&sign);
   sprintf(buf,"LST: %2d %2d %2d\0",h,m,(int)s);
   vcopy(vbuf,buf,WHITE);
   puttext(49,2,61,2,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   fprintf(fp,"<TR>\n");
   // HA
   ha = slaDrange(G->current_lasth*DH2R - G->current_obs_ra)*DR2H;
   gethms(fabs(ha),&h,&m,&s,&sign);
   if (ha >= 0)
     sprintf(buf,"HA:  %2d %2d %4.1fW\0",h,m,s);
   else
     sprintf(buf,"HA:  %2d %2d %4.1fE\0",h,m,s);
   vcopy(vbuf,buf,WHITE);
   puttext(1,3,16,3,vbuf);
   fprintf(fp,"<TD NOWRAP>%s</TD>\n",buf);

   // Rot
   if (G->z_encoder_installed) {
     sprintf(buf,"Encoder ROT:  %7.3f\0",G->current_obs_rot);
   } else {
     sprintf(buf,"Motor ROT:    %7.3f\0",G->current_obs_rot);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(21,3,40,3,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   // Airmass
   slaAltaz(ha*DH2R,G->current_obs_dec,latitude,
      &az,&azd,&azdd,&el,&eld,&eldd,&pa,&pad,&padd);
   secz = 1.0/cos(DPIBY2-el);
   airmas=secz+(secz-1.0)*(-0.0018167+(secz-1.0)*
          (-0.002875-0.0008083*(secz-1.0)));
   sprintf(buf,"Airmass: %5.2f\0",airmas);
   vcopy(vbuf,buf,WHITE);
   puttext(49,3,62,3,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   fprintf(fp,"<TR>\n");
   // Epoch
   sprintf(buf,"EPOCH:   %6.1f\0",G->current_mean_epoch);
   vcopy(vbuf,buf,WHITE);
   puttext(1,4,15,4,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   sprintf(buf,"PA:      %6.1f\0",G->current_pa*DR2D);
   vcopy(vbuf,buf,WHITE);
   puttext(1,5,15,5,vbuf);
   //fprintf(fp,"<TD>%s</TD>\n",buf);

#ifndef ALTSCREEN
   sprintf(buf,"Motor FOC:%6d %6d %6d",
      G->t_step_pos,G->u_step_pos,G->v_step_pos);
   vcopy(vbuf,buf,WHITE);
   puttext(21,4,50,4,vbuf);
#endif

   // Focus
   calc_focus(ifoc,G);
   if (G->filtfoc>0) 
     sprintf(buf,"Focus: %7.1f(+%3d)",G->foc-(G->filtfoc),G->filtfoc);
   else
     sprintf(buf,"Focus: %7.1f(%4d)",G->foc-(G->filtfoc),G->filtfoc);
   vcopy(vbuf,buf,WHITE);
   puttext(21,4,40,4,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   // Tertiary port
   if (G->tertiary_port==1||G->tertiary_port==2) {
     sprintf(buf,"Port: NA%d",G->tertiary_port);
     vcopy(vbuf,buf,WHITE);
   } else {
     sprintf(buf,"Port: ???",G->tertiary_port);
     vcopy(vbuf,buf,A_REVERSE);
   }
   puttext(21,5,29,5,vbuf);

   // Telescope status
   sprintf(buf,"Telescope status:\0");
   vcopy(vbuf,buf,WHITE);
   puttext(1,6,17,6,vbuf);
   if (G->shutdown_state) {
     sprintf(buf,"SHUTDOWN 0x%4.4x",G->shutdown_state);
     vcopy(vbuf,buf,A_REVERSE);
    sprintf(color,"<font color=red>");
     puttext(19,6,33,6,vbuf);
   } else if (!G->telescope_initialized) {
     sprintf(buf,"NOT INITIALIZED\0");
     vcopy(vbuf,buf,A_REVERSE);
    sprintf(color,"<font color=red>");
     puttext(19,6,33,6,vbuf);
   } else if (G->telescope_at_home==1) {
     sprintf(buf,"HOME           \0");
     vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
     puttext(19,6,33,6,vbuf);
   } else if (G->telescope_at_home==2) {
     sprintf(buf,"FILL           \0");
     vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
     puttext(19,6,33,6,vbuf);
   } else if (G->telescope_at_home==3) {
     sprintf(buf,"UP (Mirror Cov)\0");
     vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
     puttext(19,6,33,6,vbuf);
   } else if (G->telescope_at_home==4) {
     sprintf(buf,"STOWED         \0");
     vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
     puttext(19,6,33,6,vbuf);
   } else if (G->tracking_on) {
     sprintf(buf,"TRACKING       \0");
     vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
     puttext(19,6,33,6,vbuf);
   } else if (G->telescope_is_slewing) {
     sprintf(buf,"SLEWING %1d      \0",G->nmove);
     vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
     puttext(19,6,33,6,vbuf);
   } else {
     sprintf(buf,"STOPPED        \0");
     vcopy(vbuf,buf,WHITE);
    sprintf(color,"<font color=green>");
     puttext(19,6,33,6,vbuf);
   }
   fprintf(fp,"<TD>Status:%s %s</font></td>\n",color,buf);

#ifndef ALTSCREEN
   sprintf(buf,"Encoder status: \0");
   vcopy(vbuf,buf,WHITE);
   puttext(1,7,16,7,vbuf);
   if (G->use_encoders) {
     sprintf(buf," ON FOR POINTING\0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,7,34,7,vbuf);
   } else {
     sprintf(buf,"OFF FOR POINTING\0");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(19,7,34,7,vbuf);
   }
   if (G->x_encoder_installed && G->y_encoder_installed) {
     sprintf(buf,"    ON FOR MOVES\0");
     vcopy(vbuf,buf,WHITE);
     puttext(35,7,50,7,vbuf);
   } else {
     sprintf(buf,"   OFF FOR MOVES\0");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(35,7,50,7,vbuf);
   }

   if (G->x_encoder_tracking ||
       G->y_encoder_tracking ||
       G->z_encoder_tracking) {
     sprintf(buf," ON FOR     TRACKING");
     vcopy(vbuf,buf,WHITE);
     puttext(52,7,71,7,vbuf);
     if (G->x_encoder_tracking) {
       if (G->x_encoder_tracking>0) sprintf(buf,"X");
       else sprintf(buf,"x");
       vcopy(vbuf,buf,WHITE);
       puttext(60,7,60,7,vbuf);
     }
     if (G->y_encoder_tracking) {
       if (G->y_encoder_tracking>0) sprintf(buf,"Y");
       else sprintf(buf,"y");
       vcopy(vbuf,buf,WHITE);
       puttext(61,7,61,7,vbuf);
     }
     if (G->z_encoder_tracking) {
       if (G->z_encoder_tracking>0) sprintf(buf,"Z");
       else sprintf(buf,"z");
       vcopy(vbuf,buf,WHITE);
       puttext(62,7,62,7,vbuf);
     }
   } else {
     sprintf(buf,"    OFF FOR TRACKING");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(52,7,71,7,vbuf);
   }

   if (G->mc_enabled) {
     sprintf(buf,"MC:  ENABLED, FILE: %s",G->mc_file);
     vcopy(vbuf,buf,WHITE);
   }
   else {
     sprintf(buf,"MC: DISABLED");
     vcopy(vbuf,buf,A_REVERSE);
   }
   puttext(1,8,strlen(buf),8,vbuf);
#endif 

   fprintf(fp,"<TR>\n");
   // Dome az
   sprintf(buf,"Dome AZ: %3d",G->dome_azimuth);
   vcopy(vbuf,buf,WHITE);
   puttext(36,5,47,5,vbuf);
   fprintf(fp,"<TD>%s</TD>\n",buf);

   sprintf(buf,"3.5m slaving: ");
   vcopy(vbuf,buf,WHITE);
   puttext(63,4,76,4,vbuf);
   if (G->check_35m_closed != 0) {
     sprintf(buf," ON");
     vcopy(vbuf,buf,WHITE);
     puttext(77,4,79,4,vbuf);
   } else {
     sprintf(buf,"OFF");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(77,4,79,4,vbuf);
   }
   // fprintf(fp,"<TD>%s</TD>\n",buf);

   sprintf(buf,"Dome status:");
   vcopy(vbuf,buf,WHITE);
   puttext(36,6,47,6,vbuf);
   if (G->dome_initialized != 0) {
     sprintf(buf,"INITIALIZED  , ");
     vcopy(vbuf,buf,WHITE);
     puttext(48,6,62,6,vbuf);
     sprintf(color,"<font color=green>");
   } else {
     sprintf(buf,"UNINITIALIZED, ");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(48,6,62,6,vbuf);
     sprintf(color,"<font color=red>");
   }
   fprintf(fp,"<TD>Dome: %s%s</font>",color,buf);

   if (G->dome_slaved != 0) {
     sprintf(buf,"SLAVED    ,");
     vcopy(vbuf,buf,WHITE);
     puttext(63,6,73,6,vbuf);
     sprintf(color,"<font color=green>");
   } else {
     sprintf(buf,"NOT SLAVED,");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(63,6,73,6,vbuf);
     sprintf(color,"<font color=red>");
   }
   fprintf(fp,"%s%s</font>",color,buf);
   if (G->dome_open != 0) {
     sprintf(buf,"OPEN  ");
     vcopy(vbuf,buf,WHITE);
     puttext(74,6,79,6,vbuf);
     sprintf(color,"<font color=green>");
   } else {
     sprintf(buf,"CLOSED");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(74,6,79,6,vbuf);
     sprintf(color,"<font color=red>");
   }
   fprintf(fp,"%s%s</font></td>",color,buf);

   sprintf(buf,"Lower Dome:");
   vcopy(vbuf,buf,WHITE);
   puttext(62,7,72,7,vbuf);
   if (G->lower_dome_open != 0) {
     sprintf(buf,"OPEN  ");
     vcopy(vbuf,buf,WHITE);
     puttext(74,7,79,7,vbuf);
   } else {
     sprintf(buf,"CLOSED");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(74,7,79,7,vbuf);
   }

   // Mirror covers
   sprintf(buf,"Mirror covers: ");
   vcopy(vbuf,buf,WHITE);
   puttext(58,5,72,5,vbuf);
   fprintf(fp,"<td>%s",buf);
   if (G->mirror_covers_open != 0) {
     sprintf(buf,"OPEN  ");
     vcopy(vbuf,buf,WHITE);
     puttext(73,5,78,5,vbuf);
     sprintf(color,"<font color=green>");
   } else {
     sprintf(buf,"CLOSED");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(73,5,78,5,vbuf);
     sprintf(color,"<font color=red>");
   }
   fprintf(fp,"%s%s</font></td>\n",color,buf);

   fprintf(fp,"</table></body></html>\n");
   fclose(fp);
#ifndef RSTATUS
   system("mv /home/export/tocc/tmp.html /home/export/tocc/status.html");
#endif

#ifdef ALTSCREEN
   sprintf(buf,"Hit CR to toggle to engineering window");
#else
   sprintf(buf,"Hit CR to suspend status updates");
#endif
   vcopy(vbuf,buf,A_UNDERLINE);
   puttext(1,20,32,20,vbuf);

}
void update_engineering(struct STATUS *G)
{
   int h,m,sign;
   double s, fd, ha;
   double az, azd, azdd, el, eld, eldd, pa, pad, padd, airmas, secz;
   double latitude = 32.7867*DD2R;

   char buf[80];
   chtype vbuf[160];
   if (G->x_encoder_installed) {
     sprintf(buf,"Encoder AZ:   %7.3f\0",G->current_obs_az);
   }
   else {
     sprintf(buf,"Motor AZ:     %7.3f\0",G->current_obs_az);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(1,1,20,1,vbuf);

   if (G->y_encoder_installed) {
     sprintf(buf,"Encoder ALT:  %7.3f\0",G->current_obs_alt);
   }
   else {
     sprintf(buf,"Motor ALT:    %7.3f\0",G->current_obs_alt);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(1,2,20,2,vbuf);

   if (G->z_encoder_installed) {
     sprintf(buf,"Encoder ROT:  %7.3f\0",G->current_obs_rot);
   }
   else {
     sprintf(buf,"Motor ROT:    %7.3f\0",G->current_obs_rot);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(1,3,20,3,vbuf);

   sprintf(buf,"Motor FOC:%6d %6d %6d",
      G->t_step_pos,G->u_step_pos,G->v_step_pos);
   vcopy(vbuf,buf,WHITE);
   puttext(1,7,30,7,vbuf);

   calc_focus(ifoc,G);
   sprintf(buf,"Focus: %8.1f xtilt: %5.2f ytilt: %5.2f",
           G->foc,G->foc_theta,G->foc_phi);
   vcopy(vbuf,buf,WHITE);
   puttext(35,7,75,7,vbuf);

   sprintf(buf,"Telescope status:\0");
   vcopy(vbuf,buf,WHITE);
   puttext(1,4,17,4,vbuf);
   if (!G->telescope_initialized) {
     sprintf(buf,"NOT INITIALIZED\0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,4,33,4,vbuf);
   } else if (G->telescope_at_home==1) {
     sprintf(buf,"HOME           \0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,4,33,4,vbuf);
   } else if (G->telescope_at_home==2) {
     sprintf(buf,"FILL           \0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,6,33,6,vbuf);
   } else if (G->telescope_at_home==3) {
     sprintf(buf,"UP (Mirror Cov)\0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,6,33,6,vbuf);
   } else if (G->tracking_on) {
     sprintf(buf,"TRACKING \0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,4,27,4,vbuf);
   } else if (G->telescope_is_slewing) {
     sprintf(buf,"SLEWING %1d\0",G->nmove);
     vcopy(vbuf,buf,WHITE);
     puttext(19,4,27,4,vbuf);
   } 
   else {
     sprintf(buf,"STOPPED  \0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,4,27,4,vbuf);
   }

   sprintf(buf,"Encoder status: \0");
   vcopy(vbuf,buf,WHITE);
   puttext(1,5,16,5,vbuf);
   if (G->use_encoders) {
     sprintf(buf," ON FOR POINTING\0");
     vcopy(vbuf,buf,WHITE);
     puttext(19,5,34,5,vbuf);
   } else {
     sprintf(buf,"OFF FOR POINTING\0");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(19,5,34,5,vbuf);
   }
   if (G->x_encoder_installed && G->y_encoder_installed) {
     sprintf(buf,"    ON FOR MOVES\0");
     vcopy(vbuf,buf,WHITE);
     puttext(35,5,50,5,vbuf);
   } else {
     sprintf(buf,"   OFF FOR MOVES\0");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(35,5,50,5,vbuf);
   }

   if (G->x_encoder_tracking ||
       G->y_encoder_tracking ||
       G->z_encoder_tracking) {
     sprintf(buf," ON FOR     TRACKING");
     vcopy(vbuf,buf,WHITE);
     puttext(52,5,71,5,vbuf);
     if (G->x_encoder_tracking) {
       if (G->x_encoder_tracking>0) sprintf(buf,"X");
       else sprintf(buf,"x");
       vcopy(vbuf,buf,WHITE);
       puttext(60,5,60,5,vbuf);
     }
     if (G->y_encoder_tracking) {
       if (G->y_encoder_tracking>0) sprintf(buf,"Y");
       else sprintf(buf,"y");
       vcopy(vbuf,buf,WHITE);
       puttext(61,5,61,5,vbuf);
     }
     if (G->z_encoder_tracking) {
       if (G->z_encoder_tracking>0) sprintf(buf,"Z");
       else sprintf(buf,"z");
       vcopy(vbuf,buf,WHITE);
       puttext(62,5,62,5,vbuf);
     }
   } else {
     sprintf(buf,"    OFF FOR TRACKING");
     vcopy(vbuf,buf,A_REVERSE);
     puttext(52,5,71,5,vbuf);
   }

   if (G->mc_enabled) {
     sprintf(buf,"MC:  ENABLED, FILE: %s",G->mc_file);
     vcopy(vbuf,buf,WHITE);
   }
   else {
     sprintf(buf,"MC: DISABLED");
     vcopy(vbuf,buf,A_REVERSE);
   }
   puttext(1,6,strlen(buf),6,vbuf);
 
   sprintf(buf,"Hit CR to toggle back to science display");
   vcopy(vbuf,buf,A_UNDERLINE);
   puttext(1,20,32,20,vbuf);

}

void vcopy(chtype *vbuf, char *buf, chtype attrib)
{
  int i;

  for (i=0;i<strlen(buf);i++) {
    vbuf[i] = buf[i]|attrib;
  }
}

#ifdef PLOT
void plot(float *x, float *y, float *z, float *t, int n)
{
	float xmin, xmax, ymin, ymax, zmin, zmax, tmin, tmax;
        float xrange, yrange, zrange, trange;
	float ptype;
        int np, b1, b2, b3, b4;
        int i, i1, i2, i3;
 	float r1, r2;

        if (n < 10) return;

        xmin = 1e10;
        xmax = -1e10;
        ymin = 1e10;
        ymax = -1e10;
        zmin = 1e10;
        zmax = -1e10;
        tmin = 1e10;
        tmax = -1e10;
        for (i=0;i<n;i++) {
          xmin = min(xmin,x[i]);
          xmax = max(xmax,x[i]);
          ymin = min(ymin,y[i]);
          ymax = max(ymax,y[i]);
          zmin = min(zmin,z[i]);
          zmax = max(zmax,z[i]);
          tmin = min(tmin,t[i]);
          tmax = max(tmax,t[i]);
        }

        xrange = max(3.0,xmax-xmin);
        yrange = max(3.0,ymax-ymin);
        zrange = max(3.0,zmax-zmin);
        trange = max(0.01,tmax-tmin);

        xmin = xmin - 0.05*xrange;
        xmax = xmax + 0.05*xrange;
        ymin = ymin - 0.05*yrange;
        ymax = ymax + 0.05*yrange;
        zmin = zmin - 0.05*zrange;
        zmax = zmax + 0.05*zrange;
        tmin = tmin - 0.05*trange;
        tmax = tmax + 0.05*trange;

        erase_();
        r1 = 0;
        r2 = 0;
        submargins_(&r1,&r2);

        i1 = 2;
        i2 = 3;
        i3 = 5;
        ptype = 40.0;
        np = 1;
	b1 = 0;
        b2 = 2;
        b3 = 0;
        b4 = 0;
        window_(&i1,&i2,&i3);
        setlim_(&tmin,&xmin,&tmax,&xmax);
        abox_(&b1,&b2,&b3,&b4);
        points_(&ptype,&np,t,x,&n);

        i3 = 3;
        window_(&i1,&i2,&i3);
        setlim_(&tmin,&ymin,&tmax,&ymax);
        abox_(&b1,&b2,&b3,&b4);
        points_(&ptype,&np,t,y,&n);

	b1 = 1;
        i3 = 1;
        window_(&i1,&i2,&i3);
        setlim_(&tmin,&zmin,&tmax,&zmax);
        abox_(&b1,&b2,&b3,&b4);
        points_(&ptype,&np,t,z,&n);

// Again with fixed y limits
        xmin = ymin = zmin = -10.;
        xmax = ymax = zmax = 10.;
        i1 = 2;
        i2 = 3;
        i3 = 6;
        ptype = 40.0;
        np = 1;
	b1 = 0;
        b2 = 0;
        b3 = 0;
        b4 = 2;
        window_(&i1,&i2,&i3);
        setlim_(&tmin,&xmin,&tmax,&xmax);
        abox_(&b1,&b2,&b3,&b4);
        points_(&ptype,&np,t,x,&n);

        i3 = 4;
        window_(&i1,&i2,&i3);
        setlim_(&tmin,&ymin,&tmax,&ymax);
        abox_(&b1,&b2,&b3,&b4);
        points_(&ptype,&np,t,y,&n);

	b1 = 1;
        i3 = 2;
        window_(&i1,&i2,&i3);
        setlim_(&tmin,&zmin,&tmax,&zmax);
        abox_(&b1,&b2,&b3,&b4);
        points_(&ptype,&np,t,z,&n);

        tidle_();

}
#endif
float max(float x1, float x2)
{
  if (x1 > x2) 
    return(x1);
  else
    return(x2);
}
float min(float x1, float x2)
{
  if (x1 < x2) 
    return(x1);
  else
    return(x2);
}
#ifdef linux
MAIN__()
{
}
#endif
