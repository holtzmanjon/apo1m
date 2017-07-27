/* program to accept commands from command program over fifo and send them
    to ccd camera, get responses and send them back A

 Implemented for various cameras and camera interfaces using 
    C preprocessing as follows
 CCD array choices, choose one of
   #define KAF260
   #define LYNXX
   #define APOGEE
   #define E2V2K
   #define E2V1K
   #define HAVEPI
 command options, choose one of
   #define HAVEREMOTE to send commands via NFS file interface
   #define LEACH
   #define APOGEE
   #defin  KAF-1603	
   #define FLI
 guider or science camera?
   #define GUIDER
 accept commands from remote or local socket?
   #define LOCALCOM
*/ 

#undef DEBUG
#define HAVELOG
#define ROT
#define DISPLAY

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <sys/resource.h>
#include <netdb.h>

#include "mytype.h"
#include "filter.h"
#include "slalib.h"
#include "slamac.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
//#include "xtv/zimage.h"
Display *dpy;
int imtvnum_ = 0;

#include "power.h"
int plug_status[9];

#define COM_UNREC 1
#define COM_WRONG 2
#define COM_UNSUCCESSFUL 3

#define MAXCOMMAND 8000

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

struct CCDSTATUS ccdinfo;
struct STATUS telinfo;

#ifdef KAF260
// Spectrasource Teleris2 with KAF260
#define NCOL 524
#define NROW 512
#define NBIAS 6
#define NSPE 539458
#define XYRAT 1.
int offset = 2880;
#define DTIME 1
#define PRETIME 1
#define READTIME 1
#define HAVEDARK
unsigned char darkdata[NSPE];
int tvflip = 0;
#endif

#ifdef LYNXX
// Spectrasource Lynxx
#define NCOL 202
#define NROW 165
#define NBIAS 10
#define NSPE 69542
#define XYRAT (165./192.)
int offset = 2880;
#define DTIME 1
#define PRETIME 1
#define READTIME 1
#define HAVEDARK
unsigned char darkdata[NSPE];
double x_home_pos_arcsec = 1500.;
int tvflip = 0;
#endif

#ifdef 	KAF1603	
// KAF-1603 in APOGEE Alta F2
#define NCOL 1536
#define NROW 1024
#define NBIAS 0
#define NSPE 3145728
#define XYRAT 1.
#define PRETIME 2
#define READTIME 2
int offset = 0;
int tvflip = 1;
#endif

#ifdef AP7P
// Apogee AP7P
#define NCOL 525
#define NROW 512
#define NBIAS 10
#define NSPE 566048
#define XYRAT 1.
int offset = 0;
#define DTIME 2
#define PRETIME 2
#define READTIME 2
#define HAVEDARK
unsigned char darkdata[NSPE+2];
#define HAVEFLAT
float flatdata[1080960];
int tvflip = 1;
#endif

#ifdef E2V2K
// E2V 2048x2048
#define NCOL 2200
#define NROW 2048
#define NBIAS 50
#define NSPE 9680000
#define XYRAT 1.
int offset = 0;
#define DTIME 18
#define PRETIME 1
#define READTIME 27
#undef HAVEDARK
#undef HAVEFLAT
int tvflip = 1;
#endif

#ifdef E2V1K
// E2V 1024x1024
#define NCOL 1072
#define NROW 1024
#define NBIAS 48
//#define NSPE 2097152
#define NSPE 2195456
#define XYRAT 1.
int offset = 0;
#define DTIME 2
#define PRETIME 2
#define READTIME 2
#undef HAVEDARK
#undef HAVEFLAT
int tvflip = 1;
#endif

#ifdef HAVEPI
// Princeton CCD
#define NCOL 1050
#define NROW 1024
#define NBIAS 20
#define NSPE 2207000
#define XYRAT 1.
#define DTIME 34
#define PRETIME 34
#define READTIME 34
#undef HAVEDARK
int offset = 2000;
int tvflip = 0;
#endif

struct timeval starttime, time0;
struct timezone startzone, zone0;
unsigned short *imbuf = 0;

void inheadset(char *, int, char *);
void fheadset(char *, double, char *);
void lheadset(char *, int, char *);
void cheadset(char *, char *, char *);
int process_command(char *);
int process_exposure(unsigned short *,int, int, int, int);
#ifdef HAVEREMOTE
void record_command();
int remote_command(char *);
void send_message();
char *get_message();
char *getresp(FILE *);
#endif
int sendport(int, char *, char *, char *, int);
int display_command(char *, float *);
int misc_command(char *);
int camera_command(char *, unsigned short *);
void command_help();
#if defined(sun) || defined(linux)
int nint(float val);
#endif
void writeccdstatus(struct CCDSTATUS,char *,char *);
void myremove(char *);
void inittv();
void setup_server(int *sock,int port,char *server);
void setup_client(int *client,int port,char *server,int);
void writeterm(char *);
void readterm(char *);
void inttvbox(int *x0, int *y0, int *nx, int *ny);
void inttvcross(int *xc, int *yc);
float median(unsigned long, float *);
float variance(unsigned long, float *, double);
void update_status(int);
void readusno(char *, int, double, double, struct CCDSTATUS *);
void trap();
void window_resize();

void ccd_initialize(int);
void ccd_close();
int ccd_temp();

FILE *infile, *frommasterfp, *tomasterfp;
int idepth=0;
int havepipe=0;
FILE *outfile=NULL, *fguide=NULL;

struct sockstruct {
  int port;
  char server[24];
} totocc, toport;
int sport;

FILE *file;
char message[MAXCOMMAND];
#ifdef HAVEREMOTE
  #ifdef SPEC
  char *comfile = "/home/export/spec/speccmd.doc";
  char *comfilecheck = "/home/export/spec/speccmd.fin";
  char *respfile = "/home/export/spec/specresp.doc";
  char *respfilecheck = "/home/export/spec/specresp.fin";
  char *ccdtempfile = "/home/export/spec/temp.spe";
  #else
  char *comfile = "/home/export/pi/picmd.doc";
  char *comfilecheck = "/home/export/pi/picmd.fin";
  char *respfile = "/home/export/pi/pirespon.doc";
  char *respfilecheck = "/home/export/pi/pirespon.fin";
  char *ccdtempfile = "/home/export/pi/temp.spe";
  #endif
char *restart = "/home/export/spec/restart";
#endif
#ifdef GUIDER
  char *cstatusfile = "guidestatus.doc";
  char *cstatusreadyfile = "guidestatus.fin";
#else
#ifdef APOGEE
  char *cstatusfile = "accdstatus.doc";
  char *cstatusreadyfile = "accdstatus.fin";
#else
  char *cstatusfile = "ccdstatus.doc";
  char *cstatusreadyfile = "ccdstatus.fin";
#endif
#endif

#ifdef HAVETOCC
  char *tstatusfile = "/home/export/tocc/statr.doc";
  char *tstatusreadyfile = "/home/export/tocc/statr.doc";
#endif

FILE *lfile, *rfile, *cfile;
int ropen = 0;
int copen = 0;

#ifdef DISPLAY
extern int tvinit;
int scaletype = 2;
double dspan, dzero;
int ncol = NCOL ;
int nrow = NROW ;
float tvimage[NROW][NCOL];
int i, status;
#ifdef SLITVIEW
//double ccdscale=0.31;
double ccdxscale=0.298;
double ccdyscale=0.298;
#endif
#endif

char term[2];
double ccdmean, ccdbias, ccdsky, ccdmax, ccdmin, ccdvar;
int bell = 0;
int daytest = 0;

#if defined(SLITVIEW)
BOOL havetocc = FALSE;
#else
BOOL havetocc = TRUE;
#endif
BOOL repeat = FALSE;
BOOL fast = FALSE;
BOOL doalert = FALSE;

double **ifoc, **foc;
int calc_focus(double **, struct STATUS *);

char destdir[80], destmach[80], destacct[80];

#ifdef APOGEE
unsigned short *buf;
#endif
BOOL ctrlc=FALSE;

int client;
int dothresh=1;
long nguide = 0;
int nupdate = 0;
double badthresh = 0.5;

int guide_nsquare = 0;
double guide_squarestep = 0.;
int adj_nupdate = 0;
int adj_update = 0;
int adj_offset = 0;
int adj_flip = 0;
int adj_foc = 0;
int cur_foc = 0;
BOOL fochold = TRUE;
int doflush = 1;

