#include <stdio.h>
#undef NEW

int writehtml(int, char*, char*);
#ifdef MAIN
main()
{
  writehtml(0,"050910","050910");
}
#endif

writehtml(int tty, char *dir,char *filename)
{
#define MAXLINE 300

  char buf[32],line[MAXLINE],comment[MAXLINE],file[64],command[300],name[80];
  char line2[MAXLINE];
  static FILE *log = NULL, *fc;
  FILE *fp;
  int i,iline,iline2,s;

  if (tty != 1) {
#ifdef NEW
    sprintf(name,"/home/tcomm/%s.html",filename);
#else
#ifdef OFFSET
    sprintf(name,"public_html/%s.html",filename);
#else
    sprintf(name,"/home/tcomm/public_html/%s.html",filename);
#endif
#endif
    fp = fopen(name,"w");
    if (fp == NULL) {
      fprintf(stderr,"error opening /home/tcomm/public_html/%s.html\n",filename);
      return;
    }
  } else
    fp = stdout;

/* Fixed header */
#ifdef APOGEE
  fprintf(fp,"<html>\n"
    "<H3><center>Observing log - APO 1m telescope</center></h3>\n"
    "<H4>Date: %s<br>\n"
    "Instrument: Apogee Instruments 512x512, 24 micron pixels<br>\n"
"<form method=post action=\"http://control1m.apo.nmsu.edu/cgi-bin/comment.pl\">\n"
"<input type=\"hidden\" name=\"%s.root\" value=\"%s\">\n"
"<input type=\"hidden\" name=\"%s.dir\" value=\"%s\">\n",
    filename,filename,filename,filename,dir);
#else
  fprintf(fp,"<html>\n"
    "<H3><center>Observing log - APO 1m telescope</center></h3>\n"
    "<H4>Date: %s<br>\n"
    "Instrument: E2V 2048x2048, 13.5 micron pixels<br>\n"
"<form method=post action=\"http://control1m.apo.nmsu.edu/cgi-bin/comment.pl\">\n"
"<input type=\"hidden\" name=\"%s.root\" value=\"%s\">\n"
"<input type=\"hidden\" name=\"%s.dir\" value=\"%s\">\n",
    filename,filename,filename,filename,dir);
#endif
/* Weather information or form query */
    sprintf(file,"/home/tcomm/images/%s/%s.weather.comment",filename,filename);
    fc = fopen(file,"r");
    if (fc == NULL) {
      sprintf(file,"%s.weather.comment",filename);
      fprintf(fp,
"Weather: <input type=\"text\" name=\"%s\" size=60 maxsize=80>"
"<br></td></tr>\n",file);
    } else {
      sprintf(file,"%s.weather.reset",filename);
      fgets(comment,MAXLINE-1,fc);
      fprintf(fp,
"Weather: %s <input name=\"%s\" type=\"checkbox\"><br></td></tr>\n",
       comment,file);
      fclose(fc);
    }

/* Program information or form query */
    sprintf(file,"/home/tcomm/images/%s/%s.program.comment",filename,filename);
    fc = fopen(file,"r");
    if (fc == NULL) {
      sprintf(file,"%s.program.comment",filename);
      fprintf(fp,
"Program: <input type=\"text\" name=\"%s\" size=60 maxsize=80>"
"<br></td></tr>\n",file);
    } else {
      sprintf(file,"%s.program.reset",filename);
      fgets(comment,MAXLINE-1,fc);
      fprintf(fp,
"Program: %s <input name=\"%s\" type=\"checkbox\"><br></td></tr>\n",
       comment,file);
      fclose(fc);
    }

/* Observer information or form query */
    sprintf(file,"/home/tcomm/images/%s/%s.observer.comment",filename,filename);
    fc = fopen(file,"r");
    if (fc == NULL) {
      sprintf(file,"%s.observer.comment",filename);
      fprintf(fp,
"Observer(s): <input type=\"text\" name=\"%s\" size=60 maxsize=80>"
"<br></td></tr>\n",file);
    } else {
      sprintf(file,"%s.observer.reset",filename);
      fgets(comment,MAXLINE-1,fc);
      fprintf(fp,
"Observer(s): %s <input name=\"%s\" type=\"checkbox\"><br></td></tr>\n",
       comment,file);
      fclose(fc);
    }

/* Start observing log table */
  fprintf(fp,"</H4>\n <table border cellspacing=0 cellpadding=5>\n");
  fprintf(fp,"<TR><th>#</th><th>Object</th>"
             "<th>UT</th><th>RA</th><th>DEC</th><th>Epoch</th>"
             "<th>PA</th><th>HA</th><th>X</th><th>Exp</th><th>Filt</th>"
             "<th>Focus</th><th>Comment</th><th>Reset</th></tr>\n");

#ifdef NEW
/* Sort the log file by observation number, so we will be able to
   remove duplicate entries if observer reset the file number */
   sprintf(command,"sort -n /home/tcomm/images/%s/%s.log >log.txt",dir,filename);
   system(command);

/* For each line in log.txt file, fill in the HTML table. If comment file
   from frame exists, print it, otherwise put in a form for it */
  log = fopen("log.txt","r");
  if (log != NULL) {
    if ( fgets(line,MAXLINE-1,log) != NULL) {
      sscanf(line,"%d&",&iline);
      iline2=0;
      do {
        while ( iline2<=iline && (s=fgets(line2,MAXLINE-1,log)) != NULL ) {
          sscanf(line2,"%d&",&iline2);
        }
        while ( sameobj(line,line2) && (s=fgets(line2,MAXLINE-1,log))!=NULL) {
          sscanf(line2,"%d&",&iline2);
        }

        fprintf(fp,"<TR><TD>");
        for (i=0 ; i < strlen(line); i++) {
          if (line[i] == '&') {
            fprintf(fp,"</td><td>",line[i]);
          } else {
            fprintf(fp,"%c",line[i]);
          }
        }
        sprintf(file,"/home/tcomm/images/%s/%s.%d.comment",filename,filename,iline);
        fc = fopen(file,"r");
        if (fc == NULL) {
          sprintf(file,"%s.%d.comment",filename,iline);
          fprintf(fp,
"<input type=\"text\" name=\"%s\" size=30 maxsize=80></td></tr>\n",
            file);
        } else {
          sprintf(file,"%s.%d.reset",filename,iline);
          fgets(comment,MAXLINE-1,fc);
          fprintf(fp,"%s </td><td><input name=\"%s\" type=\"checkbox\"></td></tr>\n",
                comment,file);
          fclose(fc);
        }
        sprintf(line,"%s",line2);
        iline = iline2;
      } while (s != NULL);

    }
  }

#else
  sprintf(file,"/home/tcomm/images/%s/%s.log",dir,filename);
  log = fopen(file,"r");
  while (log != NULL && fgets(line,MAXLINE-1,log) != NULL) {
   fprintf(log,"line: %s\n",line); 
    sscanf(line,"%d&",&iline);

    fprintf(fp,"<TR><TD>");
    for (i=0 ; i < strlen(line); i++) {
      if (line[i] == '&') {
        fprintf(fp,"</td><td>",line[i]);
      } else {
        fprintf(fp,"%c",line[i]);
      }
    }
    sprintf(file,"/home/tcomm/images/%s/%s.%d.comment",filename,filename,iline);
    fc = fopen(file,"r");
    if (fc == NULL) {
      sprintf(file,"%s.%d.comment",filename,iline);
      fprintf(fp,
"<input type=\"text\" name=\"%s\" size=30 maxsize=80></td></tr>\n",
        file);
    } else {
      sprintf(file,"%s.%d.reset",filename,iline);
      fgets(comment,MAXLINE-1,fc);
      fprintf(fp,"%s </td><td><input name=\"%s\" type=\"checkbox\"></td></tr>\n",
              comment,file);
      fclose(fc);
    }
  }
#endif

  if (log != NULL) fclose(log);

  fprintf(fp ,"</table>\n");
  fprintf(fp,"<CENTER> <input type=\"submit\" value=\"Update comments\"> </CENTER>\n");

  fprintf(fp,"</form> </body> </html> \n");

  if (tty != 1) fclose(fp);
  return;
}

