#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

main()
{
  int s, client, n, nread;
#define MAXBUF 80
  char buf[MAXBUF];
  fd_set readfds;
  struct timeval timeout;
  int status,maxset;
  
  setup_server(&s,0,"toport");

 while (1) {
  FD_CLR(s,&readfds);
  FD_SET(s,&readfds);
  maxset = 0;
  maxset = s+1 > maxset ? s+1 : maxset ;
  fprintf(stderr,"waiting for select...");
  if ((status=select(maxset,&readfds,NULL,NULL,NULL))>0) {
      fprintf(stderr,"select returned...\n");
      if (FD_ISSET(s,&readfds)) {
        read_server(&s,&client,0,"toport",buf,sizeof(buf));
fprintf(stderr,"read: %s\n",buf);
        if (client <0) 
          perror("accept error:");
        else {
          if (nread>=0) write(client,"acknowledged\n",12);
          close(client);
        }
      }
  } else
        fprintf(stderr,"select timed out...\n");
 }

}
