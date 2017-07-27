#include <unistd.h> /* for libc5 */

#include <sys/io.h> /* for glibc */
#include <stdio.h>
#include <unistd.h>

main(int argc, char *argv[])
{
 int status;
 if (ioperm(0,1000,1) !=0) {
   fprintf(stderr ,"io permission failure\n");
   exit(-2);
 }
 setpriority(PRIO_PROCESS,0,-20);
 setuid(1012);
 status = execl("/usr/local/bin/accd","accd",0);
 perror("execl returns:");
 exit(0);
}
