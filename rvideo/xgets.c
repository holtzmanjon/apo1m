/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C file is Copyrighted software.
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
/*  The routines in this file are designed to make it possible for
/*  command-driven programs to use the X11 Window System from MIT.
/*  More specifically, command-driven programs usually spend most of
/*  their time blocking on a read from the standard input.  They are
/*  waiting for the user to type in the carriage return for the next line.
/*  However, in order to use X11, the program must also be able to
/*  respond to asynchronous events coming from the connection to the
/*  X11 Server(s).  The user may be resizing, uncovering, or otherwise
/*  manipulating the X11 window(s) to which graphical output has been sent.
/*
/*  These routines use the bsd Unix select(2) call to watch for input on
/*  stdin and on any other file descriptors (presumably the file
/*  descriptors for the connections to the X11 Server(s) ).
/*  The basic routine is mx11gets, which is designed to look much like
/*  the Unix gets(2) function.  This is used to watch for the next
/*  line of user input on stdin while also watching for any input on
/*  other previously registered file descriptors.  The function mx11gets
/*  has been designed to allow it to be used from Fortran programs.
/*  Thus instead of doing a Fortran "read(5,'(a)') oneline" it is possible
/*  to do "call mx11gets(oneline)".
/*
/*  Registration of other file descriptors to be watched is done by making
/*  calls to the function mx11register.  This routine takes a file descriptor
/*  and a routine to be called when input is seen on that file descriptor.
/*  Henceforth, mx11gets will call the appropriate routine(s) when input
/*  appears on the given file descriptor(s).
/*
/*  Take notice that the action routines which are called to handle X11
/*  events should be careful only to grab events which they need.  Other
/*  X11 Windows may exist on the same X Server connection, and the
/*  event handler for one window should not consume another window's events.
*/

#include "Config.h"
#ifdef IBMAIX
#   define _ALL_SOURCE
#   define _BSD
#else  /* IBMAIX */
#   undef _POSIX_SOURCE
#endif /* IBMAIX */
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#ifdef __READLINE
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define fd_for_stdin    0       /* hopefully nobody mucks with this     */
#define MAXREGISTER     10      /* Maximum allowed # of registered fds  */

static  int     num_registered = 0;     /* # of registered fds and actions  */
static  int     fd[MAXREGISTER];        /* list of registered fds           */
typedef void    (*PFRV)();              /* ptr to func returning void       */
typedef char    *(*PFRPC)();            /* ptr to func returning ptr to char*/
static  PFRPC   action[MAXREGISTER];    /* registered actions to call       */

static  int     flusix = 6;     /* Fortran Logical Unit # 6 is stdout   */
static  int     sel_wid;

int ux11gets = 0;               /* set if mx11gets has been called */
extern int update;

