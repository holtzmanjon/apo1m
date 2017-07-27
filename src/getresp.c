#include <stdio.h>
#include <string.h>
#include <errno.h>
#define MAXCOMMAND 3000

//char *getresp(char *respfile, char *respfilecheck)
//{
// static int iresp = 0;
// char file[64];
// char response[MAXCOMMAND];
// FILE *rfile;
// int ierr, n;
//
// sprintf(file,"%s%3.3d.fin",respfilecheck,iresp);
//// fprintf(stderr,"trying to open %s\n",file);
// rfile = fopen(file,"r");
// if (rfile != NULL) {
//   fclose(rfile);
//fprintf(stderr,"opened and closed: %s\n",file);
//   rfile = fopen(respfile,"r");
//   iresp++;
//   n = 0;
//   memset(response,0,sizeof(response));
//
//   while ((ierr=fscanf(rfile,"%1c",response+n)) >0 ) {
//            n++;
//   }
//fprintf(stderr,"read: %s\n",response);
//   if (n>0) {
//     errno = 0;
//     if (remove(file) != 0) {
//       perror("remove respfilecheck:");
//     }
//     errno = 0;
//     if (remove(respfile) != 0) {
//       perror("remove respfile:");
//     } else
//       fprintf(stderr,"removed respfile %s\n",respfile);
//     return(response);
//   }
// }
// return(NULL);
//}



char *getresp(FILE *rfile)
{
   int ierr, n;
   static char response[MAXCOMMAND];

   if (rfile !=NULL) {
/* Has the telescope sent a message? If so, print it out */
     n = 0;
     memset(response,0,sizeof(response));

     while (((ierr=fscanf(rfile,"%1c",response+n)) >0 )
           && strstr(response,"\n")==NULL) {
            if (*(response+n) != 0) n++;
     }
if (n >= MAXCOMMAND-1) fprintf(stderr,"***** n: %d\n",n);
     if (n>0) {
       return(response);
     }
     else
       return(NULL);
   } else
     return(NULL);
}
