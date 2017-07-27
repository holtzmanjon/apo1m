#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>


#define TRUE 1
#define FALSE 0
main()
{
  int s, istat, n, m, pending, port;
  char buf[3000], server[24];

  fprintf(stderr,"Enter port: ");
  gets(buf);
  sscanf(buf,"%d",&port);
  fprintf(stderr,"Enter server: ");
  gets(server);

  while (1) {
    fprintf(stderr,"Command: ");
    gets(buf);

    setup_client(&s,port,server);
        n=write(s,buf,strlen(buf));;
        n=write(s,"\n",1);;
        if (n>=0) {
          m = read(s,buf,3000);
          if (m>=0) {
            buf[m] = 0;
            pending = FALSE;
          }
          fprintf(stderr,"m: %d buf: %s\n",m,buf);
        }
    close(s);
  }
}
