#include <stdio.h>
#include <strings.h>
#include "lamps.h"
void power_on(int plug)
{
  char com[80];

  if (plug == 0) 
    sprintf(com,"lamps on");
  else if (plug > 0 && plug <= NPLUG) 
    sprintf(com,"lamps on %d",plug);
  else {
    fprintf(stderr,"ERROR: invalid plug specified\n");
    return;
  }
fprintf(stderr,"com: %s\n",com);
//  system(com);
}

void power_off(int plug)
{
  char com[80];

  if (plug == 0) 
    sprintf(com,"lamps off");
  else if (plug> 0 && plug <= NPLUG) 
    sprintf(com,"lamps off %d",plug);
  else {
    fprintf(stderr,"ERROR: invalid plug specified\n");
    return;
  }
fprintf(stderr,"com: %s\n",com);
//  system(com);
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
      sprintf(com,"lamps on %d",plug);
    else
      sprintf(com,"lamps off %d",plug);
  } else {
    fprintf(stderr,"ERROR: invalid plug specified\n");
    return;
  }
fprintf(stderr,"com: %s\n",com);
//  system(com);
}

int power_status(int *plug_status)
{
  char com[80];
  FILE *fp, *lock;
  int i;

  while ( (lock=fopen("/home/export/images/lock","r")) != NULL) {
    fclose(lock);
    sleep(1);
  }
  lock = fopen("/home/export/power/lock","w");

  remove("/home/export/power/status");
  sprintf(com,"lamps list >/home/export/images/status");
  system(com);

  fp = fopen("/home/export/images/status","r");
  if (fp == NULL) return(-1);
  for (i=1 ; i<=NPLUG ; i ++) {
    if (fgets(com, 79, fp) != NULL) {
      if (strstr(com,"Off") != NULL) 
        plug_status[i] = OFF;
      else if (strstr(com,"On") != NULL)
        plug_status[i] = ON;
      else
        plug_status[i] = UNKNOWN;
    } else {
      fclose(fp);
      fclose(lock);
      remove("/home/export/images/status");
      remove("/home/export/images/lock");
      return(-1);
    }
  }

  fclose(fp);
  fclose(lock);
  remove("/home/export/images/status");
  remove("/home/export/images/lock");
  return(0); 
}

