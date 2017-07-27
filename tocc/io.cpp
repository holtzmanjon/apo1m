#undef HANGTEST
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#include "globals.h"
#include "io.h"
#include "key_util.h"
#include "systimer.h"

char outbuf[8000];
char outbuf2[8000];
BOOL do_display = FALSE;

#undef DEBUG
#undef NEWCOM
#ifdef SOCKET
#undef PCNFS
#else
#define PCNFS
#endif

char *logfile = TOCC"\\tocc\\tocc.log";
FILE *lfile = NULL;
FILE *jfile = NULL;

#ifdef PCNFS
FILE* remoteopen(char *file,char *flag);
void remoteremove(char *);
char *getresp(FILE *);
char *junkfile = TOCC"\\tocc\\junk.doc";
char *respfile = TOCC"\\tocc\\toccr.doc";
char *respfilecheck = TOCC"\\tocc\\toccr";
char *statusfile = TOCC"\\tocc\\statr.doc";
char *statusfilecheck = TOCC"\\tocc\\statr";
char *comfile = TOCC"\\tocc\\tocccmd.doc";
char *comfilecheck = TOCC"\\tocc\\tocccmd.fin";
char *guidefile = TOCC"\\tocc\\guidecmd.doc";
char *guidefilecheck = TOCC"\\tocc\\guidecmd.fin";
BOOL ropen = FALSE;
BOOL jopen = FALSE;
BOOL statopen = FALSE;
BOOL copen = FALSE;
FILE *rfile = NULL;
FILE *sfile = NULL;
FILE *cfile = NULL;
char *restart = TOCC"\\tocc\\restart.doc";
#endif
#ifdef SOCKET
char *ioserver = "10.75.0.19";
#endif

int command_serv=0, command_sock=0, outsock;
int nudp;
char udp_command[80];

