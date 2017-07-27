/* Routines to set up socket servers and clients, both for IP and local
   connection */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

void setup_server(int *sock,int port,char *server);
void read_server(int *sock, int *client, int port,char *server, char *buf, int maxlen);
void read_udp(int *sock, char *buf, int maxlen);

#undef MAIN
#ifdef MAIN

#undef WRITE
#ifdef WRITE
main()
{
 int sock,port,nread;
 char server[16], line[80];

 port = 7;
 sprintf(server,"172.16.48.129"); // ccd1m
 port = 1050;
 sprintf(server,"192.41.211.22"); // tocc1m
 sprintf(server,"tocc1m.apo.nmsu.edu");
 setup_client(&sock,port,server,1);

 if (sock>0) {

  fprintf(stderr,"enter string to send: ");
  gets(line);
  strcat(line,"\n");

  write(sock,line,strlen(line));

  line[0] = 0;
  while (strstr(line,"DONE") == NULL) {
    nread=0;
    memset(line,0,80);
fprintf(stderr,"starting read\n");
    while (strstr(line,"\n") == NULL) {
      nread += read(sock,line+nread,1);
fprintf(stderr,"nread: %d %s\n",nread,line);
    }
    fprintf(stderr,"received: %d %s\n",nread,line);
  }
 } else {
   fprintf(stderr,"no server available!\n");
 }
}
#else
main()
{
  int sock, port, client;
  char server[16], line[800];

  port=1051;
  port=-19999;
  sprintf(server,"172.16.48.1");
  sprintf(server,"192.41.211.24");
  sprintf(server,"command1m.apo.nmsu.edu");
  setup_server(&sock,port,server);

  while (1) {
   // read_server(&sock, &client, port, server, line, 800);
    read_udp(&sock, line, 800);
    fprintf(stderr,"line: %s\n",line);
    close(client);
  }
}
#endif

#endif
/* setup_server returns a fd for the listening socket (sock) and a fd for
   the first client to connect to this socket (client). The server will
   listen from passed server/port. If port==0, then connection is assumed
   local with identifier given by server */

void setup_server(int *sock,int port,char *server)
{
  struct sockaddr_in addr;
  struct sockaddr_un uaddr;
  struct in_addr in_addr, **addr_list;
  int addrlen, status, n, i, maxset, domain;
  short timer = 300;
  short p;
  struct hostent *host;

  if (port != 0)
    domain = AF_INET;
  else
    domain = PF_UNIX;

  memset((char *)&addr,0,sizeof(addr));
  if (port != 0) {
    addr.sin_family = AF_INET;
    p = abs(port);
    addr.sin_port = htons(p);
    if (port>0) {

      host = gethostbyname(server);
      if (host==NULL) {
        fprintf(stderr,"gethostbyname failed\n");
        exit(-1);
      }
      addr_list = (struct in_addr **) host->h_addr_list;
      addr.sin_addr.s_addr = *(long *)(addr_list[0]);
/*
      status = inet_aton(server,&in_addr);         
      if (status<0) {
        fprintf(stderr,"inet_aton returns %d\n",status); exit(-1);
      }
      addr.sin_addr = in_addr;
*/
    }
    addrlen = sizeof(addr);
  } else {
    uaddr.sun_family = AF_UNIX;
    uaddr.sun_path[0] = 0;
fprintf(stderr,"need to code for server name instead of IP!");
exit(0);
    for (i=0; i<strlen(server); i++) uaddr.sun_path[i+1] = server[i];
    addrlen = sizeof(uaddr.sun_family) + strlen(server) + 1;
  }

  status = -1;
  n=0;
  if (port>=0) 
    *sock = socket(domain, SOCK_STREAM, 0);
  else {
    *sock = socket(PF_INET, SOCK_DGRAM, 0);
  }
  if (*sock<0) {
      fprintf(stderr,"socket returns %d\n",status);
      exit(-1);
  }
  while (status<0&&n<timer) {
      if (port!=0) {
        status = bind(*sock, (struct sockaddr *)&addr, sizeof(addr));
      } else {
        status = bind(*sock, (struct sockaddr *)&uaddr, addrlen);
      }
      if (status<0) perror("bind error:");
      n++;
      sleep(1);
  }
  if (n<timer) {
     if (port>=0) listen(*sock,5);
  } else {
      fprintf(stderr,"bind timed out...");
      *sock = -1;
  }
}

void read_udp(int *sock, char *buf, int maxlen)
{
  int flags=0;
  struct sockaddr addr;
  socklen_t addrlen;

  recvfrom(*sock, buf, maxlen, flags, &addr, &addrlen);
}

