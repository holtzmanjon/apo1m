#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "slalib.h"
#include "slamac.h"

#include "mytype.h"
#define NEWFILT
#include "filter.h"

#include "focus.h"

/* function prototypes and global variables */
#include "command.h"

#include "io.h"

double latitude = 0.572124 ;
double longitude = -1.846907 ;
double altitude=2798.0;

double current_epoch = 2000.;

int default_timeout = 0;
int ccd_timeout = 0;
struct STATUS statusinfo, *G;

FILE *infile[MAXDEPTH];
FILE *outfile[MAXDEPTH];
int idepth = 0;
int havepipe[MAXDEPTH];
FILE *frommasterfp, *tomasterfp, *fromalertfp;
char message[3000];

#define MAXARG 8

#define MAXUSER 1000

BOOL ctrlc =FALSE;
void trap();

int sendport(int, char *, char *, char *, int);

// Stuff for computing focus positions
int autofoc(int, char *);
int autodfoc(double *, int, int);
int calc_focus(double **, struct STATUS *);
int calc_focus_steps(double **, double*, struct STATUS *);
int setup_focus(double **);
int telescope_startup(int, char *, int);
int dome_open_close(int, char *, char*);
int telescope_init(int, char *, int);
int telescope_settime(int, char *);
int readuser(char *, int);
void gaussj(double**,int,double**,int);
void vmul(double **,int ,int ,double *,double *);
double **foc;
double **ifoc;
double **junk;
double out[3];
int focrun(int, char *, int, char *, int, int, int);
double airmass(double, double);
int nint(float val);

char userobj[2][MAXUSER][16];
double usermag[2][MAXUSER][6];
double userra[2][MAXUSER],userdec[2][MAXUSER],userepoch[2][MAXUSER];
double userpmra[2][MAXUSER],userpmdec[2][MAXUSER];
int nuser[2] = {0,0};

#include "power.h"
int plug_status[9];

BOOL guiding = FALSE;
BOOL remote = FALSE;

BOOL havetocc;
char *tstatusfile = "/home/export/tocc/statr.doc";
char *tstatusreadyfile = "/home/export/tocc/statr.fin";
char *cstatusfile = "ccdstatus.doc";
char *cstatusreadyfile = "ccdstatus.fin";

main(argc, argv)
int argc;
char *argv[];
{

  char command[MAXCMD], tmp[MAXCMD], ret[MAXCMD], ans[10];
  char input[MAXCMD], inputline[MAXCMD], filename[MAXCMD], line[MAXCMD];
  char sargv[MAXARG][MAXCMD];
  char fname[16];
  int i, n, status = 0;
  struct sockstruct {
    int port;
    char server[28];
  } totocc, toport, toccd, togccd, toapogee;
  char server[24];
  int frommaster=0, tomaster=0, fromalert=0, nread;
  int ntry, ndark;
  int command_mode = 1;
  double current_pa;

  int rah, ram, decd, decm, sign;
  double ras, decs;
  double rasec, decsec, dx, dy, guidefac;
  double ra, dec, newpa, pmra, pmdec, epoch;
  BOOL havepa;
  int dfoc;
  FILE *falert;
  int type, tjd, tsod;
  BOOL donealert = FALSE;
  BOOL doalert = FALSE;

  time_t t;
  struct tm *tm;
  struct ALLTIMES timeRec;

  FILE *pointing_file;
  int istan,iuser;

  FILE *tstatus, *tstatusready;
  FILE *cstatus, *cstatusready;

  double exptime;
  int nexp, filter=4, cleans, newext, ginst = 2, inst=1;
  int iret, faint;
  int foc0;
  double temp0;

  BOOL priv = TRUE;
  char *getenv(), *display;
  double displaynum;

  int maxset;
  fd_set readfds;
  struct timeval timeout;

  struct sockaddr_in addr;
  struct in_addr in_addr;
  int s, sock;

  signal(SIGINT,trap);

/* Open FIFOs for communication with a master program */
  fromalert = open("alert2com",O_RDWR);
  if (fromalert<0) {
     perror("PROGRAM command open fromalert");
  } else
    fromalertfp = fdopen(fromalert,"r+");
  frommaster = open("master2com",O_RDWR);
  if (frommaster<0) {
     perror("PROGRAM command open frommaster");
  } else
    frommasterfp = fdopen(frommaster,"r+");
  tomaster = open("com2master",O_RDWR);
  if (tomaster<0) {
     perror("PROGRAM command open tomaster");
  } else
    tomasterfp = fdopen(tomaster,"r+");
  for (i=0 ;i < MAXDEPTH ; i++)
    havepipe[i] = 0;

/* Open the socket for communication with port program */
#ifdef SOCKET
  toport.port = 1053;
  sprintf(toport.server,"ccd1m.apo.nmsu.edu"); 
  totocc.port = 1053;
  sprintf(totocc.server,"ccd1m.apo.nmsu.edu"); 
  toapogee.port = 1050;
  sprintf(toapogee.server,"sdss4-apogee.apo.nmsu.edu"); 
#else
  toport.port = 0;
  sprintf(toport.server,"toport");
#endif
  

/* Open the socket for communication with CCD program */
#ifdef NOCCD
  toccd.port = -1;
#else
#ifdef LOCALCOM
  toccd.port = 0;
  sprintf(toccd.server,"toccd");
#else
  toccd.port = 1050;
  sprintf(toccd.server,"192.41.211.21"); // eyeball
  sprintf(toccd.server,"eyeball.apo.nmsu.edu"); 
  sprintf(toccd.server,"192.41.211.19"); // ccd1m
  sprintf(toccd.server,"ccd1m.apo.nmsu.edu"); 
#endif
#endif

#ifdef NOGCCD
  togccd.port = -1;
#else
  //togccd.port = 0;
  //sprintf(togccd.server,"togccd");
  togccd.port = 1049;
  sprintf(togccd.server,"192.41.211.19"); // ccd1m
  sprintf(togccd.server,"ccd1m.apo.nmsu.edu"); 
#endif

/* Set default input to be stdin */
  infile[0] = stdin;
  outfile[0] = NULL;

/* Are we running remotely */
  display = getenv("DISPLAY");
  sscanf(strstr(display,":")+1,"%lf",&displaynum);
  if ((int)displaynum != 0) remote = TRUE;

#ifdef NEWFILT
/* Initialize filter information */
  status = initfilt();
  if (status != 0) {
    fprintf(stderr,"error initialing filter data!\n");
    for (i=0; i<MAXFILT; i++) {
      sprintf(filtname[i]," ");
      sprintf(longfiltname[i],"None");
    }
  }
  foc0=focoff[filter];
#endif

/* Initialize status information */
  initstatus(&statusinfo);

#ifdef HAVETOCC
  havetocc = TRUE;
#else
  havetocc = FALSE;
#endif

/* Setup focus matrix */
  foc = (double **)malloc(3*sizeof(double *));
  ifoc = (double **)malloc(3*sizeof(double *));
  junk = (double **)malloc(3*sizeof(double *));
  setup_focus(foc);
  setup_focus(ifoc);
  setup_focus(junk);
  gaussj(ifoc,3,junk,3);

  coord_init(longitude, latitude, altitude);

  fprintf(stderr,"power_status...\n");
  while (power_status(plug_status)<0) {
    sleep(2);
  }
/* Is telescope available? If so, initialize telescope if necessary */
  if (havetocc) {
    if (plug_status[MOTORS] == OFF) {
      fprintf(stderr,"Motor power is OFF\n");
      fprintf(stderr,"\n Do you wish to turn it ON (Y or N)? ");
      mygetline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0]=='y') {
        power_on(RACKFAN);
        power_on(MOTORS);
        while (power_status(plug_status) < 0) {
          sleep(2);
        }
      }
    }
    fprintf(stderr,"Waiting to get initial telescope status from dome....\n");
    ntry = 0;
#ifdef SOCKET
    status=getstatus(totocc.port, totocc.server, &statusinfo);
    while (ntry < 5 && 
           (status=getstatus(totocc.port, totocc.server, &statusinfo))<0) {
      sleep(2);
fprintf(stderr,"ntry: %d\n",ntry);
      ntry++;
    } 
#else
    while (ntry < 5 && 
           (status=readstatus(tstatusfile, tstatusreadyfile, &statusinfo))<0) {
      sleep(2);
      ntry++;
    } 
#endif
    if (status<0) {
      fprintf(stderr,"Telescope is not responding\n");
      if (plug_status[TOCC] == OFF) {
        fprintf(stderr,"Telescope control computer is powered OFF\n");
        fprintf(stderr,"\n Do you wish to turn it ON (Y or N)? ");
      } else if (plug_status[TOCC] == ON) {
        fprintf(stderr,
"\n Telescope control computer is powered ON, but is not responding"
 "You may wish to reboot it. However, if the control program is currently "
  "running, a reboot will stop the program ungracefully, and a restart will not"
  "be permitted without a call to Dave or Jon! BE CAREFUL!");
        fprintf(stderr,"\n Do you wish to reboot it (Y or N)? ");
      }
      mygetline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0]=='y') {
          fprintf(stderr,
"\n It will take some time for the control software to start. Be patient.\n");
          power_off(TOCC);
          sleep(5);
          power_on(TOCC);
          sleep(60);
#ifdef SOCKET
          ntry=0;
          while (getstatus(totocc.port, totocc.server, &statusinfo)<0) {
            sleep(2);
fprintf(stderr,"ntry b: %d\n",ntry);
            ntry++;
          }
#else
          while (readstatus(tstatusfile, tstatusreadyfile, &statusinfo)<0) {}
#endif
          fprintf(stderr,"Status received.\n");
          G = &statusinfo;
      } else {
        fprintf(stderr,
"\n Telescope control and status return has been disabled. "
 "You will not be able to communicate with the telescope\n\n");
        havetocc = FALSE;
        toport.port = -1;
      }

    } else {
//    while (readstatus(tstatusfile, tstatusreadyfile, &statusinfo)<0) {}
      fprintf(stderr,"Status received.\n");
      G = &statusinfo;
    }
    if (havetocc) {
      status = telescope_startup(toport.port,toport.server,0);
      error_code(status);

      status = telescope_settime(toport.port,toport.server);
      error_code(status);
    }
  }

/* Is CCD available? This assumes that if PI computer is on, the CCD software
   will be running */
  while (power_status(plug_status) < 0) {
    sleep(2);
  }
#ifdef HAVEPI
  if (plug_status[PI] != ON || plug_status[CCD] != ON) {
    fprintf(stderr,
     "Primary CCD control program or controller is powered off.\n");
    fprintf(stderr,"\n Do you wish to reboot them (Y or N)? ");
    mygetline(ans,sizeof(ans));
    if (ans[0]=='Y' || ans[0]=='y') {
          fprintf(stderr,
"\n It will take some time for the software to start. Be patient.\n");
          power_off(PI);
          power_off(CCD);
          power_off(FILL);
          sleep(5);
          power_on(PI);
          power_on(CCD);
          sleep(115);
    } else {
        fprintf(stderr,
"\n CCD control has been disabled.  You will not be able to communicate with the CCD\n\n");
        toccd.port = -1;
    }
  }
#endif
#ifdef OLDCOMPUTER
/* Check whether guider computer is on. This code implicitly assumes that
   if computer is on, it will be running the GCS software */
  if (plug_status[GCS] != ON) {
    fprintf(stderr,
     "Guider CCD control program is powered off.\n");
    fprintf(stderr,"\n Do you wish to reboot it (Y or N)? ");
    mygetline(ans,sizeof(ans));
    if (ans[0]=='Y' || ans[0]=='y') {
          fprintf(stderr,
"\n It will take some time for the software to start. Be patient.\n");
          power_off(GCS);
          sleep(5);
          power_on(GCS);
          sleep(35);


    } else {
        fprintf(stderr,
"\n Guider CCD control has been disabled.  You will not be able to communicate with the guider\n\n");
        togccd.port = -1;
    }
  }

/* Set the time on the guider computer if available */
  if (togccd.port >= 0) {
    status = telescope_settime(togccd.port,togccd.server);
    error_code(status);
  }
#endif
  if (!havetocc) {
    sprintf(command,"NOTOCC");
    if (toccd.port >= 0) sendccd(toccd.port,toccd.server,command,default_timeout);        
    if (togccd.port >= 0) sendccd(togccd.port,togccd.server,command,default_timeout);        
  }
  timeout.tv_sec = 0;
  timeout.tv_usec = 1000;

  fprintf(stderr,"Are you using the Leach CCD at NA1 or the APOGEE CAMERA at NA2?\n");
  fprintf(stderr,"  1. Leach / NA1\n");
  fprintf(stderr,"  2. APOGEE / NA2\n");
  fprintf(stderr,"Enter your choice: ");
  int choice;
  scanf("%d",&choice);
  if (choice==2) {
    //sprintf(toccd.server,"192.41.211.21"); // eyeball
    //sprintf(toccd.server,"eyeball.apo.nmsu.edu"); 
    sprintf(toccd.server,"ccd1m.apo.nmsu.edu"); 
    togccd.port = 1049;
    sprintf(togccd.server,"192.41.211.19"); // ccd1m
    sprintf(togccd.server,"ccd1m.apo.nmsu.edu"); 
    inst=3;
  } else {
    sprintf(toccd.server,"192.41.211.19"); // ccd1m
    sprintf(toccd.server,"ccd1m.apo.nmsu.edu"); 
    togccd.port = 1049;
    sprintf(togccd.server,"192.41.211.19"); // ccd1m
    sprintf(togccd.server,"ccd1m.apo.nmsu.edu"); 
    inst=1;
  }

/* Setup and read in initial standards file */
  sprintf(input,"scripts/standards.apo");
  status = readuser(input,1);

