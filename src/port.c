#undef DEBUG

/* program to handle communication between PC and UNIX host computer 
   This program talks to the PC via a serial link. It can talk with any
   variety of UNIX programs using a FIFO to get commands to send. */

#include <fcntl.h>
#include <termios.h>
#include <sys/uio.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXCOMMAND 8000

char *process_command(char *,char *, char *,char *,char *);
void timestring(char *);
char *getresp(FILE *);

/* 
  We expect several command line arguments 
    argv[0] = port  
    argv[1] = the name of the FIFO 
    argv[2] = the name of the status file
    argv[3] = the name of the status ready file
    argv[4] = the name of the status file
    argv[5] = the name of the status ready file
*/ 

FILE *lfile, *rfile, *cfile;
int ropen = 0;
int copen = 0;

main(argc, argv)
int argc;
char *argv[];

{
  int serial, status, i, irfile, ifile;
  int localsock, remotesock, client;
  struct termios termios;
  char *comfile = "/home/export/tocc/tocccmd.doc";
  char *comfilecheck = "/home/export/tocc/tocccmd.fin";
  char *respfile = "/home/export/tocc/toccr.doc";
  char *respfilecheck = "/home/export/tocc/toccr.fin";
  char *restart = "/home/export/tocc/restart";
  char command[MAXCOMMAND+1];
  char *acknowledgement, *s;
  int n, nread, nack, nreceived, ierr, maxset;
  fd_set readfds;
  struct timeval timeout;
  FILE *file;

  struct sockaddr_in addr;
  struct in_addr in_addr;
  int port, sock, addrlen;
  
  char tstring[16], server[24];

  signal(SIGINT,SIG_IGN);

  lfile = fopen("port.log","a");

/* Open the socket for communication with UNIX command program(s) */
  fprintf(stderr,"setting up server and waiting for command to connect...");
  setup_server(&localsock,0,"toport");
  port = 1050;
  sprintf(server,"192.41.211.24");  
  setup_server(&remotesock,port,server);

  fprintf(stderr,"respfile: %s\n",respfile);
  fprintf(stderr,"respfilecheck: %s\n",respfilecheck);
  fprintf(stderr,"comfile: %s\n",comfile);
  fprintf(stderr,"comfilecheck: %s\n",comfilecheck);

/* Now we just sit and wait until we get a command to send or until we
   receive status information from the PC */

  while (1) {

    file = fopen(restart,"r");
    if (file != NULL) {
      fclose(file);
      if (ropen == 1) fclose(rfile);
      ropen = 0;
      remove(restart);
    }

/* Has the telescope sent a message? if so, print it out */
    if (ropen == 0) {
      rfile = fopen(respfile,"r");
      irfile = open(respfile,O_RDWR);
// fprintf(stderr,"respfile: %s irfile: %d  rfile: %d\n",respfile,irfile,rfile);
      if (rfile != NULL) ropen = 1;
    }
    //if (irfile <= 0)  irfile = open(respfile,O_RDWR);

#ifdef DEBUG
  fprintf(stderr,"checking for message..\n");
#endif
    s = getresp(rfile);
    while (s != NULL) {
          fprintf(stderr,"%s", s);
          fprintf(lfile,"%s", s);
          s = getresp(rfile);
//          sleep(1);
    }

/* Check to see whether anything has come in on the fifos. 
   If not, just wait */
    FD_ZERO(&readfds);
    maxset = 0;
    FD_SET(STDIN_FILENO,&readfds);  
    maxset = STDIN_FILENO+1 > maxset ? STDIN_FILENO+1 : maxset ;
    FD_SET(localsock,&readfds);
    maxset = localsock+1 > maxset ? localsock+1 : maxset ;
    FD_SET(remotesock,&readfds);
    maxset = remotesock+1 > maxset ? remotesock+1 : maxset ;
    timeout.tv_sec = 1;
    timeout.tv_usec = 1000;

    if ((ierr=select(maxset,&readfds,NULL,NULL,&timeout))>0 ) { 

    if (FD_ISSET(STDIN_FILENO,&readfds)) {
      gets(command);
      sendtocc(comfile,comfilecheck,command);
    }

    else if (FD_ISSET(localsock,&readfds)||
             FD_ISSET(remotesock,&readfds)) {
      if (FD_ISSET(localsock,&readfds)) {
        read_server(&localsock,&client,0,"toport",command,sizeof(command));
      } else {
        read_server(&remotesock,&client,port,server,command,sizeof(command));
      }

      if ((acknowledgement=process_command(comfile,comfilecheck,
           respfile,respfilecheck, command))==NULL) {
	fprintf(stderr,"got command timeout error\n");
	fprintf(lfile,"got command timeout error\n");
          write(client,"ERROR\n",6);
      }
      else {
          write(client,acknowledgement,strlen(acknowledgement));
      }
      close(client);

    }
  
    else if (irfile > 0 && FD_ISSET(irfile,&readfds)) {
      s = getresp(rfile);
      FD_CLR(irfile,&readfds);
      if (s != NULL) {
          fprintf(stderr,"%s", s);
          fprintf(lfile,"%s", s);
      }

    }

    } 
  }
}

