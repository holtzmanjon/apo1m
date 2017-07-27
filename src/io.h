#ifndef _IO_H
#define _IO_H

#include "mytype.h"

#define MAXCMD 1000

/* prototypes for i/o functions */

extern void writeline(char *, int);
extern int mygetline(char *, int);
extern int getcoord(char *, double *, int);

extern void update_all();
extern void update_display();
extern void process_command();
extern void command_help();

/* general purpose outbuf buffer */
extern char outbuf[2000];
/* should we update the fixed screen display in command mode? */
extern BOOL do_display;
/* Flag for whether we are in command mode or not */
extern BOOL command_mode;

#define MAXDEPTH 5
extern FILE *infile[MAXDEPTH], *tomasterfp, *frommasterfp;
extern FILE *outfile[MAXDEPTH], *tomasterfp, *frommasterfp;
extern int idepth;
extern int havepipe[MAXDEPTH];
#endif