void read_server(int *sock, int *client, int port,char *server, char *buf, int maxlen)
{
  struct sockaddr_in addr;
  struct sockaddr_un uaddr;
  struct in_addr in_addr, **addr_list;
  int addrlen, status, n, i, maxset, domain, nread, nfail;
  struct hostent *host;
  FILE *fp;

  if (port > 0)
    domain = PF_INET;
  else
    domain = PF_UNIX;

  if (port > 0) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    nfail=0;
    while (  (host=gethostbyname(server)) == NULL && nfail< 5 ) {
      nfail++;
      fprintf(stderr,"gethostbyname failed, try again...\n");
      sleep(5);
    }
    //host = gethostbyname(server);
    if (host==NULL) {
      fprintf(stderr,"gethostbyname failed: %d\n",h_errno);
      fprintf(stderr,"using fixed IP address\n");
      fprintf(stderr,"s_addr: %d  %s\n", addr.sin_addr.s_addr,
         inet_ntoa(addr.sin_addr.s_addr), inet_addr("10.75.0.24"));
      fp=fopen("socket.err","a");
      fprintf(fp,"gethostbyname failed: %d\n",h_errno);
      fclose(fp);
      //exit(-1);
      addr.sin_addr.s_addr = inet_addr("10.75.0.24");
    } else {
      addr_list = (struct in_addr **) host->h_addr_list;
      addr.sin_addr.s_addr = *(long *)(addr_list[0]);
    }


/*
    status = inet_aton(server,&in_addr);         
    if (status<0) {
      fprintf(stderr,"inet_aton returns %d\n",status);
      exit(-1);
    }
    addr.sin_addr = in_addr;
*/
    addrlen = sizeof(addr);
  } else {
    uaddr.sun_family = AF_UNIX;
    uaddr.sun_path[0] = 0;
fprintf(stderr,"need to code for server name instead of IP!");
exit(0);
    for (i=0; i<strlen(server); i++) uaddr.sun_path[i+1] = server[i];
    addrlen = sizeof(uaddr.sun_family) + strlen(server) + 1;
  }

  *client = 0;
  if (domain==PF_INET) 
      *client = accept(*sock,(struct sockaddr *)&addr,&addrlen);
  else
      *client = accept(*sock,(struct sockaddr *)&uaddr,&addrlen);
  if (*client <0) perror("accept error:");

  nread = 0;
  memset(buf,0,maxlen);
  /* If we've read anything, keep reading until we encounter a newline */
  while ( strstr(buf,"\n") == NULL ) {
     if ( (n = read(*client,buf+nread,maxlen-nread)) > 0)  {
       nread += n;
     }
  }
  buf[nread] = 0;
}

void setup_client(int *sock, int port, char *server, short timer)
{
  int status, n, i, addrlen, domain;
  struct sockaddr_in addr;
  struct sockaddr_un uaddr;
  struct in_addr in_addr, **addr_list;
  struct hostent *host;

  if (port > 0) {
    domain = PF_INET;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    host = gethostbyname(server);
    if (host==NULL) {
      fprintf(stderr,"gethostbyname failed\n");
      exit(-1);
    }
    addr_list = (struct in_addr **) host->h_addr_list;
    addr.sin_addr.s_addr = *(long *)(addr_list[0]);
/*
    status = inet_aton(server,&in_addr);
    if (status<0) {
      perror("inet_aton returns:");
      exit(0);
    }
    addr.sin_addr = in_addr;
*/
  } else {
    domain = PF_UNIX;
    uaddr.sun_family = AF_UNIX;
    uaddr.sun_path[0] = 0;
fprintf(stderr,"need to code for server name instead of IP!");
exit(0);
    for (i=0; i<strlen(server); i++) uaddr.sun_path[i+1] = server[i];
    addrlen = sizeof(uaddr.sun_family) + strlen(server) + 1;
  }

  status = -1;
  n=0;
  while (status<0&&n<timer) {
    *sock = socket(domain, SOCK_STREAM, 0);
    if (*sock<0) {
      fprintf(stderr,"socket returns %d\n",status);
      perror("socket error:");
      exit(-1);
    }
    if (domain==PF_INET)
      status = connect(*sock,(struct sockaddr *)&addr,sizeof(addr));
    else 
      status = connect(*sock,(struct sockaddr *)&uaddr,addrlen);
    if (status < 0) {
      perror("connect error:");
/*
      fprintf(stderr,"sending server command via udp\n");
      system("pysend server");
*/
      sleep(5);
      close(*sock);
      *sock = 0;
    }
    n++;
  }
  if (*sock==0) {
    fprintf(stderr,"connect timed out.... no connection available\n");
  }
}

