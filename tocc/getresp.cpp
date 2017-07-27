#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#define MAXCOMMAND 3000
char *getresp(FILE *rfile)
{
   int ierr, n, nn;
   char response[MAXCOMMAND], junk;

//   system("type e:\\tocc\\tocccmd.doc");
   if (rfile !=NULL) {
/* Has the telescope sent a message? If so, print it out */
     n = 0;
     memset(response,0,sizeof(response));

cprintf("len: %d response: %s\n",strlen(response),response);
     while ((ierr=fscanf(rfile,"%1c",response+n)) >0
//           && (nn=strlen(strstr(response,"\n")))==0) {
           ) {
            n++;
 cprintf("ierr: %d  n:%d nn: %d response: %s\r\n",ierr,n,nn,response);
    }
 cprintf("ierr: %d  n:%d nn: %d\r\n",ierr,n,nn);
// cprintf("hit key to continue:\n\r");
// scanf("%c",&junk);
     if (n>0) {
       return(response);
     }
     else
       return(NULL);
   }
// cprintf("rfile null!\r\n");
// cprintf("hit key to continue:\n\r");
// scanf("%c",&junk);
   return(NULL);
}