/* Command prompt */
  fprintf(stderr,"Command: ");

  while (command_mode) {

/*  Read the most current status information */
    if (havetocc) {
/*      while (readstatus(tstatusfile, tstatusreadyfile, &statusinfo)<0) {} */
#ifdef SOCKET
      status=getstatus(totocc.port, totocc.server, &statusinfo);
#else
      readstatus(tstatusfile, tstatusreadyfile, &statusinfo);
#endif
      G = &statusinfo;
    }

/* Check to see whether anything has come in on the fifos.
   If not, just wait */
    FD_ZERO(&readfds);
    FD_CLR(STDIN_FILENO,&readfds);
    if (frommaster>0) FD_CLR(frommaster,&readfds);
    if (fromalert>0) FD_CLR(fromalert,&readfds);
    FD_SET(STDIN_FILENO,&readfds);
    if (frommaster>0) FD_SET(frommaster,&readfds);
    if (fromalert>0) FD_SET(fromalert,&readfds);
#ifdef linux
    maxset = 0;
    maxset = STDIN_FILENO+1 > maxset ? STDIN_FILENO+1 : maxset ;
    if (frommaster>0) maxset = frommaster+1 > maxset ? frommaster+1 : maxset ;

    if (idepth>0 || select(maxset,&readfds,0,0,NULL)>0 ) {
#else
    if (select(FD_SETSIZE,&readfds,0,0,&timeout)>0 ) {
#endif

/*  Default status is OK */
    status = TCSERR_OK; 

    if ( FD_ISSET(STDIN_FILENO,&readfds) ) {
      infile[0] = stdin;
      outfile[0] = NULL;
    } else if ( fromalert>0 && FD_ISSET(fromalert,&readfds) ) {
      infile[0] = fromalertfp;
      outfile[0] = NULL;
    } else if ( frommaster>0 && FD_ISSET(frommaster,&readfds) ) {
      infile[0] = frommasterfp;
      outfile[0] = tomasterfp;
    }

/*  Command-mode commands: these are not acted upon until a CR is received */
/*
    if (frommaster>0 && idepth==0 && FD_ISSET(frommaster,&readfds) ) {
      infile[idepth+1] = frommasterfp;
      idepth++;
      havepipe[idepth]=1;
    } 
*/

    sprintf(inputline,"\n");
    if (ctrlc || mygetline(inputline,sizeof(inputline)) == -1) {
      if (idepth>0) {
        if (havepipe[idepth]==0) 
          fclose(infile[idepth]);
        else
          havepipe[idepth]=0;
        idepth--;
        if (havepipe[idepth]==1) error_code(0);
        if (ctrlc) fprintf(stderr,"Level %d terminated by CTRL-C\n",idepth+1);
      }
    }
    ctrlc = FALSE;
    if ( infile[0] != stdin) fprintf(stderr,"%s\n",inputline);

/*  Get the command name */
    sscanf(inputline,"%s",command);

/*  Convert command name to upper case */
    strupr(command);

    if (strlen(inputline)>1) 
    { 

/*  Read the most current status information */
    if (havetocc) {
     /* while (readstatus(tstatusfile, tstatusreadyfile, &statusinfo)<0) {} */
#ifdef SOCKET
      status=getstatus(totocc.port, totocc.server, &statusinfo);
#else
      readstatus(tstatusfile, tstatusreadyfile, &statusinfo);
#endif
      G = &statusinfo;
    }

/*  Log the command to the moves file if were in debug option */
    writeline(inputline,2);

    if ( donealert && infile[0] == frommasterfp) {
               donealert = FALSE; 
               status = -999;
    } else if (strcmp(command,"HP")==0 || strcmp(command,"?") == 0) {
      /* Help list */
               command_help();
    }
    else if (strcmp(command,"ALERT") == 0) {
               if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                 sscanf(sargv[1],"%d",&type);
               } else if (getargs(inputline,sargv,MAXCMD,MAXARG) == 6) {
                 sscanf(sargv[1],"%d",&type);
                 sscanf(sargv[2],"%lf",&ra);
                 sscanf(sargv[3],"%lf",&dec);
                 sscanf(sargv[4],"%d",&tjd);
                 sscanf(sargv[5],"%d",&tsod);
               } else {
                 fprintf(stderr,"Enter alert type :");
                 mygetline(input,sizeof(input));
                 sscanf(input,"%d",&type);
               }

               if (doalert && type != 3) {
		  #include "alert.c"
               }
    }
    else if (strcmp(command,"+ALERT") == 0) {
		doalert = TRUE;
                status = sendccd(toccd.port, toccd.server,inputline,default_timeout);            
    }
    else if (strcmp(command,"-ALERT") == 0) {
		doalert = FALSE;
                status = sendccd(toccd.port, toccd.server,inputline,default_timeout);            
    }
    else if (strcmp(command,"INPUT") == 0) {
               sscanf(&inputline[6],"%s",input);
               sprintf(filename,"scripts/%s",input);
               if (idepth+1<MAXDEPTH-1 && 
                   (infile[idepth+1] = fopen(filename,"r")) != NULL) {
                 idepth++;
               } else if (idepth+1 == MAXDEPTH-1) {
                 fprintf(stderr,"Too many input layers!\n");
               } else {
                 fprintf(stderr,"Cannot open input file: %s\n",&inputline[6]);
               }
    }
    else if (strcmp(command,"SLEEP") == 0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&i);
                 } else {
                  fprintf(stderr,"Enter number of seconds to sleep:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&i);
                 }
                 sleep(i);
    }
    // Coordinate move to a desired RA/DEC
    else if (strcmp(command,"CO")==0 || strcmp(command,"HA") ==0) {
		  sprintf(message,"  Using epoch : %.1f\r\n",current_epoch);
                  writeterm(message);
		  sprintf(message,"    (Use NE command to change)\r\n\r\n");
                  writeterm(message);

                 havepa = FALSE;
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&ra);
                   sscanf(sargv[2],"%lf",&dec);
                 } else if (getargs(inputline,sargv,MAXCMD,MAXARG) == 4) {
                   sscanf(sargv[1],"%lf",&ra);
                   sscanf(sargv[2],"%lf",&dec);
                   sscanf(sargv[3],"%lf",&newpa);
                   havepa = TRUE;
                 } else {

                 if (strcmp(command,"CO")==0) {
                    do {
                      getcoord("    RA\0",&ra,1);
                    } while (illegal_ra(ra));
                  }
                  else {
                    do {
                      getcoord("    HA\0",&ra,1);
                    } while (0);
                    ra = G->current_lasth - ra;
                  }
                  do {
                    getcoord("    DEC\0",&dec,1);
	          } while (illegal_dec(dec));
                  }

                  if (newpa > 360) {
                    double mjd_utc, az, alt, rot;
                    time_t *t;
                    mjd_utc = time(t)/3600./24. + 40587.0;
                    coord_altaz(mjd_utc,ra*DH2R,dec*DD2R,current_epoch,&az,&alt,&rot);
                    newpa = 90. * nint((newpa-1000.-(rot*DR2D))/90.);
                  }
		  sprintf(message,"  Proposed move to: \r\n");
                  writeterm(message);
		  sprintf(message,"     RA:  %f \r\n",ra);
                  writeterm(message);
		  sprintf(message,"     DEC: %f \r\n",dec);
                  writeterm(message);
                  if (havepa) {
		    sprintf(message,"      PA: %f \r\n",newpa);
                    writeterm(message);
                  }
		  sprintf(message,"     Airmass: %f \r\n",airmass(ra,dec));
                  writeterm(message);
		  sprintf(message,"  Hit <CR>/Y/y to move, anything else to abort: ");
                  writeterm(message);
	          if (mygetline(ans,sizeof(ans))>=0 && 
                      (ans[0]==0 || ans[0]==13) || ans[0]=='Y' || ans[0]=='y') {
                    ra = slaDranrm(ra*DH2R);
                    dec *= DD2R;
                    pmra = 0;
                    pmdec = 0;
                    sprintf(tmp,"GUIDEOFF");
                    status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                    status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                    guiding = FALSE;
                    sprintf(tmp,"OBJECT ");
                    status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                    sprintf(tmp,"OBJNUM 0");
                    status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                    if (havepa)
                      sprintf(tmp,
                       "XMOVE %10.7f %10.7f %f %f %f %f %f %f %f %f",
                       ra,dec,current_epoch,pmra,pmdec,0.,0.,0.55,0.0065,newpa);
                    else
                      sprintf(tmp,
                       "XMOVE %10.7f %10.7f %f %f %f %f %f %f %f",
                       ra,dec,current_epoch,pmra,pmdec,0.,0.,0.55,0.0065);
                    status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                    // store this position in list of saved positions
                    if (status==TCSERR_OK) {
                       savedra[isave] = ra * DR2H;
                       saveddec[isave] = dec * DR2D;
                       savedepoch[isave] = current_epoch;
                       isave = (++isave) % MAXSAVE;
                    }
		  }
    }

    // Change the current instrument position angle
    else if (strcmp(command,"PA")==0) {
              /*
                  if (!G->tracking_on)
                    fprintf(stderr,"Tracking is OFF!"
                            " Turn it on before rotating");
                  else {
              */
                    if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                      sscanf(sargv[1],"%lf",&current_pa);
                    } else {
                      fprintf(stderr,"Enter desired PA: ");
                      mygetline(input,sizeof(input));
                      sscanf(input,"%lf",&current_pa);
                    }
                    // Now move the telescope
                    sprintf(tmp,"GUIDEOFF");
                    status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                    status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                    guiding = FALSE;
                    sprintf(tmp,"XMOVEPA %f",current_pa);
                    status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
              /*
                  }
              */
    }

    // Get SAO star and move to it
    else if (strcmp(command,"SA")==0 || strcmp(command,"SAFOC")==0 ||
             strcmp(command,"HIP")==0 ||
             strcmp(command,"ALTAZ")==0 ) {

                  double outra, outdec, outv, vmin, vmax, epoch;
                  long outid, oldid[10];
                  int istat, nold;

                  // Get the current time for proper motion and for alt-az opt.
                  time(&t);
                  tm = gmtime(&t);
                  get_all_times_at(*tm, &timeRec);

		  if (strcmp(command,"SA")==0 || strcmp(command,"SAFOC")==0 || strcmp(command,"HIP")==0) {
                    fprintf(stderr,
                    "Find star near current telescope position (T),"
                    " other ra/dec (O), other alt/az (A), other ha/dec (H): ");
                    mygetline(ans,sizeof(ans));
                    fprintf(stderr,"\r\n");
                    ra = -1;
                    faint=0;
                    if (strcmp(command,"SAFOC")==0) faint=1;
                  }
                  if (strcmp(command,"ALTAZ")==0 || ans[0]=='A' || ans[0]=='a'){
                      double alt, az;
		      do {
                        getcoord("  Desired AZ\0",&az,1);
	              } while (illegal_az(az));

                      do {
                        getcoord("  Desired ALT\0",&alt,1);
         	      } while (illegal_alt(alt));
    // Convert the current alt-az to ra and dec and load into variables
                      slaDh2e(az*DD2R, alt*DD2R, latitude, &ra, &dec);
	              ra = slaDranrm(timeRec.last - ra);
                      slaPreces("FK4",timeRec.equinox,current_epoch,&ra,&dec);
                      ra *= DR2H ;
                      dec *= DR2D ;
                      newpa = 0;
                  } else if (ans[0]=='T' || ans[0]=='t') {
                    ra = G->current_obs_ra;
                    dec = G->current_obs_dec;
                    ra *= DR2H ;
                    dec *= DR2D ;
                    newpa =  G->current_pa * DR2D;
                  } else if (ans[0]=='H' || ans[0]=='h') {
		      do {
                          getcoord("  Desired HA\0",&ra,1);
	              } while (0);
                      ra = timeRec.last*DR2H - ra;
                      ra = (ra<0 ? ra+24 : ra);
                      do {
                        getcoord("  Desired DEC\0",&dec,1);
         	      } while (illegal_dec(dec));
                      newpa = 0;
                  } else if (ans[0]=='O' || ans[0]=='o') {
		      do {
                        getcoord("  Desired RA\0",&ra,1);
	              } while (illegal_ra(ra));
                      do {
                        getcoord("  Desired DEC\0",&dec,1);
         	      } while (illegal_dec(dec));
                      newpa = 0;
                  } else {
                    fprintf(stderr,"Unrecognized option - must be T,A,O, or H!");
                  }
                  if (strcmp(command,"ALTAZ")==0) {
                    ra *= DH2R;
                    dec *= DD2R;
                    pmra = 0;
                    pmdec = 0;
                    sprintf(tmp,"GUIDEOFF");
                    status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                    status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                    guiding = FALSE;
                    sprintf(tmp,"XMOVE %10.7f %10.7f %f %f %f %f %f %f %f %f",
                           ra,dec,2000.,pmra,pmdec,0.,0.,0.55,0.0065,newpa);
                    status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                  }
                  else if (ra >= 0) {
                    fprintf(stderr,"    Enter min, max magnitude (<cr> for all): ");
                    mygetline(input,sizeof(input));
                    vmin = -1;
                    vmax = 99;
                    sscanf(input,"%lf %lf",&vmin,&vmax);

                    ans[0] = 'a';
                    nold = 0;
                    while (ans[0]=='a' || ans[0]=='A' && 
                         ans[0] != 'q' && ans[0] != 'Q') {
                      if ( strcmp(command,"HIP")==0 )  {
                        G->current_mean_epoch = 2000.;
                        istat=gethip(ra,dec,latitude*DR2D,timeRec.equinox,vmin,vmax,
                          &outra,&outdec,&outid,&outv,25.0,oldid,nold,faint);
                      } else {
                        G->current_mean_epoch = 1950.;
                        istat=getsao(ra,dec,latitude*DR2D,timeRec.equinox,vmin,vmax,
                          &outra,&outdec,&outid,&outv,25.0,oldid,nold,faint);
                      }
                      if (istat != 0) {
                        fprintf(stderr,"error opening sao file\r\n");
                        ans[0] = 'q';
                      }
                      else if (outid<0) {
                        fprintf(stderr,"No star found in specified range\r\n");
                        ans[0] = 'q';
                      }
                      else {
		        sprintf(message,"  Proposed move to SAO star: %ld \r\n",outid);
                        writeterm(message);
		        sprintf(message,"      RA: %f \r\n",outra);
                        writeterm(message);
		        sprintf(message,"     DEC: %f \r\n",outdec);
                        writeterm(message);
                        sprintf(message,"       V: %f \r\n",outv);
                        writeterm(message);
		  sprintf(message,"     Airmass: %f \r\n",airmass(outra,outdec));
                        writeterm(message);
		        sprintf(message,"\n Hit <CR>/Y/y to move, "
                             "A to select another star, "
                             "anything else to abort: ");
                        writeterm(message);
	                if (mygetline(ans,sizeof(ans))>=0 && 
                            (ans[0]=='Y'||ans[0]=='y'||ans[0]==0||ans[0]==13)) {
                          outra *= DH2R;
                          outdec *= DD2R;
                          pmra = 0;
                          pmdec = 0;
                          sprintf(tmp,"GUIDEOFF");
                          status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                          status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                          guiding = FALSE;
                          sprintf(tmp,
                           "XMOVE %10.7f %10.7f %f %f %f %f %f %f %f %f",
                            outra,outdec,G->current_mean_epoch,pmra,pmdec,0.,0.,0.55,
                            0.0065,newpa);
                          status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                          // store this position in list of saved positions
                          if (status==TCSERR_OK) {
                           savedra[isave] = outra * DR2H;
                           saveddec[isave] = outdec * DR2D;
                           savedepoch[isave] = G->current_mean_epoch;
                           isave = (++isave) % MAXSAVE;
                           sprintf(tmp,"OBJECT SAO %d",outid);
                           status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                          }
                          sprintf(tmp,"OBJECT ");
                          status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                          sprintf(tmp,"OBJNUM 0");
                          status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
		        }
                        else if (ans[0]=='A' || ans[0] == 'a' && nold<9) {
                          oldid[nold++] = outid;
                        } 
                        else
                          ans[0] = 'q'; 
                      }
                    }
                  }
    }                      


    // Previous move
    else if (strcmp(command,"PM")==0) {
                  if (getsaved(&ra,&dec,&epoch) >0) {
		    fprintf(stderr,"  Proposed move to: \r\n");
		    fprintf(stderr,"     RA:  %f \r\n",ra);
		    fprintf(stderr,"     DEC: %f \r\n",dec);
		    fprintf(stderr,"     EPOCH: %f \r\n",epoch);
		  fprintf(stderr,"     Airmass: %f \r\n",airmass(ra,dec));
		    fprintf(stderr,"  Hit <CR>/Y/y to move, anything else to abort: ");
	            if (mygetline(ans,sizeof(ans))>=0 && 
                        (ans[0]=='Y'||ans[0]=='y'||ans[0]==0||ans[0]==13)) {
                      ra *= DH2R;
                      dec *= DD2R;
                      pmra = 0;
                      pmdec = 0;
                      sprintf(tmp,"OBJECT ");
                      status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                      sprintf(tmp,"OBJNUM 0");
                      status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                      sprintf(tmp,"GUIDEOFF");
                      status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                      status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                      guiding = FALSE;
                      sprintf(tmp,
                           "XMOVE %10.7f %10.7f %f %f %f %f %f %f %f",
                           ra,dec,epoch,pmra,pmdec,0.,0.,0.55,0.0065);
                      status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                      if (status==TCSERR_OK) {
                         savedra[isave] = ra * DR2H;
                         saveddec[isave] = dec * DR2D;
                         savedepoch[isave] = epoch;
                         isave = (++isave) % MAXSAVE;
                      }
                    }
                  }
    }

    // Relative move
    else if (strcmp(command,"QM")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&rasec);
                   sscanf(sargv[2],"%lf",&decsec);
                 } else {
                 fprintf(stderr,"Enter no. of arcsec to move in RA, DEC: ");
                 mygetline(input,sizeof(input));
                 sscanf(input,"%lf %lf",&rasec,&decsec);
                 }
                 if (fabs(rasec) > 3600 || fabs(decsec) > 3600) 
                   fprintf(stderr,"Maximum relative move size is 60 arcmin\r\n");
                 else {
                   sprintf(tmp,"GUIDEOFF");
                   status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                   status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                   guiding = FALSE;
                   sprintf(tmp,"XQM %f %f",rasec,decsec);
                   status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                 }
    }

    else if (strcmp(command,"OFFSET")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&dx);
                   sscanf(sargv[2],"%lf",&dy);
                 } else {
                   fprintf(stderr,"Enter no. of pixels to move in x, y: ");
                   mygetline(input,sizeof(input));
                   sscanf(input,"%lf %lf",&dx,&dy);
                 }
                 if (fabs(dx) > 10000 || fabs(dy) > 10000) 
                   fprintf(stderr,"Maximum relative move size is 10000 pixels\r\n");
                 else {
                   guiding = FALSE;
                   sprintf(tmp,"OFFSET %d %f %f",inst,dx,dy);
                   status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                   status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                 }
    }

    else if (strcmp(command,"DAA")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&dx);
                   sscanf(sargv[2],"%lf",&dy);
                 } else {
                   fprintf(stderr,"Enter amount to offset in az and alt: ");
                   mygetline(input,sizeof(input));
                   sscanf(input,"%lf %lf",&dx,&dy);
                 }
                 if (fabs(dx) > 5000 || fabs(dy) > 5000) 
                   fprintf(stderr,"Maximum relative move size is 5000 arcsec\r\n");
                 else {
                   sprintf(tmp,"GUIDEOFF");
                   status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                   status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                   guiding = FALSE;
                   sprintf(tmp,"DAA %f %f",dx,dy);
                   status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                 }
    }

    else if (strcmp(command,"GOFFSET")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&dx);
                   sscanf(sargv[2],"%lf",&dy);
                 } else {
                   fprintf(stderr,"Enter no. of guider pixels to move in x, y: ");
                   mygetline(input,sizeof(input));
                   sscanf(input,"%lf %lf",&dx,&dy);
                 }
                 if (fabs(dx) > 5000 || fabs(dy) > 5000) 
                   fprintf(stderr,"Maximum relative move size is 5000 pixels\r\n");
                 else {
                   //sprintf(tmp,"GUIDEOFF");
                   //status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                   //status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                   guiding = FALSE;
                   sprintf(tmp,"GUIDEOFF");
                   status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                   sprintf(tmp,"OFFSET %d %f %f",ginst,dx,dy);
                   status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                   status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                   sleep(2);
                   sprintf(tmp,"GUIDEON");
                   status = sendccd(togccd.port, togccd.server, tmp,default_timeout);
                 }
    }

    else if (strcmp(command,"RATES")==0 || strcmp(command,"DRIFT")==0) {
        double dra, ddec;
        if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
          sscanf(sargv[1],"%lf",&dra);
          sscanf(sargv[2],"%lf",&ddec);
        } else {
          fprintf(stderr,"Enter new ra and dec rates (arcsec/hr)): ");
          mygetline(inputline,sizeof(inputline));
          sscanf(inputline,"%lf%lf",&dra,&ddec);
        }
        sprintf(tmp,"RATES %f %f" ,dra,ddec);
        status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    // Relative focus command
    else if (strcmp(command,"DF")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%ld",&dfoc);
                 } else {
                  fprintf(stderr,"Enter delta focus to apply (steps): ");
                  mygetline(inputline,sizeof(inputline));
                  sscanf(inputline,"%ld",&dfoc);
                 }
                 sprintf(tmp,"XFOCUS %ld %ld %ld",dfoc,dfoc,dfoc);
                 if (fabs(dfoc)>0.1) status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    // Absolute focus command
    else if (strcmp(command,"AUTOFOC")==0) {
      fprintf(stderr,"autofoc recommends: %d", autofoc(toport.port,toport.server));
      fprintf(stderr," at %lf\n",G->current_aux_temp);
    }

    else if (strcmp(command,"FO")==0 || strcmp(command,"AUTOFOC")==0) {
                 calc_focus(ifoc,G);
                 if (strcmp(command,"FO")==0 ) {
                   if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                     sscanf(sargv[1],"%lf",&G->foc);
                   } else {
                     fprintf(stderr,"Enter new focus: ");
                     mygetline(inputline,sizeof(inputline));
                     sscanf(inputline,"%lf",&G->foc);
                   }
                 } else {
		   G->foc = autofoc(toport.port,toport.server);
                 }
                 // Add in filter focus offset since we want to send RAW focus value
                 G->foc += focoff[filter];
                 // Move 50 steps short of desired position
                 G->foc -= 50;
                 calc_focus_steps(foc,out,G);
                 sprintf(tmp,"XFOCUS %ld %ld %ld",
                   (int)out[0],(int)out[1],(int)out[2]);
                 status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                 // Now move to desired position from negative side
                 sprintf(tmp,"XFOCUS 50 50 50");
                 status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
		 // Set current filter focus offset and current temperature
		 foc0 = focoff[filter];
		 temp0 = G->current_aux_temp;
    }

    else if (strcmp(command,"AUTODFOC")==0) {
		 doautodfoc(&temp0,toport.port,toport.server);
    }

    // Do a focus run
    else if (strcmp(command,"FOCRUN")==0) {
                 //status= focrun(toport.port,toport.server,toccd.port, toccd.server,0,1,filter);
                 status= focrun(toport.port,toport.server,toccd.port, toccd.server,0,0,filter);
    }

    // Do a focus run
    else if (strcmp(command,"SLOWFOC")==0) {
                 status= focrun(toport.port,toport.server,toccd.port, toccd.server,0,0,filter);
    }

    // Do a focus run with guider
    else if (strcmp(command,"GFOCRUN")==0) {
                 status =focrun(toport.port,toport.server,togccd.port, togccd.server,1,0,filter);
    }

    // Home secondary
    else if (strcmp(command,"SHO")==0 && priv) {
                 status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // Absolute tilt commands
    else if (strcmp(command,"XTILT")==0 && priv) {
                 calc_focus(ifoc,G);
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&G->foc_theta);
                 } else {
                   fprintf(stderr,"Enter new x tilt: ");
                   mygetline(inputline,sizeof(inputline));
                   sscanf(inputline,"%lf",&G->foc_theta);
                 }
                 calc_focus_steps(foc,out,G);
                 sprintf(tmp,"XFOCUS %ld %ld %ld",
                   (int)out[0],(int)out[1],(int)out[2]);
                 status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    else if (strcmp(command,"YTILT")==0 && priv) {
                 calc_focus(ifoc,G);
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&G->foc_phi);
                 } else {
                   fprintf(stderr,"Enter new y tilt: ");
                   mygetline(inputline,sizeof(inputline));
                   sscanf(inputline,"%lf",&G->foc_phi);
                 }
                 calc_focus_steps(foc,out,G);
                 sprintf(tmp,"XFOCUS %ld %ld %ld",
                   (int)out[0],(int)out[1],(int)out[2]);
                 status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    // New equinox
    else if (strcmp(command,"NE")==0) {
                 fprintf(stderr,"Current epoch for coordinates: %f\r\n",current_epoch);
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&current_epoch);
                 } else {
                   fprintf(stderr,"Enter new epoch for coordinates: ");
                   mygetline(input,sizeof(input));
                   sscanf(input,"%lf",&current_epoch);
                 }
    }

    // Update coordinates
    else if (strcmp(command,"UC")==0 || (strcmp(command,"UH")==0 && priv) ) {
                    fprintf(stderr," Enter desired position to update to from %d"
                            " last moves,\r\n"
                            " or 0 to enter a new position manually\r\n\n",
                            MAXSAVE);
                    iret = getsaved(&ra,&dec,&epoch);
                    if (iret >= 0) {
                      if (iret > 0) {
                        ra *= DH2R;
                        dec *= DD2R;
                      } else {
		        fprintf(stderr,"  Using epoch : %.1f\r\n",current_epoch);
		        fprintf(stderr,"    (Use NE command to change)\r\n\r\n");
                        epoch = current_epoch;
		        do {
                          getcoord(" Current RA\0",&ra,1);
	                } while (illegal_ra(ra));
  
                        do {
                          getcoord(" Current DEC\0",&dec,1);
	                } while (illegal_dec(dec));
                        ra *= DH2R;
                        dec *= DD2R;
                      }
                      pmra = 0;
                      pmdec = 0;
                      // Set the current coordinates
                      if (strcmp(command,"UC") == 0) {
                        sprintf(input,"XMARK %f %f %f %f %f %f %f %f %f %d",
                           ra,dec,epoch,pmra,pmdec,0.,0.,0.55,0.0065,1);
                        status = sendport(toport.port,toport.server,input,ret,MAXCMD);
                      }
                      else {
                        fprintf(stderr,
                        "\r\nEnter (Y) to confirm that you really want "
                        "to change the home positions!"); 
                        mygetline(ans,sizeof(ans));
                        if (ans[0]=='Y' || ans[0] == 'y') {
                        sprintf(input,"XMARK %f %f %f %f %f %f %f %f %f %d",
                           ra,dec,epoch,pmra,pmdec,0.,0.,0.55,0.0065,0);
                        status = sendport(toport.port,toport.server,input,ret,MAXCMD);
                        } else 
                          fprintf(stderr,"\r\nTelescope home positions have NOT"
                                " been changed");
                        status = TCSERR_OK;
                      }
                    }
    }
    // Update rotator position
    else if (strcmp(command,"UR")==0) {
                 double drot;
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&drot);
                 } else {
                  fprintf(stderr,"Enter rotator adjustment (degrees): ");
                  mygetline(inputline,sizeof(inputline));
                  sscanf(inputline,"%lf",&drot);
                 }
                 sprintf(tmp,"UR %lf",drot);
                 status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    // Open user coordinate file and read it in
    else if (strcmp(command,"OF")==0 || strcmp(command,"OS")==0) {
	          if ( strcmp(command,"OF")==0 )
                    iuser=0;
                  else
                    iuser=1;
		  fprintf(stderr,"Enter name of file to open: ");
	          mygetline(input,sizeof(input));
                  sprintf(filename,"scripts/%s",input);
                  status = readuser(filename,iuser);
 
    }

    // Get entry from user coordinate file
    else if (strcmp(command,"RF")==0 || strcmp(command,"RS")==0) {
	          if ( strcmp(command,"RF")==0 )
                    iuser=0;
                  else
                    iuser=1;
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&i);
                 } else {
                  fprintf(stderr,"Enter star number:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&i);
                 }
                 if (i<=nuser[iuser]) {
                   i--;      // Get to 0-indexed array
                   istan = i;
		   sprintf(message,"  Proposed move to star: %s \r\n",userobj[iuser][i]);
                   writeterm(message);
		   sprintf(message,"      RA: %f \r\n",userra[iuser][i]*DR2H);
                   writeterm(message);
		   sprintf(message,"     DEC: %f \r\n",userdec[iuser][i]*DR2D);
                   writeterm(message);
		   sprintf(message,"     Epoch: %f \r\n",userepoch[iuser][i]);
                   writeterm(message);
		  sprintf(message,"     Airmass: %f \r\n",airmass(userra[iuser][i]*DR2H,userdec[iuser][i]*DR2D));
                   writeterm(message);
		   fprintf(stderr,"\n Hit <CR> to move, "
                   	"A to select another star, "
                             "anything else to abort: ");
	           if (mygetline(ans,sizeof(ans))>=0 && 
                      (ans[0]=='Y'||ans[0]=='y'||ans[0]==0||ans[0]==13)) {
                     sprintf(tmp,"OBJECT %s",userobj[iuser][i]);
                     status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
	             if ( strcmp(command,"RF")==0 )
                       sprintf(tmp,"OBJNUM %d",i+1);
                     else
                       sprintf(tmp,"STANNUM %d",i+1);
                     status = sendccd(toccd.port, toccd.server, tmp,default_timeout);
                     sprintf(tmp,"GUIDEOFF");
                     status = sendccd(togccd.port, togccd.server,tmp,default_timeout);
                     status = sendccd(toccd.port, toccd.server,tmp,default_timeout);
                     guiding = FALSE;
                     sprintf(tmp,
                       "XMOVE %10.7f %10.7f %f %f %f %f %f %f %f 0.",
                       userra[iuser][i],userdec[iuser][i],userepoch[iuser][i],
                       userpmra[iuser][i],userpmdec[iuser][i],
                       0.,0.,0.55,0.0065);
                     status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                     if (status==TCSERR_OK) {
                         savedra[isave] = userra[iuser][i] * DR2H;
                         saveddec[isave] = userdec[iuser][i] * DR2D;
                         savedepoch[isave] = userepoch[iuser][i];
                         isave = (++isave) % MAXSAVE;
                     }
                   }
                 } else
                     fprintf(stderr,"No such star in user catalog\n");
    }

    // Take an exposure with precomputed exposure time in specified filter
    else if (strcmp(command,"STAN")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%15s",&fname);
                   filter = getfilt(fname);
                 } else {
#ifdef NEWFILT
                  fprintf(stderr,"Enter filter name:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%15s",&fname);
                  filter = getfilt(fname);
#else
                  fprintf(stderr,"Enter filter number:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&filter);
#endif
                 }
                 if (filter < 1 || filter > MAXFILT) {
                   fprintf(stderr, "ERROR: no such filter!\n");
                 } else if (nuser[1] <= 0) {
                   fprintf(stderr,
                    "ERROR: you must read in standards file (OS) first!\n");
                 } else {
	           i = mag[filter];
                   if (i < 0) {
	             fprintf(stderr,"No magnitude specified for this filter\n");
                   } else if (usermag[1][istan][i] < 1) {
                     fprintf(stderr,
                       "ERROR: no mag specified for this star in file!\n");
                   } else {
                     dofilter(filter,toport.port, toport.server, 
                      toccd.port, toccd.server, togccd.port, togccd.server);

                     dfoc = focoff[filter] - foc0;
                     fprintf(stderr,"Adjusting telescope focus: %d\n",dfoc);
                     sprintf(tmp,"XFOCUS %ld %ld %ld",dfoc,dfoc,dfoc);
                     if (fabs(dfoc)>0.1) status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
		     foc0 = focoff[filter];

		     exptime = 
                        pow(10,-0.4*(usermag[1][istan][i] - zero[filter]));
	             exptime = 25000. / exptime * fudge;
		     exptime = ( exptime > 1 ? ceil(exptime) : 1);
		     exptime = ( exptime > 30 ? 30 : exptime);

                     status = do_exposure(toport.port,toport.server,
                               toccd.port, toccd.server, exptime,0);

	           }
                 }
    }

    else if (strcmp(command,"MAG")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == MAXFILT+1) {
                   for (i=1 ; i <= MAXFILT ; i++)
                     sscanf(sargv[i],"%d",&mag[i]);
                 } else {
	           fprintf(stderr,"You must specify MAXFILT numbers!");
                 }
    }
    else if (strcmp(command,"ZERO")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == MAXFILT+1) {
                   for (i=1 ; i <= MAXFILT ; i++)
                     sscanf(sargv[i],"%lf",&zero[i]);
                 } else {
	           fprintf(stderr,"You must specify MAXFILT numbers!");
                 }
    }
    else if (strcmp(command,"FUDGE")==0) {
        if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
          sscanf(sargv[1],"%lf",&fudge);
        } else {
          fprintf(stderr,"Enter new fudge factor for exposure time:");
          mygetline(input,sizeof(input));
          sscanf(input,"%lf",&fudge);
        }
    }

    // Instrument selection
    else if (strcmp(command,"SETINST")==0 && priv) {
        status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }

    else if (strcmp(command,"INST")==0) {
        if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
          sscanf(sargv[1],"%d",&inst);
        } else {
          fprintf(stderr,"Enter instrument number:");
          mygetline(input,sizeof(input));
          sscanf(input,"%d",&inst);
        }
        status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }

    else if (strcmp(command,"GINST")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&ginst);
                 } else {
                  fprintf(stderr,"Enter instrument number for guider:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&ginst);
                 }
                 status = sendport(togccd.port, togccd.server,&inputline[1],ret,MAXCMD);
    }

    // Toggle tracking
    else if (strcmp(command,"DAZ")==0 || 
             strcmp(command,"DALT")==0 ||
             strcmp(command,"DROT")==0 ) {
                   status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }
    // Toggle tracking
    else if (strcmp(command,"TR")==0) {
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // Toggle full use of encoders for pointing
    else if (strcmp(command,"EP")==0 && priv) {
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // Toggle full use of encoders for tracking
    else if (strcmp(command,"ET")==0 && priv) {
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    else if (strcmp(command,"ETX")==0 && priv) {
                   sprintf(tmp,"XET 1");
                   status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    else if (strcmp(command,"ETY")==0 && priv) {
                   sprintf(tmp,"XET 2");
                   status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    else if (strcmp(command,"ETZ")==0 && priv) {
                   sprintf(tmp,"XET 3");
                   status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }

    // Adjust the tracking "delta" time
    else if (strcmp(command,"TD")==0 && priv) {
		 int dtime;
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&dtime);
                 } else {
                   fprintf(stderr,"Enter new tracking dtime: ");
                   mygetline(input,sizeof(input));
                   sscanf(input,"%d",&dtime);
                 }
                 sprintf(tmp,"XTD %d",dtime);
                 status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }
 
    // Adjust the tracking rate factors
    else if (strcmp(command,"TFX")==0 && priv) {
                  double tracking_factor;
                  int timer;
                  if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                    sscanf(sargv[1],"%lf",&tracking_factor);
                    sscanf(sargv[2],"%d",&timer);
                  } else {
                    fprintf(stderr,"Enter new tracking factor :");
                    mygetline(input,sizeof(input));
                    sscanf(input,"%lf",&tracking_factor);
                    fprintf(stderr,"Enter time in secs to implement :");
                    mygetline(input,sizeof(input));
                    sscanf(input,"%d",&timer);
                  }
                  sprintf(input,"XTF 1 %f %d",tracking_factor,timer);
                  status = sendport(toport.port,toport.server,input,ret,MAXCMD);
    }

    else if (strcmp(command,"TFY")==0 && priv) {
                  double tracking_factor;
                  int timer;
                  if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                    sscanf(sargv[1],"%lf",&tracking_factor);
                    sscanf(sargv[2],"%d",&timer);
                  } else {
                    fprintf(stderr,"Enter new tracking factor :");
                    mygetline(input,sizeof(input));
                    sscanf(input,"%lf",&tracking_factor);
                    fprintf(stderr,"Enter time in secs to implement :");
                    mygetline(input,sizeof(input));
                    sscanf(input,"%d",&timer);
                  }
                  sprintf(input,"XTF 2 %f %d",tracking_factor,timer);
                  status = sendport(toport.port,toport.server,input,ret,MAXCMD);
    }

    else if (strcmp(command,"TFZ")==0 && priv) {
                  double tracking_factor;
                  int timer;
                  if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                    sscanf(sargv[1],"%lf",&tracking_factor);
                    sscanf(sargv[2],"%d",&timer);
                  } else {
                    fprintf(stderr,"Enter new tracking factor :");
                    mygetline(input,sizeof(input));
                    sscanf(input,"%lf",&tracking_factor);
                    fprintf(stderr,"Enter time in secs to implement :");
                    mygetline(input,sizeof(input));
                    sscanf(input,"%d",&timer);
                  }
                  sprintf(input,"XTF 3 %f %d",tracking_factor,timer);
                  status = sendport(toport.port,toport.server,input,ret,MAXCMD);
    }

    else if (strcmp(command,"TF")==0 && priv) {
         double tracking_factor;
         fprintf(stderr,"Enter new tracking factor :");
         mygetline(input,sizeof(input));
         sscanf(input,"%lf",&tracking_factor);
         sprintf(input,"XTF 0 %f",tracking_factor);
         status = sendport(toport.port,toport.server,input,ret,MAXCMD);
    }

    // Following commands just get transmitted directly without any I/O, but they
    //   are split up a bit for better readability

    // home or initialize the telescope by moving to home switches
    else if ( strcmp(command,"IOSERVER")==0) {
         status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }

    else if ( strcmp(command,"INIT")==0) {
         status = telescope_startup(toport.port,toport.server,1);
    }

    else if ( strcmp(command,"REINIT")==0) {
         status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    else if ( strcmp(command,"HO")==0 || strcmp(command,"IN")==0 ) {
         if (strcmp(command,"HO")==0) 
           status = telescope_init(toport.port,toport.server,1);
         else
           status = telescope_init(toport.port,toport.server,0);
    }

    // stow the telescope in the parking position
    else if (strcmp(command,"ST")==0||strcmp(command,"STUP")==0) {
         if (G->mirror_covers_open != 0) {
                   fprintf(stderr,"WARNING: mirror covers open\n");
/*
                   fprintf(stderr,"Do you wish to close them before stowing telescope (Y/N)");
                   mygetline(ans,sizeof(ans));
                   if (ans[0]=='Y' || ans[0] == 'y') {
                     sprintf(input,"CM");
                     status = sendport(toport.port,toport.server,input,ret,MAXCMD);
                   }
*/
         }
         status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // put the rotator in the park/fill position
    else if (strcmp(command,"FI")==0 || strcmp(command,"FILL")==0 ) {
         double filltime;
         n=sscanf(inputline,"%*s%lf",&filltime);
         if (n<1) filltime=7;
         sprintf(server,"192.41.211.19"); // ccd1m
         sprintf(server,"ccd1m.apo.nmsu.edu"); 
         sprintf(inputline,"FILLRESET");
         status = sendccd(toccd.port,server,inputline,default_timeout);
         sprintf(inputline,"FI %f",filltime);
         power_on(FILL);
         status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
         power_off(FILL);
         sprintf(inputline,"FILLRESET");
         status = sendccd(toccd.port,server,inputline,default_timeout);
    }
    else if (strcmp(command,"FILLRESET")==0) {
         sprintf(server,"192.41.211.19"); // ccd1m
         sprintf(server,"ccd1m.apo.nmsu.edu"); 
         status = sendccd(toccd.port,server,inputline,default_timeout);
         //status = sendccd(toccd.port,toccd.server,inputline,default_timeout);
    }

    else if ((strcmp(command,"ZDISABLE")==0 || strcmp(command,"ZENABLE")==0 ||
              strcmp(command,"+ZENCODER")==0 || strcmp(command,"-ZENCODER")==0)
             && priv ) {
         status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }


    // Toggle dome slaving or initialize dome
    else if ( strcmp(command,"DM")==0 || strcmp(command,"DS")==0 ) {
         sprintf(input,"DM");
         status = sendport(toport.port,toport.server,input,ret,MAXCMD);
    }

    else if ( strcmp(command,"DI") == 0 || strcmp(command,"DIFORCE")==0 ) {
         status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }

    // move dome
    else if ( strcmp(command,"XDOME")==0 || strcmp(command,"DA") == 0 ||
              strcmp(command,"DOMEAZ")==0 ) {
         int domeaz;
         if (G->dome_slaved) {  
            sprintf(input,"DM");
            status = sendport(toport.port,toport.server,input,ret,MAXCMD);
         }
         n=sscanf(inputline,"%*s%d",&domeaz);
         if (n<1) {
           fprintf(stderr,"Enter desired dome azimuth: ");
           scanf("%d",&domeaz);
         }
         sprintf(input,"XDOME %d",domeaz);
         status = sendport(toport.port,toport.server,input,ret,MAXCMD);
    }

    // Open/close dome
    else if ( strcmp(command,"OD")==0 ) {
      fprintf(stderr,
        "\nOpening the dome will expose the telescope to the weather!\n");
      fprintf(stderr,"\nAre you SURE you wish to open the dome (Y or N)? ");
      mygetline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0]=='y') {
/*
        if (G->mirror_covers_open != 0) {
           fprintf(stderr,"WARNING: mirror covers open\n");
           fprintf(stderr,"In general, it is a BAD IDEA to close the dome with mirror covers open!!!\n");
           fprintf(stderr,"Do you wish to close them before closing dome (Y/N)");
           mygetline(ans,sizeof(ans));
           if (ans[0]=='Y' || ans[0] == 'y') {
             sprintf(input,"CM");
             status = sendport(toport.port,toport.server,input,ret,MAXCMD);
           }
        }
*/
        sprintf(input,"XOD");
        status = sendport(toport.port,toport.server,input,ret,MAXCMD);
        //status = dome_open_close(toport.port,toport.server,input);
      }
    }

    else if ( strcmp(command,"CD")==0 ) {
/*
        if (G->mirror_covers_open != 0) {
           fprintf(stderr,"WARNING: mirror covers open\n");
           fprintf(stderr,"In general, it is a BAD IDEA to open the dome with mirror covers open!!!\n");
           fprintf(stderr,"Do you wish to close them before closing dome (Y/N)");
           mygetline(ans,sizeof(ans));
           if (ans[0]=='Y' || ans[0] == 'y') {
             sprintf(input,"CM");
             status = sendport(toport.port,toport.server,input,ret,MAXCMD);
           }
        }
*/
        sprintf(input,"XCD");
        status = sendport(toport.port,toport.server,input,ret,MAXCMD);
        //status = dome_open_close(toport.port,toport.server,input);
    }

    else if ( strcmp(command,"OLD")==0 || strcmp(command,"CLD")==0 ||
              strcmp(command,"XOD")==0 || strcmp(command,"XCD")==0 ||
              strcmp(command,"ODS")==0 || strcmp(command,"CDS")==0 ) {
                   status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }

    // Open/close dome louvers
    else if ( strcmp(command,"OL")==0 ) {
      fprintf(stderr,"\nOpening the louvers will expose the telescope to the weather!\n");
      fprintf(stderr,"\nAre you SURE you wish to open the louvers (Y or N)? ");
      mygetline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0]=='y') {
        power_on(LOUVERS);
        fprintf(stderr,
"REMEMBER to close the vents when you are done by killing power to the dome\n");
      }
    }

    else if ( strcmp(command,"CL")==0 ) {
        power_off(LOUVERS);
    }

    // Open/close  mirror covers
    else if ( strcmp(command,"OM")==0 || strcmp(command,"CM")==0 ) {
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // Get the weather
    else if ( strcmp(command,"WE")==0 ) {
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // Toggle 3.5m slaving
    else if ( strcmp(command,"+35M")==0 || strcmp(command,"-35M")==0 ) {
                   if (strcmp(command,"+35M") == 0) 
                     system("rm -f /home/export/tocc/louver.dat");
                   else
                     system("CLEAR_LOUVER");
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // Clear shutdown flags
    else if ( strcmp(command,"CLEAR")==0 ) {
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    else if ( strcmp(command,"CLEAR_LOUVER")==0 ) {
                   system(command);
    }

    else if ( strcmp(command,"NOCCD")==0 ) {
                   toccd.port = -1;
    }

    else if ( strcmp(command,"+CCD")==0 ) {
                   toccd.port = 1050;
    }

    else if ( strcmp(command,"EYEBALL")==0 || strcmp(command,"APOGEE")==0 ) {
          sprintf(toccd.server,"192.41.211.21"); // eyeball
          sprintf(toccd.server,"eyeball.apo.nmsu.edu"); 
          sprintf(toccd.server,"ccd1m.apo.nmsu.edu"); 
          togccd.port = 1050;
          sprintf(togccd.server,"192.41.211.21"); // eyeball
          sprintf(togccd.server,"eyeball.apo.nmsu.edu"); 
          sprintf(togccd.server,"ccd1m.apo.nmsu.edu"); 
          inst = 3;
    }

    else if ( strcmp(command,"CCD1M")==0 || strcmp(command,"LEACH")==0 ) {
          sprintf(toccd.server,"192.41.211.19"); // ccd1m
          sprintf(toccd.server,"ccd1m.apo.nmsu.edu"); 
          togccd.port = 1049;
          sprintf(togccd.server,"192.41.211.19"); // ccd1m
          sprintf(togccd.server,"ccd1m.apo.nmsu.edu"); 
    }

    else if ( strcmp(command,"NOGCCD")==0 ) {
                   togccd.port = -1;
    }

    else if ( strcmp(command,"+GCCD")==0 ) {
                   togccd.port = 1050;
    }

    // Guider shutter heat on/off
    else if ( strcmp(command,"HEATON")==0 || strcmp(command,"HEATOFF")==0 ) {
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    // CCD readout speed
    else if ( strcmp(command,"CCDFAST")==0 || strcmp(command,"CCDSLOW")==0 ) {
                   status = sendccd(toccd.port,toccd.server,inputline,default_timeout);
	           //fprintf(stderr,"CCDFAST has been temporarily disabled!!\n");
                   //status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }

    else if ( strcmp(command,"FAST")==0 || strcmp(command,"NOFAST")==0 ) {
                   status = sendccd(toccd.port,toccd.server,inputline,default_timeout);
    }

    else if ( strcmp(command,"WINDOW")==0) {
                   status = sendccd(toccd.port,toccd.server,inputline,default_timeout);
    }
    else if ( strcmp(command,"GWINDOW")==0) {
                   status = sendccd(togccd.port,togccd.server,&inputline[1],default_timeout);
    }

    else if ( strcmp(command,"CCDTEMP")==0 ) {
#ifdef HAVEPI
                   status = sendport(toport.port,toport.server,command,ret,MAXCMD);
#else
                   status = sendport(toccd.port, toccd.server,command,ret,MAXCMD);
#endif
    }

    else if ( strcmp(command,"SETTEMP")==0 ) {
                   status = sendccd(toccd.port, toccd.server,inputline,default_timeout);
    }

    // Set the time
    else if ( strcmp(command,"SETTIME")==0 ) {
                   status = telescope_settime(toport.port,toport.server);
    }

    // Calibrate command
    else if (strcmp(command,"CAZ")==0 || strcmp(command,"CRO")==0 ||
             strcmp(command,"CDO")==0 ) {
                  status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }
    else if (strcmp(command,"AZSCALE")==0 || strcmp(command,"ROTSCALE")==0 ) {
                  status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }

    // Toggle use of mount correction
    else if (strcmp(command,"MC")==0 ){
                  status = sendport(toport.port,toport.server,command,ret,MAXCMD);
    }
    else if (strcmp(command,"TPOINT")==0 ) {
                  status = sendport(toport.port,toport.server,inputline,ret,MAXCMD);
    }

    else if (strcmp(command,"TPFILE")==0 ) {
                  fprintf(stderr,"Enter mount correction file name :");
                  mygetline(filename,sizeof(filename));
                  FILE *fp;
		  fp = fopen(filename,"r");
                  if (fp != NULL) {
                  double ia, ie, npae, ca, an, aw, nrx, nry;
                  ia = ie = npae = ca = an = aw = nrx = nry = 0.;
		  while (fgets(input,sizeof(input),fp) != NULL) {
                    if ( strstr(input,"IA") != NULL ) sscanf(input+8,"%lf",&ia);
                    if ( strstr(input,"IE") != NULL ) sscanf(input+8,"%lf",&ie);
                    if ( strstr(input,"NPAE") != NULL ) sscanf(input+8,"%lf",&npae);
                    if ( strstr(input,"CA") != NULL ) sscanf(input+8,"%lf",&ca);
                    if ( strstr(input,"AN") != NULL ) sscanf(input+8,"%lf",&an);
                    if ( strstr(input,"AW") != NULL ) sscanf(input+8,"%lf",&aw);
                    if ( strstr(input,"NRX") != NULL ) sscanf(input+8,"%lf",&nrx);
                    if ( strstr(input,"NRY") != NULL ) sscanf(input+8,"%lf",&nry);
                  }
                  fclose(fp);
	          sprintf(input,"TPOINT %f %f %f %f %f %f %f %f",
                          ia,ie,npae,ca,an,aw,nrx,nry);
                  fprintf(stderr,"input: %s\n",input);
                  status = sendport(toport.port,toport.server,input,ret,MAXCMD);
                  } else 
                    fprintf(stderr,"error opening file %s\n",filename);
    }

    // New mount correction file
    else if (strcmp(command,"RMC")==0 && priv) {
                  fprintf(stderr,"Enter mount correction file name (on PC) :");
                  mygetline(filename,sizeof(filename));
                  sprintf(input,"XRMC %s",filename);
                  status = sendport(toport.port,toport.server,input,ret,MAXCMD);
    }

    // Make yourself a privilidged user
    else if (strcmp(command,"PRIV")==0) {
 		  fprintf(stderr,"Enter password :");
	          mygetline(input,sizeof(input));
		  if (strcmp(input,"clyde") == 0) priv = TRUE;
    }

    // If command requires privilige, notify user
    else if (strcmp(command,"UH")==0 ||
             strcmp(command,"XTILT")==0 ||
             strcmp(command,"YTILT")==0 ||
             strcmp(command,"ET")==0 ||
             strcmp(command,"EP")==0 ||
             strcmp(command,"ETX")==0 ||
             strcmp(command,"ETY")==0 ||
             strcmp(command,"ETZ")==0 ||
             strcmp(command,"TD")==0 ||
             strcmp(command,"TF")==0 ||
             strcmp(command,"TFX")==0 ||
             strcmp(command,"TFY")==0 ||
             strcmp(command,"TFZ")==0 ) {
          fprintf(stderr,"Sorry - you must be a privilidged user to use command %s\n",
                     command); 
          fprintf(stderr,"Use the PRIV command (with password) to enable yourself\n");
    }

    // quit command mode
    else if (strcmp(command,"QU")==0 || strcmp(command,"QUIT")==0 ||
             strcmp(command,"EXIT")==0 || strcmp(command,"SHUTDOWN")==0 ) {
          sprintf(tmp,"GUIDEOFF");
          status = sendccd(togccd.port, togccd.server,tmp,default_timeout);
          status = sendccd(toccd.port, toccd.server,tmp,default_timeout);
/*
          if (idepth>0) {
            fclose(infile[idepth]);
            idepth--;
          } else {
*/
            if (havetocc && G->dome_open) {
              fprintf(stderr,"WARNING: the dome is OPEN\n");
              fprintf(stderr,"Do you wish to close the dome?");
              mygetline(ans,sizeof(ans));
              if (ans[0] == 'y' || ans[0] == 'Y')  {
                if (G->mirror_covers_open != 0) {
                   fprintf(stderr,"WARNING: mirror covers open\n");
                   fprintf(stderr,"Do you wish to close them before closing dome (Y/N)");
                   mygetline(ans,sizeof(ans));
                   if (ans[0]=='Y' || ans[0] == 'y') {
                     sprintf(input,"CM");
                     status = sendport(toport.port,toport.server,input,ret,MAXCMD);
                   }
                }
                sprintf(input,"CD");
                status = sendport(toport.port,toport.server,input,ret,MAXCMD);
              }
            }
            command_mode = !command_mode;
            if (havetocc) {
             fprintf(stderr,
                "Do you wish to shut down the TOCC program in the dome?");
             mygetline(ans,sizeof(ans));
             if (ans[0] == 'y' || ans[0] == 'Y') {
               sprintf(input,"QU");
               status = sendport(toport.port,toport.server,input,ret,MAXCMD);
               sleep(5);
             } else {
               fprintf(stderr,"The TOCC program is still running!\n"
                 " Do NOT kill power to the TOCC computer in the dome without"
                 " quitting the program \n gracefully from the dome or"
                 " from a restart of tcomm!!\n");
             }

            }

            //sprintf(command,"QU");
            //status = sendccd(toport.port, toport.server,command);
            sprintf(tmp,"QU");
            status = sendccd(toccd.port, toccd.server,tmp,default_timeout);
            sprintf(tmp,"QU");
            status = sendccd(togccd.port, togccd.server,tmp,default_timeout);
            if (!havetocc || ans[0] == 'y' || ans[0] == 'Y') {
              fprintf(stderr,
                 "Do you wish to kill power to EVERYTHING in the dome?");
              mygetline(ans,sizeof(ans));
              if (ans[0] == 'y' || ans[0] == 'Y') {
                 power_off(TOCC);
                 power_off(MOTORS);
                 power_off(RACKFAN);
                 power_off(LOUVERS);
              } else {
                 fprintf(stderr," Power is still ON in the dome!\n"
                       " Be careful about leaving the power on!!\n");
              }
            }
    }

    //CCD commands here
    else if (strcmp(command,"CCD")==0 && priv) {
       /* Send a STATUS first to make sure we get good times, coords */
                  // if (havetocc) {
                  //   sprintf(input,"STATUS");
                  //   while ( (status = sendport(toport.port,toport.server,input),ret,MAXCMD) 
                  //         != TCSERR_OK) printf("error %d\n",status);
                  // }
                  status = sendccd(toccd.port, toccd.server,&inputline[4],default_timeout);            
    }

// Imager commands
    else if (strcmp(command,"INITCCD")==0) {
                 status = sendccd(toccd.port, toccd.server, "INITCCD",default_timeout);
    }
    else if (strcmp(command,"CCDINIT")==0) {
                 status = sendccd(toccd.port, toccd.server,command,default_timeout);
    }
    else if (strcmp(command,"CENTER")==0) {
                 status = sendccd(toccd.port, toccd.server,command,default_timeout);
    }
    else if (strcmp(command,"LOCATE")==0) {
                 status = sendccd(toccd.port, toccd.server,command,default_timeout);
    }
    else if (strcmp(command,"FLAT")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) != 5) {
                   fprintf(stderr,
                     "You must enter 4 arguments with the FLAT command\n"); 
                 } else
                   status = sendccd(toccd.port, toccd.server,inputline,default_timeout);
    }
    else if (strcmp(command,">GUIDE")==0) {
                 status = sendccd(toccd.port, toccd.server,command,default_timeout);  
    }
    else if (strcmp(command,"FULLSCALE")==0 || strcmp(command,"SKYSCALE")==0) {
                 status = sendccd(toccd.port, toccd.server,command,default_timeout); 
    }
    else if (strcmp(command,"SAMESCALE")==0 || strcmp(command,"NEWSCALE")==0) {
                 status = sendccd(toccd.port, toccd.server,command,default_timeout);
    }
    else if (strcmp(command,"NEWCENT")==0) {
                 status = sendccd(toccd.port, toccd.server,inputline,default_timeout);
    }
    else if (strcmp(command,"SCALE")==0) {
                 status = sendccd(toccd.port, toccd.server,inputline,default_timeout);
    }
    else if (priv && strcmp(command,"INIT")==0) {
                 status = sendccd(toccd.port, toccd.server, command, default_timeout);
    }
    else if (strcmp(command,"FITS")==0 ||
             strcmp(command,"SDAS")==0 ||  
             strcmp(command,"+DISK")==0 ||  
             strcmp(command,"-DISK")==0 ||  
             strcmp(command,"+XFER")==0 ||  
             strcmp(command,"-XFER")==0 ||  
             strcmp(command,"+DISPLAY")==0 ||  
             strcmp(command,"-DISPLAY")==0 ||  
             strcmp(command,"IRAF")==0 ) {
                 status = sendccd(toccd.port, toccd.server,inputline,default_timeout);            
    }
    else if (strcmp(command,"TIMEOUT")==0) {
             if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&default_timeout);
             } else {
                  fprintf(stderr,"Enter default timeout:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&default_timeout);
             }
    }

    else if (strcmp(command,"XFER")==0) {
             fprintf(stderr,
   "The XFER command should be issued from the local host (e.g., ganymede)\n");
    }

    else if (strcmp(command,"APO")==0) {
        fprintf(stderr,"%d %s\n",toapogee.port, toapogee.server);
        status = sendport(toapogee.port,toapogee.server,inputline+4,ret,MAXCMD);
        if (status==0) {
          fprintf(stderr,"ret: %s\n",ret);
          sscanf(ret,"done %d",&status);
        }
    }

    else if (strcmp(command,"EXP")==0 || strcmp(command,"QCK")==0 ) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&exptime);
                 } else {
                  fprintf(stderr,"Enter exposure time:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%lf",&exptime);
                 }
                 if (strcmp(command,"QCK") == 0) {
                   sprintf(input,"-DISK");
                   status = sendccd(toccd.port,toccd.server,input,default_timeout);
                 }
                 if (exptime > 60 && !guiding ) 
                   fprintf(stderr,"WARNING: guiding is OFF\n");
                 status = do_exposure(toport.port,toport.server,toccd.port, 
                                      toccd.server, exptime,0);
                 if (strcmp(command,"QCK") == 0) {
                   sprintf(input,"+DISK");
                   status = sendccd(toccd.port,toccd.server,input,default_timeout);
                 }
    }

    else if (strcmp(command,"MEXP")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&exptime);
                   sscanf(sargv[2],"%d",&nexp);
                 } else {
                  fprintf(stderr,"Enter exposure time:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%lf",&exptime);
                  fprintf(stderr,"Enter number of exposures:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&nexp);
                 }
                 if (exptime > 60 && !guiding ) {
                   fprintf(stderr,"WARNING: guiding is OFF\n");
/*
		   fprintf(stderr,"  Hit <CR> to expose, anything else to abort: ");
	           if (mygetline(ans,sizeof(ans))>=0 && (ans[0]==0||ans[0]==13)) {
*/
                     for (i=0 ; i<nexp ; i++) {
                       fprintf(stderr,"Starting exposure %d of %d\n",i+1, nexp);
                       status = do_exposure(toport.port,toport.server,toccd.port,toccd.server,
                                 exptime,0);
                       if (ctrlc) break;
                     }
 /*                  } */
                 } else {
                   for (i=0 ; i<nexp ; i++) {
                     status = do_exposure(toport.port,toport.server,toccd.port,toccd.server,
                                 exptime,i);
                     if (ctrlc) break;
                   }
                 }
    }

