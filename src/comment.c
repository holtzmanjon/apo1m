
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ENTRIES 10000

typedef struct {
    char *name;
    char *val;
} entry;

char *makeword(char *line, char stop);
char *fmakeword(FILE *f, char stop, int *len);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);
FILE *fp;

time_t temp;
struct tm temp2;

main(int argc, char *argv[]) {
    entry entries[MAX_ENTRIES];
    register int x,m=0;
    int cl;
    char file[80], dir[24], root[24], comment[80];
    char *l;

    printf("Content-type: text/html%c%c",10,10);

    if(strcmp(getenv("REQUEST_METHOD"),"POST")) {
        printf("This script should be referenced with a METHOD of POST.\n");
        printf("If you don't understand this, see this ");
        printf("<A HREF=\"http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/fill-out-forms/overview.html\">forms overview</A>.%c",10);
        exit(1);
    }
    if(strcmp(getenv("CONTENT_TYPE"),"application/x-www-form-urlencoded")) {
        printf("This script can only be used to decode form results. \n");
        exit(1);
    }
    cl = atoi(getenv("CONTENT_LENGTH"));

    for(x=0;cl && (!feof(stdin));x++) {
        m=x;
        entries[x].val = fmakeword(stdin,'&',&cl);
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
        entries[x].name = makeword(entries[x].val,'=');
    }

/*    
    printf("<H1>Query Results</H1>");
    printf("You submitted the following name/value pairs:<p>%c",10);
    printf("<ul>%c",10); 
*/
    for(x=0; x <= m; x++) {
      if (strstr(entries[x].name,"root") != NULL) {
        sprintf(root,"%s",entries[x].val);
        sprintf(file,"/home/tmp/%s",root);
        mkdir(file,0777);
      }
    }

    for(x=0; x <= m; x++) {
/*        printf("<li> <code>%s = %s</code>%c",entries[x].name,
               entries[x].val,10);   */
      if (strlen(entries[x].val) > 0) {  
        sprintf(file,"/home/tmp/%s/%s",root,entries[x].name);
        fp = fopen(file,"w");
        fprintf(fp,"%s\n",entries[x].val); 
        fclose(fp);
        if (strstr(entries[x].name,"dir") != NULL) 
          sprintf(dir,"%s",entries[x].val);
        if (strstr(entries[x].name,"reset") != NULL && 
            strstr(entries[x].val,"on") != NULL)  {
          strcpy(file,entries[x].name);
          l = strstr(file,".reset");
          sprintf(l,".comment");
          sprintf(comment,"/home/tmp/%s/%s",root,file);
          remove(comment);
          
        }
      }
    }
    writehtml(0,dir,root);
    writehtml(1,dir,root);
/*    printf("Submission successful."); */
    time(&temp);
    temp2=*localtime(&temp);
/*
    fprintf(fp,"-------------------\n");
    fprintf(fp,"Sent at %d/%d %d:%d\n",temp2.tm_mon+1,temp2.tm_mday,
	    temp2.tm_hour,temp2.tm_min);
*/
/*    printf("</ul>%c",10); */
}
