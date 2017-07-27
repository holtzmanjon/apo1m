#include <sys/types.h>
#include <sys/socket.h>
#include <4bsddefs.h>      /* for standard socket calls */
/* #include <sys/socket.h> */
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

int setup_server(int *, int);
int read_socket(int, char *, int *);
int read_open(int, char *);
int setup_client(int *sock, int port, char *server);
int write_client(int *, char *,int);
void sock_close(int);
int read_udp(int, char *, int);

#undef MAIN
#ifdef MAIN
main()
{
  int sock, outsock, ret, nloop;
  char line[60], response[32];

fprintf(stderr,"calling setup_server\n");
  setup_server(&sock, -19999);
fprintf(stderr,"sock: %d\n",sock);

  while (TRUE) {
fprintf(stderr,"calling read_udp, no timer\n");
    read_udp(sock,line,60);
fprintf(stderr,"udp returns: %s\n",line);
 }

  nloop=0;
  while (FALSE) {
  //  fprintf(stderr,"socket: %d hit a key to wait for command, q to quit: \n",sock);
  //  gets(line);
    if (line[0] == 'q') exit(0);
    setup_client(&outsock,1051,"172.16.48.1");
    if (outsock>0) {
      sprintf(response,"loop: %d\n",nloop);
      write(outsock,response,strlen(response));
    } else
      fprintf(stderr,"no output server available\n");
    close(outsock);

    ret = read_socket(sock, line);
    fprintf(stderr,"ret: %d line: %s\n", ret, line);
    nloop++;
  }

}
#endif


// setup_client attempts to open a socket to a server. If successful, it
//  returns the socket value, otherwise it returns -1
int setup_client(int *sock, int port, char *server)
{
  struct sockaddr_in addr;
  unsigned long inetaddr;
  int status;

  /* get a socket */
  *sock = socket (AF_INET, SOCK_STREAM, 0);
  if (*sock < 0) {
    perror ("client socket()");
    return(-1);
  }
  addr.sin_family      = AF_INET;    /* "Internet" Address Family */
  if ((inetaddr = inet_addr(server)) < 0L) {
    perror("inet_addr()");
    close(*sock);
    *sock = -1;
    return(-1);
  }
  addr.sin_addr.s_addr = inetaddr;    /* Local IP address */
  addr.sin_port = htons(port);  
  status = connect(*sock, &addr, sizeof(addr));
  if (status<0) {
    perror("connect error");
    close(*sock);
    *sock = -1;
    return(status);
  }
  return(status);
//fprintf(stderr,"opened: %d %d\n",*sock,port);
  
}

// write_client writes to an open socket, then closes it
int write_client(int *sock,char *string,int close)
{
  int ntry,ret;
//fprintf(stderr,"writing to %d\n",*sock);
  if (write(*sock,string,strlen(string))<0) fprintf(stderr,"error writing\n");
  if (write(*sock,"\n",1)<0) fprintf(stderr,"error writing\n");
  if (close) {
    ntry=0;
    while (ntry<5&&(ret=close(*sock))!=0) {
      ntry++;
      fprintf(stderr,"error closing %d\n",*sock);
    }
    if (ret==0) *sock = 0;
  }
  return(0);
}

// setup_server sets us up as a listening server, ready to get commands
int setup_server(int *sock, int port)
{
  struct sockaddr_in  stLclAddr;      /* Sockets Address Structure */
  struct sockaddr_in  stRemAddr;
  struct linger       stTimeOut;      /* Linger time(out) structure */
  int         wAddrLen = sizeof (stLclAddr);  /* work variable */
  long lHostID;
  int wOpt;
  short p;

  int tmp;
  long last;

  if (*sock>0) close(*sock);

  /* get a socket */
  if (port>0) 
    *sock = socket (AF_INET, SOCK_STREAM, 0);
  else
    *sock = socket (PF_INET, SOCK_DGRAM, 0);
  if (*sock < 0) {
        perror ("socket()");
        return(-1);
  }

  /* get local IP address from pctcp.ini file */
  lHostID = gethostid();
  if (lHostID == 0) {
        perror ("gethostid()");
        return(-2);
  }

  /* initialize local sockaddr_in structure */
  bzero(&stLclAddr, sizeof(stLclAddr));
  stLclAddr.sin_family      = AF_INET;    /* "Internet" Address Family */
  p = abs(port);
  stLclAddr.sin_port = htons(p);  
  if (port>0) stLclAddr.sin_addr.s_addr = lHostID;    /* Local IP address */

  /* bind local ip address & port number to the socket */
  if (bind (*sock, (struct sockaddr*)&stLclAddr, wAddrLen) < 0) {
      perror ("bind");
      return(-3);
  }

  /* "passively open" the socket */
  if (port>0) {
    if ((listen (*sock, 3)) < 0) {
        perror("listen()");
        return(-4);
    }
  }
 
  // Setup for nonblocking socket
  last = 1;
  tmp = ioctl(*sock,FIONBIO,&last);
  return(0);
}