// tertiary commands
    else if (strcmp(command,"TMOVE")==0 || strcmp(command,"TINIT")==0 ||
             strcmp(command,"THOME")==0 || strcmp(command,"THOMER")==0 || strcmp(command,"BRAKE")==0 ||
             strcmp(command,"NA1")==0   || strcmp(command,"NA2")==0 ||
             strcmp(command,"TSETNA1")==0   || strcmp(command,"TSETNA2")==0) {
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }

// guider commands
    else if (strcmp(command,"GINITCCD")==0) {
      status = sendccd(togccd.port, togccd.server, &inputline[1],default_timeout);
      status = sendport(toport.port, toport.server, &inputline[1],ret,MAXCMD);
    }
    else if (togccd.port>0 && strcmp(command,"GUIDEINIT")==0) {
      status = sendccd(togccd.port, togccd.server, inputline,default_timeout);
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (togccd.port>0 && strcmp(command,"GUIDEMOVE")==0) {
      status = sendccd(togccd.port, togccd.server, inputline,default_timeout);
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"GUIDEPOS")==0) {
      status = sendccd(togccd.port, togccd.server, inputline,default_timeout);
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"GUIDELOC")==0) {
      status = sendccd(togccd.port, togccd.server, inputline,default_timeout);
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"GSETHOME")==0) {
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"GUIDEFOC")==0) {
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"GUIDEADJ")==0) {
      status = sendport(togccd.port, togccd.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"GFO")==0 || strcmp(command,"GDF") == 0) {
      status = sendport(toport.port, toport.server, &inputline[1],ret,MAXCMD);
    }
    else if (togccd.port>0 && strcmp(command,"GUIDEHOME")==0) {
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"PCREAD")==0) {
      status = sendport(toport.port, toport.server, inputline,ret,MAXCMD);
    }
    else if (strcmp(command,"GCENTER")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GLOCATE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GFLAT")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GUPDATE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GSIZE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GTIME")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GCCDINIT")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GUIDEINST")==0) {
      status = sendccd(toport.port, toport.server,inputline);
    }
    else if (strcmp(command,"GUIDEFAC")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&guidefac);
                 } else {
                   fprintf(stderr,"Enter new guiding factor: ");
                   mygetline(input,sizeof(input));
                   sscanf(input,"%lf",&guidefac);
                 }
                 sprintf(tmp,"XGF %f",guidefac);
                 status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
    }
    else if (strcmp(command,"GWRITE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GNOWRITE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,">CAMERA")==0) {
      status = sendccd(togccd.port, togccd.server,command,default_timeout);
    }
    else if (priv && strcmp(command,"GOS")==0 ) {
      status = sendccd(togccd.port, togccd.server,command,default_timeout);
    }
    else if (priv && strcmp(command,"GINIT")== 0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"SCIGUIDE")==0 ) {
      status = sendccd(toccd.port, toccd.server,command,default_timeout);
      guiding = TRUE;
    }
    else if (strcmp(command,"GUIDE")==0 ) {
      if (inst == 3) 
        status = sendccd(toccd.port, toccd.server,inputline,default_timeout);
      else
        status = sendccd(togccd.port, togccd.server,inputline,default_timeout);
      guiding = TRUE;
    }
    else if (strcmp(command,"NEWFOC")==0 ) {
      status = sendccd(toccd.port, toccd.server,inputline,default_timeout);
    }
    else if (strcmp(command,"GNEWFOC")==0 ) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"NEWGUIDE")==0 ) {
      status = sendccd(togccd.port, togccd.server,inputline,default_timeout);
      guiding = TRUE;
    }
    else if (strcmp(command,"READUSNO")==0 ) {
      status = sendccd(togccd.port, togccd.server,inputline,default_timeout);
    }
    else if (strcmp(command,"GUIDEOFF")==0) {
      if (inst == 3) 
        status = sendccd(toccd.port, toccd.server,command,default_timeout);
      else
        status = sendccd(togccd.port, togccd.server,command,default_timeout);
      guiding = FALSE;
    }
    else if (strcmp(command,"GUIDEON")==0 ) {
      if (inst == 3) 
        status = sendccd(toccd.port, toccd.server,command,default_timeout);
      else
        status = sendccd(togccd.port, togccd.server,command,default_timeout);
      guiding = TRUE;
    }
    else if (strcmp(command,"+FOCHOLD")==0 || strcmp(command,"-FOCHOLD")==0 ) {
      status = sendccd(togccd.port, togccd.server,command,default_timeout);
    }
    else if (strcmp(command,"GFULLSCALE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GSAMESCALE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GNEWCENT")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GSCALE")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GNEWEXT")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if ( strcmp(command,"GSETTEMP")==0 ) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }

    else if (strcmp(command,"SHUTTER")==0 || strcmp(command,"NOSHUTTER")==0 ||
             strcmp(command,"OPEN")==0 || strcmp(command,"CLOSE")==0 ) {
      status = sendccd(togccd.port, togccd.server,inputline,default_timeout);
    }
    else if (strcmp(command,"GFITS")==0 ||
             strcmp(command,"G+XFER")==0 ||  
             strcmp(command,"G-XFER")==0 ||  
             strcmp(command,"G+DISPLAY")==0 ||  
             strcmp(command,"G-DISPLAY")==0 ||  
             strcmp(command,"G+DISK")==0 ||  
             strcmp(command,"G-DISK")==0) {
      status = sendccd(togccd.port, togccd.server,&inputline[1],default_timeout);
    }
    else if (strcmp(command,"GEXP")==0 || strcmp(command,"GQCK") == 0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&exptime);
                 } else {
                  fprintf(stderr,"Enter exposure time:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%lf",&exptime);
                 }
                 if (strcmp(command,"GQCK") == 0) {
                   sprintf(input,"-DISK");
                   status = sendccd(togccd.port,togccd.server,input,default_timeout);
                 }
                 sprintf(input,"EXP %d",(int)(exptime*1000.));
                 ccd_timeout = (default_timeout != 0 ? exptime+10+default_timeout : 0);
                 status = sendccd(togccd.port, togccd.server,input,ccd_timeout);
                 if (strcmp(command,"GQCK") == 0) {
                   sprintf(input,"+DISK");
                   status = sendccd(togccd.port,togccd.server,input,default_timeout);
                 }
    }

    else if (strcmp(command,"DARK")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%lf",&exptime);
                 } else {
                  fprintf(stderr,"Enter exposure time:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%lf",&exptime);
                 }
                 sprintf(tmp,"OBJECT DARK");
                 status = sendccd(toccd.port,toccd.server, tmp,default_timeout);
                 sprintf(input,"DARK %d",(int)(exptime*1000.));
                 status = sendccd(toccd.port,toccd.server,input,default_timeout);
    }
    else if (strcmp(command,"MDARK")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&exptime);
                   sscanf(sargv[2],"%d",&nexp);
                 } else {
                  fprintf(stderr,"Enter exposure time:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%lf",&exptime);
                  fprintf(stderr,"Enter number of exposures:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&nexp);
                 }
                 sprintf(tmp,"OBJECT DARK");
                 status = sendccd(toccd.port,toccd.server, tmp,default_timeout);
                 for (i=0 ; i<nexp ; i++) {
                   sprintf(input,"DARK %d",(int)(exptime*1000.));
                   status = sendccd(toccd.port,toccd.server,input,default_timeout);            
                   if (ctrlc) break;
                 }
    }

    else if (strcmp(command,"GDARK")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 3) {
                   sscanf(sargv[1],"%lf",&exptime);
                   sscanf(sargv[2],"%d",&ndark);
                 } else {
                  fprintf(stderr,"Enter exposure time:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%lf",&exptime);
                  fprintf(stderr,"Enter number of darks:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&ndark);
                 }
                 sprintf(input,"DARK %d %d",(int)(exptime*1000.),ndark);
                 status = sendccd(togccd.port, togccd.server,input,default_timeout);            
    }
    
    else if (strcmp(command,"FILTINIT")==0) {
                 status = sendccd(togccd.port, togccd.server,inputline,default_timeout);            
                 status = sendport(toport.port, toport.server,inputline,ret,MAXCMD);
    }

    else if (strcmp(command,"FILTER")==0) {
#ifdef NEWFILT
                 fprintf(stderr,"FILTER command no longer operational!\n");
                 fprintf(stderr,"Use SETFILT name instead\n");
#else
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&filter);
                 } else {
                  fprintf(stderr,"Enter filter number:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&filter);
                 }
                 dofilter(filter,toport.port, toport.server, toccd.port, toccd.server, togccd.port, togccd.server);
#endif
    }

