#undef DEBUG

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "power.h"

/* Define the various windows */
Display *dpy;                           /* Display pointer */
Window wbase ;                          /* Base window */
Window wlgt[NPLUG+1];      /* Status light windows */

int popped = 0;
int pplug;
int pcolor[4];
char ptext[4][80];

int plug_status[NPLUG+1];
int new_status[NPLUG+1];

char *display;
char *geometry;

int screen;

Font font;
char *fontname1, *fontname2, **fontpath;
int *nfont;
XFontStruct *fontinfo[2];
int fontheight[2], fontwidth[2];

int pixels[256];                        /* color cell addresses allocated */
int planes;                             /* plane mask */
Colormap defcmap;
XColor stcolor[6];                      /* color status lights */

XSetWindowAttributes xswa;

GC emerggc, textgc, popgc;

Window inforoot;
int infox, infoy, infowidth, infoheight, infoborder, infodepth;


#define ERR_CANT_OPEN_DISPLAY                   1
#define ERR_CANT_CREATE_IMAGE_WINDOW            2
#define ERR_CANT_CREATE_ZOOM_WINDOW             3
#define ERR_INSUFF_VECTOR_COLORS                4
#define ERR_INSUFF_STATUS_COLORS                5
#define ERR_INSUFF_IMAGE_COLORS                 6
#define ERR_BAD_ALLOC_LOOKUP                    7
#define ERR_BAD_ALLOC_PALETTE                   8
#define ERR_BAD_ALLOC_IMBUF                     9
#define ERR_BAD_ALLOC_ZOOM                      10
#define ERR_NO_DATA                             11
#define ERR_BAD_DISPLAY_PIPE                    12
#define ERR_NOT_INITIALIZED                     13
#define ERR_CANT_GET_FONT                       14

#define DEFAULT_FONT1 "12x24"
#define DEFAULT_FONT2 "6x10"
//#define DEFAULT_FONT1 "-adobe-helvetica-bold-r-*-*-24-*-*-*-*-*-*-*"
//#define DEFAULT_FONT2 "-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*"
#define XYZBORDER 1

XErrorEvent _XErrorEvent;

int lgtwidth, lgtheight;
int width, height;
int pwidth, pheight;
int border_width = 2;

int whichkey;
XSizeHints sizehints;
XWMHints  wmhints;

main()

