#include <stdio.h>
#include "filter.h"
main()
{
  int i;
  char fname[16];
  initfilt();
  for (i=1 ; i<= MAXFILT ; i++) {
    fprintf(stderr," i: %d\n",i);
    fprintf(stderr," filtname: %s\n",filtname[i]);
    fprintf(stderr," longnmame: %s\n",longname[i]);
    fprintf(stderr," focoff: %d\n",focoff[i]);
    fprintf(stderr," zero: %f\n",zero[i]);
    fprintf(stderr," mag: %d\n",mag[i]);
  }

  fprintf(stderr,"Enter fname: ");
  scanf("%15s",fname);

  fprintf(stderr,"getfilt: %d\n",getfilt(fname));
}