#ifdef NEWFILT
    else if (strcmp(command,"SETFILT")==0 || 
             strcmp(command,"FFILT")==0 ||
             strcmp(command,"QFILT")==0) {
    // SETFILT : use focus offset, always approach focus from below
    // QFILT : use focus offset, go straight to focus
    // FFILT : don't use focus offset at all
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%15s",&fname);
                   filter = getfilt(fname);
                   if (filter > 0 && filter <= MAXFILT) {
                     dofilter(filter,toport.port, toport.server, toccd.port, toccd.server, togccd.port, togccd.server);
                    if (strcmp(command,"QFILT")==0 || strcmp(command,"SETFILT")==0) {
                     dfoc = focoff[filter] - foc0;
                     fprintf(stderr,"Adjusting telescope focus: %d\n",dfoc);
                     if (strcmp(command,"QFILT")==0 || dfoc>0) {
                       sprintf(tmp,"XFOCUS %ld %ld %ld",dfoc,dfoc,dfoc);
                       //fprintf(stderr,"%s\n",tmp);
                       if (fabs(dfoc)>0.1) status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                     } else if (dfoc<0) {
                       sprintf(tmp,"XFOCUS %ld %ld %ld",dfoc-50,dfoc-50,dfoc-50);
                       //fprintf(stderr,"%s\n",tmp);
                       status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                       sprintf(tmp,"XFOCUS 50 50 50");
                       //fprintf(stderr,"%s\n",tmp);
                       status = sendport(toport.port,toport.server,tmp,ret,MAXCMD);
                     }
                     if (strcmp(command,"SETFILT")==0 && dfoc != 0) {
                       sprintf(tmp,"GUIDEFOC %ld",dfoc);
                       fprintf(stderr,"Compensating guider focus...\n");
                       //fprintf(stderr,"%s\n",tmp);
                       status = sendport(toport.port, toport.server,tmp,ret,MAXCMD);
                     }
		     foc0 = focoff[filter];
                    }
                   }
                 } else {
                   fprintf(stderr,"Code\t\tFull description\n");
                   for (i=1 ; i<= MAXFILT ; i ++) {
                     fprintf(stderr,"%s\t\t%s\n",filtname[i],longfiltname[i]);
                   }
                 }
    }
