#undef DEBUG
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "power.h"

#ifdef LAMPS
char *mscommand="lamps";
char *lockfile ="/export/images/powerlock";
char *statusfile ="/export/images/powerstatus";
#else
char *mscommand="ms";
char *lockfile ="/home/export/power/lock";
char *statusfile ="/home/export/power/status";
#endif
void power_on(int plug)
{
  char com[80];

  if (plug == 0) 
    sprintf(com,"%s on",mscommand);
  else if (plug > 0 && plug <= NPLUG) 
    sprintf(com,"%s on %d",mscommand,plug);
  else {
    fprintf(stderr,"ERROR: invalid plug specified\n");
    return;
  }
  #ifdef DEBUG
  fprintf(stderr,"%s\n",com);
  #endif
  system(com);
}

void power_off(int plug)
{
  char com[80];
  int i;

  if (plug == 0) {
    for (i=1 ; i<=NPLUG-1 ; i++) {
      sprintf(com,"%s off %d",mscommand,i);
      system(com);
    }
   // sprintf(com,"%s off",mscommand);
  } else if (plug> 0 && plug <= NPLUG) 
    sprintf(com,"%s off %d",mscommand,plug);
  else {
    fprintf(stderr,"ERROR: invalid plug specified\n");
    return;
  }
  #ifdef DEBUG
  fprintf(stderr,"%s\n",com);
  #endif
  system(com);
}

void power_change(int plug)
{
  char com[80];
  int plug_status[NPLUG+1];

  if (plug > 0 && plug <= NPLUG) {
    while (power_status(plug_status) < 0) { 
      sleep(1);
    }
    if (plug_status[plug] == OFF) 
      sprintf(com,"%s on %d",mscommand,plug);
    else
      sprintf(com,"%s off %d",mscommand,plug);
  } else {
    fprintf(stderr,"ERROR: invalid plug specified\n");
    return;
  }
  #ifdef DEBUG
  fprintf(stderr,"%s\n",com);
  #endif
  system(com);
}

int power_status(int *plug_status)
{
  char com[80];
  FILE *fp, *lock;
  int i;

#ifdef nohardware
  for (i=1; i<= NPLUG; i++)
    plug_status[i] = OFF;
  return(0);
#endif

  while ( (lock=fopen(lockfile,"r")) != NULL) {
    fclose(lock);
    sleep(1);
  }
  lock = fopen(lockfile,"w");

  remove(statusfile);
  if (NPLUG > 6) {
    sprintf(com,"%s list 1 >%s",mscommand,statusfile);
    #ifdef DEBUG
    fprintf(stderr,"%s\n",com);
    #endif
    system(com);
    for (i=2 ; i<=NPLUG; i++) {
      sprintf(com,"%s list %d >>%s",mscommand,i,statusfile);
      #ifdef DEBUG
      fprintf(stderr,"%s\n",com);
      #endif
      system(com);
    }
  } else {
    sprintf(com,"%s list >%s",mscommand,statusfile);
    #ifdef DEBUG
    fprintf(stderr,"%s\n",com);
    #endif
    system(com);
  }

  fp = fopen(statusfile,"r");
  if (fp == NULL) return(-1);
  for (i=1 ; i<=NPLUG ; i ++) {
    if (fgets(com, 79, fp) != NULL) {
      if (strstr(com,"Off") != NULL) 
        plug_status[i] = OFF;
      else if (strstr(com,"On") != NULL)
        plug_status[i] = ON;
      else
        plug_status[i] = UNKNOWN;
    #ifdef DEBUG
    fprintf(stderr,"%d %d\n",i,plug_status[i]);
    #endif
    } else {
      fclose(fp);
      fclose(lock);
      remove(statusfile);
      remove(lockfile);
      return(-1);
    }
  }

  fclose(fp);
  fclose(lock);
  remove(statusfile);
  remove(lockfile);
  return(0); 
}

