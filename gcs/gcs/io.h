#ifndef _IO_H
  #define _IO_H

#include "mytype.h"

/* prototypes for i/o functions */

extern void writeline(char *, int);
extern int getline(unsigned char *, int);
extern int getcoord(char *, double *);

extern void update_all();
extern void update_display();
extern void update_status();
extern void process_command();
extern void command_help();
extern void update_all();

/* general purpose outbuf buffer */
extern char outbuf[8000], outbuf2[8000];
/* should we update the fixed screen display in command mode? */
extern BOOL do_display;
/* Flag for whether we are in command mode or not */
extern BOOL command_mode;
/* Flag for whether we are in remote mode or not */
extern BOOL remote_on;
extern BOOL remote_command;

#ifdef no_hardware
#define SERIALPORT COM2
#endif
#ifndef no_hardware
#define SERIALPORT COM1
#endif

#endif
#define MAXCMD 132

extern BOOL ropen;
extern BOOL statopen;
extern BOOL copen;
extern BOOL flushopen;
extern FILE *rfile;
extern FILE *sfile;
extern FILE *cfile;
extern FILE *ffile;

extern char *restart;