#endif

    else if (strcmp(command,"CLEANS")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&cleans);
                 } else {
                  fprintf(stderr,"Enter number of cleans:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&cleans);
                 }
                 sprintf(input,"CLEANS %d",cleans);
                 status = sendccd(toccd.port,toccd.server,input,default_timeout);            
    }

    else if (strcmp(command,"NEWEXT")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%d",&newext);
                 } else {
                  fprintf(stderr,"Enter new extension for next frame:");
                  mygetline(input,sizeof(input));
                  sscanf(input,"%d",&newext);
                 }
                 sprintf(input,"SETINCVAL %d",newext);
                 status = sendccd(toccd.port,toccd.server,input,default_timeout);            
    }

    else if (strcmp(command,"FILNAM")==0 ||
             strcmp(command,"FILENAME")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%s",filename);
                 } else {
                  fprintf(stderr,"Enter root filename:");
                  mygetline(filename,sizeof(filename));
                 }
                 sprintf(input,"NAME %s",filename);
                 status = sendccd(toccd.port,toccd.server,input,default_timeout);            
    }
    
    else if (strcmp(command,"OBJECT")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%s",filename);
                 } else {
                  fprintf(stderr,"Enter object name:");
                  mygetline(filename,sizeof(filename));
                 }
                 sprintf(input,"OBJECT %s",filename);
                 status = sendccd(toccd.port,toccd.server,input,default_timeout);            
    }

    else if (strcmp(command,"GOBJECT")==0) {
                 if (getargs(inputline,sargv,MAXCMD,MAXARG) == 2) {
                   sscanf(sargv[1],"%s",filename);
                 } else {
                  fprintf(stderr,"Enter object name:");
                  mygetline(filename,sizeof(filename));
                 }
                 sprintf(input,"OBJECT %s",filename);
                 status = sendccd(togccd.port, togccd.server,input,default_timeout);            
    }

    else if (strcmp(command,"PSTATUS") ==0) {
                 while (power_status(plug_status) < 0 ) {
                   sleep(2);
                 }
                 for (i=1 ; i<= NPLUG; i ++) {
                   if (plug_status[i] == ON) 
                     fprintf(stderr,"Plug %d ON\n",i);
                   else if (plug_status[i] == OFF)
                     fprintf(stderr,"Plug %d OFF\n",i);
                   else
                     fprintf(stderr,"Plug %d UNKNOWN\n",i);

                 }

    }

    // Unknown command
    else if (strlen(command)>0)
                  fprintf(stderr,"Unknown command!! (%s)\r\n",command);
    
    // Print status message
    error_code(status);           
  }  /* end strlen>1 */

  fflush(stdout);