{
  char input[80];
  XColor  curswcolor, cursbcolor, backcolor;
  int i, xoff, yoff, numio, fw, init, ncolors;
  Pixmap csource, cmask;
  char *option;
  char text[500];
  char title[80];

  int fd_for_X, sel_wid;
  fd_set              readfds;
  struct timeval timeout;

  dpy = NULL;
  wbase = (Window)0;
  display = NULL;
  geometry = NULL;

  /* Open the connection to the server, quit on failure */
  if( ! (dpy = XOpenDisplay(display)) ) {
    fprintf(stderr,"Can't open display %s\n",XDisplayName(display));
    return(ERR_CANT_OPEN_DISPLAY);
  }

  screen = XDefaultScreen(dpy);
  defcmap = XDefaultColormap(dpy,screen);

  /* Window sizes */
  width = DisplayWidth(dpy,screen);
#ifdef LAMPS
  height = 0;
  lgtheight = 30;
  ncolors=4;
  sprintf(title,"DIS lamp control");
#else
  height = 30;
  lgtheight = 30;
  ncolors=6;
  sprintf(title,"1m telescope power");
#endif
  lgtwidth= width/NPLUG;


 /* Ask for ncolors colors from the server for status windows */
/*
  if (XAllocColorCells(dpy, defcmap, 0, &planes, 0, pixels, ncolors) == 0) {
    fprintf(stderr,"Insufficient colors for status\n");
    return(ERR_INSUFF_STATUS_COLORS);
  }
*/
  
 /* Set the color addresses in status to the addresses returned by the server */
  for(i=0;i<ncolors;i++) {
/*    stcolor[i].pixel = pixels[i]; */
    stcolor[i].flags = DoRed | DoGreen | DoBlue;
  }
  stcolor[0].red = 65535;
  stcolor[0].green = 0;
  stcolor[0].blue = 0;
  stcolor[1].red = 65535;
  stcolor[1].green = 65535;
  stcolor[1].blue = 65535;
  stcolor[2].red = 0;
  stcolor[2].green = 65535;
  stcolor[2].blue = 0;
  stcolor[3].red = 0;
  stcolor[3].green = 0;
  stcolor[3].blue = 0;
  if (ncolors>4) {
    stcolor[4].red = 65535;
    stcolor[4].green = 65535;
    stcolor[4].blue = 0;
    stcolor[5].red = 0;
    stcolor[5].green = 0;
    stcolor[5].blue = 0;
  }
  for (i=0;i<ncolors;i++)
    XAllocColor(dpy,defcmap,stcolor+i);

/*  XStoreColors(dpy, defcmap,stcolor,ncolors);  */
  
  xswa.background_pixel = BlackPixel(dpy,screen);
  xswa.border_pixel = WhitePixel(dpy,screen);
  xswa.backing_store = WhenMapped;

  xoff = yoff = 0;
  sizehints.flags = PPosition | PSize;
  sizehints.width = width;
  sizehints.height = height+lgtheight; 
  sizehints.flags |= (USSize | USPosition);
  sizehints.x = xoff;
  sizehints.y = xoff;

  wbase = XCreateWindow(dpy,RootWindow(dpy,screen),
		xoff,yoff,width,height+lgtheight,border_width,
		0,InputOutput,CopyFromParent,0,&xswa);
	/*	CWBackPixel | CWBorderPixel, &xswa); */
/* See XSetStandardProperties for defining a fancy icon */
  XSetIconName(dpy, wbase, "Ximage");

  if (!wbase) {
    fprintf(stderr, "XCreateWindow failed\n");
    return(ERR_CANT_CREATE_IMAGE_WINDOW);
  }

/* Map the window and see what size it is (if the user has resized it) */
  XSetStandardProperties(dpy,wbase,title,title,None,NULL,0,&sizehints);
  wmhints.input = True;
  wmhints.flags = InputHint;
  XSetWMHints(dpy, wbase, &wmhints);
  XMapWindow(dpy,wbase);

/* Create the image subwindow */
#ifndef LAMPS
  wlgt[0] = XCreateSimpleWindow(dpy,wbase,       /* Parent window */
       0,0,                             /* UL location (nominal) */
       width,height,                    /* size (nominal) */
       0,                               /* border width */
       WhitePixel(dpy,screen),          /* border pixmap */
       BlackPixel(dpy,screen));         /* background pixmap */
  XChangeWindowAttributes(dpy,wlgt[0],CWBackingStore,&xswa);
#endif

/* light subwindows */
  for (i=1 ; i<=NPLUG ; i++) {
    wlgt[i] = XCreateSimpleWindow(dpy,wbase,(i-1)*lgtwidth,
       height,lgtwidth,lgtheight,XYZBORDER,
       WhitePixel(dpy,screen),BlackPixel(dpy,screen));
    XChangeWindowAttributes(dpy,wlgt[i],CWBackingStore,&xswa);
  }

/* Select inputs from the window */
  /* Get the font to be used, and get the information about it */
  fontname2 = DEFAULT_FONT2;
  fontname1 = DEFAULT_FONT1;

  if (  (fontinfo[0] = XLoadQueryFont(dpy,fontname1)) != NULL) {
  } else {
    fprintf(stderr,"Can't open font %s\n",fontname1);
    return(ERR_CANT_GET_FONT);
  }
  fontwidth[0] = fontinfo[0]->max_bounds.rbearing - fontinfo[0]->min_bounds.lbearing;
  fontheight[0] = fontinfo[0]->max_bounds.ascent + fontinfo[0]->max_bounds.descent;
  emerggc = XCreateGC(dpy, wbase, 0, NULL);
  XSetState(dpy,emerggc,
    WhitePixel(dpy,screen),BlackPixel(dpy,screen),GXcopy,AllPlanes);
  XSetFont(dpy,emerggc,fontinfo[0]->fid);
  XSetFillStyle(dpy,emerggc,FillSolid);

  if (  (fontinfo[1] = XLoadQueryFont(dpy,fontname2)) != NULL) {
  } else {
    fprintf(stderr,"Can't open font %s\n",fontname1);
    return(ERR_CANT_GET_FONT);
  }
  fontwidth[1] = fontinfo[1]->max_bounds.rbearing - fontinfo[1]->min_bounds.lbearing;
  fontheight[1] = fontinfo[1]->max_bounds.ascent + fontinfo[1]->max_bounds.descent;
  textgc = XCreateGC(dpy, wbase, 0, NULL);
  XSetState(dpy,textgc,
    WhitePixel(dpy,screen),BlackPixel(dpy,screen),GXcopy,AllPlanes);
  XSetFont(dpy,textgc,fontinfo[1]->fid);
  XSetFillStyle(dpy,textgc,FillSolid);

  //XSelectInput(dpy,wbase, ExposureMask | LeaveWindowMask | ButtonPressMask | 
  //  ButtonReleaseMask | StructureNotifyMask | PointerMotionMask | 
  //   PointerMotionHintMask | KeyPressMask );
  XSelectInput(dpy,wbase, ExposureMask | ButtonPressMask | 
     StructureNotifyMask | KeyPressMask );
#ifndef LAMPS
  XSelectInput(dpy,wlgt[0], ExposureMask);
#endif

  /* Map the subwindows */
  XMapSubwindows(dpy,wbase);

  /* Flush this pile of X output */
  XFlush(dpy);

  sprintf(ptext[1],"Are you sure?");
  sprintf(ptext[2],"Yes");
  sprintf(ptext[3],"No");
  pcolor[1] = 3;
  pcolor[2] = 2;
  pcolor[3] = 0;

  for (i=1; i<=NPLUG; i++) 
    plug_status[i] = -10;

/* Ask for non-blocking IO on the input from X */
  fd_for_X = ConnectionNumber(dpy);

  while (1) {

    FD_ZERO(&readfds);              /* clear all fds        */
    FD_SET(fd_for_X,&readfds);  /* set to watch terminal*/
    sel_wid = fd_for_X + 1;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    numio = -1;
    //while (numio < 0) {
    /* select returns a negative number if interrupted by a signal */
#ifdef DEBUG
    fprintf(stderr,"calling select\n");
#endif
      numio = select(sel_wid,&readfds,
            (fd_set *)NULL,(fd_set *)NULL,&timeout);
#ifdef DEBUG
    fprintf(stderr,"back calling select\n");
#endif
    //}
  
    if (FD_ISSET(fd_for_X,&readfds)) {
#ifdef DEBUG
    fprintf(stderr,"calling refresh from fd_for_X\n");
#endif
      xtv_refresh();
#ifdef DEBUG
    fprintf(stderr,"back calling refresh\n");
#endif
    } else if (!popped) {
    // check the power status every minute and update
#ifdef DEBUG
    fprintf(stderr,"calling power status\n");
#endif
    while (power_status(new_status) < 0) {
      sleep(1);
    }
#ifdef DEBUG
    fprintf(stderr,"back calling power status\n");
#endif
    init = 0; 
    for (i=1; i<=NPLUG; i++) {
      if (new_status[i] != plug_status[i]) init = 1;
      plug_status[i] = new_status[i];
    }
    if (init == 1) textinit();
    }
  } 
}

