#include <stdio.h>
#include <string.h>
#include <time.h>
#include "io.h"

char outbuf[2000];

void writeline(char *outbuf, int icode)
{
  char timestring[16];
  time_t ti;
  struct tm *t;

  time(&ti);
  t = localtime(&ti);

  if (strchr(outbuf,'\n') == NULL)
    sprintf(timestring,"%d:%d:%d.%d    ",t->tm_hour,t->tm_min,t->tm_sec,0);
  else
    sprintf(timestring,"%d:%d:%d.%d\n",t->tm_hour,t->tm_min,t->tm_sec,0);

/* Normal output */
  if (icode==0) {
    fprintf(stderr,"%s\n",outbuf);
  }

/* Error output */
  if (icode==-1) 
      fprintf(stderr,"%s\n\r",outbuf);

/* Debugging output */
  if (icode==1) {
      fprintf(stderr,"%s%s\n",timestring,outbuf);
  }

}

/* get a newline or CR terminated line */
int mygetline(char *s, int lim)
{

  int c, i, j;
  
  i = 0;
  while (--lim >= 0 && (c=getc(infile[idepth])) != EOF && c!= '\n') {
      s[i++] = c;
      if (idepth>0) fprintf(stderr,"%c",c);
  }
  s[i] = '\0';
  if (havepipe[idepth] && i==0) return(-1);
  if (i==0 && c == EOF) {
    fprintf(stderr,"\n");
    return(-1);
  }
  if (idepth>0 && c== '\n') fprintf(stderr,"%c",c);
  if (lim < 0) fprintf(stderr,"WARNING: input string too long!\n");
  return(i);

}


/* get a number sexageimal triplet and convert to decimal */
int getcoord(char *string, double *coord, int prompt)
{
  char buf[80],c[2];
  double h,m,s;
  int sign;
  if (prompt) {
  /* Enter prompt */
    fprintf(stderr,"%s:",string);
  
  /* Read input string */
    if (mygetline(buf,sizeof(buf))<=0) {
      return(-1);
    }
  } else {
    strcpy(buf,string);
  }

/* Check for more than 3 fields */
  if (sscanf(buf,"%*d%*d%*lf%1s",c)==1) {
    return(-1);
  }

  h = m = s = 0;

/* Read 3 possible formats */
  if (sscanf(buf," -%lf%lf%lf",&h,&m,&s)>0) {
    sign = -1;
  } else if ( sscanf(buf," +%lf%lf%lf",&h,&m,&s)>0 ||
    sscanf(buf,"  %lf%lf%lf",&h,&m,&s)>0 ) {
    sign = 1;
  } else {
    return(-1);
  }

  *coord = sign * (h + m/60. + s/3600.);
  return(0);

}
int getargs(char *string, char *sargv, int n, int nmax)
{
  int i;
  for (i=0;i<nmax;i++) 
    *(sargv+i*n) = '\0';
  return(sscanf(string,"%s%s%s%s%s%s%s%s",
       sargv,sargv+n,sargv+2*n,sargv+3*n,sargv+4*n,sargv+5*n,sargv+6*n,sargv+7*n));
}


void strupr(unsigned char *str)
{
  int i, ichar;

  i = 0;
  while (str[i] != '\0') {
    str[i] = toupper(str[i]);
    i++;
  }
}

void gethms(double in,int *h, int *m, double *s,int *sign)
{
  double tmpin, tmp;

  if (in < 0) {
    *sign = -1;
    tmpin = -1 * in;
  }
  else {
    *sign = 1;
    tmpin = in;
  }

  *h = (int) tmpin;
  tmp = (tmpin-*h) * 60.;
  *m = (int) tmp;
  *s = (tmp-*m) * 60.;

  if (*s>59.95) {
    *m = *m + 1;
    *s = 0.;
    if (*m>=60) {
      *h = *h + 1;
      *m -= 60;
    }
    if (*h>23) *h-=24;
  }
}


void getdms(double in,int *h, int *m, double *s,int *sign)
{
  double tmpin, tmp;

  if (in < 0) {
    *sign = -1;
    tmpin = -1 * in;
  }
  else {
    *sign = 1;
    tmpin = in;
  }

  *h = (int) tmpin;
  tmp = (tmpin-*h) * 60.;
  *m = (int) tmp;
  *s = (tmp-*m) * 60.;

  if (*s>59.95) {
    *m = *m + 1;
    *s = 0.;
    if (*m>=60) {
      *h = *h + 1;
      *m -= 60;
    }
    if (*h>360) *h-=360;
  }
}