/* Command prompt */
  if (command_mode) {
    if (idepth == 0) 
      fprintf(stderr,"Command: ");
    else
      fprintf(stderr,"Command[%1d]:",idepth);
  }

  }  /* end select */

  }  /* end command_mode */

  // Restore CTRL-C behavior
  signal(SIGINT,SIG_DFL);

}

int getsao(double saora, double saodec, double latitude, double epoch, double vmin,
double vmax, double *outra, double *outdec, long *outid, double *outv,
double radius, long *oldid, int nold, int faint)
{
   int i, istart, iend, ifile;
   char inputfile[60], line[80];
   int rah,ram,decd,decm;
   long id;
   char sign;
   double ra,dec,ras,rap,decs,decp,vmag,cosdec,ddec,dra,dmin,dist;
   FILE *fp;

   *outid = -1;
   cosdec = cos(saodec*DD2R);
   dra = saora - radius/15/cosdec;
   istart = ( dra < 0 ? (int)dra - 1 : (int)dra );
   iend = (int) (saora + radius/15/cosdec);
   dmin = radius*radius;
   for (i=istart; i<=iend; i++) {
     ifile= (i< 0 ? i+24 : i);
     ifile= (ifile> 23 ? ifile-24 : ifile);
     if (faint) 
       sprintf(inputfile,"/home/export/sao/faint/sao%02d.dat\0",ifile+1);
     else
       sprintf(inputfile,"/home/export/sao/sao%02d.dat\0",ifile+1);
     fp = fopen(inputfile,"r");
     if (fp==NULL) return(1);
     while (fgets(line,80,fp) != NULL) {
         sscanf(line,"%6ld%2d%2d%6lf%7lf%1c%2d%2d%5lf%6lf%4lf",
              &id,&rah,&ram,&ras,&rap,&sign,&decd,&decm,&decs,&decp,&vmag);
         if (!samestar(id,oldid,nold) && vmag>vmin && vmag<vmax) {
           dec=decd+decm/60.+decs/3600.;
           if (sign == '-') dec *= -1;
           dec += decp*(epoch-1950.)/3600.;
           ddec=saodec-dec;
           if ((saodec<latitude&&dec<latitude)||(saodec>latitude&&dec>latitude)) {
           if (fabs(ddec) < radius) {
             ra=rah+ram/60.+ras/3600.;
             ra += rap*(epoch-1950.)/3600.;
             dra=(saora-ra)*15.*cosdec;
             dra = (dra>180 ? dra-360 : dra);
             dra = (dra<-180 ? dra+360 : dra);
             if (fabs(dra) < radius) {
                   dist = dra*dra + ddec*ddec;
                   if (dist<dmin) {
                     *outra=ra;
                     *outdec=dec;
                     *outid=id;
                     *outv=vmag;
                     dmin = dist;
                   }
             }
           }
           }
        }
     }
     fclose(fp);
  }
  return(0);

}
int gethip(double hipra, double hipdec, double latitude, double epoch, double vmin,
double vmax, double *outra, double *outdec, long *outid, double *outv,
double radius, long *oldid, int nold, int faint)
{
   int i, istart, iend, ifile;
   char inputfile[60], line[80];
   int rah,ram,decd,decm;
   long id;
   char sign;
   double ra,dec,ras,rap,decs,decp,vmag,cosdec,ddec,dra,dmin,dist;
   FILE *fp;

   *outid = -1;
   cosdec = cos(hipdec*DD2R);
   dra = hipra - radius/15/cosdec;
   istart = ( dra < 0 ? (int)dra - 1 : (int)dra );
   iend = (int) (hipra + radius/15/cosdec);
   dmin = radius*radius;
   for (i=istart; i<=iend; i++) {
     ifile= (i< 0 ? i+24 : i);
     ifile= (ifile> 23 ? ifile-24 : ifile);
     if (faint) 
       sprintf(inputfile,"/home/export/hip/faint/hip%02d.dat\0",ifile+1);
     else
       sprintf(inputfile,"/home/export/hip/hip%02d.dat\0",ifile+1);
     fp = fopen(inputfile,"r");
     if (fp==NULL) return(1);
     while (fgets(line,80,fp) != NULL) {
         sscanf(line,"%d%lf%lf%lf%lf%lf",
              &id,&ra,&rap,&dec,&decp,&vmag);
         if (!samestar(id,oldid,nold) && vmag>vmin && vmag<vmax) {
           dec += decp*(epoch-1991.25)/1000./3600.;
           ddec=hipdec-dec;
           if ((hipdec<latitude&&dec<latitude)||(hipdec>latitude&&dec>latitude)) {
           if (fabs(ddec) < radius) {
             ra += rap*(epoch-1991.25)/1000./3600.;
             dra=(hipra-ra/15.)*cosdec;
             dra = (dra>180 ? dra-360 : dra);
             dra = (dra<-180 ? dra+360 : dra);
             if (fabs(dra) < radius) {
                   dist = dra*dra + ddec*ddec;
                   if (dist<dmin) {
                     *outra=ra/15.;
                     *outdec=dec;
                     *outid=id;
                     *outv=vmag;
                     dmin = dist;
                   }
             }
           }
           }
        }
     }
     fclose(fp);
  }
  return(0);

}