char *process_command(
        char *comfile,char *comfilecheck,char *respfile,char *respfilecheck,
        char *command)
{
   FILE *file;
   char com1[16], com2[16];
   char resp1[16], *s;
   int n, ierr;

   if (strlen(command) == 0) return(NULL);

   if (strcmp(command,"QU") == 0) exit(0);

   umask(000);

/*
   sleep(1);
   while ( (s=getresp(rfile)) != NULL) {
     fprintf(stderr,"flushed: %s\n",s);
     sleep(1);
   }
*/

#ifdef DEBUG
   fprintf(stderr,"sending command: %s", command);
#endif
   fprintf(stderr,"%s", command);
   fprintf(lfile,"%s", command);

#ifdef NEWCOM
   /* Dont write a command file until the check file has been deleted */
   if (copen == 0) {
      cfile = fopen(comfile,"w");
      if (cfile != NULL) copen = 1;
   }
   fprintf(cfile,"%s\n",command);
   fflush(cfile);
#else

   /* Dont write a command file until the check file has been deleted */
   while ((file=fopen(comfilecheck,"r")) !=NULL) {
        fclose(file);
   }

#ifdef DEBUG
   fprintf(stderr,"opening comfile: %s\n",comfile);
#endif

   /* Open the command file and write the command */
   file = fopen(comfile,"w");
   while (file==NULL) {
     perror("open error 2: ");
     file = fopen(comfile,"w");
   }
   fprintf(file,"%s",command);
   fclose(file);

#ifdef DEBUG
   fprintf(stderr,"opening comcheckfile: %s\n",comfilecheck);
#endif
   /* Write the command check file */
   file = fopen(comfilecheck,"w");
   while (file==NULL) {
     perror("open error 3: ");
     file = fopen(comfilecheck,"w");
   }
   fclose(file);
#endif


   /* Wait for something to come back */
#ifdef DEBUG
   fprintf(stderr,"waiting... \n");
#endif
   resp1[0] = 0;
   while (strcmp(resp1,"DONE:") != 0) {

     if (ropen == 0) {
       rfile = fopen(respfile,"r");
       if (rfile != NULL) ropen = 1;
     } 

     s = getresp(rfile);

     if (s != NULL) {
#ifdef DEBUG
fprintf(stderr,"s: %s\n",s);
#endif
          sscanf(s,"%s", resp1);
          if (strcmp(resp1,"DONE:") != 0) {
            fprintf(stderr,"%s", s);
            fprintf(lfile,"%s", s);
          }
     }
     usleep(10);
   }
#ifdef DEBUG
fprintf(stderr,"process_command returning\n");
#endif
   return(s);

}
void timestring(char *tstring)
{
  time_t ti;
  struct tm *t;
  char outbuf[200];

  time(&ti);
  t = localtime(&ti);
  if (strchr(outbuf,'\n') == NULL)
    sprintf(tstring,"%d:%d:%d.%d    ",t->tm_hour,t->tm_min,t->tm_sec,0);
  else
    sprintf(tstring,"%d:%d:%d.%d\n",t->tm_hour,t->tm_min,t->tm_sec,0);
}

int sendtocc( char *comfile,char *comfilecheck,char *command)
{
   FILE *file;

   umask(000);

#ifdef DEBUG
   fprintf(stderr,"sending tocc command: %s", command);
#endif

#ifdef NEWCOM
   if (copen == 0) {
      cfile = fopen(comfile,"w");
      if (cfile != NULL) copen = 1;
   }
   fprintf(cfile,"%s\n",command);
   fflush(cfile);
#else

   /* Dont write a command file until the check file has been deleted */
   while ((file=fopen(comfilecheck,"r")) !=NULL) {
        fclose(file);
   }

   /* Open the command file and write the command */
   file = fopen(comfile,"w");
   while (file == NULL) {
     perror("open error 7: ");
     file = fopen(comfile,"w");
   }
   fprintf(file,"%s\n",command);
   fclose(file);

   /* Write the command check file */
   file = fopen(comfilecheck,"w");
   while (file == NULL) {
     perror("open error 8: ");
     file = fopen(comfilecheck,"w");
   }
   fclose(file);
#endif

   return(0);
}
