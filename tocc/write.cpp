#include <stdio.h>
#include <dos.h>
main()
{
  FILE *fp;
  int i;

  fprintf(stderr,"starting...\n");
    for (i=0; i<500; i++) {
  fp = fopen("e:\\tmp\\test.doc","a");
  if (fp==NULL) {
    fprintf(stderr,"error opening file\n");
  } else {
      fprintf(fp,"%d\n",i);
	  fflush(fp);
	  fclose(fp);
//      sleep(1);
    }
  }
}