imageclose()
{
  return(0);
}

imageerase()
{
#ifndef LAMPS
  XClearWindow(dpy,wlgt[0]);
#endif
  XFlush(dpy);
}

imagelight(plug,mess,newcolor)
int plug;
char *mess;                     /* String to be written (null terminated) */
int newcolor;                      /* flag = 0/1 for red/green */
{
  Window *w;
  static int x[NPLUG+1], y[NPLUG+1];
  static int xw[NPLUG+1], yw[NPLUG+1];
  static int sp[NPLUG+1],color[NPLUG+1];
  static char text[NPLUG+1][300];
  int i, is, ie, fw, j;
  GC gc;

  if (plug > 0) {
    sp[plug]=0;
    sprintf(text[plug],"%s",mess);
    if (strlen(text[plug]) > 8) {
      for (j=0;j<strlen(text[plug]);j++) if (text[plug][j] == ' ') sp[plug]=j;
    }
    color[plug] = newcolor;
    fw = XTextWidth(fontinfo[1],text[plug],strlen(text[plug]));
    fw = XTextWidth(fontinfo[1],text[plug],sp[plug]>0?(sp[plug]>strlen(text[plug])-sp[plug]?sp[plug]:strlen(text[plug])-sp[plug]):strlen(text[plug]));
    x[plug] = (lgtwidth-fw)/2;
    y[plug] = fontheight[1] + (lgtheight/2-fontheight[1])/2 - 1;
    xw[plug] = lgtwidth;
    yw[plug] = lgtheight;
  } else if (plug == 0) {
    sp[plug]=0;
    sprintf(text[plug],"%s",mess);
    color[plug] = newcolor;
    fw = XTextWidth(fontinfo[0],text[plug],strlen(text[plug]));
    x[plug] = (width-fw)/2;
    y[plug] = fontheight[0] + (height-fontheight[0])/2 - 1;
    xw[plug] = width;
    yw[plug] = height;
  }

  if (plug < 0) {
    is = 0;
    ie = NPLUG;
  } else {
    is = plug;
    ie = plug;
  }
  for (i=is; i <= ie; i++) {
    if (i==0)
      gc = emerggc;
    else
      gc = textgc;
    w = &wlgt[i];
    XSetForeground(dpy,gc,stcolor[2*color[i]].pixel);
    if (_XErrorEvent.serial!=0) 
     printf("loc 11: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
    XFillRectangle(dpy,*w,gc,0,0,xw[i],yw[i]);
    XSetForeground(dpy,gc,stcolor[2*color[i]+1].pixel);
    if (_XErrorEvent.serial!=0) 
     printf("loc 12: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
    if (sp[i]>0) {
      XDrawString(dpy,*w,gc,x[i],y[i],text[i],sp[i]);
      XDrawString(dpy,*w,gc,x[i],y[i]+fontheight[1],text[i]+sp[i],strlen(text[i])-sp[i]);
    } else
      XDrawString(dpy,*w,gc,x[i],y[i],text[i],strlen(text[i]));
  }

  XFlush(dpy);
}

/*
 * Interrupt handler for X Events
 */

xtv_refresh(signo)
int signo;
{
  Window wmouse, wroot;
  XEvent event;
  XExposeEvent *expw  = (XExposeEvent *)&event;
  XButtonEvent *but = (XButtonEvent *)&event;
  XKeyEvent *key = (XKeyEvent *)&event;
  XPointerMovedEvent *pmove = (XPointerMovedEvent *)&event;
  KeySym ks;
  int wmask;
#define KEYLEN 10
  char keystring[KEYLEN];
  XComposeStatus compose_status;
  int x=0, y=0, i, ix, iy, set, iii;
  static int ibut;
  unsigned char keycode;
  char keychar;
  char outbuf[32];
  float fcoord;
  void put_text();

  static Window pop_win[4] ;                     /* Zoom window */
  static int w0, h0, w, h, pw[4], ph[4], pw0[4], ph0[4];

  whichkey = -1;

#define ALL_X_EVENTS   (~0)

while (XCheckWindowEvent(dpy,wbase,ALL_X_EVENTS,&event)  
 || (popped ? XCheckWindowEvent(dpy,pop_win[0],ALL_X_EVENTS,&event) : 0)) {

#ifdef DEBUG
fprintf(stderr,"event: %d\n",(int)event.type);
fprintf(stderr,"%d%d%d\n",XEventsQueued(dpy,QueuedAlready),
                          XEventsQueued(dpy,QueuedAfterFlush),
                          XEventsQueued(dpy,QueuedAfterReading));
#endif
/* Switch on the type of event */
      switch((int)event.type) {

/*
 * WINDOW EXPOSURE EVENT
 */
      case Expose:
      case ConfigureNotify:
/* If the window was resized, update the subwindow sizes */
	XGetGeometry(dpy,wbase,&inforoot,&infox,&infoy,&infowidth,&infoheight,
		     &infoborder,&infodepth);
	if (infowidth != width || infoheight != height+lgtheight) {
          resizesubwin();
// fprintf(stderr,"infowidth, height: %d %d %d %d %d\n",infowidth, infoheight,width,lgtwidth,height);
//	  XClearWindow(dpy,wbase);
	}

/* Rewrite the required bit of image */
        imagelight(-1);
        if (popped) {
         for (i=1;i<=3;i++) {
           put_text(pop_win[i],pw[i],ph[i],pw0[i],ph0[i],pcolor[i],ptext[i]);
         }
        }
        XFlush(dpy);
       break;

/*
 * BUTTON PRESSED: A mouse button was pressed
 */
      case ButtonPress:
	whichkey = but->button;
	ibut = 0;
	if(whichkey == Button1) ibut = 1;
	if(whichkey == Button2) ibut = 2;
	if(whichkey == Button3) ibut = 3;

/* The button was pressed in the image window, zoom and center */
#ifndef LAMPS
	if(but->subwindow == wlgt[0]) {
          fprintf(stderr,"emergency power off!!");
          power_off(MOTORS);
          break;
	}

        if (but->subwindow == pop_win[2]) {
          //fprintf(stderr,"power cycle: %d\n",pplug); 
          power_change(pplug);
          popped = !popped;
          XDestroyWindow(dpy,pop_win[0]);
          break;
        } else if (but->subwindow == pop_win[3]) {
          //fprintf(stderr,"cancel power cycle: %d\n",pplug); 
          popped = !popped;
          XDestroyWindow(dpy,pop_win[0]);
          break;
        }
#endif

/* The button was pressed in the palette window */
        for (i=1 ; i<= NPLUG; i++){
	  if(!popped && but->subwindow == wlgt[i]) {
              //fprintf(stderr,"power change plug: %d %d\n",i,popped);
#ifdef LAMPS
              imagelight(i,"cycling",2);
              //power_change(i);
              if (plug_status[i] == OFF)
                power_on(i);
              else
                power_off(i);
#else
              popped = !popped;
              pwidth = 120;
              pheight = 100;
              pplug = i;
              pop_win[0] = XCreateWindow(dpy,RootWindow(dpy,screen),
                        (DisplayWidth(dpy,screen)-pwidth)/2,
                        (DisplayHeight(dpy,screen)-pheight)/2,
                        pwidth,pheight,
                        border_width,0,InputOutput,CopyFromParent,
		        CWBackPixel | CWBorderPixel, &xswa);
           //   XChangeWindowAttributes(dpy,pop_win[0],CWBackingStore,&xswa);
           //   XSetStandardProperties(dpy,pop_win[0],"verify","verify",None,
           //      NULL,0,&sizehints);
              XSetWMHints(dpy, wbase, &wmhints);
              XSelectInput(dpy,pop_win[0], ExposureMask | StructureNotifyMask | ButtonPressMask );
              //XSelectInput(dpy,pop_win[0], ExposureMask | ButtonPressMask | 
              //    StructureNotifyMask | KeyPressMask );
              XMapWindow(dpy,pop_win[0]);

              for (i=1; i<=3; i++) {
               if (i==1) {
                 w = pwidth;
                 w0=0;
                 h0=0;
               } else { 
                 w = pwidth/2;
                 h0=pheight/2;
                 if (i==2)
                   w0=0; 
                 else
                   w0=pwidth/2;
               } 
               h = pheight/2;
               pw[i] = w;
               ph[i] = h;
               pw0[i] = w0;
               ph0[i] = h0;
                 
               pop_win[i] = XCreateSimpleWindow(dpy,pop_win[0],pw0[i],
                ph0[i],pw[i],ph[i],XYZBORDER,
                WhitePixel(dpy,screen),BlackPixel(dpy,screen));
               XChangeWindowAttributes(dpy,pop_win[i],CWBackingStore,&xswa);
              }
              popgc = XCreateGC(dpy, pop_win[0], 0, NULL);
              XSetState(dpy,popgc,
               WhitePixel(dpy,screen),BlackPixel(dpy,screen),GXcopy,AllPlanes);
              XSetFont(dpy,popgc,fontinfo[1]->fid);
              XSetFillStyle(dpy,popgc,FillSolid);

  //            XSelectInput(dpy,pop_win[0], ExposureMask | ButtonPressMask );

              XMapSubwindows(dpy,pop_win[0]);

        imagelight(-1);
              for (i=1;i<=3;i++) {
                put_text(pop_win[i],pw[i],ph[i],pw0[i],ph0[i],pcolor[i],ptext[i]);
              }
#endif
              XFlush(dpy);
          }
	}
	break;

/*
 * BUTTON RELEASED: The button was released
 */
      case ButtonRelease:
        XFlush(dpy);
	break;

/*
 * KEY PRESSED: A Keyboard key was pressed
 */
      case KeyPress:
        XLookupString(key,keystring,KEYLEN,&ks,&compose_status);
        if (key->window == wbase) {
          if (keystring[0] == 'q' || keystring[0] == 'Q') {
            for (i=0;i<2;i++)
              XUnloadFont(dpy,fontinfo[i]->fid);
            XFreeGC(dpy,emerggc);
            if (popped) XFreeGC(dpy,popgc);
            XFreeGC(dpy,textgc);
            XCloseDisplay(dpy);
            exit(-1);
          }
          keystring[0] = '\0';
        }


	break;

/*
 * MOUSE MOVED: Mouse movement
 */
      case LeaveNotify:
        XFlush(dpy);
	break;

      case MotionNotify:
        XFlush(dpy);
	break;

    }

  } /* end of while(1) */
  return(0);
} /* end of function refresh() */

void put_text(win,w,h,w0,h0,color,text)
Window win;
int w, h, w0, h0, color;
char *text;
{
    int l, x0, y0, ifont, fw;

    XSetForeground(dpy,popgc,stcolor[color].pixel);
    XFlush(dpy);
    XFillRectangle(dpy,win,popgc,0,0,w,h);
    XFlush(dpy);
    XSetForeground(dpy,popgc,WhitePixel(dpy,screen));
    XFlush(dpy);
    if (_XErrorEvent.serial!=0) 
       printf("loc 12: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);

    ifont = 1;
    fw = XTextWidth(fontinfo[ifont],text,strlen(text));
    y0=fontheight[ifont] + (h-fontheight[ifont])/2 - 1;
    x0=(w-fw)/2;
    XDrawString(dpy,win,popgc,x0,y0,text,strlen(text));
    XFlush(dpy);
}

resizesubwin()
{
  int i;

  width = infowidth;
  height = infoheight - lgtheight;
  lgtwidth = width / NPLUG;

  XSetStandardProperties(dpy,wbase,"power","power",None,NULL,0,&sizehints);
  XSetWMHints(dpy, wbase, &wmhints);
  XResizeWindow(dpy,wbase,width,height+lgtheight);

#ifndef LAMPS
  XMoveResizeWindow(dpy,wlgt[0],0,0,width,height);
#endif
  for (i=1; i<= NPLUG; i++)  {
   XMoveResizeWindow(dpy,wlgt[i],(i-1)*lgtwidth,height,lgtwidth,lgtheight);
  }
  textinit();
  XMapWindow(dpy,wbase);
  XMapSubwindows(dpy,wbase);
  XFlush(dpy);

}
textinit()
{
  int i, fw;
  char text[300];

#ifdef DEBUG
  for (i=1 ;i<=NPLUG; i++) fprintf(stderr,"%d %d\n",i, plug_status[i]);
#endif

  text[0] = 0;
#ifdef LAMPS
  imagelight(1,"He",plug_status[1]);
  imagelight(2,"Ne",plug_status[2]);
  imagelight(3,"Ar",plug_status[3]);
  imagelight(4,"Bright quartz",plug_status[4]);
  imagelight(5,"Dim quartz",plug_status[5]);
#else
  if (plug_status[MOTORS] == ON) {
    fw = XTextWidth(fontinfo[0],"Emergency stop      ",20);
    for (i=0; i<(int)width/fw; i++)
      sprintf(text+(i*20),"Emergency stop      ");
  } else{
    fw = XTextWidth(fontinfo[0],"Motor/TOCC power OFF     ",20);
    for (i=0; i<(int)width/fw; i++)
      sprintf(text+(i*20),"Motor power is OFF  ");
  }
  if (plug_status[MOTORS] == ON)
    imagelight(0,text,0);
  else
    imagelight(0,text,2);
  imagelight(1,"Telescope control",plug_status[1]);
  imagelight(2,"Eyeball",plug_status[2]);
  imagelight(3,"Weather station",plug_status[3]);
  imagelight(7,"Autofill system",plug_status[7]);
  imagelight(8,"TemPageR A",plug_status[8]);
  imagelight(10,"Video",plug_status[10]);
  imagelight(12,"Dome fan",plug_status[12]);
  imagelight(13,"Dome louvers",plug_status[13]);
  imagelight(14,"Webcam lights",plug_status[14]);
  imagelight(15,"Rack/pier fan",plug_status[15]);
  imagelight(16,"Telescope motors",plug_status[16]);

//  imagelight(17,"Fiber/USB",plug_status[17]);
  //imagelight(18,"Mirror fans",plug_status[18]);
  imagelight(18,"HSP power",plug_status[19]);
  imagelight(20,"Apogee power",plug_status[20]);
  imagelight(21,"Leach/FLI power",plug_status[21]);
  imagelight(24,"TemPageR B",plug_status[24]);

/*
  imagelight(17,"TemPageR B",plug_status[17]);
  imagelight(18,"Leach/FLI power",plug_status[18]);
  imagelight(19,"Apogee power",plug_status[19]);
  imagelight(20,"HSP power",plug_status[20]);
*/
 // imagelight(19,"Netswitch power",plug_status[19]);
 // imagelight(20,"FLI power",plug_status[20]);
 // imagelight(22," power",plug_status[22]);
 // imagelight(23,"Dorothea",plug_status[23]);
 // imagelight(24,"Astrotimer",plug_status[24]);
#define TOCC 1
#define EYEBALL 2
#define WEATHER 3
#define FILL 7
#define MONITOR 8
#define DOMEFAN 12
#define LOUVERS 13
#define LIGHTS 14
#define RACKFAN 15
#define MOTORS 16
#define FIBERUSB 17
//#define MIRRORFANS 18
#define HSP 18
//#define HSP 19
//#define NETPOW 19
#define GCS 20
#define LEACHPOW 21
//#define ROPERPOW 22
//#define DOROTHEA 23
//#define ASTROTIMER 24

#endif
  XFlush(dpy);
}

#ifdef DEBUGIT
void power_off(plug)
int plug;
{
  plug_status[plug] = OFF;
}
void power_change(plug)
int plug;
{
  plug_status[plug] = !plug_status[plug];
}

int power_status(p)
int *p;
{
  int i;
  for (i=1; i<= NPLUG; i++) 
    p[i] = (plug_status[i] > -2 ? plug_status[i] : 0);
}
#endif
