#include <stdio.h>
#include <string.h>

main(argc, argv)
int argc;
char *argv[];
{
  FILE *fp, *fout;
  char line[201];
  char *save[999], command[80];
  char root[24],name[24],dir[24],file[80];
  int i, nlines;

  for (i=0;i<1000;i++) save[i] = (char *)malloc(200);

  strcpy(root,argv[1]);
  sprintf(dir,"images/%s",root);

  strcpy(file,"/home/tcomm/");
  strcat(file,dir);
  strcat(file,"/");
  strcat(file,root);
  strcat(file,".log");
  sprintf(command,"mv %s tmp.log",file);
  system(command);
  fp = fopen("tmp.log","r");

  fout = fopen(file,"w");

  nlines=0;
  while (fgets(line,200,fp) != NULL) {
    sscanf(line,"%d%&",&i);
    strcpy(save[i],line);
    nlines= i>nlines ? i : nlines;
  }

  for (i=1;i<=nlines;i++)
    fprintf(fout,"%s",save[i]);

  printf("dir: %s\n root:%s\n",dir,root);
  writehtml(0,dir,root);
  
}