#ifdef PCNFS
void openrfile()
{
  rfile = fopen(respfile,"w");
  cprintf("opening respfile: %s\r\n",respfile);
  if (rfile != NULL) ropen = TRUE;
  jfile = fopen(junkfile,"w");
  cprintf("opening junkfile: %s\r\n",junkfile);
  if (jfile != NULL) jopen = TRUE;
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
#endif //PCNFS

// Function writeline outputs a line to terminal and possibly output file,
//    depending on value of icode
void writeline(char *out, int icode)
{
  struct date d;
  struct mytime t;
  char timestring[16];
  char file[64], tmpbuf[80];
  int i;

// Get the time and make a timestamp
  mygettime(&d,&t);
  double secs;
  secs = t.ti_sec + t.ti_hund/100.;
  if (strchr(out,'\n') == NULL)
    sprintf(timestring,"%d:%d:%f    ",t.ti_hour,t.ti_min,secs);
  else
    sprintf(timestring,"%d:%d:%f\n",t.ti_hour,t.ti_min,secs);

// Normal output is without timestamp, using cprintf if necessary
  if (icode==0) {
    if (command_mode)
      cprintf("%s",out);
    else
      printf("%s",out);
  }

// Output to remote
  if (remote_on && (icode==0 || icode==1 || icode>=10 || icode==6)) {
#ifdef SOCKET
    if (icode==0||icode==1) {
      // Output to port server
      outsock=-1;
      while (outsock<0 ) {
        setup_client(&outsock,1052,ioserver);
        if (outsock>0) {
          //if (icode==1) write_client(&outsock,timestring,0);
          //strcat(timestring,out);
          write_client(&outsock,out,1);
        } else {
          sprintf(tmpbuf,"error setting up client: %d\n",outsock);
          writelog(tmpbuf,18);
          sleep(1);
        }
      }
    } else if (icode==6) {
      // Output to status server
      setup_client(&outsock,1051,ioserver);
      if (outsock>0) write_client(&outsock,out,1);
    } else if (icode==10) {
      // Output to command_sock, final time
      if (command_sock>0) write_client(&command_sock,out,1);
    } else if (icode==11) {
      // Output to command_sock, without closing
      if (command_sock>0) write_client(&command_sock,out,0);
    }
#endif
#ifdef PCNFS

    if (ropen == FALSE) openrfile();
    if (icode==1)
      fprintf(rfile,"%s\n%s\n",timestring,out);
    else
      fprintf(rfile,"%s\n",out);
    fflush(rfile);
    if (ferror(rfile) != 0) cprintf("error writing file\r\n");
    fprintf(jfile,"x");
    fflush(jfile);
    if (icode==10) {
      for (i=0; i<2 ; i ++ ) {
             fprintf(rfile,"%s\n",out);
             fflush(rfile);
             fprintf(jfile,"x");
             fflush(jfile);
      }
    }
    if (icode == 6) {
      if (statopen == FALSE) opensfile();
      fprintf(sfile,"%s\n",out);
      fflush(sfile);
      if (ferror(sfile) != 0) cprintf("error writing status file\r\n");
      fprintf(jfile,"x");
      fflush(jfile);
    }
#endif //PCNFS
  }

// Error output
  if (icode==-1) 
      cprintf("%s\n\r",out);

// Debugging output
  if (icode==1) {
    if (command_mode)
      cprintf("%s\n\r%s\n\r",timestring,out);
    else
      printf("%s%s\n",timestring,out);
  }
#ifdef debug_move_file
  if (icode==2)
    if (G->move_file != NULL) fprintf(G->move_file,"%s%s\n",timestring,out);
#endif
#ifdef debug_tracking_file
  if (icode==3)
    if (G->track_file != NULL) fprintf(G->track_file,"%s%s\n",timestring,out);
#endif
#ifdef debug_pcx_file
  if (icode==4)
    if (G->pcx_file != NULL) fprintf(G->pcx_file,"%s%s\n",timestring,out);
#endif
#ifdef have_debug_file
  if (icode==9)
    if (G->debug_file != NULL) fprintf(G->debug_file,"%s%s\n",timestring,out);
#endif

}

void writelog(char *out, int icode)
{
  struct date d;
  struct mytime t;
  char timestring[32];

// Get the time and make a timestamp
  mygettime(&d,&t);
  double secs;
  secs = t.ti_sec + t.ti_hund/100.;
  sprintf(timestring,"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%6.3f",
          d.da_year,d.da_mon,d.da_day,t.ti_hour,t.ti_min,secs);

  if (lfile == NULL)
    lfile = fopen(logfile,"a");
  else {
    fprintf(lfile,"%3d %s %s\n",icode, timestring,out);
    fflush(lfile);
    if (jfile != NULL) {
      fprintf(jfile,"x");
      fflush(jfile);
    }
  }
}

// get a newline or CR terminated line
int getline(unsigned char *line, int linesize)
{
  int i, j, k, n, ns, nk, ng, ntot = 0, err;
  char key, junk;
  char inputline[MAXCMD], com[MAXCMD], *s;
  FILE *file;

  i = 0;
  ns = 0;
  nk = 0;
  ng = 0;

  remote_command = FALSE;

  while (TRUE) {
    nudp=0;
    update_all();
    n = 0;
    key = 0;

#ifndef HANGTEST
    if (nudp>0) {
      strncpy(inputline,udp_command,MAXCMD-2);
      sprintf(outbuf,"udp_command: %s\n",inputline);
      writeline(outbuf,1);
      n = nudp;
    }

#define HAVEKB
#ifdef HAVEKB
//  if we haven't gotten anything from the remote line, look for a keyboard hit
    while (nudp==0 && ns==0 && ng==0 && kbhit() && n<MAXCMD-2) {
      inputline[n++] = getch();
      nk++;
    }
#endif
#endif

//  look for command on socket
#ifndef HANGTEST
    if (remote_on && nk==0 && nudp == 0) {
      if (command_sock<=0) {
        // Is there a new connection waiting?
        command_sock = read_socket(command_serv,inputline,&err);
        if (command_sock < 0 && err != 35) {
          sprintf(outbuf,"error opening command_sock: %d %d\n",command_sock,err);
          writeline(outbuf,1);
        }

      } 
      else {
        sprintf(outbuf,"command_sock open: %d\n",command_sock);
        writeline(outbuf,1);
        // If we have an open connection, is there anything new there to read?
//        read_open(command_sock,inputline);
//sprintf(outbuf,"read_open: %s\n",inputline);
//writeline(outbuf,1);
      }

      if (command_sock>=0) {
        fprintf(stderr,"received: %d %s\n",command_sock,inputline);
        n=strlen(inputline);
        remote_command = TRUE;
      }
    }
#endif

#ifdef PCNFS
//  if remote_on and we haven't gotten anything from the keyboard, 
//     try to read a remote command, first from guider, then from command file
//    if (remote_on && nk==0 && SysTimer[SYSTMR_CHECK_COMMAND].Expired()) {
    if (remote_on && SysTimer[SYSTMR_CHECK_COMMAND].Expired()) {
      SysTimer[SYSTMR_CHECK_COMMAND].NewTimer(SYSTMR_CHECK_COMMAND_INC);

      // Guider command file
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

        s = getresp(cfile);
        ns = strlen(s);
        ntot += ns;
        if (ns>0) cprintf("received: %d %d %s\n",ntot, ns, s);
        for (k=0;k<ns;k++) inputline[k] = *s++;
        n = ns;
        remote_command = TRUE;
#else
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
#endif   // PCNFS

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
          sprintf(outbuf,"line too long\n");
          writeline(outbuf,1);
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

#ifdef PCNFS
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
#endif
