#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#include "io.h"
#include "key_util.h"
#include "systimer.h"

FILE* remoteopen(char *file,char *flag);
void remoteremove(char *);
char *getresp(FILE *);

char outbuf[8000];
char outbuf2[8000];
BOOL do_display = FALSE;

#ifdef no_hardware
 #define SPEC "d:"
#else
 #define SPEC "e:"
#endif

#undef DEBUG
#undef NEWCOM
char *respfile = SPEC"\\spec\\specresp.doc";
char *respfilecheck = SPEC"\\spec\\specresp.fin";
char *statusfile = SPEC"\\tocc\\guidecmd.doc";
char *statusfilecheck = SPEC"\\tocc\\guidecmd.fin";
char *comfile = SPEC"\\spec\\speccmd.doc";
char *comfilecheck = SPEC"\\spec\\speccmd.fin";
char *flushfile = SPEC"\\spec\\junk.doc";
BOOL ropen = FALSE;
BOOL statopen = FALSE;
BOOL copen = FALSE;
BOOL flushopen = FALSE;
FILE *rfile = NULL;
FILE *sfile = NULL;
FILE *cfile = NULL;
FILE *ffile = NULL;

char *restart = SPEC"\\spec\\restart.doc";
void writejunk(FILE *);

void openrfile()
{
  rfile = fopen(respfile,"w");
  cprintf("opening respfile: %s\r\n",respfile);
  if (rfile != NULL) ropen = TRUE;
}

void opensfile()
{
  sfile = fopen(statusfile,"w");
  cprintf("opening statusfile: %s\r\n",statusfile);
  if (sfile != NULL) statopen = TRUE;
}

void opencfile()
{
  cfile = fopen(comfile,"r");
  cprintf("opening comfile: %s\r\n",comfile);
  if (cfile != NULL) copen = TRUE;
}

void openffile()
{
  ffile = fopen(flushfile,"w");
  cprintf("opening flushfile: %s\r\n",flushfile);
  if (ffile != NULL) flushopen = TRUE;
}

// Function writeline outputs a line to terminal and possibly output file,
//    depending on value of icode
void writeline(char *outbuf, int icode)
{
  struct date d;
  struct mytime t;
  char timestring[16];
  char file[64];
  FILE *gfile;

// Get the time and make a timestamp
  mygettime(&d,&t);
  double secs;
  secs = t.ti_sec + t.ti_hund/100.;
  if (strchr(outbuf,'\n') == NULL)
    sprintf(timestring,"%d:%d:%f    ",t.ti_hour,t.ti_min,secs);
  else
    sprintf(timestring,"%d:%d:%f\n",t.ti_hour,t.ti_min,secs);

// Normal output is without timestamp, using cprintf if necessary
  if (icode==0) {
      printf("%s",outbuf);
  }

// Output to remote
  if (remote_on && (icode==0 || icode==1 || icode==10)) {
          if (ropen == FALSE) openrfile();
          if (icode==1)
              fprintf(rfile,"%s\n%s\n",timestring,outbuf);
          else
              fprintf(rfile,"%s\n",outbuf);
          fflush(rfile);
          if (ferror(rfile) != 0) cprintf("error writing file\r\n");
  }

// Error output
  if (icode==-1) 
      cprintf("%s\n\r",outbuf);

// Debugging output
  if (icode==1) {
      printf("%s%s\n",timestring,outbuf);
  }
#ifdef debug_move_file
  if (icode==2)
    if (G->move_file != NULL) fprintf(G->move_file,"%s%s\n",timestring,outbuf);
#endif
#ifdef debug_tracking_file
  if (icode==3)
    if (G->track_file != NULL) fprintf(G->track_file,"%s%s\n",timestring,outbuf);
#endif
#ifdef debug_pcx_file
  if (icode==4)
    if (G->pcx_file != NULL) fprintf(G->pcx_file,"%s%s\n",timestring,outbuf);
#endif
#ifdef have_debug_file
  if (icode==9)
    if (G->debug_file != NULL) fprintf(G->debug_file,"%s%s\n",timestring,outbuf);
#endif

  if (remote_on && icode==6) {
          SysTimer[SYSTMR_REMOTE].NewTimerSecs(SYSTMR_REMOTE_INC);
          while ( (gfile=fopen(statusfilecheck,"r")) != NULL
                 && !SysTimer[SYSTMR_REMOTE].Expired() ) {
            fclose(gfile);
            SysTimer[SYSTMR_NFS].NewTimer(SYSTMR_NFS_INC);
            while ( !SysTimer[SYSTMR_NFS].Expired() ) {};
          }
          if (gfile == NULL) {
#ifdef DEBUG
            fprintf(stderr,"remote status file write: %s\n\r",outbuf);
#endif
            gfile = remoteopen(statusfile,"w");
            if (gfile != NULL) {
              fprintf(gfile,"%s\n",outbuf);
              fclose(gfile);
              gfile = remoteopen(statusfilecheck,"w");
              if (gfile != NULL) fclose(gfile);
#ifdef DEBUG
              fprintf(stderr,"done\n ");
#endif
            }
            else {
              fclose(gfile);
              cprintf("remote write 6 failed - couldnt open status file!\r\n");
            }
          } else {
            fclose(gfile);
            cprintf("remote write 6 failed - check file never removed!\r\n");
          }
  }

// Write something to a dummy status file because this appears to be
//  be necessary to flush the NFS cache!!!
  if (remote_on) {
          if (flushopen == FALSE) openffile();
          fprintf(ffile,".");
          fflush(ffile);
          if (ferror(ffile) != 0) cprintf("error writing flush file\r\n");
  }

}