int read_udp(int lsock, char *line, int maxlen) 
{
  int flags=0;
  struct sockaddr addr;
  int addrlen,len;
  struct itimerval itimer;
  void timeout_handler();
  
  len = recvfrom(lsock, line, maxlen-1, flags, &addr, &addrlen);
  line[len] = 0;
  return(len);

}

// read_socket sees if there is anything to accept/read on our listening
//   port that was set up by setup_server. If something is read, it is
//   returned along with the socket of the client. Otherwise it returns -1
int read_socket(int lsock, char *line, int *err)
{
  int i, nline, nread, sock;
  struct itimerval itimer;
  void timeout_handler();
#define BUFLEN 132
  char templine[BUFLEN], response[BUFLEN];
  struct sockaddr_in  stRemAddr;
  int wAddrLen = sizeof (stRemAddr);  /* work variable */
  struct linger       stTimeOut;      /* Linger time(out) structure */
  int wOpt;

  line[0] = 0;
  nline = 0;

/*
  itimer.it_value.tv_sec = -1;
  itimer.it_value.tv_usec = 1000;
  setitimer(ITIMER_REAL, &itimer, NULL);
  signal(SIGALRM, timeout_handler);
*/
  *err = 0;
  sock = accept (lsock, (struct sockaddr*) &stRemAddr, &wAddrLen);
  if (sock < 0) {
       // perror ("accept()");
        *err = errno;
        return(-1);
  }
/*
  stTimeOut.l_onoff = 1;
  stTimeOut.l_linger = 0;
  if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_LINGER,
                   (char *) &stTimeOut,
                   sizeof(stTimeOut)))
           perror ("setsockopt (SO_LINGER)");
*/
  wOpt = 1;
  if (ioctl(sock, FIONBIO, (char *)&wOpt))
        perror ("ioctl (non-block)");

  while (TRUE) {
    if ((nread = read (sock, templine, BUFLEN-1)) <= 0) {
      if (errno != EWOULDBLOCK) {
        perror ("read()");
        close (sock);
        return(-1);
      }
    } else {
      for (i=0; i<nread; i++) line[nline+i] = templine[i];
      line[nline+nread] = 0;
      nline += nread;
    }
    if (strstr(line,"\n") != NULL) {
   //   sprintf(response,"working on your request: %s\n",line);
   //   write(sock,response,strlen(response));
   //   write(sock,"DONE\n",5);
   //   close(sock);
   //   return(0);
      close(sock);
      return(0);
      //return(sock);
    }
  }

}
int read_open(int sock,char *line)
{
#define BUFLEN 132
  char templine[BUFLEN], response[BUFLEN];
  int nread, i, nline;

  line[0] = 0;
  nline = 0;
  while (TRUE) {
    if ((nread = read (sock, templine, BUFLEN-1)) <= 0) {
      if (errno != EWOULDBLOCK) {
        perror ("read()");
        close (sock);
        return(-1);
      }
    } else {
      for (i=0; i<nread; i++) line[nline+i] = templine[i];
      line[nline+nread] = 0;
      nline += nread;
    }
fprintf(stderr,"nread: %d %s\n",nread,line);
    if (nread<=0 || strstr(line,"\n") != NULL) {
      return(sock);
    }
  }
}
// Dummy function to run on accept timeouts
void timeout_handler()
{
  //fprintf(stderr,"timeout\n");
  return;
}
void sock_close(int sock)
{
  close(sock);
}
