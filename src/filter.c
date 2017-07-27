#include <stdio.h>
#include "filter.h"

#define MAXCMD 132

// Note these arrays have an extra element to allow values to start at
//  index 1 to match filter convention

double zero[MAXFILT+1] = {0.,22.000,22.300,22.500,22.600,21.900,22.000,0.,0.,0.,0.,};
int mag[MAXFILT+1] = {0,1,2,3,4,5,3,3,3,3,3};
char filtname[MAXFILT+1][6]  = {"dummy","u","b","v","R","I","HA"," "," "," "," "};
char longfiltname[MAXFILT+1][30]  = {"dummy","u","b","v","R","I","HA"," "," "," "," "};
int focoff[MAXFILT+1] = {0,0,0,0,0,0,0,0,0,0,0};
double fudge = 1.;

int initfilt()
{
  FILE *fp;
  int iret,ifilt,i;
  char input[MAXCMD];
  char name[24];
  int j, j1, j2, j3, j4, j5, j6;

  fp = fopen("/home/tcomm/filters.dat","r");
  if (fp==NULL) {
    fprintf(stderr,"error opening filters file\n");
    return(-1);
  }
  for (i=1 ; i <= MAXFILT ; i++) {
    if (fgets(input,sizeof(input),fp) != NULL) {

#ifdef DEBUG
	fprintf(stderr,"input: %s\n",input);
#endif
  // Find first tab
      j1=j2=j3=j4=j5=j6=0;
      while (input[j1++] != '\t' && j1<strlen(input) ) {}
      if (j1==strlen(input)) {
        fprintf(stderr,"format error\n");
        return(-1);
      }
      while (input[j1+j2++] != '\t' && (j1+j2)<strlen(input)) {}
      if (j1+j2==strlen(input)) {
        fprintf(stderr,"format error\n");
        return(-1);
      }
      while (input[j1+j2+j3++] != '\t' && (j1+j2+j3)<strlen(input)) {}
      if (j1+j2+j3==strlen(input)) {
        fprintf(stderr,"format error\n");
        return(-1);
      }
      while (input[j1+j2+j3+j4++] != '\t' && (j1+j2+j3+j4)<strlen(input)) {}
      if (j1+j2+j3+j4==strlen(input)) {
        fprintf(stderr,"format error\n");
        return(-1);
      }
      while (input[j1+j2+j3+j4+j5++] != '\t' && (j1+j2+j3+j4+j5)<strlen(input)) {}
      if (j1+j2+j3+j4+j5==strlen(input)) {
        fprintf(stderr,"format error\n");
        return(-1);
      }
      j6 = strlen(input) - (j1+j2+j3+j4+j5);
#ifdef DEBUG
fprintf(stderr,"j1: %d j2: %d j3: %d j4: %d j5: %d j6: %d\n",j1,j2,j3,j4,j5,j6);
#endif

      strncpy(name,input,j1-1);
      name[j1-1] = 0;
      sscanf(name,"%d",&ifilt);
      if (ifilt != i) {
        fprintf(stderr,"ifilt ne i!! %d %d\n",ifilt, i);
        return(-1);
      }

      strncpy(name,input+j1,j2-1);
      name[j2-1] = 0;
      sscanf(name,"%6s",filtname[i]);

      strncpy(name,input+j1+j2,j3-1);
      name[j3-1] = 0;
      sscanf(name,"%30c",longfiltname[i]);
      j=30;
      while (longfiltname[i][j] == ' ') longfiltname[i][j--] = 0;

      strncpy(name,input+j1+j2+j3,j4-1);
      name[j4-1] = 0;
      sscanf(name,"%d",&focoff[i]);
     
      strncpy(name,input+j1+j2+j3+j4,j5-1);
      name[j5-1] = 0;
      sscanf(name,"%lf",&zero[i]);
     
      strncpy(name,input+j1+j2+j3+j4+j5,j6-1);
      name[j6-1] = 0;
      sscanf(name,"%d",&mag[i]);

    } else {
      fprintf(stderr,"Not enough lines in filters.dat!!\n");
      return(-1);
    }
  }
  return(0);
}

int getfilt(char *fname)
{
  int i, j;
  char a;
  for (i=1 ; i<=MAXFILT ; i++) {
    for  (j=0; j<strlen(fname) ; j++) {
      a = toupper(fname[j]);
      fname[j] = a;
    }
    if (strcmp(fname,filtname[i]) == 0) return(i); 
  }

  fprintf(stderr,"No such filter!\n");
  return(-1);
}