main(argc, argv)
int argc;
char *argv[];
{
  double **junk;
  double vec[3], out[3], t;
  int status, n, nselect, maxset, sock, port, ts, nfail;
  char command[MAXCOMMAND], ans[10], server[24], *s;
  fd_set readfds;
  struct timeval timeout;

  // For slitview, use curses interface
  // Setup CTRL-C behavior
#ifdef SLITVIEW
  win_init();
  signal(SIGINT,trap);
  signal(SIGWINCH,window_resize);
#else
  signal(SIGINT,SIG_IGN);
#endif
  signal(SIGINT,trap);
 
  umask(000);
  sprintf(destdir,"/1m");
  sprintf(destmach,"astronomy.nmsu.edu");
  sprintf(destacct,"observe");

  fguide = fopen("guide.dat","a");

/* Invert the focus matrix */
  foc = (double **)malloc(3*sizeof(double *));
  ifoc = (double **)malloc(3*sizeof(double *));
  junk = (double **)malloc(3*sizeof(double *));
  setup_focus(foc);
  setup_focus(ifoc);
  setup_focus(junk);
  gaussj(ifoc,3,junk,3);

/* Open the socket for communication with UNIX command program, and telescope */
  writeterm("startup up ccd server socket...\n");
#ifndef SLITVIEW
  #ifdef LOCALCOM
    port = 0;
    #ifdef GUIDER
      sprintf(server,"togccd");
    #else
      sprintf(server,"toccd");
    #endif
    toport.port = 0;
    sprintf(toport.server,"toport");
  #else
fprintf(stderr,"gethost\n");
struct hostent *host = gethostbyname("ccd1m.apo.nmsu.edu");
fprintf(stderr,"done gethost\n");
//if (host==NULL) fprintf(stderr,"gethostname failed"); 
//else 
//printf( "address: %s ", inet_ntoa( *( struct in_addr*)( host -> h_addr_list[0])));

    #ifdef GUIDER
    port = 1049;
    sprintf(server,"ccd1m.apo.nmsu.edu");
    #else
    port = 1050;
    #ifdef APOGEE
    sprintf(server,"eyeball.apo.nmsu.edu");
    sprintf(server,"ccd1m.apo.nmsu.edu");
    #else
    sprintf(server,"ccd1m.apo.nmsu.edu");
    #endif
    #endif
    toport.port = 1053;
    sprintf(toport.server,"ccd1m.apo.nmsu.edu");
    totocc.port = 1053;
    sprintf(totocc.server,"ccd1m.apo.nmsu.edu");
  #endif
  printf("port: %d server:%s\n",port,server);
fprintf(stderr,"setup_server\n");
  setup_server(&sock,port,server);
fprintf(stderr,"done setup_server\n");
 
/* 
  #ifdef APOGEE 
    while (power_status(plug_status) < 0) {
      sleep(2);
    }
    if (plug_status[APOGEEPOW] == OFF) {
      fprintf(stderr,"Apogee power is OFF\n");
      fprintf(stderr,"Turning it on now...\n");
      power_on(APOGEEPOW);
      sleep(2);
    }    
  #endif
*/
#endif

#ifdef HAVEREMOTE
  file = fopen(restart,"r");
  if (file != NULL) {
      fclose(file);
      if (ropen == 1) {
        writeterm("closing rfile\n");
        fclose(rfile);
      }
      ropen = 0;
      remove(restart);
  }
#endif

/* Initialize CCD */
fprintf(stderr,"initialize\n");
  ccd_initialize(0);
fprintf(stderr,"done initialize\n");

/* Initialize some stuff */
  sprintf(ccdinfo.filename,"dummy");
#ifdef SLITVIEW
  sprintf(command,"FILTER 1");
  process_command(command);
#else
  sprintf(command,"FILTER 4");
  process_command(command);
  sprintf(command,"EXPOSURE 1.");
  process_command(command);
#endif
  writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
  sprintf(command,"INITCCD");
  process_command(command);
  sprintf(command,"FILLRESET");
  process_command(command);

  writeterm("Command (? for help): ");

/* Now we just sit and wait until we get a command to send */
  nselect = 0;

  while (1) {

/* Has the CCD program sent a message? If so, print it out */
    #ifdef HAVEREMOTE
    #ifdef NEWCOM
    while ((s=get_message())!=NULL) 
      writeterm(s);
    #else
    s=get_message();
    if (s != NULL) writeterm(s);
    #endif
    #endif

/* Update CCD temperature */
#ifndef FLI
    if (nselect==0) {
      sprintf(command,"CCDTEMP");
      process_command(command);
    }
#endif

#ifdef FLI
    if (doflush==1 && ccdinfo.guiding == 0) 
       status = ccd_flush(imbuf, ccdinfo.nx, 100);
#endif

    if (ctrlc) repeat=FALSE;
    ctrlc = FALSE;

    if (repeat) {
#ifndef SLITVIEW
      sprintf(command,"EXP %lf",ccdinfo.exposure*1000);
#else
      sprintf(command,"EXP %lf",ccdinfo.exposure);
#endif
      process_command(command);
    }

    if (ccdinfo.guiding == 1) {
      sprintf(command,"GUIDING");
      process_command(command);
    }

/*  Update information on telescope status */
#ifdef HAVETOCC
      // while (readstatus(tstatusfile, tstatusreadyfile, &telinfo) != 0) {} 
#endif

#ifdef VERBOSE
gettimeofday(&time0,&zone0);
fprintf(stderr,"select: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif

/* Check to see whether anything has come in on the fifos or STDIN
   If not, just wait */
    FD_ZERO(&readfds);
    FD_CLR(sock,&readfds);
    if (sock>0) FD_SET(sock,&readfds);
    FD_CLR(STDIN_FILENO,&readfds);
    FD_SET(STDIN_FILENO,&readfds);
#ifdef DISPLAY
    if (tvinit==1) {
      FD_CLR(ConnectionNumber(dpy),&readfds);
      FD_SET(ConnectionNumber(dpy),&readfds);
    }
#endif
    if (repeat || ccdinfo.guiding == 1)
      timeout.tv_sec = 0;
    else
      timeout.tv_sec = 5;
    timeout.tv_usec = 100000;

#ifdef linux
    maxset = 0;
    maxset = sock+1 > maxset ? sock+1 : maxset ;
    #ifdef DISPLAY
    if (tvinit==1) 
      maxset = 
       ConnectionNumber(dpy)+1 > maxset ? ConnectionNumber(dpy)+1 : maxset ;
    #endif
    maxset = STDIN_FILENO+1 > maxset ? STDIN_FILENO+1 : maxset ;
    if ((nselect=select(maxset,&readfds,NULL,NULL,&timeout))>0 ) {
#else
    if (select(FD_SETSIZE,&readfds,NULL,NULL,&timeout)>0 ) {
#endif

#ifdef VERBOSE
gettimeofday(&time0,&zone0);
fprintf(stderr,"done select: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif

/* Process a keyboard command */
      if (FD_ISSET(STDIN_FILENO,&readfds)) {
        readterm(command);
        status = process_command(command);
        writeterm("Command (? for help): ");
      }

/* Process command from command pipe */
      else if (FD_ISSET(sock,&readfds)) {
        read_server(&sock,&client,port,server,command,sizeof(command));
        /* Echo command */
        sprintf(message,"%s\n",command);
        writeterm(message);
        status = process_command(command);
        if (status == 0) 
          write (client,"DONE: 0\n",8);
        else {
#ifdef GUIDER
          sprintf(command,"DONE: %3d\n",100+status);
#else
          sprintf(command,"DONE: %3d\n",200+status);
#endif
          write (client,command,10);
          writeterm("Command: ");
        }
        close(client);
      }

#ifdef DISPLAY
/* Process display input */
      else if (tvinit==1 && FD_ISSET(ConnectionNumber(dpy),&readfds)) {
        xtv_refresh();
      }
#endif
    }
  }
}

/* process_command takes care of actually processing commands.
   It also updates the CCD status information depending
   on the command given. It also will handling writing/displaying image if
   command involves an exposure */
int process_command(char *command)
{
  char message[MAXCOMMAND];
  char com1[80], com2[80], newcom[80], ret[80];
  char *s;
  int status, n, i,j, ifile, nfail;
  BOOL is_exposure;
  size_t nin = NSPE;
  //unsigned char ccddata[NSPE+2];
  unsigned short *ccddata;
#ifdef DISPLAY
  float *fdata, amin, amax;
#endif
  double junk;
  unsigned short *data;

  ccddata = imbuf;

/* split up command into words */
   com1[0] = 0;
   com2[0] = 0;
   sscanf(command,"%s%s",com1, com2);
   strupr(com1);

/* If command was for an exposure, then query the telescope status file
         for telescope information to go into fits header */
   is_exposure = FALSE;
   #ifdef HAVEPI
   if (strcmp(com1,"COLLECTDATA") == 0) {
   #else
   if (strcmp(com1,"EXP") == 0 || strcmp(com1,"EXPGUIDE") == 0 || 
       strcmp(com1,"DARK") == 0) {
#ifdef VERBOSE
         gettimeofday(&time0,&zone0);
         fprintf(stderr,"preprocess EXP: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
         n=sscanf(com2,"%lf",&ccdinfo.exposure);
         #ifndef SLITVIEW
         if (n>0) ccdinfo.exposure /= 1000.;
         #endif
	 if (!fast) usleep(1000000);
   #endif
         is_exposure = TRUE;
         ccdinfo.expstatus = 1;
         #ifdef HAVETOCC
         nfail=0;
#ifdef VERBOSE
         gettimeofday(&time0,&zone0);
         fprintf(stderr,"getstatus: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
         fprintf(stderr,"totocc.port: %d totocc.server: %s\n",totocc.port,totocc.server);
#endif
#ifdef SOCKET
         while (havetocc && nfail<100 &&
                getstatus(totocc.port, totocc.server, &telinfo) != 0) {
#else
         while (havetocc && nfail<100 &&
                readstatus(tstatusfile, tstatusreadyfile, &telinfo) != 0) {
#endif
#ifdef VERBOSE
         gettimeofday(&time0,&zone0);
         fprintf(stderr,"donegetstatus: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
           fprintf(stderr,"x");
           nfail++;
         }
         if (nfail>=100) telinfo.current_utc=-99;
         #endif
         #ifdef SLITVIEW
         ccdinfo.end_time = time(NULL) + ccdinfo.exposure;
         #else
         ccdinfo.end_time = telinfo.current_utc + ccdinfo.exposure/24./3600.;
         //ccdinfo.end_time += 2*DTIME/3600./24.;
         ccdinfo.end_time += (PRETIME+READTIME)/3600./24.;
         #endif
         #ifdef HAVEPI
	 ccdinfo.ccd_temp = telinfo.ccd_temp;
         #endif
         sprintf(newcom,"CCDTEMP");
         if (!fast) process_command(newcom);
         writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
#ifdef VERBOSE
gettimeofday(&time0,&zone0);
fprintf(stderr,"done preprocess EXP: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
   } 

/* Process any misc commands - these don't require CCD communication. If
   command is recognized, misc_command returns 0 if no error, >0 if error.
   If command is unrecognized by misc_command, -1 is returned and we proceed 
   on. 
*/
   status = misc_command(command);
   if (status>=0) return(status);

#ifdef DISPLAY
/* Process any display commands - these don't require CCD communication */
   fdata = (float *)tvimage;
   status = display_command(command,fdata);
   if (status>=0) return(status);
#endif

/* Process any direct CCD commands */
#ifdef VERBOSE
gettimeofday(&time0,&zone0);
fprintf(stderr,"process camera_command: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
   status = camera_command(command, ccddata);
   if (status>=0) return(status);

/* If we are here, we have a command to send to remote computer */
#ifdef HAVEREMOTE
   status = remote_command(command);
   if (status>0) return(status);

/* Command has returned successfully!
   Update the ccdinfo block and output the new status */
   record_command(command);

   if (is_exposure) {
     /* open the input SPE file */
     if ((ifile=open(ccdtempfile,O_RDWR)) <= 0) {
       writeterm("error opening input temp.spe file\n");
       return(-1);
     }
 
     /* read the input SPE file */
     if (read(ifile,ccddata,nin)==-1)
           perror("read error");
     close(ifile);
     #ifdef SPEC
     remove(ccdtempfile);
     #endif
     status = process_exposure(ccddata,ccdinfo.x0,ccdinfo.nx,ccdinfo.y0,ccdinfo.ny);
   }
   return(status);
#endif
   if (strlen(command) > 0) {
     sprintf(message,"unknown command: %s\n", command);
     writeterm(message);
   }
   return(0);

}

#ifdef HAVEREMOTE
void record_command(char *command)
{
  char com1[80], com2[80], newcom[80];

  com1[0]=0;
  com2[0]=0;
  sscanf(command,"%s%s",com1, com2);
  strupr(com1);

  readccdstatus(cstatusfile,cstatusreadyfile,&ccdinfo);
  if (strcmp(com1,"CLEANS") == 0) 
    sscanf(com2,"%d",&ccdinfo.cleans);
  else if (strcmp(com1,"EXPOSURE") == 0)  
    sscanf(com2,"%lf",&ccdinfo.exposure);
  else if (strcmp(com1,"FILTER") == 0)  
    sscanf(com2,"%d",&ccdinfo.filter);
  else if (strcmp(com1,"FILTFOC") == 0) 
    sscanf(com2,"%d",&ccdinfo.filtfoc);
  else if (strcmp(com1,"NUMSEQ") == 0) 
    sscanf(com2,"%d",&ccdinfo.numseq);
  else if (strcmp(com1,"SETINCVAL") == 0) 
    sscanf(com2,"%d",&ccdinfo.incval);
  else if (strcmp(com1,"SHUTTERCMD") == 0) 
    sscanf(com2,"%d",&ccdinfo.shutter);
  else if (strcmp(com1,"NAME") == 0) 
    sscanf(com2,"%s",ccdinfo.filename);
  else if (strcmp(com1,"EXPOSURE") == 0) 
    sscanf(com2,"%lf",&ccdinfo.exposure);
  else if (strcmp(com1,"UPDATE") == 0) 
    sscanf(com2,"%d",&ccdinfo.guide_update);
  else if (strcmp(com1,"SIZE") == 0) 
    sscanf(com2,"%d",&ccdinfo.guide_size);
  else if (strcmp(com1,"FO") == 0)  {
    sscanf(com2,"%lf",&ccdinfo.focus);
  } else if (strcmp(com1,"GUIDELOC") == 0)  {
    sscanf(com2,"%lf",&ccdinfo.cy);
    sprintf(message,"Issue: SETINST 2 %f %f %f %f %f\n",
            ccdinfo.sx, ccdinfo.sy, ccdinfo.cx, ccdinfo.cy, ccdinfo.theta);
    writeterm(message);
    sprintf(newcom,"SETINST 2 %f %f %f %f %f\n",
            ccdinfo.sx, ccdinfo.sy, ccdinfo.cx, ccdinfo.cy, ccdinfo.theta);
    if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
  }
  else if (strcmp(com1,"GUIDEHOME") == 0)  {
    sprintf(message,"Issue: SETINST 2 %f %f %f %f %f\n",
         ccdinfo.sx, ccdinfo.sy, ccdinfo.cx, x_home_pos_arcsec, ccdinfo.theta);
    writeterm(message);
    sprintf(newcom,"SETINST 2 %f %f %f %f %f\n",
         ccdinfo.sx, ccdinfo.sy, ccdinfo.cx, x_home_pos_arcsec, ccdinfo.theta);
    if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
  }
  else if (strcmp(com1,"GUIDEOFF") == 0) 
    ccdinfo.guiding = 0;
  writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
}
#endif

int process_exposure(unsigned short *ccddata, int sc, int ncol, int sr, int nrow )
{
  char command[200];
  int ifile,i,j;
#ifdef DISPLAY
  float *fdata, amin, amax;
#endif
  FILE *fp;
  static int image=0;
  /* Write out FITS image */
  ccdinfo.expstatus = 2;
#ifdef VERBOSE
  gettimeofday(&time0,&zone0);
  fprintf(stderr,"writefits: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
  writefits(ccddata,&ccdinfo,&telinfo,sc,ncol,sr,nrow);
#ifdef VERBOSE
  gettimeofday(&time0,&zone0);
  fprintf(stderr,"done writefits: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
  writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);

  /* Transfer file if necessary */
  if (ccdinfo.autoxfer) {
     /* Copy the file */
#ifdef SLITVIEW
     sprintf(command, "scp %s/%s.%04d.fits %s@%s:%s/%s &",
               ccdinfo.dirname,ccdinfo.filename,ccdinfo.incval,
               destacct, destmach, destdir,ccdinfo.dirname);
#else
     sprintf(command, "scp %s/%s.%03d.fits %s@%s:%s/%s &",
               ccdinfo.dirname,ccdinfo.filename,ccdinfo.incval,
               destacct, destmach, destdir,ccdinfo.dirname);
#endif
     sprintf(message,"%s\n",command);
     writeterm(message);
     system(command);

  }

#ifdef DISPLAY
  if (ccdinfo.autodisplay) {
    /* Convert image to floating point, and compute bias and mean in process */
    fdata = (float *)tvimage;
    convert_image(ccddata, fdata, nrow, ncol, &amin, &amax, &ccdinfo);
  
    /* Display image */
    ccdinfo.expstatus = 3;
    writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);

    if (tvinit==0) inittv();

    if (scaletype == 0) {
            dspan = amax-amin;
            dzero = amin;
    } else if (scaletype == 1) {
	    dspan = MAX(50.,4.*ccdmean);
            dzero = ccdbias - 25;
	    dspan = MAX(50.,4.*(ccdsky-ccdbias));
            dzero = ccdsky - 25;
    } else if (scaletype == 2) {
	    dspan = 200.;
            dzero = ccdsky - 50;
	    dspan = MAX(50.,10*ccdvar);
            dzero = ccdsky - 3*ccdvar;
    } else {
            sprintf(message,"dzero: %lf  dspan: %lf  scaletype: %d\n",
                    dzero,dspan,scaletype);
            writeterm(message);
    }
    fdata = (float *)tvimage;
    if (tvinit==1) {
            fprintf(stderr,"tvload\n");
            tvload(fdata,nrow,ncol,ncol,0,0,sr,sc,
               dspan,dzero,tvflip,1,0);
            fprintf(stderr,"tvimnum\n");
            tvimnum("IMAGE",ccdinfo.incval);
            fprintf(stderr,"updatename\n");
            updatename(ccdinfo.object,strlen(ccdinfo.object));
    }
#ifndef SLITVIEW
#ifdef WEBVIEW
    sprintf(command, "fitstopnm -min %f -max %f %s/%s.%03d.fits > /export/tocc/image.pnm ",
            dzero,dzero+dspan,
            ccdinfo.dirname,ccdinfo.filename,ccdinfo.incval);
    fprintf(stderr,"%s\n",command);
    system(command);
    sprintf(command,"convert /export/tocc/image.pnm /export/tocc/image%03d.jpg ",ccdinfo.incval);
    fprintf(stderr,"%s\n",command);
    system(command);
    sprintf(command,"ln -s /export/tocc/image%03d.jpg /home/httpd/html/1mcontrol/image%03d.jpg",ccdinfo.incval,ccdinfo.incval);
    fprintf(stderr,"%s\n",command);
    system(command);
    fp = fopen("/export/tocc/image.html","w");
    //fprintf(fp," <HTML><BODY> <meta http-equiv=Refresh content=5>\n");
    fprintf(fp," <HTML><BODY>\n");
    fprintf(fp," <CENTER> %s/%s.%03d.fits </center>\n",
            ccdinfo.dirname,ccdinfo.filename, ccdinfo.incval);
    fprintf(fp," <img src=http://control1m.apo.nmsu.edu/1mcontrol/image%03d.jpg width=100%>\n",ccdinfo.incval);
    fprintf(fp," </BODY></HTML>\n");
    fclose(fp);
    image = ++image%2;
#endif
#endif

  }
#endif

  if (ccdinfo.filetype > 0) {
#ifdef HAVELOG
#ifdef VERBOSE
gettimeofday(&time0,&zone0);
fprintf(stderr,"writelog: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
    writelog(&ccdinfo,&telinfo);
fprintf(stderr,"done writelog: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
    ccdinfo.incval++;
  }
  writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
  if (bell) printf("%c%c",7,7);

#ifdef VERBOSE
gettimeofday(&time0,&zone0);
fprintf(stderr,"done process_exposures: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
  ccdinfo.expstatus = 0;
  return(0);
}

#define NHEADER 5760

/* function to write a FITS file given a .spe file and structures with CCD
   and telescope status information. Also loads data into floating point array*/
writefits(unsigned char *ccddata, struct CCDSTATUS *ccdinfo, struct STATUS *G, int sc, int ncol, int sr, int nrow)
{
      int status;
      char header[NHEADER];	
      int ifile, i, j;
      char *c;
      unsigned short *sdata;
      char file[80];
      char buf[64];
      double myut, ut, lst, s;
      int h, m, sign;
      struct tm *utd;
      time_t ut0;
      double el, az, pa, eld, azd, pad, eldd, azdd, padd, ha, secz;

      double latitude = 0.572124 ;
      double longitude = -1.846907 ;

    /* set up the FITS header */
      memset(header,(int)' ',NHEADER);
      sprintf(header,"END ");
      header[4] = ' ';
      header[NHEADER-1] = '\0';
      lheadset("SIMPLE",TRUE,header);
      inheadset("BITPIX",16,header);
      inheadset("NAXIS",2,header);
      inheadset("NAXIS1",ncol,header);
      inheadset("NAXIS2",nrow,header);
      inheadset("CRVAL1",sc,header);
      inheadset("CRVAL2",sr,header);
      inheadset("CDELT1",1,header);
      inheadset("CDELT2",1,header);
      fheadset("EXPOSURE",ccdinfo->exposure,header);
      fheadset("CLEANS",ccdinfo->cleans,header);
      //inheadset("FILTER",ccdinfo->filter,header);
      inheadset("FILTER",G->guider_filtpos,header);
    long guider_x_pos;
    long guider_y_pos;
    long guider_z_pos;
    int  guider_filtpos;

      cheadset("FILTNAME",longfiltname[G->guider_filtpos],header);
      if (ccdinfo->shutter == 0)
        cheadset("IMAGTYPE","DARK",header);
      else
        cheadset("IMAGTYPE","OBJECT",header);
      inheadset("OBSNUM",ccdinfo->incval,header);

      gethms(G->current_obs_ra*DR2H,&h,&m,&s,&sign);
      sprintf(buf,"%02d:%02d:%04.1f",h,m,s);
      cheadset("RA",buf,header);

      getdms(fabs(G->current_obs_dec*DR2D),&h,&m,&s,&sign);
      if (G->current_obs_dec >= 0)
        sprintf(buf,"%02d:%02d:%04.1f",h,m,s);
      else
        sprintf(buf,"-%02d:%02d:%04.1f",h,m,s);
      cheadset("DEC",buf,header);

      fheadset("EPOCH",G->current_mean_epoch,header);
      fheadset("PA",G->current_pa*DR2D,header);

      ha = slaDrange(G->current_lasth*DH2R - G->current_obs_ra)*DR2H;
      gethms(fabs(ha),&h,&m,&s,&sign);
      if (ha >= 0)
        sprintf(buf,"%02d:%02d:%04.1f",h,m,s);
      else
        sprintf(buf,"-%02d:%02d:%04.1f",h,m,s);
      cheadset("HA",buf,header);

  /* for the UT and LST, add DTIME seconds */
      //ut = G->current_utc + DTIME/3600./24.;
      ut = G->current_utc + PRETIME/3600./24.;
    fprintf(stderr,"telescope UT: %f ",ut);
      ut = (starttime.tv_sec+starttime.tv_usec*1.e-6)/24./3600.+40587;
    fprintf(stderr,"  UNIX UT: %f\n",ut);
#ifdef SLITVIEW
      time(&ut0);
      utd = gmtime(&ut0);
      ut = ut0/24./3600.+40587;
#endif
      fheadset("UTC",ut,header);
      gethms((ut - (long)ut)*24,&h,&m,&s,&sign);
      sprintf(buf,"%02d:%02d:%05.2f",h,m,s);
      cheadset("UT",buf,header);

  /* convert the UT into a DATE-OBS string */
      ut0 = (ut - 40587.)*24*3600.;
      utd = gmtime(&ut0);
    /*  sprintf(buf,"%02d/%02d/%02d",utd->tm_mday,utd->tm_mon+1,utd->tm_year);*/
     sprintf(buf,"%04d-%02d-%02d",utd->tm_year+1900,utd->tm_mon+1,utd->tm_mday);
      cheadset("DATE-OBS",buf,header);

   /* Sidereal time */
      //lst = G->current_lasth + DTIME/3600.;
      lst = G->current_lasth + PRETIME/3600.;
      gethms(lst,&h,&m,&s,&sign);
      sprintf(buf,"%02d:%02d:%02d",h,m,(int)s);
      cheadset("LST",buf,header);

   /* Airmass calculation */
      slaAltaz(ha*DH2R,G->current_obs_dec,latitude,
        &az,&azd,&azdd,&el,&eld,&eldd,&pa,&pad,&padd);
      secz = 1.0/cos(DPIBY2-el);
      G->airmass=secz+(secz-1.0)*(-0.0018167+(secz-1.0)*
          (-0.002875-0.0008083*(secz-1.0)));
      fheadset("AIRMASS",G->airmass,header);

   /* put az-el-phi in header */
      slaAltaz((lst*DH2R-G->current_obs_ra),G->current_obs_dec,latitude,
          &az,&azd,&azdd,&el,&eld,&eldd,&pa,&pad,&padd);
      fheadset("AZ",az*DR2D,header);
      fheadset("ALT",el*DR2D,header);
      fheadset("PARANG",pa*DR2D,header);
      fheadset("OBS_AZ",G->current_obs_az,header);
      fheadset("OBS_ALT",G->current_obs_alt,header);
      fheadset("OBS_ROT",G->current_obs_rot,header);

    /* put in object name */
      cheadset("OBJECT",ccdinfo->object,header);

    /* put in object number if not zero */
      if (ccdinfo->objnum != 0) 
        inheadset("OBJNUM",ccdinfo->objnum,header);
      if (ccdinfo->stannum != 0) 
        inheadset("STANNUM",ccdinfo->stannum,header);

    /* put in temperature */
      fheadset("OUT_TEMP",G->current_out_temp,header);     
      fheadset("CAB_TEMP",G->current_cab_temp,header);     
      fheadset("AUX_TEMP",G->current_aux_temp,header);     
      inheadset("WIND_DIR",G->current_winddir,header);     
      inheadset("WIND_VEL",G->current_windspeed,header);     
      fheadset("CCD_TEMP",ccdinfo->ccd_temp,header);     
   
    /* put in secondary information */
      calc_focus(ifoc,G);
      fheadset("FOCUS",G->foc-(ccdinfo->filtfoc),header);
      fheadset("FILTFOC",ccdinfo->filtfoc,header);
      fheadset("X_TILT",G->foc_theta,header);
      fheadset("Y_TILT",G->foc_phi,header);
      fheadset("RAWFOCUS",G->foc,header);
      inheadset("T_POS",G->t_step_pos,header);
      inheadset("U_POS",G->u_step_pos,header);
      inheadset("V_POS",G->v_step_pos,header);

    /* put in mount correction information */
      inheadset("MC_ENAB",G->mc_enabled,header);
      cheadset("MC_FILE",G->mc_file,header);

    /* Guider information */
      fheadset("GUIDELOC",ccdinfo->cy,header);
      fheadset("GUIDEFOC",ccdinfo->focus,header);
      ccdinfo->focus = G->foc-(ccdinfo->filtfoc);
      inheadset("G_XPOS",G->guider_x_pos,header);
      inheadset("G_YPOS",G->guider_y_pos,header);
      inheadset("G_ZPOS",G->guider_z_pos,header);

    /* open the output FITS file */
#ifdef SLITVIEW
      sprintf(file,"%s/%s.%04d.fits",ccdinfo->dirname,ccdinfo->filename,ccdinfo->incval); 
#else
      if (ccdinfo->incval<1000)
        sprintf(file,"%s/%s.%03d.fits",ccdinfo->dirname,ccdinfo->filename,ccdinfo->incval); 
      else
        sprintf(file,"%s/%s.%04d.fits",ccdinfo->dirname,ccdinfo->filename,ccdinfo->incval); 
#endif
      if ((ifile=open(file,O_CREAT | O_WRONLY, 0666)) <= 0) {
        sprintf(message, "error opening output fits file: %s %d\n", file,ifile);
        writeterm(message);
	perror(" ");
        return(-1);
      }

#if defined(APOGEE) || defined(LEACH)
      fheadset("BZERO",32768.,header);
      fheadset("BSCALE",1.,header);
//      sdata = (short *)ccddata+offset;
//      for (i=0; i<nrow*ncol; i++) *sdata++ -= 32768;
#ifdef LEACH
      sdata = (short *)ccddata+offset;
      for (i=0; i<nrow*ncol; i++) *sdata++ ^= 0x8000;
#endif
#endif

    /* byte swap the data */
//fprintf(stderr,"packfit\n");
      packfit(ccddata+offset,ccddata+offset,nrow*ncol*2);
//fprintf(stderr,"back packfit\n");

    /* write the FITS header */
      header[NHEADER-1] = ' ';
      write(ifile,header,NHEADER);
    /* write the data */
      write(ifile,ccddata+offset,nrow*ncol*2);

    /* pad to fill the FITS record */
      write(ifile,ccddata,960);
      close(ifile); 
 
#ifndef MSBF
    /* byte swap the data back into native machine byte order if necessary */
//fprintf(stderr,"packfit\n");
      packfit(ccddata+offset,ccddata+offset,nrow*ncol*2);
//fprintf(stderr,"back packfit\n");
#endif 
#ifdef LEACH
      sdata = (short *)ccddata+offset;
      for (i=0; i<nrow*ncol; i++) *sdata++ ^= 0x8000;
#endif
}

#ifdef DISPLAY 
convert_image(unsigned char *ccddata, float *fdata, int nrow, int ncol, 
              float *amin, float *amax, struct CCDSTATUS *ccdinfo)
{
      #if defined(APOGEE)||defined(LEACH)
      unsigned short *sdata;
      unsigned short *ddata;
      #else
      short *sdata;
      short *ddata;
      #endif
      float *pflat;
      int nmean, nbias, i, j;	 
//      float temp[NROW*NCOL], *t;
      float *temp, *t;
      double ccdbias1;

      temp = (float *)malloc(NROW*NCOL*4);

    /* fill in floating data */
      sdata = (short *)(ccddata+offset);
#ifdef HAVEDARK
      ddata = (short *)(darkdata+offset);
#endif
#ifdef HAVEFLAT
      pflat = flatdata;
#endif
      *amax = -32768;
      *amin = 32767;
      ccdmean = 0;
      ccdbias = 0;
      ccdbias1 = 0;
      nmean = 0;
      nbias = 0;
      t = temp;
#ifdef HAVEFLAT
      if (ccdinfo->autoflat && !ccdinfo->autodark) {
        for ( i=0;  i<nrow; i++){
          for (j=0; j<ncol; j++) {
            if (j>(ncol-ccdinfo->nbias)) {
              ccdbias1 += (float)*sdata;
              nbias++;
            }
            sdata++;
          }
        }
        ccdbias1/=nbias;
        nbias = 0;
        sdata = (short *)(ccddata+offset);
      }
#endif
      for ( i=0;  i<nrow; i++){
        for (j=0; j<ncol; j++) {
          *amin = MIN(*amin,*sdata);
          *amax = MAX(*amax,*sdata);
          #ifdef AP7P
          *fdata = (float)((*sdata++)^0x8000);
          #else
          *fdata = (float)(*sdata++);
          #endif
          #ifdef HAVEDARK
          if (ccdinfo->autodark && ccdinfo->darkexposure == ccdinfo->exposure) *fdata -= *ddata++; 
          #endif
          #ifdef HAVEFLAT
          if (ccdinfo->autoflat) {
            if (!ccdinfo->autodark) *fdata -= ccdbias1;
            if (j<(ncol-ccdinfo->nbias)) *fdata /=*pflat;
            pflat++;
          }
          #endif

          if (i>nrow/4 && i < 3*nrow/4 && j>ncol/4 && j<3*ncol/4) {
            ccdmean += (double)(*fdata);
            nmean++;
            *t++ = (double)(*fdata);
          } 
          if (j>(ncol-ccdinfo->nbias) && j<ncol) {
            ccdbias += (double)(*fdata);
            nbias++;
          }
          fdata++;
        }
      }
      if (nbias>0) ccdbias /= nbias;
      ccdmean /= nmean;
      ccdmean -= ccdbias;
      ccdsky = median(nmean,temp);
      ccdvar = variance(nmean,temp,ccdsky);
      free(temp);
      ccdmax = *amax;
      ccdmin= *amin;
      sprintf(message,"ccdmean: %8.1lf  ccdbias: %8.1lf ccdsky: %8.1lf ccdvar: %8.2lf\n",ccdmean,ccdbias,ccdsky,ccdvar);
      writeterm(message);
      write(client,message,strlen(message));

      return(0); 
}
#endif

/* misc_command process misc commands. It returns 0 if command was succesful,
   >0 if command was unsuccessful, and -1 if command was unrecognized */
int misc_command(char *inputline)
{ 
  char newcom[300], command[80];
  int nin,offset,ifile,n;
  FILE *fp;
  double ccdtemp;

  sscanf(inputline,"%s",command);
  strupr(command);

  if (strcmp(command,"QU") == 0 || strcmp(command,"QUIT") ==0 || strcmp(command,"EXIT") ==0) {
        ccd_close();
#ifdef SLITVIEW
        win_close();
#endif
        exit(0);
  }
  else if (strcmp(command,"HELP") ==0 || strcmp(command,"?") == 0) {
        command_help();
  }
  else if (strcmp(command,"REPEAT") ==0 || strcmp(command,"LOOP") == 0) {
        repeat = TRUE;
        return(0);
  }
  else if (strcmp(command,"STOP") ==0) {
        writeterm("auto-exposure loop will stop after next exposure is complete\n");
        repeat = FALSE;
        return(0);
  }
  else if (strcmp(command,"FITS") ==0 || strcmp(command,"+DISK")==0) {
        ccdinfo.filetype = 1;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"+ALERT")==0) {
        doalert = TRUE;
        return(0);
  }
  else if (strcmp(command,"-ALERT")==0) {
        doalert = FALSE;
        return(0);
  }
  else if (strcmp(command,"+DAYTEST")==0) {
        daytest = TRUE;
        return(0);
  }
  else if (strcmp(command,"-DAYTEST")==0) {
        daytest = FALSE;
        return(0);
  }
  else if (strcmp(command,"IRAF") ==0) {
        ccdinfo.filetype = 2;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"-DISK") ==0) {
        ccdinfo.filetype = 0;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"SDAS") ==0) {
        ccdinfo.filetype = 2;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"+PLANE") == 0) {
        dothresh=2;
        return(0);
  }
  else if (strcmp(command,"+THRESH") == 0) {
        dothresh=1;
        return(0);
  }
  else if (strcmp(command,"-THRESH") == 0) {
        dothresh=0;
        return(0);
  }
  else if (strcmp(command,"-XFER") == 0) {
        ccdinfo.autoxfer = FALSE;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"+XFER") == 0) {
        ccdinfo.autoxfer = TRUE;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"DESTMACH") == 0) {
        sprintf(destmach,"%s",inputline+9);
  }
  else if (strcmp(command,"DESTACCT") == 0) {
        sprintf(destacct,"%s",inputline+9);
  }
  else if (strcmp(command,"DESTDIR") == 0) {
        sprintf(destdir,"%s",inputline+9);
  }
  else if (strcmp(command,"-DARK") == 0) {
        ccdinfo.autodark = FALSE;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"+DARK") == 0) {
        ccdinfo.autodark = TRUE;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
#ifdef HAVEFLAT
  else if (strcmp(command,"-FLAT") == 0) {
        ccdinfo.autoflat = FALSE;
        return(0);
  }
  else if (strcmp(command,"+FLAT") == 0) {
        ccdinfo.autoflat = TRUE;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        if ((ifile=open("/home/arc/flat.fits",O_RDONLY)) <= 0) {
          writeterm("error opening input flat.fits file\n");
          return(1);
        }
        nin=525*512*4;
        offset=5760;
        status=0;
        if ((n=read(ifile,flatdata,offset))==-1) {
          perror("read error");
          status=1;
        }
        if ((n=read(ifile,flatdata,nin))==-1) {
          perror("read error");
          status=1;
        } else
          packfit4(flatdata,flatdata,525*512*4);
        close(ifile);
        return(status);
  }
#endif
  else if (strcmp(command,"AUTODARK") == 0) {
        strupr(inputline);
        if (strstr(inputline,"ON") != 0)
          ccdinfo.autodark = TRUE;
        else if (strstr(inputline,"OFF") != NULL)
          ccdinfo.autodark = FALSE;
        else {
          sprintf(message,"You must specify ON or OFF with AUTODARK command\n");
          writeterm(message);
        }
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"OBJECT") ==0) {
        memset(ccdinfo.object,0,NOBJECT);
        // Remember to strip off trailing CR
        strncpy(ccdinfo.object,inputline+7,strlen(inputline+7)-1);
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"OBJNUM") ==0) {
        sscanf(inputline+7,"%d",&ccdinfo.objnum);
        ccdinfo.stannum = 0;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"NEWEXT") == 0) {
        sscanf(inputline+7,"%d",&ccdinfo.incval);
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"SETINCVAL") == 0) {
        sscanf(inputline+10,"%d",&ccdinfo.incval);
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"NAME") == 0) {
        sscanf(inputline+5,"%s",ccdinfo.filename);
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"STANNUM") ==0) {
        sscanf(inputline+8,"%d",&ccdinfo.stannum);
        ccdinfo.objnum = 0;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"NOTOCC") ==0) {
        havetocc = FALSE;
        return(0);
  }
  else if (strcmp(command,"TOCC") ==0) {
        havetocc = TRUE;
        return(0);
  }
  else if (strcmp(command,"NOFAST") ==0) {
        fast = FALSE;
        return(0);
  }
  else if (strcmp(command,"FAST") ==0) {
        fast = TRUE;
        return(0);
  }
  else if (strcmp(command,"BELL") ==0 || strcmp(command,"BEEP") == 0) {
        if (strstr(inputline,"y") != NULL || strstr(inputline,"Y") !=NULL ||
            strstr(inputline,"ON") != NULL || strstr(inputline,"on") !=NULL)
           bell = 1;
        else if (strstr(inputline,"n") != NULL || strstr(inputline,"N") !=NULL ||
            strstr(inputline,"OFF") != NULL || strstr(inputline,"off") !=NULL)
           bell = 0;
        
        return(0);
  }
#ifndef GUIDER
// If the science camera gets a filter command, just change the internal 
// software setting,
// since the filter is actually changed through the guider computer. This
// assumes that the software will reliably always actually move the filter
// wheel successfully through the guider computer first!!
  else if (strcmp(command,"FILTER") ==0) {
        sscanf(inputline+7,"%d",&ccdinfo.filter);
	sprintf(message,"setting filter: %d\n",ccdinfo.filter);
        writeterm(message);
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
#endif
  else if (strcmp(command,"FILTFOC") ==0) {
        sscanf(inputline+8,"%d",&ccdinfo.filtfoc);
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  else if (strcmp(command,"INITCCD") ==0) {
//        sprintf(ccdinfo.filename,"junk");
        ccdinfo.x0 = 1;
        ccdinfo.y0 = 1;
        ccdinfo.nx = NCOL;
        ccdinfo.ny = NROW;
        ccdinfo.xc = (NCOL-NBIAS)/2.;
        ccdinfo.yc = NROW/2.;
        ccdinfo.nbias = NBIAS;
        ccdinfo.expstatus = 0;
        ccdinfo.end_time = 0;
        ccdinfo.filetype = 1;
        #ifdef SPEC
        ccdinfo.filetype = 0;
        #endif
        ccdinfo.autodisplay = TRUE;
        ccdinfo.offsettype = 1;
        ccdinfo.guiding = 0;
        ccdinfo.guide_update = 1;
        ccdinfo.guide_size = 15;
        memset(ccdinfo.object,0,NOBJECT);
        #ifdef HAVEDARK
        ccdinfo.autodark = FALSE;
        ccdinfo.darkexposure = -1;
        #endif
        #ifdef HAVEFLAT
        ccdinfo.autoflat = FALSE;
        #endif
        ccdinfo.autoxfer = FALSE;
        ccdinfo.ax = 0.812;
        ccdinfo.ay = 0.;
        ccdinfo.bx = 0.;
        ccdinfo.by = 0.812;
	ccdinfo.sx = -0.812;
	ccdinfo.sy = 0.812;
	ccdinfo.theta = 0.;
	ccdinfo.cx = -48;
	ccdinfo.cy = 20;
        #ifdef LYNXX
        ccdinfo.ax = -0.11;
        ccdinfo.bx = 0.525;
        ccdinfo.ay = -0.445;
        ccdinfo.by = -0.125;
        ccdinfo.sx =0.455;
        ccdinfo.sy =0.52;
        ccdinfo.theta = 90.8;
        ccdinfo.cx = -45;
        ccdinfo.cy = 900;
        ccdinfo.focus = 0;
        #endif
        #ifdef E2V1K
        ccdinfo.ax = 0.437;
        ccdinfo.ay = 0.0;
        ccdinfo.bx = 0.0;
        ccdinfo.by = -0.437;
        ccdinfo.sx =0.437;
        ccdinfo.sy =-0.437;
        ccdinfo.theta = 88.9;
        ccdinfo.cx = -45;
        ccdinfo.cy = 450;
        ccdinfo.focus = 0;
        #endif
        #ifdef SLITVIEW
        ccdinfo.xc = 198.;
        ccdinfo.yc = 296;
	fp = fopen("/home/arc/slitview.dat","r");
        if (fp != NULL) {
          fgets(message,MAXCOMMAND-1,fp);
          sscanf(message,"%lf%lf\n",&ccdinfo.xc,&ccdinfo.yc);
          fgets(message,MAXCOMMAND-1,fp);
          sscanf(message,"%lf\n",&ccdtemp);
          if (fgets(message,MAXCOMMAND-1,fp) != NULL)
            sscanf(message,"%lf%lf\n",&ccdxscale,&ccdyscale);
          fclose(fp);
          fprintf(stderr,"Using scales: %f %f \n", ccdxscale, ccdyscale);
          sprintf(message,"SETTEMP %lf\n",ccdtemp);
          process_command(message);
        }
        #endif
        #ifdef APOGEE
        ccdinfo.xc = 237.;
        ccdinfo.yc = 271;
        #endif

/* Initialize filter information */
        #ifdef SLITVIEW
	for (i=0; i<MAXFILT; i++) {
          sprintf(filtname[i]," ");
          sprintf(longfiltname[i],"None");
        }
        #else
        status = initfilt();
        if (status != 0) {
          writeterm("error initialing filter data!\n");
	  for (i=0; i<MAXFILT; i++) {
            sprintf(filtname[i]," ");
            sprintf(longfiltname[i],"None");
          }
        }
        #endif

        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        initccd();
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
  return(-1);
}

/* display_command process display commands. It returns 0 if command was 
   successful, >0 if command was unsuccessful, and -1 if command was 
   unrecognized */
#define MAXCMD 300
#define MAXARG 8
int display_command(char *inputline, float *fdata)
{
  double row, col, torow, tocol, offset, peak, total, fwhm;
  float rasec, decsec; 
  char newcom[80], command[300], file[80], ret[80];
  char sargv[MAXARG][MAXCMD];
  double d[6][3], x1[10], x2[10], y1[10], y2[10], xold, yold;
  int ix, iy, i, j, k, l, npar, istar, istat, flip, n, nfail, nskip;
  double **c, **v, xs, ys, omega1, omega2;
  static double flatexp = 0.5, fudge = 1.5;
  static int nflat = 3, maxtry = 10;
  #ifdef GUIDER
  static int inst = 2;
  #else
  #ifdef APOGEE
  static int inst = 3;
  #else
  static int inst = 1;
  #endif
  #endif
  #ifdef SLITVIEW
  int compute_offset();
  #endif

  double newflatexp=0., newfudge=0.;
  int newnflat=0, newmaxtry=0, ndone, ntry;

  sscanf(inputline,"%s",command);
  strupr(command);

#ifdef DISPLAY      
   /* First consider commands which do NOT actually communicate with the CCD */
  if (strcmp(command,"LOCATE")==0 || strcmp(command,"CENTER")==0) {	
        if (tvinit==0) return(COM_UNSUCCESSFUL);
        sprintf(message,
          "Hit I (cursor position) or C (centroid around cursor) on position to move from\n");
        writeterm(message);
        #ifdef SLITVIEW
        imageuninstallkey('c');
        imageuninstallkey('C');
        imageuninstallkey('i');
        imageuninstallkey('I');
        #endif
        write(client,message,strlen(message));
        status = getcentroid(fdata,&row,&col);
        if (status == 0) {
          if (strcmp(command,"LOCATE")==0) {
            sprintf(message,"Hit I or C on position you want to move to\n");
            writeterm(message);
            write(client,message,strlen(message));
            status = getcentroid(fdata,&torow,&tocol);
          }
          else {
            torow = ccdinfo.yc;
            tocol = ccdinfo.xc;
          }
        #ifdef SLITVIEW
          sprintf(newcom,"OFFSET %8.2f %8.2f\n",
              -(col-tocol)*ccdxscale,(row-torow)*ccdyscale);
          writeterm(newcom); 
        #else
          sprintf(newcom,"OFFSET %d %f %f",inst,col-tocol,row-torow);
          if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
          if (status != 0)
            status = COM_UNSUCCESSFUL;
        #endif
        } else {
          writeterm("Error computing centroid\n");
          status = COM_UNSUCCESSFUL;
        }
        #ifdef SLITVIEW
        imageinstallkey('c',0,compute_offset);
        imageinstallkey('C',0,compute_offset);
        imageinstallkey('i',0,compute_offset);
        imageinstallkey('I',0,compute_offset);
        #endif
        return(status);
  }

  else if (strcmp(command,"FIND")==0) {
        row = ccdinfo.y0+ccdinfo.ny/2;
        col = ccdinfo.x0+ccdinfo.nx/2;
        status = findstar(fdata,ccdinfo.x0,ccdinfo.x0+ccdinfo.nx-1,
                                ccdinfo.y0,ccdinfo.y0+ccdinfo.ny-1,
                                NROW/4,NROW*3/4,NCOL/4,NCOL*3/4,
                                ccdinfo.guide_size,
                                &row,&col,&peak,&total,&fwhm);
	sprintf(message,"found star: %d %f %f\n",status,row,col);
        writeterm(message);
  }

  else if (strcmp(command,"NEWCENT")==0) {
    for (i=0 ; i<strlen(inputline); i++)
      if (inputline[i] == ':' || inputline[i] == ',') inputline[i] = ' ';
        if ( (n=sscanf(inputline+7,"%lf %lf\n",&ccdinfo.xc,&ccdinfo.yc)) <= 0) {
          if (tvinit) {
            vecclear();
            inttvcross(&ix, &iy);
            imagecross(ix,iy);
            ccdinfo.xc = ix;
            ccdinfo.yc = iy;
          }
        }
        else if (n!=2) {
          sprintf(message,"You must enter either 0 or 2 arguments with NEWCENT\n");
          writeterm(message);
        }
        update_status(-1);
        return(0);
  }
  
  else if (strcmp(command,"INST")==0) {
        sscanf(inputline+4,"%d",&inst);
        return(0);
  }
  
  else if (strcmp(command,">GUIDE")==0 || strcmp(command,">CAMERA")==0 ) {
        if (tvinit==0)
          return(COM_UNSUCCESSFUL);
        if (getcentroid(fdata,&row,&col) == 0) {
          rasec = (ccdinfo.xc-col)*ccdinfo.ax + (ccdinfo.yc-row)*ccdinfo.bx;
          decsec = (ccdinfo.xc-col)*ccdinfo.ay + (ccdinfo.yc-row)*ccdinfo.by;
          if (strcmp(command,">GUIDE")==0) {
	    // rasec-=212;
	    rasec-=92;
            decsec-=1028;
          } else {
	    rasec+=212;
            decsec+=1028;
          }
          sprintf(message,"issue: QM %lf %lf\n",rasec,decsec);
          writeterm(message);
          write(client,message,strlen(message));
          sprintf(newcom,"XQM %f %f",rasec,decsec);
          status = 0;
          // if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
          if (status == 0)
            return(0);
          else
            return(COM_UNSUCCESSFUL);
        } else {
          writeterm("Error computing centroid\n");
          return(COM_UNSUCCESSFUL);
        }
        return(0);
  }

  else if (strcmp(command,"FULLSCALE") == 0) {
        scaletype = 0;
        dspan = ccdmax-ccdmin;
        dzero = ccdmin;
        resetcolors();
        tvload(fdata,nrow,ncol,ncol,0,0,ccdinfo.y0,ccdinfo.x0,
               dspan,dzero,tvflip,1,0);
        return(0);
  }

  else if (strcmp(command,"SKYSCALE") == 0) {
        scaletype = 1;
	dspan = MAX(50.,4.*ccdmean);
        dzero = ccdbias - 25;
        resetcolors();
        tvload(fdata,nrow,ncol,ncol,0,0,ccdinfo.y0,ccdinfo.x0,
               dspan,dzero,tvflip,1,0);
        return(0);
  }
  else if (strcmp(command,"NEWSCALE") == 0) {
        scaletype = 2;
	dspan = 100.;
        dzero = ccdsky - 25;
	dspan = MAX(50.,10*ccdvar);
        dzero = ccdsky - 3*ccdvar;
        resetcolors();
        tvload(fdata,nrow,ncol,ncol,0,0,ccdinfo.y0,ccdinfo.x0,
               dspan,dzero,tvflip,1,0);
        return(0);
  }
  else if (strcmp(command,"SAMESCALE") == 0) {
        scaletype = -1;
        return(0);
  }

  else if (strcmp(command,"-DISPLAY") == 0) {
        ccdinfo.autodisplay = FALSE;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }

  else if (strcmp(command,"+DISPLAY") == 0) {
        ccdinfo.autodisplay = TRUE;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }

  else if (strcmp(command,"+REMARK") == 0) {
        ccdinfo.offsettype |= 0x1;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }

  else if (strcmp(command,"-REMARK") == 0) {
        ccdinfo.offsettype &= 0xe;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }

  else if (strcmp(command,"+TUI") == 0) {
        ccdinfo.offsettype |= 0x2;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }

  else if (strcmp(command,"-TUI") == 0) {
        ccdinfo.offsettype &= 0xd;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }

  else if (strcmp(command,"SCALE") == 0) {
        if (tvinit==0)
          return(COM_UNSUCCESSFUL);
        n=sscanf(inputline+5,"%lf %lf\n",&dzero,&dspan);
        if (n==2) {
          dspan = dspan - dzero;
          resetcolors();
          tvload(fdata,nrow,ncol,ncol,0,0,ccdinfo.y0,ccdinfo.x0,
               dspan,dzero,tvflip,1,0);
        } else 
          writeterm("You must enter two parameters with SCALE command\n"); 
        return(0);
  }

  else if (strcmp(command,"FLAT")==0) {
        sprintf(ccdinfo.object,"Flat, filter %d",ccdinfo.filter);
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);

        sscanf(inputline+4,"%d %d %lf %lf",
               &newnflat,&newmaxtry,&newflatexp,&newfudge);
        if (newnflat > 0 ) nflat = newnflat;
        if (newmaxtry > 0 ) maxtry = newmaxtry;
        if (newflatexp > 0 ) flatexp = newflatexp;
        if (newfudge > 0 ) fudge = newfudge;
        ndone = 0;
        ntry = 0;
        while (ndone < nflat && ntry < maxtry && !ctrlc) {
          ntry++;
          sprintf(message,"ntry: %d  ndone:  %d  flatexp: %lf\n",
                  ntry, ndone, flatexp);
          writeterm(message);
#ifdef HAVEPI
          sprintf(newcom,"EXPOSURE %lf",flatexp);
          process_command(newcom);
          sprintf(newcom,"COLLECTDATA 0");
#else
          sprintf(newcom,"EXP %ld",(long)(flatexp*1000));
#endif
          process_command(newcom);

          // Increment the done counter if count level is acceptable or if
          //  we're at maximum exptime in the evening or
          //  we're at minimum exptime in the morning
          if ((flatexp>29&&fudge>1) ||
              (flatexp<1&&fudge<1) ||
              (flatexp >=1 && ccdmean > 5000 && ccdmean < 50000) ) ndone++;
          // If we're too bright and in the evening, wait 20 seconds before
          //  trying again, and decrement the file counter and ntry counter
          if ((ccdmean>50000||flatexp<1)&&fudge>1) {
              sleep(20);
              ccdinfo.incval--;
              ntry--;
          }
          flatexp = (int)(10 * flatexp * 10000./ccdmean * fudge) / 10.;
          flatexp = (flatexp > 30) ? 30 : flatexp;
          flatexp = (flatexp > 0.25 && flatexp < 1) ? 1. : flatexp;
          flatexp = (flatexp < 0.5) ? 0.5 : flatexp;
       
        // Offset the telescope
          sprintf(newcom,"XQM 0 30");
#ifdef HAVETOCC
          if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
          if (status != 0) return(COM_UNSUCCESSFUL);
          sleep(3);
#endif
        }
        ctrlc = FALSE;
        return(0);
  }

  else if (strcmp(command,"GOS") == 0 || strcmp(command,"INIT") == 0) {
        n=sscanf(inputline+4,"%lf\n",&offset);
        if (n!=1) offset=50;

        #if defined(APOGEE)||defined(E2V1K)
        flip = 1;
        #else
        flip = -1;
        #endif
        #ifdef HAVEPI
        sprintf(newcom,"COLLECTDATA 0");
        #else
        sprintf(newcom,"EXP %ld",(long)(ccdinfo.exposure*1000));
        #endif
        if (strcmp(command,"GOS") == 0) {
          process_command(newcom);
          if (getcentroid(fdata,&row,&col) == 0) {
            sprintf(message,"centroid: %f %f\n", row, col);
            writeterm(message);
            write(client,message,strlen(message));
          } else {
            sprintf(message,"Error computing centroid\n");
            writeterm(message);
            write(client,message,strlen(message));
            return(COM_UNSUCCESSFUL);
          }
        }
    // Do a couple of moves to figure out separate x and y scale
        #ifdef ROT
        npar = 4;
        #else
        npar = 6;
        #endif
        // Allocate matrices
        c = (double **)malloc(npar*sizeof(double *));
        for (i=0; i<npar; i++) 
          c[i] = (double *)malloc(npar*sizeof(double));
        v = (double **)malloc(npar*sizeof(double *));
        for (i=0; i<npar; i++) 
          v[i] = (double *)malloc(sizeof(double));

        // Initialize matrices
        for (j=0;j<npar;j++) {
          for (l=0;l<npar;l++) c[l][j] = 0.;
          v[j][0] = 0.;
        }

	xold = yold = 0;
        istar = 0;
        for (iy=-1; iy<=1; iy=iy+1) {
          for (ix=-1; ix<=1; ix=ix+1) {
            x1[istar] = ix*offset;
            y1[istar] = iy*offset;
            sprintf(newcom,"XQM %f %f", x1[istar]-xold, y1[istar]-yold);
#ifdef HAVETOCC
            if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
            // if (status != 0) return(COM_UNSUCCESSFUL);
            sleep(5);
#endif
            xold = x1[istar];
            yold = y1[istar];

            #ifdef HAVEPI
            sprintf(newcom,"COLLECTDATA 0");
            #else
            sprintf(newcom,"EXP %ld",(long)(ccdinfo.exposure*1000));
            #endif
#ifdef HAVECCD
            do {
             process_command(newcom);
             if ( strcmp(command,"GOS") == 0) {
               sprintf(message,"%d %f %f\n", 
                     findstar(fdata,ccdinfo.x0,ccdinfo.x0+ccdinfo.nx-1,
                                    ccdinfo.y0,ccdinfo.y0+ccdinfo.ny-1,
                                    NROW/4,NROW*3/4,NCOL/4,NCOL*3/4,
                                    ccdinfo.guide_size, 
                                    &row,&col,&peak,&total,&fwhm),row,col);
               writeterm(message);
               istat = getcentroid(fdata,&y2[istar],&x2[istar]);
             } else {
#ifdef LYNXX
               istat = findstar(fdata,ccdinfo.x0,ccdinfo.x0+ccdinfo.nx-1,
                                ccdinfo.y0,ccdinfo.y0+ccdinfo.ny-1,
                                NROW/8,NROW*7/8,NCOL/8,NCOL*7/8,
                                ccdinfo.guide_size,
                                &y2[istar],&x2[istar],&peak,&total,&fwhm);
#else
               istat = findstar(fdata,ccdinfo.x0,ccdinfo.x0+ccdinfo.nx-1,
                                ccdinfo.y0,ccdinfo.y0+ccdinfo.ny-1,
                                NROW/4,NROW*3/4,NCOL/4,NCOL*3/4,
                                ccdinfo.guide_size,
                                &y2[istar],&x2[istar],&peak,&total,&fwhm);
#endif
               istat=0;
             }
             if (istat == 0) {
              sprintf(message,"centroid: %f %f\n", x2[istar], y2[istar]);
              writeterm(message);
              write(client,message,strlen(message));
#else
            if (1) {
              y2[istar] = NROW/2 + y1[istar]/ccdinfo.by;
              x2[istar] = NCOL/2 + x1[istar]/ccdinfo.ax;
#endif
            // Load up basis vectors for full linear xformation
              d[0][1] = flip*x2[istar] *XYRAT;
              d[1][1] = y2[istar];
              d[2][1] = 1.;
              for (j=0;j<3;j++) {
                d[j][2] = 0.;
                d[j+3][1] = 0.;
                d[j+3][2] = d[j][1];
              }
            // If we only want to solve for rotation, modify the basis vectors
              #ifdef ROT
              d[0][2] = y2[istar];
              d[1][2] = -flip * x2[istar] *XYRAT ;
              d[3][2] = 1.;
              #endif
              // Fill in matrices
              for (l=0;l<npar;l++) {
                for (j=0;j<npar;j++) {
                  for (k=1;k<=2;k++) {
                    c[j][l] += d[l][k]*d[j][k];
                  }
                }
                v[l][0] += x1[istar]*d[l][1];
                v[l][0] += y1[istar]*d[l][2];
              }   
              istar++;
            } else {
              sprintf(message,"Error computing centroid\n");
              writeterm(message);
              write(client,message,strlen(message));
              // return(COM_UNSUCCESSFUL);
            }
           } while (istat !=0);
          }
        }
      for (i=0;i<npar;i++){
        for (j=0;j<npar;j++) {
          sprintf(message,"%f ",c[i][j]);
          writeterm(message);
        }
        writeterm("\n");
      }
      writeterm("\n");
      for (i=0;i<npar;i++) {
        sprintf(message,"%f ",v[i][0]);
        writeterm(message);
      }
        gaussj(c,npar,v,1);
        writeterm("done inverting... fit parameters:\n");
        for (i=0;i<npar;i++) {
          sprintf(message,"%d %f\n",i,v[i][0]);
          writeterm(message);
        }
        #ifdef ROT
        ccdinfo.ax = v[0][0];
        ccdinfo.bx = v[1][0];
        ccdinfo.ay = -v[1][0];
        ccdinfo.by = v[0][0];
        xs = sqrt(v[0][0]*v[0][0] + v[1][0]*v[1][0]);
        omega1 = acos(v[0][0]/xs) * 180./DPI;
        omega2 = atan2(v[1][0],v[0][0]) * 180./DPI ;
	ccdinfo.sx = flip*xs*XYRAT;
	ccdinfo.sy = xs;
	ccdinfo.theta = omega1;
        sprintf(message,"Scale x: %f  Scale y: %f Rotation: %f %f\n",
                flip*xs*XYRAT,xs,omega1, omega2);
        #else
        ccdinfo.ax = v[0][0];
        ccdinfo.bx = v[1][0];
        ccdinfo.ay = v[3][0];
        ccdinfo.by = v[4][0];
        xs = sqrt(v[0][0]*v[0][0]+v[3][0]*v[3][0]);
        ys = sqrt(v[1][0]*v[1][0]+v[4][0]*v[4][0]);
        omega1 = atan2(-ccdinfo.ax,ccdinfo.ay) * 180./DPI ;
        omega2 = atan2(ccdinfo.by,ccdinfo.bx) * 180./DPI ;
        sprintf(message, "xscale: %f  yscale: %f  xomega: %f  yomega: %f\n",
                  xs, ys, omega1, omega2);
        #endif
        writeterm(message);
        write(client,message,strlen(message));
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        // Residuals
        sprintf(message,"Residuals: \n");
        writeterm(message);
        write(client,message,strlen(message));
        for (i=0;i<istar;i++) {
          sprintf(message,"%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f\n",
              x1[i],y1[i],x2[i],y2[i],
        #ifdef ROT
              v[0][0]*flip*x2[i]*XYRAT+v[1][0]*y2[i]+v[2][0],
              -v[1][0]*flip*x2[i]*XYRAT+v[0][0]*y2[i]+v[3][0],
              v[0][0]*flip*x2[i]*XYRAT+v[1][0]*y2[i]+v[2][0]-x1[i],
              -v[1][0]*flip*x2[i]*XYRAT+v[0][0]*y2[i]+v[3][0]-y1[i]);
        #else
              v[0][0]*flip*x2[i]*XYRAT+v[1][0]*y2[i]+v[2][0],
              v[3][0]*flip*x2[i]*XYRAT+v[4][0]*y2[i]+v[5][0],
              v[0][0]*flip*x2[i]*XYRAT+v[1][0]*y2[i]+v[2][0]-x1[i],
              v[3][0]*flip*x2[i]*XYRAT+v[4][0]*y2[i]+v[5][0]-y1[i]);
        #endif
          writeterm(message);
          write(client,message,strlen(message));
        }
        for (i=0;i<npar;i++) {
          free(c[i]);
          free(v[i]);
        }
	free(c);
	free(v);
//  Determine cx, cy to put sky coordinates in center of chip
        #ifdef ROT
        ccdinfo.cx = -1*(v[0][0]*flip*(NCOL-NBIAS)/2.*XYRAT+v[1][0]*NROW/2.);
        ccdinfo.cy = -1*(-v[1][0]*flip*(NCOL-NBIAS)/2.*XYRAT+v[0][0]*NROW/2.);
        #endif
        return(0);
  }
  else if (strcmp(command,"INSTBLOCK") == 0 ) {
        double dra=0, ddec=0;
        n=sscanf(inputline+9,"%lf %lf\n",&dra, &ddec);
        // Allocate matrices
	npar=2;
        c = (double **)malloc(npar*sizeof(double *));
        for (i=0; i<npar; i++) 
          c[i] = (double *)malloc(npar*sizeof(double));
        v = (double **)malloc(npar*sizeof(double *));
        for (i=0; i<npar; i++) 
          v[i] = (double *)malloc(sizeof(double));

        c[0][0] = ccdinfo.ax;
        c[0][1] = ccdinfo.bx;
        c[1][0] = ccdinfo.ay;
        c[1][1] = ccdinfo.by;
        v[0][0] = dra;
        v[1][0] = ddec;
        gaussj(c,npar,v,1);
        fprintf(stderr,"rotator center: %lf %lf \n", v[0][0],v[1][0]);
        //printf("issue SETINST 2 %f %f %f %f %f\n",
        //        ccdinfo.sx,ccdinfo.sy,v[0][0],v[1][0],ccdinfo.theta);
        fprintf(stderr,"issue SETINST 2 %f %f %f %f %f\n",
                ccdinfo.sx,ccdinfo.sy,dra,-ddec,ccdinfo.theta);
        return(0);
  }

  else if (strcmp(command,"NEWFOC") == 0) {
        n=sscanf(inputline+6,"%lf%lf%d%lf%d%d",
            &ccdinfo.guide_x0,&ccdinfo.guide_y0,&ccdinfo.guide_size,&ccdinfo.exposure,&ccdinfo.guide_update,&adj_foc);
        ccdinfo.guiding = 1;
        adj_nupdate = -1*(ccdinfo.guide_update);
        adj_offset = 0;
        adj_flip = 0;
        cur_foc = 0;
        status=0;
        while (status==0 && ccdinfo.guiding > 0 && !ctrlc) {
          status = process_command("GUIDING");
        }

        return(0);
  }
  else if (strcmp(command,"OFFSET") == 0) {
        float dx, dy;
        n=sscanf(inputline+6,"%*d%f%f", &dx, &dy);
        if (n==2) {
          ccdinfo.guide_xoff += dx;
          ccdinfo.guide_yoff += dy;
        }
        return(0);
  }
  else if (strcmp(command,"NEWGUIDE")==0 || strcmp(command,"GUIDE") == 0 || strcmp(command,"SCIGUIDE") == 0 ) {
        n=sscanf(inputline,"%*s%lf%lf%d%lf%d", 
            &ccdinfo.guide_x0,&ccdinfo.guide_y0,
            &ccdinfo.guide_size,&ccdinfo.exposure,&ccdinfo.guide_update);
        if (n<2) {
          sprintf(newcom,"EXP %ld",(long)(ccdinfo.exposure*1000));
          process_command(newcom);
          if (tvinit==0)
            return(COM_UNSUCCESSFUL);
          if (getcentroid(fdata,&row,&col) == 0) {
            sprintf(message,"centroid (x,y): (%f,%f)\n", col, row);
            writeterm(message);
            write(client,message,strlen(message));
            //ccdinfo.guide_x0 = col;
            //ccdinfo.guide_y0 = row;
          } else {
            sprintf(message,"getcentroid failed\n");
            writeterm(message);
            write(client,message,strlen(message));
            return(COM_UNSUCCESSFUL);
          }
        }
        if (inst==3) {
          ccdinfo.guide_rot0 = telinfo.current_obs_rot-telinfo.last_rot_error;
          ccdinfo.guide_dist = sqrt(
                 pow(ccdinfo.guide_x0-ccdinfo.xc,2)+
                 pow(ccdinfo.guide_y0-ccdinfo.yc,2));
          ccdinfo.guide_theta0 = 
                 atan2(ccdinfo.guide_y0-ccdinfo.yc,ccdinfo.guide_x0-ccdinfo.xc);
        }
        if (strcmp(command,"GUIDE") == 0 || strcmp(command,"NEWGUIDE")==0)
          ccdinfo.guiding = 1;
        else
          ccdinfo.guiding = 9;
        ccdinfo.guide_xoff=ccdinfo.guide_yoff=0;
        adj_nupdate = 0;
        guide_nsquare = 0;
        nguide = 0;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
/*
  else if (strcmp(command,"NEWGUIDE") == 0) {
        n=sscanf(inputline+8,"%lf%lf%d%lf%d",
            &ccdinfo.guide_x0,&ccdinfo.guide_y0,
            &ccdinfo.guide_size,&ccdinfo.exposure,&ccdinfo.guide_update);
        ccdinfo.guiding = 1;
        ccdinfo.guide_xoff=ccdinfo.guide_yoff=0;
        adj_nupdate = 0;
        guide_nsquare = 0;
        nguide = 0;
        return(0);
  }
*/
  else if (strcmp(command,"SIZE") == 0) {
        n=sscanf(inputline+4,"%d", &ccdinfo.guide_size);
        return(0);
  }
  else if (strcmp(command,"TIME") == 0) {
        n=sscanf(inputline+4,"%lf",&ccdinfo.exposure);
        return(0);
  }
  else if (strcmp(command,"BADTHRESH") == 0) {
        n=sscanf(inputline+9,"%lf",&badthresh);
        return(0);
  }
  else if (strcmp(command,"UPDATE") == 0) {
        n=sscanf(inputline+6,"%d",&ccdinfo.guide_update);
        return(0);
  }
#ifdef FLI
  else if (strcmp(command,"FLUSH") == 0) {
        doflush = 1;
        return(0);
  }
  else if (strcmp(command,"NOFLUSH") == 0) {
        doflush = 0;
        return(0);
  }
  else if (strcmp(command,"OPEN") == 0) {
        fli_openshutter();
        return(0);
  }
  else if (strcmp(command,"CLOSE") == 0) {
        fli_closeshutter();
        return(0);
  }
#endif

#ifndef HAVEREMOTE
  else if (strcmp(command,"GUIDEOFF") == 0 ) {
        ccdinfo.guiding = 0;
        //adj_nupdate = 0;
        //adj_update = 0;
        //adj_offset = 0;
        //adj_foc = 0;
        //cur_foc = 0;
        return(0);
  }
  else if (strcmp(command,"GUIDEON") == 0 ) {
        nupdate = 0;
        nguide = 0;
        ccdinfo.guiding = 1;
  }
#endif
  else if (strcmp(command,"+FOCHOLD") == 0 ) {
        fochold = TRUE;
  }

  else if (strcmp(command,"-FOCHOLD") == 0 ) {
        fochold = FALSE;
  }

  else if (strcmp(command,"GUIDEADJ")==0) {
        sscanf(inputline+8,"%d %d",&adj_nupdate,&adj_foc);
        adj_update = 0;
        adj_offset = 0;
        adj_flip = 0;
        cur_foc = 0;
        return(0);
  }

  else if (strcmp(command,"GUIDESQUARE")==0) {
        sscanf(inputline+11,"%d%lf",&guide_nsquare,&guide_squarestep);
        return(0);
  }

  else if (strcmp(command,"GUIDING") == 0 || strcmp(command,"SCIGUIDING")==0 ) {
   // For GUIDING, take a guide exposure
   // For SCIGUIDING, just use the science exposure
        static float totsave[50];
        double dx, dy;
        static double totdx, totdy, tottot=0, meantot=0;
        static int adj_sign = 1;
        static double ratsum = 0;
        static double ratsum2 = 0;
        double avgrat, avgsig;
        static double avgold = 0;
        int xs, xe, ys, ye;
        BOOL wait = FALSE;
        FILE *fp;
        char *holdfile = "/home/export/tocc/fochold.doc";

        // Determine desired position for guide star
        if (inst==3) {
          double desired_rot;
          desired_rot = telinfo.current_obs_rot-telinfo.last_rot_error;
          ccdinfo.guide_x0 = ccdinfo.xc + ccdinfo.guide_dist * 
            cos(ccdinfo.guide_theta0 -
                (desired_rot-ccdinfo.guide_rot0)*DPI/180.);
          ccdinfo.guide_y0 = ccdinfo.yc + ccdinfo.guide_dist * 
            sin(ccdinfo.guide_theta0 -
                (desired_rot-ccdinfo.guide_rot0)*DPI/180.);
fprintf(stderr,"inst 3 guiding: %f %f %f\n",ccdinfo.guide_x0, ccdinfo.guide_y0, ccdinfo.guide_dist);
        }
        col = ccdinfo.guide_x0 - ccdinfo.guide_xoff;
        row = ccdinfo.guide_y0 - ccdinfo.guide_yoff;

        if (strcmp(command,"GUIDING")==0) {
          // For guide exposure, if detector has been windowed, use the defined
          //   window; otherwise define a small window around guide star
           if (ccdinfo.x0==1&&ccdinfo.y0==1&&ccdinfo.nx==NCOL&&ccdinfo.ny==NROW) {
             xs=(int)ccdinfo.guide_x0-ccdinfo.guide_size/2-2;
             xe=(int)ccdinfo.guide_x0+ccdinfo.guide_size/2+2;
             ys=(int)ccdinfo.guide_y0-ccdinfo.guide_size/2-2;
             ye=(int)ccdinfo.guide_y0+ccdinfo.guide_size/2+2;
           } else {
             xs=ccdinfo.x0;
             xe=ccdinfo.x0+ccdinfo.nx-1;
             ys=ccdinfo.y0;
             ye=ccdinfo.y0+ccdinfo.ny-1;
           }
          sprintf(newcom,"EXPGUIDE %f %d %d %d %d",ccdinfo.exposure*1000,xs,xe,ys,ye);
fprintf(stderr,"%s\n",newcom);
          process_command(newcom);
        } else {
          xs=ccdinfo.x0;
          xe=ccdinfo.x0+ccdinfo.nx-1;
          ys=ccdinfo.y0;
          ye=ccdinfo.y0+ccdinfo.ny-1;
        }
        // Is desired position off of image?
        if (col<(xs+3)||col>(xe-3)||row<(ys+3)||row>(ye-3) ) {
          sprintf(message,"guide star off image %f %f %d %d %d %d\n",col,row,xs,xe,ys,ye);
          writeterm(message);
          write(client,message,strlen(message));
          return(COM_UNSUCCESSFUL);
        }

        // Find the centroid
        if (!daytest && findcent(fdata,xs,xe,ys,ye,
                 ccdinfo.guide_size,&row,&col,&peak,&total,&fwhm) != 0)  {
          sprintf(message,"getcentroid failed\n");
          writeterm(message);
          write(client,message,strlen(message));
          return(COM_UNSUCCESSFUL);
        }
        if (total!=0) fprintf(fguide,"%f %f %f %f %f\n",
           telinfo.current_utc, peak/total, peak, total, meantot);
        fflush(fguide);
        if (daytest) {
          row=ccdinfo.guide_y0;
          col=ccdinfo.guide_x0;
        }
        fprintf(stderr,"GUIDING: (%8.2f,%8.2f) %5d %3d %3d %8.1f %8.1f %8.1f %7.1f\n", col, row,nguide,ccdinfo.guide_update,nupdate,total,meantot,tottot,badthresh);
       // Accumulate mean counts for last update exposures. Once mean has been
       //   determined, reject observations where total counts is less than
       //   some fraction (badthresh) of mean rate from last update exposures.
        if (ccdinfo.guide_update==0) ccdinfo.guide_update = 1;
        if (nguide<ccdinfo.guide_update) {
          totsave[nguide] = total;
          if (nguide==0) tottot=0;
          tottot += total;
          nguide++;
        } else {
          meantot = tottot / ccdinfo.guide_update;
          if (total>0 && (daytest || badthresh<=0 || total > badthresh*meantot)) {
            tottot -= totsave[nguide%ccdinfo.guide_update];
            totsave[nguide%ccdinfo.guide_update] = total;
            tottot += totsave[nguide%ccdinfo.guide_update];
            nguide++;
          } else {
	    fprintf(stderr,"total < badthresh*meantot or total<0\n");
            return(COM_UNSUCCESSFUL);
          }
        }
        // Display desired and measured position on image
fprintf(stderr,"cross: %f %f %f %f\n",ccdinfo.guide_x0,ccdinfo.guide_xoff,ccdinfo.guide_y0,ccdinfo.guide_yoff);
        imagecross((int)(ccdinfo.guide_x0-ccdinfo.guide_xoff), 
                   (int)(ccdinfo.guide_y0-ccdinfo.guide_yoff));
        imagebox((int)(col-3), (int)(row-3), 7, 7, 0);
        // Now compute offsets, using transformation matrix to convert to ra/dec
        dy = row - (ccdinfo.guide_y0 - ccdinfo.guide_yoff);
        dx = col - (ccdinfo.guide_x0 - ccdinfo.guide_xoff);
        if (guide_nsquare>0) {
          dx += (nguide%guide_nsquare) * guide_squarestep;
          dy += ((nguide%(guide_nsquare*guide_nsquare))/guide_nsquare) * guide_squarestep;
        }

        // Accumulate offsets for averaging
        if (nupdate>=0) {
          totdx += dx;
          totdy += dy;
        } else {
          totdx = 0;
          totdy = 0;
        }
        nupdate++;
        if (nupdate>0 && nupdate >= ccdinfo.guide_update) {
#ifdef GUIDER
          sprintf(newcom,"GUIDEINST 2 %f %f",totdx/nupdate,totdy/nupdate);
#else
          sprintf(newcom,"GUIDEINST %d %f %f",inst,totdx/nupdate,totdy/nupdate);
#endif
          if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
        }
        if (nupdate >= ccdinfo.guide_update || ccdinfo.guide_update==0) {
          nupdate = 0;
          totdx = totdy = 0.;
        }

        if (abs(adj_nupdate) > 0) {
          /* Do we have focus update turned on? */
          /* Add one to update counter, reset sum to zero if this is first cycle */
          if (adj_update++ == 0) ratsum = ratsum2 = 0;
          ratsum += peak/total;
          ratsum2 += (peak/total)*(peak/total);
          fprintf(stderr,"update: %4d  nupdate: %4d   rat: %7.3f  peak: %7.1f   total: %8.1f  fwhm: %7.2f\n\n",
              adj_update,adj_nupdate,peak/total,peak, total,fwhm);
//          fprintf(fguide,"update: %4d  nupdate: %4d   rat: %7.3f  peak: %7.1f   total: %8.1f  fwhm: %7.2f\n\n",
//              adj_update,adj_nupdate,peak/total,peak, total,fwhm);
          /* Do we have focus update turned on? */
          if (adj_update >= 0 && adj_update >= abs(adj_nupdate)) {
           wait = FALSE;
           if (fochold) {
              fp = fopen(holdfile,"r");
              if (fp != NULL) {
                wait = TRUE;
                fclose(fp);
              }
           }
           if ( !wait ) {
            avgrat = ratsum/adj_update;
            avgsig = (ratsum2 - ratsum*ratsum/adj_update);
            avgsig =
              (avgsig>0&&adj_update>1 ? sqrt(avgsig/adj_update/(adj_update-1)): 0.05 );
            fprintf(stderr,"offset: %4d cur_foc: %5d avgrat: %8.3f  avgold: %8.3f  avgsig: %9.4f\n",
                    adj_offset, cur_foc, avgrat, avgold, avgsig);
//            fprintf(fguide,"offset: %4d cur_foc: %5d avgrat: %8.3f  avgold: %8.3f  avgsig: %9.4f\n",
//                    adj_offset, cur_foc, avgrat, avgold, avgsig);
            if (adj_offset == 0) {
              /* If this is first focus set, just do a guider focus offset */
              avgold = avgrat;
#ifdef GUIDER
              sprintf(newcom,"GUIDEFOC %d",adj_sign*adj_foc);
#else
              sprintf(newcom,"XFOCUS %d %d %d",
                      adj_sign*adj_foc,adj_sign*adj_foc,adj_sign*adj_foc);
#endif
              if (havetocc) status = 
                      sendport(toport.port,toport.server,newcom,ret,80);
              cur_foc += adj_sign*adj_foc;
              fprintf(stderr,"offsetting focus: %d\n",adj_sign*adj_foc);
              adj_offset = 1;
            } else if (avgrat > (avgold+avgsig) ) {
              /* If ratio is better then previous, change telescope focus
                 and zero out focus offset. Keep guider offset in same direction */
              if (adj_nupdate>0) {
                fprintf(stderr,"offsetting telescope: %d\n",-adj_sign*adj_foc);
                sprintf(newcom,"XFOCUS %d %d %d",
                  -adj_sign*adj_foc,-adj_sign*adj_foc,-adj_sign*adj_foc);
              } else { 
                fprintf(stderr,"offsetting focus: %d\n",adj_sign*adj_foc);
#ifdef GUIDER
                sprintf(newcom,"GUIDEFOC %d",adj_sign*adj_foc);
#else
                sprintf(newcom,"XFOCUS %d %d %d",
                        adj_sign*adj_foc,adj_sign*adj_foc,adj_sign*adj_foc);
#endif
              }
              if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
              usleep(200000);
              avgold = avgrat;
              adj_offset = 2;
            } else {
              /* If ratio is worse then previous, retake zero position, then 
                 try other direction */
#ifdef GUIDER
              sprintf(newcom,"GUIDEFOC %d",-adj_sign*adj_foc);
#else
              sprintf(newcom,"XFOCUS %d %d %d",
                      -adj_sign*adj_foc,-adj_sign*adj_foc,-adj_sign*adj_foc);
#endif
              if (havetocc) status = 
                   sendport(toport.port,toport.server,newcom,ret,80);
              cur_foc -= adj_sign*adj_foc;
              fprintf(stderr,"offsetting focus: %d\n",-adj_sign*adj_foc);
              adj_sign *= -1;
              adj_flip++;
              adj_offset = 0;
              if ( adj_nupdate < 0 && (adj_offset>1 || adj_flip > 3)) {
                fprintf(stderr,"Focus has converged !\n");
                adj_nupdate = 0;
                ccdinfo.guiding = 0;
              } 
            }
            adj_update = -3;
           }
          }
        }
    return(0);
  }

#ifdef GUIDER
  else if (strcmp(command,"READUSNO") == 0 ) {
        nskip=0;
        n=sscanf(inputline+9,"%s%d", file,&nskip);
        readccdstatus(cstatusfile,cstatusreadyfile,&ccdinfo);
        #ifdef HAVETOCC
        nfail=0;
#ifdef SOCKET
        while (havetocc && nfail<100 &&
               getstatus(totocc.port, totocc.server, &telinfo) != 0) {
#else
        while (havetocc && nfail<100 &&
               readstatus(tstatusfile, tstatusreadyfile, &telinfo) != 0) {
#endif
           fprintf(stderr,"x");
           nfail++;
        }
        ccdinfo.sx=telinfo.sx[2];
        ccdinfo.sy=telinfo.sy[2];
        ccdinfo.cx=telinfo.cx[2];
        ccdinfo.cy=telinfo.cy[2];
        ccdinfo.theta=telinfo.theta[2];
        #endif
        readusno(file,nskip,telinfo.cx[1],telinfo.cy[1],&ccdinfo);
        ccdinfo.guiding = 2;
        writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
        return(0);
  }
#endif

#endif
  return(-1);
}

int camera_command(char *command, unsigned short *ccddata)
{
  char com[300], newcom[80], ret[80];
  double temp, dummy;
  int tempstatus, n, xe, ye, nexp, motors, dofill;
  static time_t last_fill = 0;
  static double temp_fill = -110.;

  com[0] = 0;
  sscanf(command,"%s",com);
  strupr(com);

#ifdef HAVEREMOTE
  return(-1);
#else
  if (strcmp(com,"MEXP") ==0) {
    sscanf(command+4,"%f%d",&dummy,&nexp);
    for (n=0; n<nexp; n++) {
      fprintf(stderr,"exposure %d %d\n",n,nexp);
      status=camera_command("EXP",ccddata);
    }

  } else if (strcmp(com,"EXP") ==0) {

#ifdef HAVEDARK
    if (ccdinfo.autodark && ccdinfo.darkexposure != ccdinfo.exposure) {
       writeterm("doing dark... (CTRL-C to interrupt)\n");
       status=ccd_expose(darkdata, ccdinfo.exposure, FALSE, fast,
                  ccdinfo.x0, ccdinfo.x0+ccdinfo.nx-1, 
                  ccdinfo.y0, ccdinfo.y0+ccdinfo.ny-1, 0); 
       if (status>0) return(status);
       ccdinfo.darkexposure = ccdinfo.exposure;
    }
#endif
    #ifdef SLITVIEW
    ccdinfo.end_time = time(NULL) + ccdinfo.exposure;
    #endif

    writeterm("doing exposure... (CTRL-C to interrupt)\n");
#ifdef VERBOSE
    gettimeofday(&time0,&zone0);
    fprintf(stderr,"ccd_expose: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
    status = ccd_expose(ccddata, ccdinfo.exposure, TRUE, fast,
                  ccdinfo.x0, ccdinfo.x0+ccdinfo.nx-1, 
                  ccdinfo.y0, ccdinfo.y0+ccdinfo.ny-1, 0); 
#ifdef VERBOSE
    gettimeofday(&time0,&zone0);
    fprintf(stderr,"done ccd_expose: %f\n",time0.tv_sec+time0.tv_usec*1.e-6);
#endif
    if (status>0) return(status);
    ccdinfo.shutter = 1;
    status = process_exposure(ccddata,ccdinfo.x0,ccdinfo.nx,ccdinfo.y0,ccdinfo.ny);
    if (ccdinfo.guiding == 9) process_command("SCIGUIDING");
    
    return(status); 
    
  } 
  else if (strcmp(com,"EXPGUIDE") ==0) {
    int xs, xe, ys, ye;
    sscanf(command+8,"%*f%d%d%d%d",&xs,&xe,&ys,&ye);
    status=ccd_expose(ccddata, ccdinfo.exposure, TRUE, fast,
                      xs, xe, ys, ye, 0);
    if (status>0) return(status);
    ccdinfo.shutter = 1;
    status = process_exposure(ccddata,xs,xe-xs+1,ys,ye-ys+1);
    return(status); 
  }
  else if (strcmp(com,"DARK") ==0) {
    writeterm("doing dark... (CTRL-C to interrupt)\n");
    status = 
       ccd_expose(ccddata, ccdinfo.exposure, FALSE, fast,
                  ccdinfo.x0, ccdinfo.x0+ccdinfo.nx-1, 
                  ccdinfo.y0, ccdinfo.y0+ccdinfo.ny-1, 0); 
    if (status>0) return(status);
    ccdinfo.darkexposure = ccdinfo.exposure;
    ccdinfo.shutter = 0;
    status = process_exposure(ccddata,ccdinfo.x0,ccdinfo.nx,ccdinfo.y0,ccdinfo.ny);
    return(status); 
  } 
  else if (strcmp(com,"FILLRESET") == 0) {
    dummy = 0;
    sscanf(command+9,"%lf",&dummy);
    last_fill = time(NULL)-dummy*3600.;
    return(0);
  }

  else if (strcmp(com,"TEMPRESET") == 0) {
    dummy = 0;
    sscanf(command+9,"%lf",&temp_fill);
    fprintf(stderr,"temp_fill: %f\n",temp_fill);
    return(0);
  }

  else if (strcmp(com,"CCDTEMP") == 0) {
    status = ccd_temp(&temp, &tempstatus);
    ccdinfo.ccd_temp = temp;
    ccdinfo.ccd_temp_status = tempstatus;
    writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
    dofill = FALSE;
#ifdef LEACH
    fprintf(stderr,"temp: %f  time since last AUTOFILL (not commanded fill): %lf hrs\n",
       temp, (time(NULL) - last_fill)/3600.);
    if (temp > 250 && temp < 260) process_command("CCDINIT");
    if (( (time(NULL) > (last_fill + 4*3600) && temp > temp_fill) ||
          (time(NULL) > (last_fill + 0.5*3600) && temp > (temp_fill+20)) )
        && temp < 50) {
      dofill = TRUE;
      fprintf(stderr,"power_on(FILL)\n");
      power_on(FILL);
      while (power_status(plug_status) < 0) {
        sleep(2);
      }
      if (plug_status[MOTORS] == OFF) {
        fprintf(stderr,"power_on(MOTORS)\n");
        power_on(MOTORS);
        sleep(10);
        motors = TRUE;
      } else {
        motors = FALSE;
      }
      sprintf(newcom,"FI 6");
      fprintf(stderr,"%s\n",newcom);
      if (havetocc) status = sendport(toport.port,toport.server,newcom,ret,80);
      power_off(FILL);
      if (motors) {
        power_off(MOTORS);
        fprintf(stderr,"power_off(MOTORS)\n");
      }
      last_fill = time(NULL);
    }
#endif

    FILE *tfp;
    tfp = fopen("temp.dat","a");
    if (tfp != NULL) {
      fprintf(tfp,"%d %f %d\n",time(NULL),temp,dofill);
      fclose(tfp);
    }
    return(0);
  }
  else if (strcmp(com,"SETTEMP") == 0) {
    sscanf(command+7,"%lf",&temp);
    status = set_ccd_temp(temp);
    temp_fill = (temp+15<-105) ? -105 : temp+15;
    sprintf(message,"ccd temp status: %d\n",status);
    writeterm(message);
    return(status);
  }
  else if (strcmp(com,"STATUS") == 0) {
    status = ccd_status();
    return(status);
  }
  else if (strcmp(com,"CCDINIT") == 0) {
    ccd_initialize(0);
    ccdinfo.autodisplay = TRUE;
    writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
    fast = FALSE;
    return(0);
  }
  else if (strcmp(com,"CCDFAST") == 0) {
    ccd_initialize(1);
    ccdinfo.autodisplay = FALSE;
    writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
    fast = TRUE;
    return(0);
  }
  else if (strcmp(com,"WINDOW") == 0 || strcmp(com,"SUBFRAME") == 0) {
    for (i=0 ; i<strlen(command); i++)
      if (command[i] == ':' || command[i] == ',') command[i] = ' ';
    strupr(command);
    if (strstr(command,"FULL") != NULL) {
      ccdinfo.x0 = 1;
      ccdinfo.y0 = 1;
      ccdinfo.nx = NCOL;
      ccdinfo.ny = NROW;
    } else if ( ( strcmp(com,"WINDOW")== 0 &&
                  (n=sscanf(command+6,"%d %d %d %d", 
                  &ccdinfo.x0, &ccdinfo.nx, &ccdinfo.y0, &ccdinfo.ny)) <= 0 ) ||
                ( strcmp(com,"SUBFRAME") == 0 &&
                  (n=sscanf(command+8,"%d %d %d %d", 
                  &ccdinfo.x0, &ccdinfo.nx, &ccdinfo.y0, &ccdinfo.ny)) <= 0 ) ){
       if (tvinit) {
         vecclear();
         inttvbox(&ccdinfo.x0, &ccdinfo.y0, &ccdinfo.nx, &ccdinfo.ny);
#ifdef LEACH
         //ccdinfo.nx += NCOL - NROW;
#endif
       }
    } else if (n==4) {
      ccdinfo.nx = ccdinfo.nx - ccdinfo.x0 + 1;
#ifdef LEACH
      //ccdinfo.nx += NCOL - NROW;
#endif
      ccdinfo.ny = ccdinfo.ny - ccdinfo.y0 + 1;
    } else {
      sprintf(message,"You must specify either 0 or 4 arguments with WINDOW\n");
      writeterm(message);
    }
    if (tvinit) {
      vecclear();
      imagebox(ccdinfo.x0, ccdinfo.y0, ccdinfo.nx, ccdinfo.ny, 0);
    }
    ccdinfo.x0 = (ccdinfo.x0 < 1) ? 1 : ccdinfo.x0;
    ccdinfo.y0 = (ccdinfo.y0 < 1) ? 1 : ccdinfo.y0;
    ye = ccdinfo.y0 + ccdinfo.ny - 1;
    xe = ccdinfo.x0 + ccdinfo.nx - 1;
    ccdinfo.nx = (xe > NCOL) ? NCOL+1-ccdinfo.x0 : ccdinfo.nx;
    ccdinfo.ny = (ye > NROW) ? NROW+1-ccdinfo.y0 : ccdinfo.ny;
    sprintf(message,"New region: (%3d:%3d,%3d:%3d)\n",
          ccdinfo.x0,ccdinfo.x0+ccdinfo.nx-1,
          ccdinfo.y0,ccdinfo.y0+ccdinfo.ny-1);
    writeterm(message);
    ccdinfo.darkexposure = -1;
    if (ccdinfo.nx<NCOL || ccdinfo.ny<NROW) ccdinfo.autoflat = FALSE;
    return(0);
  }
  else if (strcmp(com,"FULL") == 0) {
    ccdinfo.x0 = 1;
    ccdinfo.y0 = 1;
    ccdinfo.nx = NCOL;
    ccdinfo.ny = NROW;
    ccdinfo.darkexposure = -1;
    return(0);
  }
#endif
  return(-1);
}

getcentroid(float *fdata, double *row, double *col)
{
  int irow, icol, status, boxcolor;
  double peak, tot, fwhm;
  char key;

#ifdef DISPLAY
// Mark returns coordinates with (1,1) origin
  sprintf(message,"Mark the desired location\n");
  writeterm(message);
  write(client,message,strlen(message));
  imageread(&icol,&irow,&key);
  *row = irow;
  *col = icol;
  if (key=='E' || key == 'e') {
    return(-1);
  }
  else if (key=='I' || key== 'i') {
    status = 0;
  }
  else if (key=='C' || key== 'c') {
    status = findcent(fdata,ccdinfo.x0,ccdinfo.x0+ccdinfo.nx-1
                           ,ccdinfo.y0,ccdinfo.y0+ccdinfo.ny-1,
                            ccdinfo.guide_size,row,col,&peak,&tot,&fwhm);
/*
    status = findcent(fdata,1,ccdinfo.nx,1,ccdinfo.ny,
                            ccdinfo.guide_size,row,col,&peak,&tot,&fwhm);
*/
  }
  else {
    return(-1);
  }

  boxcolor=0;
  icol = nint(*col)-ccdinfo.guide_size/2;
  irow = nint(*row)-ccdinfo.guide_size/2;
  imagerelocate(icol,irow);
  irow = nint(*row)+ccdinfo.guide_size/2;
  imagedraw(icol,irow,boxcolor);
  icol = nint(*col)+ccdinfo.guide_size/2;
  imagedraw(icol,irow,boxcolor);
  irow = nint(*row)-ccdinfo.guide_size/2;
  imagedraw(icol,irow,boxcolor);
  icol = nint(*col)-ccdinfo.guide_size/2;
  imagedraw(icol,irow,boxcolor);

  return(status);
#else
  *row = ccdinfo.ny/2;
  *col = ccdinfo.nx/2;
  return(0);
#endif
}

// Findstar finds a peak from data array (xs:xe,ys:ye) within region 
//    (sc:ec,sr:er), and calculates the centroid around this peak within
//    a square box of size, loads row, col, peak, total
findstar(float *data, int xs, int xe, int ys, int ye, 
         int sr, int er, int sc, int ec, int size,
         double *row, double *col, double *peak, double *tot, double *fwhm)
{
  float px, *t, *temp, perc, gain, rn, sig;
  int i, j, asr, aer, asc, aec, tty, noisemod, nr, nc;

  nr = ye-ys+1;
  nc = xe-xs+1;
  asr = ys;
  asc = xs;
  aer = ye;
  aec = xe;

  // Start by median filtering image
  size = 5;
  perc = 0.5;
  sig = 0;
  tty = noisemod = 0;
  gain=1;
  rn=0.;
  temp = (float *)malloc(nr*nc*sizeof(float));
  zapthatsucker_(data,&asr,&aer,&asc,&aec,temp,&asr,&aer,&asc,&aec,&size,&size,
                 &sig, &perc,&tty,&noisemod,&gain,&rn);

  // Now find the peak
  *peak=-100;
  for (i=sr ; i<=er ; i++) {
    for (j=sc ; j<=ec ; j++) {
      px = *(temp+(i-ys)*nc+j-xs);
      if (px>*peak) {
        *col=j;
        *row=i;
        *peak=px;
      }
    }
  }
  free(temp);
  sprintf(message,"peak: %f %f %f\n",*row,*col,*peak);
  writeterm(message);
 
  // Now get the centroid
  findcent(data,xs,xe,ys,ye,size,row,col,peak,tot,fwhm);
}

// Findcent takes starting guess coordinates and does a centroid.
// Both input and output are (1,1) origin
findcent(float *data, int xs, int xe, int ys, int ye, int size,
         double *row, double *col, double *peak, double *sum, double *fwhm)
{
  #define MAXRAD 512
  int iter, imin, imax, jmin, jmax, i, j, icol, irow, pcol, prow, nr, nc;
  int isize, niter = 6;
  unsigned long n;
  float temp[4*(2*MAXRAD+1)], *zap;
  double sumx, sumy, dx, dy, sumxx, sumyy;
  float px, thresh, perc, sig, gain, rn;
  int asr, asc, aer, aec, tty, noisemod, zsize;
  double term[3];
  int npar, ipar, jpar;
  double **c, **v;

  nr = ye-ys+1;
  nc = xe-xs+1;
  asr = ys;
  asc = xs;
  aer = ye;
  aec = xe;
  zsize = 5;
  perc = 0.5;
  sig = 5;
  tty = 0;
  noisemod = 0;
  gain=1;
  rn=0.;

  zap = (float *)malloc(nr*nc*sizeof(float));
  zapthatsucker_(data,&asr,&aer,&asc,&aec,zap,&asr,&aer,&asc,&aec,&zsize,&zsize,
                 &sig, &perc,&tty,&noisemod,&gain,&rn);

  icol = nint(*col);
  irow = nint(*row);
  isize = size/2;
fprintf(stderr,"in findcent %d %d %d %d %d\n",nc, nr, icol, irow, isize);
fprintf(stderr,"in findcent %d %d %d %d %f %f\n",xs,xe, ys,ye,row,col);
  for (iter=0;iter<niter;iter++) { 


/* Compute a background value using a median of the box edge values */
    // Allocate matrices
    if (iter==0) {
      imin = MAX(xs,icol-isize);
      imax = MIN(xe,icol+isize);
      jmin = MAX(ys,irow-isize);
      jmax = MIN(ye,irow+isize);
      npar=3;
      c = (double **)malloc(npar*sizeof(double *));
      for (i=0; i<npar; i++) 
        c[i] = (double *)malloc(npar*sizeof(double));
      v = (double **)malloc(npar*sizeof(double *));
      for (i=0; i<npar; i++) 
        v[i] = (double *)malloc(sizeof(double));
      for (ipar=0;ipar<npar;ipar++) {
        v[ipar][0] = 0;
        for (jpar=0;jpar<npar;jpar++) {
            c[ipar][jpar] = 0;
        }
      }
  
      n=1;
      term[0] = 1;
      for (j=jmin;j<=jmax;j++) {
        term[1] = imin-xs;
        term[2] = j-ys;
        temp[n] = *(data+(j-ys)*nc+(imin-xs));
        for (ipar=0;ipar<npar;ipar++) {
          v[ipar][0] += temp[n]*term[ipar];
          for (jpar=0;jpar<npar;jpar++) {
            c[ipar][jpar] += term[ipar]*term[jpar];
          }
        }
        n++;
        term[1] = imax-xs;
        temp[n] = *(data+(j-ys)*nc+(imax-xs));
        for (ipar=0;ipar<npar;ipar++) {
          v[ipar][0] += temp[n]*term[ipar];
          for (jpar=0;jpar<npar;jpar++) {
            c[ipar][jpar] += term[ipar]*term[jpar];
          }
        }
        n++;
  
      }
      for (i=imin+1;i<=imax-1;i++) {
        term[1] = i-xs;
        temp[n] = *(data+(jmin-ys)*nc+i-xs);
        term[2] = jmin-ys;
        for (ipar=0;ipar<npar;ipar++) {
          v[ipar][0] += temp[n]*term[ipar];
          for (jpar=0;jpar<npar;jpar++) {
            c[ipar][jpar] += term[ipar]*term[jpar];
          }
        }
        n++;
        term[2] = jmax-ys;
        temp[n] = *(data+(jmax-ys)*nc+i-xs);
        for (ipar=0;ipar<npar;ipar++) {
          v[ipar][0] += temp[n]*term[ipar];
          for (jpar=0;jpar<npar;jpar++) {
            c[ipar][jpar] += term[ipar]*term[jpar];
          }
        }
        n++;
      }
      int bad=0;
      for (i=0;i<3;i++) if (c[i][i]==0) bad = 1;
      if (!bad) {
        gaussj(c,3,v,1);
        thresh = median(n-1,temp);
        //fprintf(stderr,"Computed surface: %f %f %f  thresh: %f\n",
        // v[0][0],v[1][0],v[2][0],thresh);
      } else {
        thresh=0;
      }
      imin = MAX(xs,icol-isize*3/4);
      imax = MIN(xe,icol+isize*3/4);
      jmin = MAX(ys,irow-isize*3/4);
      jmax = MIN(ye,irow+isize*3/4);
    } else {
      //imin = MAX(xs,icol-10);
      //imax = MIN(xe,icol+10);
      //jmin = MAX(ys,irow-10);
      //jmax = MIN(ye,irow+10);
      imin = MAX(xs,icol-isize*3/4);
      imax = MIN(xe,icol+isize*3/4);
      jmin = MAX(ys,irow-isize*3/4);
      jmax = MIN(ye,irow+isize*3/4);
    }

    *sum = sumx = sumy = 0;
    *peak = -1e10;
    pcol = prow = 0;
/*
    for (j=jmin+isize/2;j<=jmax-isize/2;j++) {
      for (i=imin+isize/2;i<=imax-isize/2;i++) {
*/
    for (j=jmin;j<=jmax;j++) {
      for (i=imin;i<=imax;i++) {
         px = *(data+(j-ys)*nc+(i-xs));
       
         if (dothresh==1) 
           px -= thresh;
         else if (dothresh==2) {
           px -= (v[0][0]+v[1][0]*(i-xs)+v[2][0]*(j-ys));
         }
         *sum += px;
         sumx += px*i;
         sumy += px*j;
         if (px > *peak) {
           *peak = px;
           prow = j;
           pcol = i;
           //fprintf(stderr,"%lf %d %d %d %d %d\n",*peak,i,j,iter,icol,irow);
           //writeterm(message);
         }
      }
    }
    if (prow==0 || pcol==0) return(1);
    if (*sum != 0) {
      sumx /= *sum;
      sumy /= *sum;
    } else {
      sumx = *col;
      sumy = *row;
    }
    fprintf(stderr,"found centroid: %d %f %f %d %d %d %d %d %d\n",iter,sumx, sumy,pcol,prow,imin,imax,jmin,jmax);
    writeterm(message);
    dx = sumx - *col;
    dy = sumy - *row;
    *col = sumx;
    *row = sumy;
    if (*col>=imin&&*col<=imax&&*row>=jmin&&*row<=jmax) {
       if (nint(*col) == icol && nint(*row) == irow) {
        sprintf(message,"findcent: cent/peak: %d %8.2f %8.2f %d %d %8.2f\n",iter, sumx, sumy,pcol,prow,thresh);
        writeterm(message);
        sumxx = sumyy = 0;
        for (j=jmin;j<=jmax;j++) {
          for (i=imin;i<=imax;i++) {
             px = *(data+(j-ys)*nc+(i-xs));
             if (dothresh) px -= thresh;
             sumxx += px*(i-sumx)*(i-sumx);
             sumyy += px*(j-sumy)*(j-sumy);
          }
        }
        *fwhm = sqrt(sumxx / *sum + sumyy / *sum);
        return(0);
       }
    } else {
        sprintf(message,"findcent: peak: %d %d\n",pcol,prow);
        writeterm(message);
        *col = (double)pcol;
        *row = (double)prow;
        return(0);
    }
      
    icol = nint(*col);
    irow = nint(*row);
  }
  sprintf(message,"findcent: no converge: %d %d\n",pcol,prow);
  writeterm(message);
  *col = (double)pcol;
  *row = (double)prow;
  return(0);
}

initccd()
{
  char command[MAXCOMMAND],filename[200],rootname[200], dirname[64];
  int incval;
  time_t tloc;
  struct tm *tt;
  FILE *f = NULL;

  time(&tloc);
  tt = gmtime(&tloc);

  // If its near end of UT date, assume we're starting early for next night,
  //   so add 5 hours to local time
  if (tt->tm_hour > 19) {
    tloc += 5 * 3600;
    tt = gmtime(&tloc);
  }

  // Set default output directory to go with current UT date and create
  // Set default root file name to go with current UT date
#ifdef SLITVIEW
  sprintf(rootname,"%04d%02d%02d",
          tt->tm_year+1900,tt->tm_mon+1,tt->tm_mday);
  sprintf(ccdinfo.dirname,"images");
#else
  sprintf(rootname,"%02d%02d%02d",
          tt->tm_year%100,tt->tm_mon+1,tt->tm_mday);
  sprintf(ccdinfo.dirname,"images/%02d%02d%02d",tt->tm_year%100,tt->tm_mon+1,tt->tm_mday);
#endif
  mkdir(ccdinfo.dirname,0755);
  chown(ccdinfo.dirname,getuid(),-1);

  // Set initial extension number to be first available extension 
  //    not already written
  incval = 0;
  do {
    if (f != NULL) fclose(f);
    incval++;
#ifdef GUIDER
    sprintf(filename,"%s/%sg.%3.3d.fits",ccdinfo.dirname,rootname,incval);
#else
  #ifdef SLITVIEW
    sprintf(filename,"%s/%s.%04d.fits",ccdinfo.dirname,rootname,incval);
  #else
    sprintf(filename,"%s/%s.%3.3d.fits",ccdinfo.dirname,rootname,incval);
  #endif
#endif
  } while ((f=fopen(filename,"r"))!=NULL);

#ifndef SLITVIEW
  /* Make output directory */
  sprintf(command, "ssh -l %s %s mkdir %s/%s &",
               destacct, destmach, destdir, ccdinfo.dirname);
  sprintf(message,"%s\n",command);
  writeterm(message);
//  system(command);
#endif


// NOTE: NAME must be set before doing any writeccdstatus, else blank string
//  screws things up!!
#ifdef GUIDER
  sprintf(command,"NAME %sg",rootname);
  process_command(command);
  sprintf(command,"-DISK");
  process_command(command);
//  sprintf(command,"GUIDEHOME");
//  process_command(command);
#else
  sprintf(command,"NAME %s",rootname);
  process_command(command);
  sprintf(command,"FITS");
  process_command(command);
#endif
#ifdef HAVEPI
  sprintf(command,"CLEANS 0");
  process_command(command);
  sprintf(command,"NUMSEQ 1");
  process_command(command);
  sprintf(command,"SHUTTERCMD 1");
  process_command(command);
#endif
  sprintf(command,"FILTER %d",ccdinfo.filter);
  process_command(command);
  sprintf(command,"FILTFOC %d",focoff[ccdinfo.filter]);
  process_command(command);
  sprintf(command,"SETINCVAL %d",incval);
  process_command(command);
#ifndef SLITVIEW
  sprintf(command,"EXPOSURE %lf",ccdinfo.exposure);
  process_command(command);
#endif

 /* Initialize a log file */
  writehtml(0,ccdinfo.dirname,ccdinfo.filename);  
 /* Update master log file */
  writelogpage();  
}


#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

float median(unsigned long n, float arr[])
{
	unsigned long i,ir,j,l,mid,k;
	float a,temp;

        k=n/2;
	l=1;
	ir=n;
	for (;;) {
		if (ir <= l+1) {
			if (ir == l+1 && arr[ir] < arr[l]) {
				SWAP(arr[l],arr[ir])
			}
			return arr[k];
		} else {
			mid=(l+ir) >> 1;
			SWAP(arr[mid],arr[l+1])
			if (arr[l] > arr[ir]) {
				SWAP(arr[l],arr[ir])
			}
			if (arr[l+1] > arr[ir]) {
				SWAP(arr[l+1],arr[ir])
			}
			if (arr[l] > arr[l+1]) {
				SWAP(arr[l],arr[l+1])
			}
			i=l+1;
			j=ir;
			a=arr[l+1];
			for (;;) {
				do i++; while (arr[i] < a);
				do j--; while (arr[j] > a);
				if (j < i) break;
				SWAP(arr[i],arr[j])
			}
			arr[l+1]=arr[j];
			arr[j]=a;
			if (j >= k) ir=j-1;
			if (j <= k) l=i;
		}
	}
}
#undef SWAP

#if defined(sun) || defined(linux)
int nint(float val)
{
  if (fmod(val,1.) < 0.5)
    return((int)floor(val));
  else
    return((int)ceil(val));
}
#endif

void myremove(char *removefile)
{
  int i, n, maxtry = 10;

  i = 0;
  usleep(100000);
  while (n=remove(removefile) !=0 && i<maxtry) {
    usleep(100000);
    i++;
  }

  if (i == maxtry) {
    sprintf(message,"error removing %s\n", removefile);
    writeterm(message);
  }
}

void inittv()
{
#ifdef DISPLAY
  short r[256], g[256], b[256];
  int ncd = 263;
  int nrd = 256;
  int ierr, zf=0, yup=0;
#ifdef GUIDER
  int ncold = 16;
  int inst=2;
  int xoff=275;
  int yoff=110;
  char *resourcename="gccddisp";
  char *windowname="1m Guider Acquisition";
#else
  int ncold = 32;
  #ifdef SLITVIEW
  int inst=3;
  int xoff=1;
  int yoff=110;
  char *resourcename="disslit";
  char *windowname="DIS Slit Viewer";
  #else
  int inst=1;
  int xoff=1;
  int yoff=110;
  char *resourcename="ccddisp";
  char *windowname="1m Data Acquisition";
  #endif
#endif
#endif
#ifdef SLITVIEW
  int compute_offset();
#endif

  ierr = imageinit(&ncd,&nrd,&ncold,zf,yup,resourcename,windowname,xoff,yoff);

  if (ierr==0) {
    r[0] = b[0] = 255;
    g[0] = 0;
    imagepalette(1,r,g,b,1);
    for (i=0;i<256;i++)
      r[i] = g[i] = b[i] = i;
    imagepalette(255,r,g,b,0);
    tvinit = 1;
    #ifdef SLITVIEW
    imageinstallkey('c',0,compute_offset);
    imageinstallkey('C',0,compute_offset);
    imageinstallkey('i',0,compute_offset);
    imageinstallkey('I',0,compute_offset);
    #endif
  } else
    writeterm("Could not open display!!");
}

#ifdef HAVELOG
writelog(struct CCDSTATUS *ccdinfo, struct STATUS *G)
{
#define MAXLINE 132

  char buf[32],line[MAXLINE],comment[MAXLINE],name[64],command[80];
  static FILE *fp = NULL, *log = NULL, *fc;
  int h,m,sign,iline;
  static int lopen=0;
  double s, ut, ha;

  sprintf(name,"%s/%s.log",ccdinfo->dirname,ccdinfo->filename);
  log = fopen(name,"a");

  fprintf(log,"%4d&",ccdinfo->incval);
  fprintf(log,"%s&",ccdinfo->object);

  //ut = G->current_utc + DTIME/3600./24.;
  ut = G->current_utc + PRETIME/3600./24.;
  gethms((ut - (long)ut)*24,&h,&m,&s,&sign);
  sprintf(buf,"%02d:%02d:%02d",h,m,(int)s);
  fprintf(log,"%s&",buf);

  gethms(G->current_obs_ra*DR2H,&h,&m,&s,&sign);
  sprintf(buf,"%02d:%02d:%04.1f",h,m,s);
  fprintf(log,"%s&",buf);

  getdms(fabs(G->current_obs_dec*DR2D),&h,&m,&s,&sign);
  if (G->current_obs_dec >= 0)
        sprintf(buf,"%02d:%02d:%04.1f",h,m,s);
  else
        sprintf(buf,"-%02d:%02d:%04.1f",h,m,s);
  fprintf(log,"%s&",buf);

  fprintf(log,"%6.1lf&",G->current_mean_epoch);
  fprintf(log,"%5.1f&",G->current_pa*DR2D);

  ha = slaDrange(G->current_lasth*DH2R - G->current_obs_ra)*DR2H;
  gethms(fabs(ha),&h,&m,&s,&sign);
  if (ha >= 0)
    sprintf(buf,"%02d:%02d:%04.1f",h,m,s);
  else
    sprintf(buf,"-%02d:%02d:%04.1f",h,m,s);
  fprintf(log,"%s&",buf);

  fprintf(log,"%5.2f&",G->airmass);
  fprintf(log,"%6.1lf&",ccdinfo->exposure);
  fprintf(log,"%s&",filtname[ccdinfo->filter]);
  //fprintf(log,"%7.1f&",G->foc-(ccdinfo->filtfoc));
  if (G->filtfoc>0)
    fprintf(log,"%4d(+%3d)&",(int)(G->foc-(ccdinfo->filtfoc)),ccdinfo->filtfoc);
  else
    fprintf(log,"%4d(%4d)&",(int)(G->foc-(ccdinfo->filtfoc)),ccdinfo->filtfoc);
  fprintf(log,"\n");

  fclose(log);

/* Now write HTML file */
  if (!fast) writehtml(0,ccdinfo->dirname,ccdinfo->filename);
 
}
#endif

void readterm(char *command)
{
  command[0] = 0;
  #ifdef SLITVIEW
  win_read(command);
  #else
  //fgets(command, MAXCOMMAND-1, stdin);
  gets(command);
  #endif
}

void writeterm(char *command)
{
  #ifdef SLITVIEW
  win_write(command,1);
  #else
  fprintf(stderr,"%s",command);
  #endif
}

void trap()
{
  ctrlc = TRUE;
  sprintf(message,
"\nCommand string interrupted by CTRL-C. Waiting for current command to complete\n");
  writeterm(message);
  writeterm("You may need to hit a <CR> to get the command prompt\n");
  signal(SIGINT,trap);
}

void command_help()
{
   char ans[10];

#ifdef SLITVIEW
   win_write("ALL COMMANDS ARE INSENSITIVE TO CASE\n",0);
   win_write("Exposure commands:\n",0);
   win_write(" EXP t              : take exposure of length t seconds\n",0);
   win_write(" REPEAT             : continue taking exposures indefinitely\n",0);
   win_write(" STOP (or CTRL-c)   : stop exposure loop\n",0);
   win_write(" WINDOW [x1 x2 y1 y2] : set up windowing region on chip to read\n",0);
   win_write(" FULL               : reset to full chip\n",0);
   win_write("Options\n");
   win_write(" +DARK/-DARK        : turn on(+)/off(-) automatic dark subtraction\n",0);
   win_write(" SETTEMP temp       : set target CCD temperature to temp\n",0);
   win_write(" +DISPLAY/-DISPLAY  : turn on(+)/off(-) automatic disply\n",0);
   win_write(" +XFER/-XFER        : turn on(+)/off(-) automatic file transfer\n",0);
   win_write(" +REMARK/-REMARK        : turn on(+)/off(-) REMARK convention offset reporting \n",0);
   win_write(" +TUI/-TUI        : turn on(+)/off(-) REMARK convention offset reporting \n",0);
//   win_write("<CR> for more....\n");
//   readterm(ans);
   win_write("Display commands:\n",0);
   win_write(" SCALE low high     : redisplay current image from low (black) to high(white)\n",0);
   win_write(" NEWSCALE           : autoscale future images based on sky variance (default)\n",0);
   win_write(" SKYSCALE           : autoscale future images based on sky level\n",0);
   win_write(" FULLSCALE          : autoscale future images based on min to max value\n",0);
   win_write(" SAMESCALE          : scale future images with same scaling as current image\n",0);
   writeterm("File naming/transfer:\n");
   writeterm("Other:\n");
   win_write(" QU                 : exits program\n",0);
#endif

}
void inttvcross(int *xc, int *yc)
{
  int row, col;
  char key;

  writeterm("Move cursor to display window and make sure mouse focus is there\n");
  writeterm("Position cursor on desired target location and hit space bar\n");
  imageread(&col,&row,&key);
  *xc=col;
  *yc=row;
}

void inttvbox(int *x0, int *y0, int *nx, int *ny)
{
  int row, col;
  char key;
  int x1, x2, y1, y2;

  writeterm("Move cursor to display window and make sure mouse focus is there\n");
  writeterm("Position cursor and hit space bar on desired corner \n");
  imageread(&col,&row,&key);
  x1=col;
  y1=row;
  writeterm("Position cursor and hit space bar on opposite corner \n");
  imageread(&col,&row,&key);
  x2=col;
  y2=row;
  *x0 = (x1<x2 ? x1 : x2);
  *y0 = (y1<y2 ? y1 : y2);
  *nx= (x2-x1)>0 ? x2-x1 : x1-x2;
  *ny= (y2-y1)>0 ? y2-y1 : y1-y2;
}

#ifdef SLITVIEW
void window_resize()
{
  writeterm("Resizing window not yet supported: strongly recommend resizing to 80x32!\n");
  flushinp();
 // clear out keyboard buffer which can get filled on resizing
 //while (getchar() != EOF) {}

  //writeterm(getenv("TERMCAP"));
  //use_env(TRUE);
  //setupterm((char *)0, 1, (int *)0);

  //resizeterm(0,0);
}

compute_offset(x,y,xuser,yuser,key)
{
  int status, nbox, irow, icol;
  double row, col, torow, tocol, peak;
  float *fdata;
  char newcom[80];

  //sprintf(newcom,"xuser: %d yuser: %d x: %d y: %d\n",xuser,yuser,x,y);
  //writeterm(newcom);
  col = xuser; 
  row = yuser;
  fdata = (float *)tvimage;
  status = 0;
  if (key == 'c' || key == 'C') 
    status = findcent(data,ccdinfo.x0,ccdinfo.x0+ccdinfo.nx-1,
                           ccdinfo.y0,ccdinfo.y0+ccdinfo.ny-1,
                           ccdinfo.guide_size,&row,&col,&peak,&tot,&fwhm);
  torow = ccdinfo.yc;
  tocol = ccdinfo.xc;
  if (status != 0) {
    sprintf(newcom,"\nError computing centroid, using peak at: %6.0f %6.0f\n",col, row);
    writeterm(newcom);
  }
  if (ccdinfo.offsettype&0x1 ) {
      sprintf(newcom,"Remark offset: %8.1f %8.1f\n",
      -(col-tocol)*ccdxscale,(row-torow)*ccdyscale);
      writeterm(newcom);
  }
  if (ccdinfo.offsettype&0x2 ) {
      sprintf(newcom,"TUI offset: %8.1f %8.1f\n",
      (col-tocol)*ccdxscale,(row-torow)*ccdyscale);
      writeterm(newcom);
  }
  nbox = ccdinfo.guide_size;
  vecclear();
  irow = (int)(row-nbox/2.+0.5);
  icol = (int)(col-nbox/2.+0.5);
  imagebox(icol, irow, nbox, nbox, 0);
}
#endif

float variance(unsigned long n,float *temp, double ccdsky)
{
  int i;
  double sum=0;
  for (i=0 ; i<n ; i++) {
    sum += (*temp-ccdsky)*(*temp-ccdsky);
    temp++;
  }
  if (sum>0)
   return(sqrt(sum/(n-1.)));
  else
   return(ccdsky);
  
}

void update_status(int status)
{
  if (status>0) ccdinfo.expstatus = status;
  writeccdstatus(ccdinfo,cstatusfile,cstatusreadyfile);
}

#ifdef HAVEREMOTE
char *get_message()
{
  FILE *file;
  char message[MAXCOMMAND];
  int n;
  char *s;

#ifdef NEWCOM
  file = fopen(restart,"r");
  if (file != NULL) {
      fclose(file);
      if (ropen == 1) {
        writeterm("closing rfile\n");
        fclose(rfile);
      }
      ropen = 0;
      remove(restart);
  }

  if (ropen == 0) {
      rfile = fopen(respfile,"r");
      if (rfile != NULL) ropen = 1;
  }
  return(getresp(rfile));

#else

    file = fopen(respfilecheck,"r");
    if (file != NULL) {
        fclose(file);
        usleep(100000);
        myremove(respfilecheck);
        file = fopen(respfile,"r");
        if (file == NULL) {
          perror("open error 1: ");
        } else {
          n = 0;
          while (fgets(message+n,MAXCOMMAND-1-n,file)!= NULL) {
            n = strlen(message);
          }
          fclose(file);
          writeterm(message);
          myremove(respfile);
        }
        return(s=message);
    }
    return(NULL);

#endif

}

void send_message(char *command)
{
   FILE *file;

#ifdef DEBUG
   sprintf(message,"writing command file: %s %s\n",comfile,command);
   writeterm(message);
#endif

   /* Dont write a command file until the check file has been deleted */
   while ((file=fopen(comfilecheck,"r")) !=NULL) {
        fclose(file);
   }

   /* write out the command file */
   file = fopen(comfile,"w");
   while (file==NULL) {
     perror("open error 2: ");
     file = fopen(comfile,"w");
   }
#ifdef SPEC
   fprintf(file,"%s%c\n",command,13);
#else
   fprintf(file," %s%c",command,13);
#endif
   fclose(file);

#ifdef DEBUG
   sprintf(message,"writing command check file: %s %s\n",comfilecheck, command);
   writeterm(message);
#endif
      /* write the command check file */
   file = fopen(comfilecheck,"w");
   while (file==NULL) {
     perror("open error 3: ");
     file = fopen(comfilecheck,"w");
   }

   fclose(file);

}

int remote_command(char *command)
{
   char resp[300];
   char *s;
   int status;

   strupr(command);
   send_message(command);

   /* Now sit and wait for responses. If we get a DONE message, we are done,
      otherwise just print out whatever message comes across */
    resp[0] = 0;
    while(strcmp(resp,"DONE") != 0) {
#ifdef HAVECCD
      s = get_message();
#else
      strcpy(s,"DONE\0");
#endif
      status = 0;
      if (s != NULL) {
        sscanf(s,"%s", resp);
        if (strcmp(resp,"DONE") != 0) {
            /* If we didn't get DONE, just print out the message */
            writeterm(s);
        } else {
           /* If we got DONE, did command return successfully? if not, 
              just print out response */
            sscanf(s,"DONE %d",&status);
            if (status != 0) {
              writeterm(s);
              return(status);
            }
        }
      }
      if (tvinit) xtv_refresh(0);
      usleep(100000);
    }
    return(status);
}
#endif

int ccd_flush(unsigned short *buf, int nx, int ny)
{
#ifdef FLI
  fli_flush(buf, nx, ny);
#endif
}

int ccd_expose(unsigned short *buf, double exptime, BOOL use_shutter, BOOL fast,
               int xs, int xe, int ys, int ye, int writeout)
{

  gettimeofday(&starttime,&startzone);
#ifdef APOGEE
  apogee_expose(buf, exptime, use_shutter, fast, xs, xe, ys, ye, writeout);
#endif
#ifdef LEACH
  if (xs>xe||ys>ye||(xe-xs)<50||(ye-ys)<50) return(-1);
  leach_expose(buf, exptime, use_shutter, fast, xs, xe, ys, ye, writeout,&starttime,toport.port,toport.server);
#endif
#ifdef FLI
  fli_expose(buf, exptime, use_shutter, fast, xs, xe, ys, ye, writeout);
#endif
  return(0);
}
int ccd_status()
{
  #ifdef APOGEE
    return(apogee_status());
  #endif
}
int set_ccd_temp(double t)
{
  #ifdef APOGEE
    return(set_apogee_temp(t));
  #endif
  #ifdef LEACH
    return(set_leach_temp(t));
  #endif
  #ifdef FLI
    return(set_fli_temp(t));
  #endif
}
int fli_temp(double *, int *);
int ccd_temp(double *t, int *ts)
{
  *t = 0;
  *ts = 0;
  #ifdef APOGEE
    apogee_temp(t, ts);
  #endif
  #ifdef LEACH
    leach_temp(t, ts);
  #endif
  #ifdef FLI
    *ts = fli_temp(t, ts);
  #endif
}
void ccd_initialize(int fast)
{
  #ifndef LEACH
  imbuf = (unsigned short *)malloc(NSPE+2);
  #endif
  #ifdef APOGEE
    apogee_initialize();
  #endif
  #ifdef LEACH
    if (imbuf > 0) leach_close();
    imbuf = leach_initialize(fast);
    printf("imbuf: %d\n",imbuf);
  #endif
  #ifdef FLI
    fli_initialize();
  #endif
}
void ccd_close()
{
  #ifdef APOGEE
    apogee_close();
  #endif
  #ifdef LEACH
    leach_close();
  #endif
  #ifdef FLI
    fli_close();
  #endif
}