#define TIMEOUT
char *UMGO(mx11gets)(string,length)
/* get input from the keyboard while handling X events on registered fds */
char    *string;
int     length;
{
    int                 numio, i;
    fd_set              readfds;
    char                *cp, *cp0;
    int                 slen;
#ifdef TIMEOUT
    struct timeval timeout;
#endif
#define NewStuff
#ifdef NewStuff
    struct rlimit       rl;
#endif /* NewStuff */

    /* inform others that mx11gets is being used */
    ux11gets = 1;

    /* Fortran needs a string full of SPACEs to be happy   */
    /*fprintf(stderr,"gonna fill %d chars with SPACEs\n\r",length);  */
    numio = 0;
    while (numio < length) {
	string[numio++] = ' ';
    }

    /*  Get ready for doing the select() call   */
#ifdef NewStuff
    if (!getrlimit(RLIMIT_NOFILE,&rl)) {
	sel_wid = rl.rlim_cur;
    } /* else { well, what would a good thing to do here be ?
    } */
#else /* NewStuff */
    sel_wid = getdtablesize();
#endif /* NewStuff */

    /* Handle X events forever, or until there's a line of input on stdin */
    while (1) {
	/*fprintf(stderr,"looking for event--fd_for_X = %d...\n\r",fd_for_X);*/
	FD_ZERO(&readfds);              /* clear all fds        */
//	FD_SET(fd_for_stdin,&readfds);  /* set to watch terminal*/
#ifdef TIMEOUT
        if (update==1) {
          timeout.tv_sec = 0;
          timeout.tv_usec = 500000;
        } else {
          timeout.tv_sec = 1;
          timeout.tv_usec = 0;
        }
#endif
	for (numio = 0; numio < num_registered; numio++) {
	    FD_SET(fd[numio],&readfds);         /* set to watch X server*/
	}

	/* wait for input from one of the selected file descriptors     */
	numio = -1;
	while (numio < 0) {
	    /* select returns a negative number if interrupted by a signal */
	    /*fprintf(stderr,"doing the select\n\r");       */
#ifdef TIMEOUT
            numio = select(sel_wid,&readfds,
            (fd_set *)NULL,(fd_set *)NULL,&timeout);
#else
	    numio = select(sel_wid,&readfds,
	    (fd_set *)NULL,(fd_set *)NULL,(struct timeval *)NULL);
#endif
	    /*fprintf(stderr,"did the select\n\r"); */
	}

	/* handle the input from all selected file descriptors  */

	/* handle any X events first    */
	for (numio = 0; numio < num_registered; numio++) {
	    /* we loop through all the registered input file descriptors */
	    if (FD_ISSET(fd[numio],&readfds)) {
		/* this one had input, so we call its action */
		cp = (*action[numio])();
		/* if text was returned by the action, we return that text */
		if (cp) {
		    strncpy(string,cp,length);
		    /* many FORTRANs cannot tolerate a trailing NULL */
		    slen = strlen(string);
		    if (slen < length) string[slen] = ' ';
		    return cp;
		}
	    }
	}

	/*fprintf(stderr,"done with X events, looking for line\n\r");   */
	/* if there is a line of input ready, return it */
	if (FD_ISSET(fd_for_stdin,&readfds)) {
	    /* return value indicates if input from stdin was EOF   */
	  /*  cp = gets(string); */
	    cp = fgets(string,length,stdin);
            for (i=0; i<strlen(string);i++) {
            if (string[i] == 0 || string[i] == 10 || string[i] == 13)
              string[i] = ' ';
            }
#ifdef __READLINE
            if (strcmp(string,"e ") ==0) {
              cp0 = readline("");
              cp = cp0;
              slen = strlen(cp);
              for (i=0; i<(length < slen ? length : slen);i++) {
                string[i] = *cp++;
              }
              if (cp0) free(cp0);
              string[slen] = 0;
            }
#endif

#ifdef      ALAS_NOGO
	    /* if the pointer is null, presume we were interrupted by   */
	    /* some kind of signal, and do not exit this routine        */
	    if (cp)
#endif      /* ALAS_NOGO */
	    {
		/* many FORTRANs cannot tolerate a trailing NULL */
		slen = strlen(string);
		if (slen < length) string[slen] = ' ';
		return cp;
	    }
	}
#ifdef TIMEOUT
        return;
#endif
	/*fprintf(stderr,"bottom of while loop\n\r");      */
    }
}
/************************************************************************/
/************************************************************************/
void UTUV(mx11register)(newfd,newaction)
int     *newfd;
PFRPC   newaction;
{
    if (num_registered < MAXREGISTER) {
	action[num_registered] = newaction;
	fd[num_registered] = *newfd;
	num_registered++;
    } else {
	fprintf(stderr,"mx11register: Too many things registered\n\r");
    }
}
/************************************************************************/
/************************************************************************/
void UTUV(mx11unregister)(newfd)
int     *newfd;
{
	int     i,j;

#ifdef VERBOSE
      fprintf(stderr,"Before unregistering:\n");
      for(i=0; i<=num_registered; i++)
		 fprintf(stderr,"fd[%d] = %d\n",i,fd[i]);
#endif
	for (i=0; i<=num_registered; i++) {
	  if(fd[i] == *newfd) {
	    for(j=i; j<num_registered; j++)
	      fd[j] = fd[j+1];
	      num_registered--;
#ifdef VERBOSE
      fprintf(stderr,"After unregistering:\n");
      for(i=0; i<=num_registered; i++)
		 fprintf(stderr,"fd[%d] = %d\n",i,fd[i]);
#endif
	    return;
		}
	}
	fprintf(stderr,"mx11unregister called for missing fd\n");
}
/************************************************************************/
/************************************************************************/