writelogpage()
{
  FILE *fp, *fp2;
  char file[80], root[24], name[80];
  int f,n;

  fp = fopen("public_html/logs.html","w");
  if (fp == NULL) {
    fprintf(stderr,"error opening public_html/logs.html\n");
    return;
  }

  fprintf(fp,"<HTML>\n"
    "<H3><center>Observing logs - APO 1m telescope</center></h3>\n"
    "<p>\nIf current log doesn't appear, wait for CCD program to start "
    "and then hit reload\n<p>\n");

  remove("logs.txt");
/*  system("ls -t public_html/0?????.html public_html/9?????.html > logs.txt");*/
  system("ls -t public_html/[01]?????sum.html > logs.txt");
  fp2 = fopen("logs.txt","r");
  n = 0;
  while ((f=(int)fgets(file,80-1,fp2)) != 0 && n++<100)  {
    strncpy(root,file+12,6);
    root[6] = 0;
    strncpy(name,file+12,14);
    name[14] = 0;
    fprintf(fp,"<a href=%s> %s </a><br>\n",name,root);
  }
  fclose(fp2);

  fprintf(fp,"</HTML>\n");
  fclose(fp);
}
int sameobj(char *line1,char *line2)
{
  int i, n;
  double epoch, pa, airmass, exptime;
  int obs, foc;
  char obj[24], ra[24], dec[24], ut[24], ha[24], filt[24];
  double bepoch, bpa, bairmass, bexptime;
  int bobs, bfoc;
  char bobj[24], bra[24], bdec[24], but[24], bha[24], bfilt[24];

  for (i=0; i< strlen(line1); i++) if (line1[i] == ' ') line1[i] = '0';
  for (i=0; i< strlen(line2); i++) if (line2[i] == ' ') line2[i] = '0';
  for (i=0; i< strlen(line1); i++) if (line1[i] == '&') line1[i] = ' ';
  for (i=0; i< strlen(line2); i++) if (line2[i] == '&') line2[i] = ' ';

fprintf(stderr,"1: %s\n",line1);
fprintf(stderr,"2: %s\n",line2);
  n=sscanf(line1,"%d%s%s%s%s%lf%lf%s%lf%lf%s%d(",
    &obs,obj,ut,ra,dec,&epoch,&pa,ha,&airmass,&exptime,filt,&foc);
fprintf(stderr,"1: %d\n",n);
  n=sscanf(line2,"%d%s%s%s%s%lf%lf%s%lf%lf%s%d(",
    &bobs,bobj,but,bra,bdec,&bepoch,&bpa,bha,&bairmass,&bexptime,bfilt,&bfoc);
fprintf(stderr,"2: %d\n",n);

fprintf(stderr,"%s %s\n %f %f\n %s %s\n %s %s\n %s %s\n %f %f\n %f %f\n", filt, bfilt, exptime, bexptime, obj, bobj, ra, bra, dec, bdec, pa, bpa, epoch, bepoch);
  for (i=0; i< strlen(line1); i++) if (line1[i] == ' ') line1[i] = '&';
  for (i=0; i< strlen(line2); i++) if (line2[i] == ' ') line2[i] = '&';

  if (strcmp(filt,bfilt)==0 && exptime==bexptime && strcmp(obj,bobj)==0 && pa==bpa && epoch==bepoch)
    return(1);
  else
    return(0);
}