// get a newline or CR terminated line
int getline(unsigned char *line, int linesize)
{
  int i, j, k, n, ns, nk, ng, ntot = 0;
  char key, junk;
  char inputline[MAXCMD], com[MAXCMD], *s;
  FILE *file;
  char *guidefile = SPEC"\\spec\\guidecmd.doc";
  char *guidefilecheck = SPEC"\\spec\\guidecmd.fin";

  i = 0;
  ns = 0;
  nk = 0;
  ng = 0;

  remote_command = FALSE;

  while (TRUE) {
    update_all();

    n = 0;
    key = 0;

//  if we haven't gotten anything from the remote line, look for a keyboard hit
    while (ns==0 && ng==0 && kbhit()) {
      inputline[n++] = getch();
      nk++;
    }

//  if remote_on and we haven't gotten anything from the keyboard, 
//     try to read a remote command, first from guider, then from command file
    if (remote_on && nk==0 && SysTimer[SYSTMR_CHECK_COMMAND].Expired()) {
      SysTimer[SYSTMR_CHECK_COMMAND].NewTimer(1);

      // Guider command file
//writeline("trying to open guidefilecheck",9);
      file = fopen(guidefilecheck,"r");
      if (file != NULL) {
        fclose(file);
        file = remoteopen(guidefile,"r");
        if (file != NULL) {
          fgets(inputline,MAXCMD-1,file);
          fclose(file);
          ng = strlen(inputline);
          n = ng;
          remote_command = TRUE;
          remoteremove(guidefile);
          remoteremove(guidefilecheck);
        } else {
          cprintf("Cant open guidefile\n\r");
          remoteremove(guidefilecheck);
        }
      } 

      // Regular command file if we haven't gotten a guide command
      if (ng == 0) {
#ifdef NEWCOM
        if (copen == FALSE) opencfile();

// cprintf("calling getresp: %d\r\n",cfile);
        s = getresp(cfile);
        ns = strlen(s);
ntot += ns;
if (ns>0) cprintf("received: %d %d %s\n",ntot, ns, s);
        for (k=0;k<ns;k++) inputline[k] = *s++;
        n = ns;
        remote_command = TRUE;
#else
//writeline("trying to open comfilecheck",9);
        file = fopen(comfilecheck,"r");
        if (file != NULL) {
          fclose(file);
          file = remoteopen(comfile,"r");
          if (file != NULL) {
            fgets(inputline,MAXCMD-1,file);
            fclose(file);
            ns = strlen(inputline);
            n = ns;
            remote_command = TRUE;
            remoteremove(comfile);
            remoteremove(comfilecheck);
          } else {
            cprintf("Cant open comfile\n\r");
            remoteremove(comfilecheck);
          }
        }

#endif
      }
    }

// At this point we've either gotten a single character from the keyboard or
//   a full command from the remote line, or else we've gotten nothing

    for (k=0;k<n;k++) {
      key = inputline[k];

//   if linesize is 1, then just return the first character which is hit without
//     echoing
      if (linesize==1) {
           line[0] = key;
           return(0);
         }

//   if we get a CR or NL, return the null-terminated string
      if (key == CR || key == NL) {
        line[i] = '\0';
        cprintf("\r\n");
        return(0); 
      }
//   if we get a BS , erase the last character in the string and on screen
      else if (nk>0 && key == BS) {
        if (i!=0) {
          cprintf("\b \b");
          i--;
        }
      }
//   any other character, just store it in the string unless it gets too long
      else {
        if (i==60)
          cprintf("\r\n");
        putchar(key);
        line[i++] = key;
        if (i==linesize) {
          line[0] = '\0';
          cprintf("\r\nLine too long\r\n");
          return(-1);
        }
      }
    }
  }
// The following to keep compiler happy
  return(0);
}


// get a number sexageimal triplet and convert to decimal
int getcoord(char *string, double *coord)
{
  unsigned char buf[80],c[2];
  double h,m,s;
  int sign;

// Enter prompt
  cprintf("%s:",string);

// Read input string
  if (getline(buf,sizeof(buf))!=0)
    return(-1);

// Check for more than 3 fields
  if (sscanf(buf,"%*lf%*lf%*lf%1s",c)==1)
    return(-1);

  h = m = s = 0;

// Read 3 possible formats
  if (sscanf(buf," -%lf%lf%lf",&h,&m,&s)>0)
    sign = -1;
  else if ( sscanf(buf," +%lf%lf%lf",&h,&m,&s)>0 ||
    sscanf(buf,"  %lf%lf%lf",&h,&m,&s)>0 )
    sign = 1;
  else
    return(-1);

  *coord = sign * (h + m/60. + s/3600.);
  return(0);

}

FILE* remoteopen(char *remotefile,char *flag)
{
        FILE *file;

        SysTimer[SYSTMR_REMOTE].NewTimerSecs(SYSTMR_REMOTE_INC);
        while ( (file = fopen(remotefile,flag)) ==NULL && 
             !SysTimer[SYSTMR_REMOTE].Expired() ) {  
          SysTimer[SYSTMR_NFS].NewTimer(SYSTMR_NFS_INC);
          while ( !SysTimer[SYSTMR_NFS].Expired() ) {
           fprintf(stderr,"waiting for NFS timer\n");
          }
          fprintf(stderr,"trying remote open again\n");
        }
        return(file);
}

void remoteremove(char *remotefile)
{
        SysTimer[SYSTMR_REMOTE].NewTimerSecs(SYSTMR_REMOTE_INC);
        while ( remove(remotefile)!= 0 && 
                  !SysTimer[SYSTMR_REMOTE].Expired() ) {
             fprintf(stderr,"error removing %s\n",remotefile);
        }
}
