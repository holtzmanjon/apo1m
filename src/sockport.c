#define DEBUG

/* program to handle communication between PC and UNIX host computer 
   This program talks to the PC via a serial link. It can talk with any
   variety of UNIX programs using a FIFO to get commands to send. */

#include <fcntl.h>
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
  int sock, lsock, client, cmdclient;
  char command[201], ret[MAXCOMMAND], tempret[MAXCOMMAND];
  char *acknowledgement, *s;
  int n=0, nret, nread, nack, nreceived, ierr, maxset;
  fd_set readfds;
  struct timeval timeout;
  FILE *file;

  struct sockaddr_in addr;
  struct in_addr in_addr;
  int toccport, port, addrlen, waiting;
  
  char tstring[16], server[24], toccserver[24];

  signal(SIGINT,SIG_IGN);

  // Set up to listen on port 1052 for info from the telescope
  port = 1052;
  sprintf(server,"ccd1m.apo.nmsu.edu");
  setup_server(&sock,port,server);

  // Set up to listen on port 1053 for commands  from command, ccd, etc.
  port = 1053;
  sprintf(server,"ccd1m.apo.nmsu.edu");
  setup_server(&lsock,port,server);

  // Setup to write commands to telescope computer
  toccport = 1050;
  sprintf(toccserver,"tocc1m.apo.nmsu.edu");

  waiting = 0;
  while (1) {

/* Check to see whether anything has come in on the socket or keyboard */
    FD_ZERO(&readfds);
    maxset = 0;
    FD_SET(STDIN_FILENO,&readfds);  
    maxset = STDIN_FILENO+1 > maxset ? STDIN_FILENO+1 : maxset ;
    FD_SET(sock,&readfds);
    maxset = sock+1 > maxset ? sock+1 : maxset ;
    // Only accept new commands if we've finished with the preceding command
    if (waiting==0) {
      FD_SET(lsock,&readfds);
      maxset = lsock+1 > maxset ? lsock+1 : maxset ;
    }
    timeout.tv_sec = 1;
    timeout.tv_usec = 1000;

    if ((ierr=select(maxset,&readfds,NULL,NULL,&timeout))>0 ) { 
      if (FD_ISSET(STDIN_FILENO,&readfds)) {
        // we have input from the keyboard
        gets(command);
        sendtocc(toccport,toccserver,command,ret,MAXCOMMAND);
      }

      else if (FD_ISSET(sock,&readfds)) {
        // we have input from the telescope
        read_server(&sock,&client,port,server,tempret,sizeof(tempret));
        fprintf(stderr,"%s",tempret);
        close(client);
#ifdef DEBUG
        fprintf(stderr,"waiting: %d %d\n",waiting,nret);
#endif
        if (waiting) {
          if (strlen(tempret)+nret>MAXCOMMAND-1) {
            fprintf(stderr,"return buffer string too long: %s + %s",ret, tempret);
            fprintf(stderr,"from command: %s",command);
            nret=0;
          }
          strncpy(ret+nret,tempret,strlen(tempret));
          nret += strlen(tempret);
#ifdef DEBUG
          fprintf(stderr,"nret: %d %d\n",nret,strlen(tempret));
#endif
          if (strstr(ret,"DONE") != NULL) {
            write(cmdclient,ret,strlen(ret));
            write(cmdclient,"\r\n",2);
            close(cmdclient);
#ifdef DEBUG
            fprintf(stderr,"closed client: %d\n",cmdclient);
            fprintf(stderr,"returning: %s\n",ret);
#endif
            waiting = 0;
          }
        }
      }

      else if (waiting == 0 && FD_ISSET(lsock,&readfds)) {
        // we have input from the local client
        read_server(&lsock,&cmdclient,port,server,command,sizeof(command));
#ifdef DEBUG
        fprintf(stderr,"opened client: %d\n",cmdclient);
#endif
        fprintf(stderr,"%s",command);
        // send the command, then set flag to wait until DONE is received
        sendtocc(toccport,toccserver,command,ret,MAXCOMMAND);
        waiting = 1;
        nret = 0;
        memset(ret,0,sizeof(ret));
        ////write(client,"DONE\n",5);
        //write(client,ret,strlen(ret));
        //close(client);
      }
    } 
/*
    if (n>=10) {
        fprintf(stderr,"sending ALIVE\n");
        sendport(toccport,toccserver,"ALIVE");
        n=0;
    }
*/
    n++;
  }
}

