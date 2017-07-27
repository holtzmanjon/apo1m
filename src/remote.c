#include <stdio.h>

extern FILE *rfile;
extern int ropen;
extern char comfile[80];
extern char comfilecheck[80];
extern char respfile[80];
extern char respfilecheck[80];
extern char restart[100];

char *getresp(FILE *);

#define MAXCOMMAND 8000

char *get_message()
{
  FILE *file;
  char message[MAXCOMMAND];
  int n;

#ifdef NEWCOM
  file = fopen(restart,"r");
  if (file != NULL) {
      fclose(file);
      if (ropen == 1) {
        fprintf(stderr,"closing rfile\n");
        fclose(rfile);
      }
      ropen = 0;
      remove(restart);
  }

  if (ropen == 0) {
      rfile = fopen(respfile,"r");
      if (rfile != NULL) ropen = 1;
  }
  return(getresp(rfile));

#else

    file = fopen(respfilecheck,"r");
    if (file != NULL) {
        fclose(file);
        usleep(100000);
        myremove(respfilecheck);
        file = fopen(respfile,"r");
        if (file == NULL) {
          perror("open error 1: ");
        } else {
          n = 0;
          while (fgets(message+n,MAXCOMMAND-1-n,file)!= NULL) {
            n = strlen(message);
          }
          fclose(file);
          fprintf(stderr,"%s", message);
          myremove(respfile);
        }
        return(message);
    }
    return(NULL);

#endif

}

void send_message(char *command)
{
   FILE *file;

#ifdef DEBUG
   fprintf(stderr,"writing command file: %s %s\n",comfile,command);
#endif

   /* Dont write a command file until the check file has been deleted */
   while ((file=fopen(comfilecheck,"r")) !=NULL) {
        fclose(file);
   }

   /* write out the command file */
   file = fopen(comfile,"w");
   while (file==NULL) {
     perror("open error 2: ");
     file = fopen(comfile,"w");
   }
#ifdef SPEC
   fprintf(file,"%s%c\n",command,13);
#else
   fprintf(file," %s%c",command,13);
#endif
   fclose(file);

#ifdef DEBUG
   fprintf(stderr,"writing command check file: %s %s\n",comfilecheck, command);
#endif
      /* write the command check file */
   file = fopen(comfilecheck,"w");
   while (file==NULL) {
     perror("open error 3: ");
     file = fopen(comfilecheck,"w");
   }

   fclose(file);

}