BOOL samestar(long id,long *oldid,int nold)
{
  int i;

  for (i=0; i<nold; i++) {
    if (id == *(oldid+i)) return(TRUE);
  }

  return(FALSE);
}


void command_help()
{
   char ans[10];

   fprintf(stderr,"General commands: \r\n");
   fprintf(stderr," HP: print help menu\r\n");
   fprintf(stderr," INPUT: run a script file\r\n");
   fprintf(stderr," OF: open an input coordinate file\r\n");
   fprintf(stderr," RF: read star position from coordinate file and move to it\r\n");
   fprintf(stderr," WE: read weather station\r\n");
   fprintf(stderr," QU: quit command mode\r\n");
   fprintf(stderr,"Commands to move/initialize telescope: \r\n");
   fprintf(stderr," CO: move to specified coordinates\r\n");
   fprintf(stderr," SA: move to SAO star\r\n");
   fprintf(stderr," PM: move to a previous position\r\n");
   fprintf(stderr," QM: relative move from current position\r\n");
   fprintf(stderr," PA: change position angle\r\n");
   fprintf(stderr," HO: initialize telescope by moving to home\r\n");
   fprintf(stderr," IN: initialize telescope with saved coords if available\r\n");
   fprintf(stderr," ST: stow telescope in parked position\r\n");
   fprintf(stderr,"Commands to alter equinox, current position\r\n");
   fprintf(stderr," NE: change to new equinox\r\n");
   fprintf(stderr," UC: update coordinates at current position\r\n");
   fprintf(stderr," UP: update current instrument position angle\r\n");
   fprintf(stderr," UH: update home coordinates (beware!)\r\n");

   fprintf(stderr,"\r\nHit any key to continue: ");
   mygetline(ans,sizeof(ans));

   fprintf(stderr,"Tracking/moving moving commands: \r\n");
   fprintf(stderr," TR: toggle tracking on/off\r\n");
   fprintf(stderr,"Focus commands: \r\n");
   fprintf(stderr," DF: relative focus change\r\n");
   fprintf(stderr," FO: move to new absolute focus\r\n");
   fprintf(stderr," XTILT: change secondary x tilt (engineering only!)\r\n");
   fprintf(stderr," YTILT: change secondary y tilt (engineering only!)\r\n");
   fprintf(stderr,"Dome/mirror covers commands: \r\n");
   fprintf(stderr," OD: open dome slit\r\n");
   fprintf(stderr," CD: close dome slit\r\n");
   fprintf(stderr," OM: open mirror covers\r\n");
   fprintf(stderr," CM: close mirror covers\r\n");
   fprintf(stderr," OL: open louvers \r\n");
   fprintf(stderr," CL: close louvers \r\n");
   fprintf(stderr," DI: initialize dome\r\n");
   fprintf(stderr," DM: toggle dome following on/off\r\n");
   fprintf(stderr,"Engineering commands: \r\n");
   fprintf(stderr," RMC: read mount correction file\r\n");
   fprintf(stderr," MC: toggle mount correction on/off\r\n");
   fprintf(stderr," EP: toggle use of encoders for pointing on/off\r\n");
   fprintf(stderr," ET,ETX,ETY,ETZ: toggle use of encoders for tracking on/off\r\n");
   fprintf(stderr," TD: change tracking delta time \r\n");
   fprintf(stderr," TF,TFX,TFY,TFZ: change tracking factors\r\n");
   fprintf(stderr," PC: issue a PC38 command directly\r\n");
   fprintf(stderr," CAZ: calibrate azimuth axis\r\n");
   fprintf(stderr," CRO: calibrate rotator axis\r\n");

   fprintf(stderr,"\r\nHit any key to continue: ");
   mygetline(ans,sizeof(ans));

   fprintf(stderr,"CCD commands: \r\n");
   fprintf(stderr," EXP: \r\n");
   fprintf(stderr," SETFILT:\r\n");
   fprintf(stderr," CLEANS:\r\n");
   fprintf(stderr," NEWEXT:\r\n");
   fprintf(stderr," FILENAME:\r\n");
   fprintf(stderr," CENTER:\r\n");
   fprintf(stderr," NEWCENT:\r\n");
   fprintf(stderr," SCALE:\r\n");
   fprintf(stderr," FULLSCALE:\r\n");
   fprintf(stderr," SKYSCALE:\r\n");

   fprintf(stderr,"\r\nHit any key to continue: ");
   mygetline(ans,sizeof(ans));


}
  
BOOL illegal_ra(double ra) 
{
  if (ra < 0 | ra >= 24) {
    fprintf(stderr,"Illegal value of RA\r\n");
    return(TRUE);
  }
  else
    return(FALSE);
}

BOOL illegal_dec(double dec) 
{
  if (dec < -90 | dec > 90) {
    fprintf(stderr,"Illegal value of DEC\r\n");
    return(TRUE);
  }
  else
    return(FALSE);
}

BOOL illegal_az(double az) 
{
  if (az < 0 | az > 360) {
    fprintf(stderr,"Illegal value of AZ\r\n");
    return(TRUE);
  }
  else
    return(FALSE);
}

BOOL illegal_alt(double alt) 
{
  if (alt < 0 | alt > 90) {
    fprintf(stderr,"Illegal value of ALT\r\n");
    return(TRUE);
  }
  else
    return(FALSE);
}

int getsaved(double *ra, double *dec, double *epoch)
{
   int i, j, rah, ram, decd, decm, sign;
   double ras, decs;
   char input[10];

   sprintf(outbuf,"  STAR     RA          DEC         EPOCH");
   writeline(outbuf,0);
   j=0;
   for (i=MAXSAVE+isave-1; i>isave-1; i--)
   {
     j++;
     gethms(savedra[i%MAXSAVE],&rah,&ram,&ras,&sign);
     getdms(saveddec[i%MAXSAVE],&decd,&decm,&decs,&sign);
     if (sign < 0) 
       sprintf(outbuf,"  %d: %2d %2d %4.1f -%2d %2d %4.1f  %6.1f",
         j, rah, ram, ras, decd, decm, decs, 
         savedepoch[i%MAXSAVE]);
     else
       sprintf(outbuf,"  %d: %2d %2d %4.1f  %2d %2d %4.1f  %6.1f",
         j, rah, ram, ras, decd, decm, decs, 
         savedepoch[i%MAXSAVE]);
     writeline(outbuf,0);
   }
   fprintf(stderr,"\r\n Enter star number (0 for manual entry, -1 to quit): ");
   mygetline(input,sizeof(input));
   sscanf(input,"%d",&i);

   if (i <0 || i>MAXSAVE) 
     return(-1);
   else if (i==0) {
     fprintf(stderr,"Current epoch for coordinates: %f\r\n",current_epoch);
     getcoord("    RA\0",ra,1);
     getcoord("   DEC\0",dec,1);
     fprintf(stderr,"WARNING: THIS OPTION HAS NOT BEEN TESTED YET. PLEASE CHECK!!\n");
     fprintf(stderr,"INFORM Jon about success/failure\n");
   }
   else {
     i = 2*MAXSAVE + isave-i;
     *ra = savedra[i%MAXSAVE];
     *dec = saveddec[i%MAXSAVE];
     *epoch = savedepoch[i%MAXSAVE];
     return(i);
   }
     
}


// get_all_times_at
//
// return ALLTIMES structure filled in according to the date/time supplied
//
// d - date
// t - time
//
// timeRec - ALLTIMES structure filled in according to d&t
//
extern int get_all_times_at( const struct tm t,
		             struct ALLTIMES *timeRec)
	{
		int rval;
		int stat = 0;

		// calculate the equinox
		int nday;
		int nyear;
		double fday, fmjd, fsec;
                double mjd_ut1, gmst, delta_tt;


		slaClyd(t.tm_year+1900, t.tm_mon+1, t.tm_mday, &nyear, &nday, &rval);
		stat |= rval;
		timeRec->equinox = (double)nyear + ((double)nday / 365.0);
		fday = (double)t.tm_hour * 3600.0;
		fday += (double)t.tm_min * 60.0;
		fday += (double)t.tm_sec;
		fday /= (86400.0 * 365.25);
		timeRec->equinox += fday;

		// calculate the UTC as mjd

		slaCldj(t.tm_year+1900, t.tm_mon+1, t.tm_mday, &timeRec->mjd_utc, &rval);
		stat |= rval;
		fsec = (double)t.tm_sec ;
		slaDtf2d(t.tm_hour, t.tm_min, fsec, &fmjd, &rval);
		stat |= rval;
		timeRec->mjd_utc += fmjd;

		// calculate the terrestrial time
//		mjd_ut1 = timeRec->mjd_utc + (G->ut1_minus_utc / 86400.0);
		mjd_ut1 = timeRec->mjd_utc ;
		delta_tt = slaDtt(mjd_ut1);
		timeRec->mjd_tt = mjd_ut1 + (delta_tt / 86400.0);

		// calculate the apparent LST
		gmst = slaGmst(mjd_ut1);
		timeRec->last = slaDranrm(gmst + longitude + slaEqeqx(mjd_ut1));
		timeRec->lasth = timeRec->last * DR2H;
		return stat;
	}

void trap()
{
  ctrlc = TRUE;
  fprintf(stderr,
"\nCommand string interrupted by CTRL-C. Waiting for current command to complete\n");
  signal(SIGINT,trap);
}

int focrun(int port, char *server, int ccdport, char *ccdserver, 
           int pi, int fast, int filter)
{
  char inputline[80], command[80], ret[MAXCMD];
  double startfoc, dfoc, exptime;
  int nfoc, i, status;

  calc_focus(ifoc,G);

  fprintf(stderr,"Enter parameters for a focus run. Note that you should choose\n");
  fprintf(stderr," a starting focus such that you use a POSITIVE delta focus.\n");
  fprintf(stderr," This will insure that all focus positions are approached \n");
  fprintf(stderr," from the same direction, and should improve repeatability\n");

  fprintf(stderr,"Enter start focus: ");
  mygetline(inputline,sizeof(inputline));
  sscanf(inputline,"%lf",&startfoc);

  fprintf(stderr,"Enter delta focus: ");
  mygetline(inputline,sizeof(inputline));
  sscanf(inputline,"%lf",&dfoc);
  if (dfoc < 0) {
    fprintf(stderr," To insure repeatability, use a POSITIVE delta focus\n");
    return(-1);
  }

  fprintf(stderr,"Enter number of exposures: ");
  mygetline(inputline,sizeof(inputline));
  sscanf(inputline,"%d",&nfoc);

  fprintf(stderr,"Enter exposure time: ");
  mygetline(inputline,sizeof(inputline));
  sscanf(inputline,"%lf",&exptime);

// Compensate for filter focus offset since we are sending RAW focus to telescope
  startfoc += focoff[filter];

// Move to start focus position - 50 steps, to always approach from 
//   negative direction
  G->foc = startfoc - 50;
  calc_focus_steps(foc,out,G);

// Check for very large moves
  if (fabs(out[0]>500)||fabs(out[1]>500)||fabs(out[2]>500)) {
    fprintf(stderr,"You have requested a move of more than 500 steps!!\n");
    fprintf(stderr,"  Are you sure you want this??\n");
    fprintf(stderr,"Remember that focus values are NEGATIVE\n");
    fprintf(stderr,"\n If you really want to move this much, you will need");
    fprintf(stderr,"  to use the FO or DF commands");
    return(-1);
  }
// Send command to put CCD in fast readout mode
  if (pi==0 && fast==1) {
    sprintf(command,"CCDFAST");
    fprintf(stderr,"%s\n",command);
    status = sendport(port,server,command,ret,MAXCMD);
  }

// Send command to move to 50 steps short of start focus
  sprintf(command,"XFOCUS %ld %ld %ld",(int)out[0],(int)out[1],(int)out[2]);
  fprintf(stderr,"%s\n",command);
  status = sendport(port,server,command,ret,MAXCMD);

// Now move to dfoc short of start position
  sprintf(command,"XFOCUS %ld %ld %ld",
      (int)(50-dfoc),(int)(50-dfoc),(int)(50-dfoc));
  fprintf(stderr,"%s\n",command);
  status = sendport(port,server,command,ret,MAXCMD);

  fprintf(stderr,"Starting focus run. Hit CTRL-C to abort\n\n");

// Now do the focus run, moving and exposing each step
  for (i=0; i<nfoc; i++) {
    sprintf(command,"XFOCUS %ld %ld %ld",(int)dfoc,(int)dfoc,(int)dfoc);
    fprintf(stderr,"%s\n",command);
    status = sendport(port,server,command,ret,MAXCMD);

    fprintf(stderr,"Exposure %d of %d, focus: %lf\n", i+1, nfoc, startfoc + i*dfoc);

    if (pi==0) {
      status = do_exposure(port,server,ccdport,ccdserver, exptime,i);

    } else {

      sprintf(command,"EXP %f",exptime*1000);
      fprintf(stderr,"%s\n",command);
      status = sendccd(ccdport,ccdserver,command,default_timeout);            
    }

    if (ctrlc) break;
  }

// Send command to put CCD in slow readout mode
  if (pi==0 && fast==1) {
    sprintf(command,"CCDSLOW");
    fprintf(stderr,"%s\n",command);
    status = sendport(port,server,command,ret,MAXCMD);
  }
  return(status);
}

