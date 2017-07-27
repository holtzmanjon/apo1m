#define DEBUG
/* program to handle communication between PC and UNIX host computer 
   This program talks to the PC via a serial link. It can talk with any
   variety of UNIX programs using a FIFO to get commands to send. */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define MAXCOMMAND 8000

int process_command(char *,char *, char *,char *,char *, char *);
void timestring(char *);

/* 
  We expect several command line arguments 
    argv[0] = port  
    argv[1] = the name of the FIFO 
    argv[2] = the name of the status file
    argv[3] = the name of the status ready file
    argv[4] = the name of the status file
    argv[5] = the name of the status ready file
*/ 

FILE *lfile;

main(argc, argv)
int argc;
char *argv[];

{
  int fromcom, tocom, serial, status, i;
  int fromstat, tostat;
  struct termios termios;
  char comfile[100], comfilecheck[100];
  char respfile[100], respfilecheck[100];
  char command[MAXCOMMAND+1];
  char acknowledgement[MAXCOMMAND+1];
  int n, nread, nack, nreceived ;
  fd_set readfds;
  struct timeval timeout;
  FILE *file;

  char tstring[16];

  lfile = fopen("port.log","a");

  if (argc < 7) {
    fprintf(stderr, "PORT requires 6 command line arguments\n");
    exit();
  }

/* Open the FIFO for communication with UNIX command program */
  fromcom = open(argv[1],O_RDWR);
  if (fromcom<0) {
     perror("PROGRAM port open fromcom");
     exit();
  }
  tocom = open(argv[2],O_RDWR);
  if (tocom<0) {
     perror("PROGRAM port open tocom");
     exit();
  }

/* Open the FIFO for communication with UNIX status program */
  fromstat = open(argv[3],O_RDWR);
  if (fromstat<0) {
     perror("PROGRAM port open fromstat");
     exit();
  }
  tostat = open(argv[4],O_RDWR);
  if (tostat<0) {
     perror("PROGRAM port open tostat");
     exit();
  }

/* Get the names of the command and response files */
  strcpy(comfile,argv[5]);
  strcat(comfile,".doc\0");
  strcpy(comfilecheck,argv[5]);
  strcat(comfilecheck,".fin\0");
  strcpy(respfile,argv[6]);
  strcat(respfile,".doc\0");
  strcpy(respfilecheck,argv[6]);
  strcat(respfilecheck,".fin\0");

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

/* Now we just sit and wait until we get a command to send or until we
   receive status information from the PC */

  while (1) {

/* Has the telescope sent a message? If so, print it out */
    file = fopen(respfile,"r");
    if (file != NULL) {
        n = 0;
        while (fgets(acknowledgement+n,MAXCOMMAND-1-n,file)!= NULL) {
            n = strlen(acknowledgement);
fprintf(stderr,"n: %d %s\n",n,acknowledgement);
        }
        fclose(file);
fprintf(stderr,"done reading\n");
        fprintf(stderr,"%s", acknowledgement);
        fprintf(lfile,"%s", acknowledgement);

	errno = 0;
        //if (remove(respfile) != 0) {
        //   perror("remove respfile:");
        // }
    }

/* Check to see whether anything has come in on the fifos. 
   If not, just wait */
    FD_ZERO(&readfds);
    FD_CLR(fromcom,&readfds);
    FD_CLR(fromstat,&readfds);
    FD_CLR(STDIN_FILENO,&readfds);  
    FD_SET(fromcom,&readfds);
    FD_SET(fromstat,&readfds);
    FD_SET(STDIN_FILENO,&readfds);  
    if (select(FD_SETSIZE,&readfds,0,0,&timeout)>0 ) { 

    if (FD_ISSET(STDIN_FILENO,&readfds)) {
      gets(command);
      sendtocc(comfile,comfilecheck,command);
    }

    else if (FD_ISSET(fromcom,&readfds)) {
      nread = 0;
      memset(command,0,sizeof(command));
      /* If we've read anything, keep reading until we encounter a newline */
      while ( strstr(command,"\n") == NULL ) {
        if ( (n = read(fromcom,command+nread,MAXCOMMAND-nread)) > 0) 
          nread += n;
      }
      /* send the command to the PC */
      command[nread] = '\0';

      memset(acknowledgement,0,sizeof(acknowledgement));
#ifdef DEBUG
      fprintf(stderr,"\ncommand sending: %d %s",strlen(command),command); 
      timestring(tstring);
      fprintf(lfile,"%s: command sending %d %s",
              tstring,strlen(command),command); 
#endif
      if (process_command(comfile,comfilecheck,respfile,respfilecheck,
          command,acknowledgement)<0) {
	fprintf(stderr,"got command timeout error\n");
	fprintf(lfile,"got command timeout error\n");
        write(tocom,"ERROR\n",6);
      }
      else {
#ifdef DEBUG
	fprintf(stderr,"got command ok\n");
#endif
        write(tocom,acknowledgement,strlen(acknowledgement));
      }

    }
  
    /* Is there a request for status information? If so ask for it & read it */
    else if (FD_ISSET(fromstat,&readfds)) {
      nread = 0;
      memset(command,0,sizeof(command));
      /* If we've read anything, keep reading until we encounter a newline */
      while ( strstr(command,"\n") == NULL ) {
        if ( (n = read(fromstat,command+nread,MAXCOMMAND-nread)) > 0) 
          nread += n;
      }
      /* send the command to the PC */
      command[nread] = '\0';

      memset(acknowledgement,0,sizeof(acknowledgement));
#ifdef DEBUG
      fprintf(stderr,"\nstatus sending: %d %s",strlen(command),command); 
      timestring(tstring);
      fprintf(lfile,"%s: status sending %d %s",
              tstring,strlen(command),command); 
#endif
      if (process_command(comfile,comfilecheck,respfile,respfilecheck,
          command,acknowledgement)<0) {
	fprintf(stderr,"got status timeout error\n");
	fprintf(lfile,"got status timeout error\n");
        write(tostat,"ERROR\n",5);
      }
      else {
        write(tostat,acknowledgement,strlen(acknowledgement));
      }

    }
    } 
  }
}

int process_command(
        char *comfile,char *comfilecheck,char *respfile,char *respfilecheck,
        char *command,char *inputline)
{
   FILE *file;
   char com1[16], com2[16];
   char resp1[16];
   int n;

   umask(000);

#ifdef DEBUG
   fprintf(stderr,"sending command: %s", command);
#endif
   fprintf(stderr,"%s", command);
   fprintf(lfile,"%s", command);

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

   /* Wait for something to come back */
#ifdef DEBUG
   fprintf(stderr,"waiting... \n");
#endif
   resp1[0] = 0;
   while (strcmp(resp1,"DONE:") != 0) {

        /* read the response file */
        file = fopen(respfile,"r");
        while (file==NULL) {
          file = fopen(respfile,"r");
        }
        n = 0;
        while (fgets(inputline+n,MAXCOMMAND-1-n,file)!= NULL) {
          n = strlen(inputline);
        }
        fclose(file);
        sscanf(inputline, "%s", resp1);
        if (strcmp(resp1,"DONE:") != 0) {
          fprintf(stderr,"%s", inputline);
          fprintf(lfile,"%s", inputline);
        }

	errno = 0;
        if (remove(respfile) != 0) {
           perror("remove respfile:");
        }
   }
   return(0);

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

   return(0);
}

