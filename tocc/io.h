#ifndef _IO_H
  #define _IO_H

#include "mytype.h"

/* prototypes for i/o functions */

extern void writeline(char *, int);
extern void writelog(char *, int);
extern int getline(unsigned char *, int);
extern int getcoord(char *, double *);

#ifdef SOCKET
#ifdef __cplusplus
        extern "C" {
#endif
int setup_server(int *, int);
int read_socket(int, char *, int *);
int read_open(int, char *);
int setup_client(int *sock, int port, char *server);
int write_client(int *, char *,int);
#ifdef __cplusplus
        }
#endif
extern int command_serv, command_sock, outsock;
#endif

extern void update_all();
extern void update_display();
extern void write_telescope_position(int);
extern void update_status(int);
extern void process_command();
extern void command_help();

/* general purpose outbuf buffer */
extern char outbuf[8000], outbuf2[8000];
/* should we update the fixed screen display in command mode? */
extern BOOL do_display;
/* Flag for whether we are in command mode or not */
extern BOOL command_mode;
/* Flag for whether we are in remote mode or not */
extern BOOL remote_on;
extern BOOL remote_command;

extern BOOL ropen;
extern BOOL statopen;
extern BOOL copen;
extern FILE *rfile;
extern FILE *sfile;
extern FILE *cfile;

extern char *ioserver;
extern int nudp;
extern char udp_command[80];

#ifdef no_hardware
 #define SERIALPORT COM2
 #define TOCC "c:"
#else
 #define SERIALPORT COM1
#ifdef SOCKET
 #define TOCC "c:"
#else
 #define TOCC "e:"
#endif
#endif

extern char *restart;

#define MAXCMD 132


#endif
