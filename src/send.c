#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include "io.h"
#define NEW
void getfiforesp(int, char *);

#ifdef SOCKET

// sendtocc sends a command and does not wait for any response
int sendtocc(int port,char *server,char *command, char *ret, int maxret)
{
   int sock, nread, nret, flags, n;
   char line[MAXCMD];

//fprintf(stderr,"sending command %d %s %s\n",port, server,command);
   setup_client(&sock,port,server,5);
//fprintf(stderr,"sock: %d\n",sock);
   if (sock>0) {
     write(sock,command,strlen(command));
     write(sock,"\n",1);
     close(sock);
   }
}

// sendport sends a command and waits for something (anything if WAIT is not set) to come back
int sendport(int port,char *server,char *command, char *ret, int maxret)
{
   int sock, nread, nret, flags, n;
   char line[MAXCMD];

//fprintf(stderr,"sending command %d %s %s %d\n",port, server,command, strlen(command));
   setup_client(&sock,port,server,3);
   if (sock>0) {
     write(sock,command,strlen(command));
     if (strstr(command,"\n") == NULL) write(sock,"\n",1);
  
     nread = 0;
     memset(ret,0,maxret);
     n = read(sock,ret,maxret);
#ifdef WAIT
     line[0] = 0;
     nret = 0;
     memset(ret,0,maxret);
     while (strstr(line,"DONE") == NULL) {
      nread=0;
      memset(line,0,MAXCMD);
      while (strstr(line,"\n") == NULL) {
        n = read(sock,line+nread,1);
        if (n>0) 
          nread += n;
        else {
          sleep(1);
        }
      }
      strncpy(ret+nret,line,nread);
      nret+=nread;
      // fprintf(stderr,"received: %d %s\n",nread,line);
     }
#endif
     close(sock);
     if (nread>maxret)
       return(1);
     else
       return(0);
   } else {
     fprintf(stderr,"no server available!\n");
     return(-1);
   }
}

#else
int sendport(int port,char *server,char *command)
{
    char input[MAXCMD];
    int status, n, fifo, fifo2;

    if (port<0) return(0);

    setup_client(&fifo,port,server,1);
    fifo2 = fifo;
    strcat(command,"\n");

    if (fifo == 0) {
      fprintf(stderr,"telescope not accepting commands: %s\n",command);
      return(-1);
    }
    write(fifo,command,strlen(command));

    if (strcmp(command,"QU") == 0) return(0);

    memset(input,0,sizeof(input));
#ifdef NEW
    getfiforesp(fifo2,input);
    while (sscanf(input,"DONE:%d",&status) <= 0) {
      fprintf(stderr,"%s\n",input);
      getfiforesp(fifo2,input);
    }
    close(fifo);
    return(status);
#else
    while ( (n=read(fifo2,input,MAXCMD-1)) <= 0) {};
    if (sscanf(input,"DONE:%d",&status) <= 0)
      return(-1);
    else
      return(status);
#endif
}
#endif

int sendccd(int port, char *server, char *command, int timeout)
{
    char input[MAXCMD];
    int n, status, itimeout, fifo, fifo2;

    if (port < 0) {
      fprintf(stderr,"ccd not accepting commands: %s\n",command);
      return(-1);
    }
    setup_client(&fifo,port,server,1);
    fifo2= fifo;

    if (fifo == 0) {
      fprintf(stderr,"ccd not accepting commands: %s\n",command);
      return(-1);
    }

    write(fifo,command,strlen(command));
    write(fifo,"\n",1);
    if (strcmp(command,"QU") == 0) {
      close(fifo);
      return(0);
    }

    memset(input,0,sizeof(input));
#ifdef NEW
#define TIMEOUT
#ifdef TIMEOUT
    if (timeout != 0) {
      sprintf(input,"/usr/local/bin/ringbell 60 %d &", timeout>0 ? timeout : -timeout);
/*
      itimeout = (int)ceil((timeout>0 ? timeout : -timeout)/60.);
      itimeout = itimeout > 2 ? itimeout : 3 ;
      sprintf(input,"at -f /usr/local/bin/restart_ccd now+%05d minutes >&/dev/null",
             itimeout); 
*/
/*fprintf(stderr,"%s\n",input);*/
      system(input);
    }
#endif

    getfiforesp(fifo2,input);
    while ((n=sscanf(input,"DONE: %d",&status)) <= 0) {
      fprintf(stderr,"%s\n",input);
      getfiforesp(fifo2,input);
    }
#ifdef TIMEOUT
    if (timeout != 0) {
      sprintf(input,"kill `ps uxww | grep ringbell | grep -v grep | awk '{print $2}'`"); 
/*      sprintf(input,"atrm `atq | awk '{print $1}'`"); */
/*fprintf(stderr,"%s\n",input);*/
      system(input);
    }
    if (status==201) {
      fprintf(stderr,"CCD appears to have hung.. resetting and trying again!");
      status = sendccd(port,server,"INITCCD",100);
      status = sendccd(port,server,command,timeout+100);
    }
#endif
    close(fifo);
    if (status>0) {
      fprintf(stderr,"Error with command!!\n");
      // error_code(status); if we have this, then error_code is called twice!
      //                     which is bad for master control!
      return(-1);
    }
    else
      return(status);
#else
    while ( (n=read(fifo2,input,MAXCMD-1)) <= 0) {};
    n = sscanf(input,"DONE: %d",&status);
    close(fifo);
    if (status>0 || n<=0)
      return(-1);
    else
      return(status);
#endif
}

#ifdef NEW

void getfiforesp(int rfile, char *input)
{
   int ierr, n;
   char response[MAXCMD];

   if (rfile > 0) {
/* Has the telescope sent a message? If so, print it out */
     n = 0;
     memset(response,0,sizeof(response));

     while ((ierr=read(rfile,response+n,1)) >0
           && strstr(response,"\n")==NULL) {
            n++;
     }
     strcpy(input,response);
     if (n>0) {
       return;
     }
     else
       return;
   }
}
#endif