telescope_startup(int port, char *server, int force)
{

  int status;
  char ans[10];
  char command[10], ret[MAXCMD];

  if (force || !G->telescope_initialized) {
    if (!remote) {
      fprintf(stderr,"You must initialize the telescope from the dome. \n"
             "  Use the HO command in the dome and make sure \n"
             "    to watch the rotator in particular!\n");
      status = TCSERR_OK;
    }  else {
      // Initialize the telescope
      fprintf(stderr,"Do you wish to initialize the telescope with a :\r\n"
" I: normal init (uses stored coords to find home positions - will be very\r\n"
"    slow if stored coords are wrong)\r\n"
" F: full init - finds home positions without any previous knowledge\r\n"
" M: manual init - input estimated coordinates, then full init\r\n"
" Q: quick init (stored coords - assumes telescope hasn't been moved!)\r\n"
" S: skip initialization (default) \r\n\n"
"Enter your choice: ");
      mygetline(ans,sizeof(ans));

      if (ans[0]=='Q' || ans[0]=='q') {
        sprintf(command,"IN");
        status = sendport(port,server,command,ret,MAXCMD);
      } else if (ans[0] == 'M' || ans[0] == 'm') {
        status = telescope_init(port, server, 2);
      } else if (ans[0] == 'F' || ans[0] == 'f') {
        status = telescope_init(port, server, 1);
      } else if (ans[0] == 'I' || ans[0] == 'i') {
        status = telescope_init(port, server, 0);
      } else {
        status = TCSERR_OK;
      }
      if (status != TCSERR_OK) return(status);
    }
  }

  if (!G->dome_initialized) {  
    // Initialize the dome?
    fprintf(stderr,"\nDo you wish to initialize the dome (Y or N)? ");
    mygetline(ans,sizeof(ans));
    if (ans[0]=='Y' || ans[0]=='y') {
      sprintf(command,"DI");
      status = sendport(port,server,command,ret,MAXCMD);
      if (status != TCSERR_OK) return(status);
    } else
      fprintf(stderr,"dome is not initialized...\n");
  }

  if (!G->dome_slaved) {  
    // Slave the dome?
    fprintf(stderr,"\nDo you wish to slave the dome (Y or N)? ");
    mygetline(ans,sizeof(ans));
    if (ans[0]=='Y' || ans[0]=='y') {
      sprintf(command,"DM");
      status = sendport(port,server,command,ret,MAXCMD);
      if (status != TCSERR_OK) return(status);
    } else 
      fprintf(stderr,"dome is not slaved...\n");
  }

/* 
  while (power_status(plug_status) < 0) {
    sleep(2);
  }
  if (plug_status[LOUVERS] == OFF) {
    fprintf(stderr,"\nDo you wish to open the louvers (Y or N)? ");
    mygetline(ans,sizeof(ans));
    if (ans[0]=='Y' || ans[0]=='y') {
      fprintf(stderr,"\nOpening the louvers will expose the telescope to the weather!\n");
      fprintf(stderr,"\nAre you SURE you wish to open the louvers (Y or N)? ");
      mygetline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0]=='y') {
        power_on(LOUVERS);
        fprintf(stderr,
"REMEMBER to close the vents when you are done by killing power to the dome\n");
      }
    }
  }
*/
  return(status);

}

telescope_settime(int port, char *server)
{
  time_t timep;
  struct tm *timerec;
  char command[80], ret[MAXCMD];
  int status;

  timep = time(NULL);
  timerec = gmtime(&timep);

  sprintf(command,"SETTIME %d %d %d %d %d %d\n",
      timerec->tm_year-80, timerec->tm_mon+1, timerec->tm_mday,
      timerec->tm_hour, timerec->tm_min, timerec->tm_sec);

  status = sendport(port,server,command,ret,MAXCMD);
  return(status);
}

int telescope_init(int port,  char *server, int full) 
{
   char command[80], ret[MAXCMD];
   char ans[10];
   int status;
   FILE *fp;

   if (plug_status[MOTORS] == OFF) {
     fprintf(stderr,"Motor power is OFF\n");
     fprintf(stderr,"\n Do you wish to turn it ON (Y or N)? ");
     mygetline(ans,sizeof(ans));
     if (ans[0]=='Y' || ans[0]=='y') {
       power_on(RACKFAN);
       power_on(MOTORS);
     } else
       return(-1);
   }

   fprintf(stderr,
"You must watch the rotator in the video to check that cables do not wrap.\r\n"
" When you start the initialization, the telescope will first move in \r\n"
" azimuth and altitude. Before the rotator moves, the telescope will move \r\n"
" back in azimuth to a position in which you can see the rotator.\r\n "
" This is when you should watch carefully. If cables appear to be wrapping,\r\n"
" hit any key in the Emergency Stop section of the power bar.\r\n\r\n"
" You must confirm that you are seeing updates in the video before starting\r\n");
   //power_on(LIGHTS);
   fp = fopen("/home/export/rvideo/start","w");
   fclose(fp);

   fprintf(stderr,"\r\nAre you sure you want to initialize the telescope? ");
   mygetline(ans,sizeof(ans));

   status = 0;
   if (ans[0] == 'Y' || ans[0] == 'y') {
     if (full==2) {
       fprintf(stderr,"AZ runs from 0 (N) to 450 (E)\n");
       fprintf(stderr,"ALT runs from 0(horizontal) to 90 (vertical)\n");
       fprintf(stderr,"ROT is -94 at home (filter wheel handle up) and increases CW\n");
       fprintf(stderr,"Enter approximate AZ, ALT, ROT: ");
       float az, alt, rot;
       scanf("%f%f%f",&az,&alt,&rot);
       sprintf(command,"SAVED %f %f %f",az,alt,rot);
       status = sendport(port,server,command,ret,MAXCMD);
       sprintf(command,"IN");
       status = sendport(port,server,command,ret,MAXCMD);
       //sprintf(command,"REINIT");
       sprintf(command,"HO");
       status = sendport(port,server,command,ret,MAXCMD);
     } else if (full==1) {
       sprintf(command,"REINIT");
       status = sendport(port,server,command,ret,MAXCMD);
     } else {
       sprintf(command,"IN");
       status = sendport(port,server,command,ret,MAXCMD);
       sprintf(command,"HO");
       status = sendport(port,server,command,ret,MAXCMD);
     }
   }

   fprintf(stderr,
"Initialization complete. Lights in dome will now be turned off.\r\n ");
   power_off(LIGHTS);
   sleep(2);
   fp = fopen("/home/export/rvideo/stop","w");
   fclose(fp);

   return(status);
}

int readuser(char *file, int iuser)
{
  int i, iret, j;
  char input[MAXCMD], name[MAXCMD];
  int rah, ram, decd, decm, sign;
  double ras, decs, epoch;
  double rasec, decsec, pmra, pmdec;
  int j1, pmflag;
  FILE *userfile;

  userfile = fopen(file,"r");
  i = 0;
  if (userfile != NULL) {
    while (fgets(input,sizeof(input),userfile) != NULL && i < MAXUSER) {
  // Find first tab 
      j1=0; 
      while (input[j1++] != '\t' && j1<strlen(input) ) {}
      if (j1==strlen(input)) {
        fprintf(stderr,"format error\n");
        return(-1);
      }
      strncpy(name,input,j1-1);
      name[j1-1] = 0;
      for (j=0; j<16; j++) userobj[iuser][i][j] = 0;
      sscanf(name,"%15c",userobj[iuser][i]);

      strncpy(name,input+j1,MAXCMD);

      sign = -1;
      pmflag = 0;
      iret = sscanf(name,
"%d:%d:%lf\t-%d:%d:%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%d",
&rah,&ram,&ras,&decd,&decm,&decs,&epoch,
&pmra,&pmdec,&usermag[iuser][i][1],&usermag[iuser][i][2],&usermag[iuser][i][3],
&usermag[iuser][i][4],&usermag[iuser][i][5],&pmflag);
      if (iret < 4)  {
         sscanf(name,
"%d:%d:%lf\t%d:%d:%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%d",
&rah,&ram,&ras,&decd,&decm,&decs,&epoch,
&pmra,&pmdec,&usermag[iuser][i][1],&usermag[iuser][i][2],&usermag[iuser][i][3],
&usermag[iuser][i][4],&usermag[iuser][i][5],&pmflag);
         sign = 1;
      }
      userra[iuser][i] = (rah+ram/60.+ras/3600.)*DH2R;
      userdec[iuser][i] = sign*(decd+decm/60.+decs/3600.)*DD2R;
      userepoch[iuser][i] = epoch;
      if (pmflag==0) {
        // Proper motions give in arcsec/year
        userpmra[iuser][i] = pmra*DAS2R;
        userpmdec[iuser][i] = pmdec*DAS2R;
      } else {
        // Proper motions give in arcsec/sec
        userpmra[iuser][i] = pmra*DAS2R*86400*365.25;
        userpmdec[iuser][i] = pmdec*DAS2R*86400*365.25;
      }
      i++;
    }
    nuser[iuser] = i;
    fclose(userfile);
  } else
    fprintf(stderr,"Error opening file %s\n",file);
  return(0);
}                  

int dome_open_close(int port, char *server, char *command)
{
   char newcom[10], ret[MAXCMD];
   char ans[10];
   int status;
   FILE *fp;


   if (!G->dome_initialized) {  
     fprintf(stderr,
  "The dome must be initialized before opening/closing so it can be rotated\r\n"
  "into a position where it can be seen by the video camera.\n");
    fprintf(stderr,"\nDo you wish to initialize the dome (Y or N)? ");
    mygetline(ans,sizeof(ans));
    if (ans[0]=='Y' || ans[0]=='y') {
      sprintf(newcom,"DI");
      status = sendport(port,server,newcom,ret,MAXCMD);
      if (status != TCSERR_OK) return(status);
    } else {
      fprintf(stderr,
  "The dome slit condition (open/close) has NOT been changed!!\n");
      return(-1);
    }
   }

   //power_on(LIGHTS);
   fp = fopen("/home/export/rvideo/start","w");
   fclose(fp);

   // Temporarily turn off dome slaving if on
   if (G->dome_slaved) {  
     sprintf(newcom,"DM");
     status = sendport(port,server,newcom,ret,MAXCMD);
   }

   fprintf(stderr,
"You must watch to make sure that the dome opens/closes.\r\n");
//" Sending the dome to azimuth 160 (where you can see it with the video)\r\n");
   //sprintf(newcom,"XDOME 160");
   //status = sendport(port,server,newcom,ret,MAXCMD);
   //sleep(2);

   fprintf(stderr,"\r\nAre you ready to open/close the dome? ");
   mygetline(ans,sizeof(ans));
   status = 0;
   if (ans[0] == 'Y' || ans[0] == 'y')
          status = sendport(port,server,command,ret,MAXCMD);

   fprintf(stderr,
"Dome open/close complete. Lights in dome will now be turned off.\r\n ");
   power_off(LIGHTS);
   sleep(5);
   fp = fopen("/home/export/rvideo/stop","w");
   fclose(fp);

   // Turn on dome slaving if it was on
   if (G->dome_slaved) {  
     sprintf(newcom,"DM");
     status = sendport(port,server,newcom,ret,MAXCMD);
   }

   return(status);
}
int dofilter(int filter,int port, char *server,int ccdport, char *ccdserver, int gccdport, char* gccdserver)
{
   char input [80];
   int status;
  
   sprintf(input,"FILTER %d",filter);
// First send command to actually move filter wheel
   status = sendccd(port,server,input);            

// If successful, then send command to CCD command to update filter status
   if (status == 0) {
     status = sendccd(ccdport,ccdserver,input,default_timeout);            
     status = sendccd(gccdport,gccdserver,input,default_timeout);            
     sprintf(input,"FILTFOC %d",focoff[filter]);
     status = sendccd(ccdport,ccdserver,input,default_timeout);            
     status = sendccd(gccdport,gccdserver,input,default_timeout);            
   }

   return(status);

}

int autofoc(int port,char *server)
{
   int status;
   char command[80], ret[MAXCMD];
  
   sprintf(command,"WE"); 

   status = sendport(port,server,command,ret,MAXCMD);
   if (havetocc) {
#ifdef SOCKET
        status=getstatus(port, server, &statusinfo);
#else
        readstatus(tstatusfile, tstatusreadyfile, &statusinfo);
#endif
        G = &statusinfo;
   }
   return(
     (int)(G->current_aux_temp-focus_temp_t0)*focus_temp_slope + focus_temp_zero   );
}

int doautodfoc(double *temp0,int port,char *server)
{
   int dfoc, status;
   char command[80], ret[MAXCMD];
  
   sprintf(command,"WE"); 
   status = sendport(port,server,command,ret,MAXCMD);
   if (havetocc) {
#ifdef SOCKET
        status=getstatus(port, server, &statusinfo);
#else
        readstatus(tstatusfile, tstatusreadyfile, &statusinfo);
#endif
        G = &statusinfo;
   }

   dfoc = (int)(G->current_aux_temp - *temp0) * focus_temp_slope;
   if (dfoc>0)  {
     sprintf(command,"XFOCUS %ld %ld %ld",dfoc,dfoc,dfoc);
     fprintf(stderr,"want to execute: DF %ld\n",dfoc);
     //status = sendport(port,server,command,ret,MAXCMD);
   } else if (dfoc<0) {
     sprintf(command,"XFOCUS %ld %ld %ld",dfoc-50,dfoc-50,dfoc-50);
     fprintf(stderr,"want to execute: DF %ld\n",dfoc-50);
     //status = sendport(port,server,command,ret,MAXCMD);
     sprintf(command,"XFOCUS 50 50 50");
     fprintf(stderr,"want to execute: DF %ld\n",dfoc);
     //status = sendport(port,server,command,ret,MAXCMD);
   }
   *temp0 = G->current_aux_temp;

}

double airmass(double ra, double dec)
{
  double ha, s, secz, az, azd, azdd, el, eld, eldd, pa, pad, padd;
  int h, m, sign;

  ha = slaDrange(G->current_lasth*DH2R - ra*DH2R)*DR2H;
  gethms(fabs(ha),&h,&m,&s,&sign);

  slaAltaz(ha*DH2R,dec*DD2R,latitude,
      &az,&azd,&azdd,&el,&eld,&eldd,&pa,&pad,&padd);
  secz = 1.0/cos(DPIBY2-el);
  return(secz+(secz-1.0)*(-0.0018167+(secz-1.0)*
          (-0.002875-0.0008083*(secz-1.0))));
}

int do_exposure(int port,char *server,int ccdport,char *ccdserver, double exptime, int nowe)
{
   char input[80], ret[MAXCMD];
   int status;

   if (nowe==0) {
     sprintf(input,"WE");
//     status = sendport(port,server,input,ret,MAXCMD);
   }
   ccd_timeout = (default_timeout != 0 ? exptime+30+default_timeout : 0);
#ifdef HAVEPI
   sprintf(input,"EXPOSURE %f",exptime);
   status = sendccd(ccdport,ccdserver,input,default_timeout);  
   sprintf(input,"COLLECTDATA 0");
   status = sendccd(ccdport,ccdserver,input,ccd_timeout); 
#else
   sprintf(input,"EXP %f",exptime*1000.);
   status = sendccd(ccdport,ccdserver,input,ccd_timeout);  
#endif
   return(status);
}
writeterm(char *message)
{
  fprintf(stderr,"%s",message);
//  if (havepipe[idepth]=1 )
//    fprintf(tomasterfp,"%s",message);

}
int nint(float val)
{
  if (fmod(val,1.) < 0.5)
    return((int)floor(val));
  else
    return((int)ceil(val));
}

