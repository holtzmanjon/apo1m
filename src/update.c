#include <ncurses.h>
#include "mytype.h"
#include "filter.h"

#define APOGEE
#undef RSTATUS

void update_ccd_display(struct CCDSTATUS *,int offset,char *);
void vcopy(chtype *vbuf, char *buf, chtype attrib);

WINDOW *wstatus;
WINDOW *wcommand;

void win_init() {
  char message[80];

  initscr();
  cbreak();
  halfdelay(1);
  //echo();
  noecho();
  curs_set(0);
  wstatus = newwin(6,80,0,0);
  if (wstatus == NULL) writeterm("error opening wstatus");
  wcommand = newwin(LINES-6,0,6,0);
  if (wcommand == NULL) writeterm("error opening wcommand");
  idlok(wcommand,TRUE);
  scrollok(wcommand,TRUE);
  sprintf(message,"erasechar: %c %d\n",erasechar(),erasechar());
  writeterm(message);

  //nodelay(wcommand,TRUE);
}

void win_close() {
  endwin();
}

void win_read(char *command) {
  int i, xcurs, ycurs;
  char message[80];
  //wgetnstr(wcommand,command,8000);
  i=0;
  while ( (command[i]=wgetch(wcommand)) != '\n' && i<8000 ) {
    xtv_refresh(0);
    if (command[i] == erasechar() ) {
      getyx(wcommand,ycurs,xcurs);
      mvwdelch(wcommand,ycurs,xcurs-1);
      wrefresh(wcommand);
      i--;
    } else if (command[i] != ERR) {
      wechochar(wcommand,command[i]);
      i++;
    }
  }
  command[i] = 0;
  wechochar(wcommand,'\n');
}

int ycurs, xcurs;

void win_write(char *command, int refresh) {
  char message[80];
  waddnstr(wcommand,command,strlen(command));
  getyx(wcommand,ycurs,xcurs);
  //wmove(wcommand,ycurs,xcurs);
  if (refresh) {
    clearok(wcommand,TRUE);
    wrefresh(wcommand);
  }
}

void puttext(int x1,int y1,int x2,int y2,chtype *buf)
{
  int i,status;
  chtype c;
  char message[80];

  if (x1<0 || x2>79 || y1 <0 || y2 > 5) {
    sprintf(message,"out of bounds: %d %d %d %d\n",x1,y1,x2,y2);
    writeterm(message);
    if (x1<0) x1=0;
    if (x2>79) x2=79;
    if (y1<0) y1=0;
    if (y2>4) y2=4;
  }
  status=wmove(wstatus,y1,x1);
  if (status != OK) writeterm("error with wmove");
  for (i=0;i<x2-x1+1;i++) {
    waddch(wstatus,buf[i]);
  }
  //sprintf(message,"xcurs: %d ycurs: %d\n",xcurs,ycurs);
  //writeterm(message);
  //readterm(message);
  //wmove(wcommand,ycurs,xcurs);
  //wrefresh(wcommand);
}

#define WHITE A_NORMAL

void update_ccd_display(struct CCDSTATUS *ccdinfo, int offset, char *title)
{
  char buf[80];
  chtype vbuf[80];
  int h,m,sign,i,start,l;
  double s, xs, ys, timeleft;

  for (i=0;i<23;i++) buf[i] = '-';
  sprintf(buf+23,"%s",title);
  start = strlen(buf);
  for (i=start;i<79;i++) buf[i] = '-';
  buf[79] = 0;
  vcopy(vbuf,buf,WHITE);
  puttext(1,9+offset,79,9+offset,vbuf);
  for (i=0;i<80;i++) buf[i] = '-';
  buf[79] = 0;
  vcopy(vbuf,buf,WHITE);
  puttext(1,14+offset,79,14+offset,vbuf);

  sprintf(buf,"Next file: %s.%04d.fits",ccdinfo->filename,ccdinfo->incval);
  vcopy(vbuf,buf,WHITE);
  puttext(1,10+offset,strlen(buf),10+offset,vbuf);

  if (ccdinfo->expstatus == 1)
    sprintf(buf,"Status: EXPOSING                                     ");
  else if (ccdinfo->expstatus == 2)
    sprintf(buf,"Status: WRITING                                      ");
  else if (ccdinfo->expstatus == 3)
    sprintf(buf,"Status: DISPLAYING                                   ");
  else if (ccdinfo->expstatus == 4)
    sprintf(buf,"Status: READING                                      ");
  else 
    sprintf(buf,"Status: WAITING                                      ");
  vcopy(vbuf,buf,WHITE);
  puttext(25,11+offset,25+strlen(buf)-1,11+offset,vbuf);

  if (ccdinfo->expstatus == 1) {
    //timeleft = (ccdinfo->end_time - G->current_utc)*24*3600;
timeleft = 0;
    timeleft = ccdinfo->end_time - time(NULL);
    sprintf(buf,"Approximate time left: %4d",(int)timeleft);
    vcopy(vbuf,buf,WHITE);
    puttext(46,11+offset,46+strlen(buf),11+offset,vbuf);
  }

  if (ccdinfo->guiding ==1) {
    sprintf(buf,"GUIDING x:%5.1f y:%5.1f",
                 ccdinfo->guide_x0,ccdinfo->guide_y0);
    vcopy(vbuf,buf,WHITE);
    puttext(41,11+offset,40+strlen(buf),11+offset,vbuf);
  } 
  sprintf(buf,"Exptime: %8.1f",ccdinfo->exposure);
  vcopy(vbuf,buf,WHITE);
  puttext(1,11+offset,17,10+offset,vbuf);



  sprintf(buf,"Filter: %d %s(%s)",ccdinfo->filter,filtname[ccdinfo->filter],longfiltname[ccdinfo->filter]);
  sprintf(buf,"Slit center: (%5.1f,%5.1f)",ccdinfo->xc, ccdinfo->yc);
  l = strlen(buf);
  for (i=l; i< 80; i++) buf[i] = ' ';
  vcopy(vbuf,buf,WHITE);
  puttext(1,12+offset,28,12+offset,vbuf);

  sprintf(buf,"Cleans: %3d",ccdinfo->cleans);
 // sprintf(buf,"Cleans: %3d",ccdinfo->offsettype);
  vcopy(vbuf,buf,WHITE);
  puttext(29,12+offset,28+strlen(buf),12+offset,vbuf);

  sprintf(buf,"size:%3d  update:%3d",
                 ccdinfo->guide_size, ccdinfo->guide_update);
  //sprintf(buf,"size:%3d  update:%3d",
  //               ccdinfo->offsettype&0x1, ccdinfo->offsettype&0x2);
  vcopy(vbuf,buf,WHITE);
  puttext(41,12+offset,40+strlen(buf),12+offset,vbuf);

  sprintf(buf,"CCD Temp:%4d",(int)ccdinfo->ccd_temp);
#ifndef APOGEE
  if (ccdinfo->ccd_temp < -100) 
    vcopy(vbuf,buf,WHITE);
  else
    vcopy(vbuf,buf,A_REVERSE);
#else
  if (ccdinfo->ccd_temp_status ==0)
    vcopy(vbuf,buf,WHITE);
  else
    vcopy(vbuf,buf,A_REVERSE);
#endif
  puttext(66,12+offset,65+strlen(buf),12+offset,vbuf);

  if (ccdinfo->filetype==1) 
    sprintf(buf,"Filetype: FITS      ");
  else if (ccdinfo->filetype==2)
    sprintf(buf,"Filetype: SDAS/IRAF ");
  else
    sprintf(buf,"Filetype: Not Saving");
  vcopy(vbuf,buf,WHITE);
  puttext(1,13+offset,strlen(buf),13+offset,vbuf);

  if (ccdinfo->autodark)
    sprintf(buf,"Autodark: ON ");
  else
    sprintf(buf,"Autodark: OFF");
  vcopy(vbuf,buf,WHITE);
  puttext(17,13+offset,16+strlen(buf),13+offset,vbuf);

  if (ccdinfo->autodisplay)
    sprintf(buf,"Autodisplay: ON ");
  else
    sprintf(buf,"Autodisplay: OFF");
  vcopy(vbuf,buf,WHITE);
  puttext(31,13+offset,30+strlen(buf),13+offset,vbuf);

  if (ccdinfo->autoxfer)
    sprintf(buf,"Autoxfer: ON ");
  else
    sprintf(buf,"Autoxfer: OFF");
  vcopy(vbuf,buf,WHITE);
  puttext(48,13+offset,47+strlen(buf),13+offset,vbuf);

  sprintf(buf,"Offset:          ");
  vcopy(vbuf,buf,WHITE);
  puttext(63,13+offset,62+strlen(buf),13+offset,vbuf);

  buf[0]=0;
  if (ccdinfo->offsettype&1) sprintf(buf,"Remark ");
  if (ccdinfo->offsettype&2) sprintf(buf+strlen(buf),"TUI");
  vcopy(vbuf,buf,WHITE);
  puttext(70,13+offset,69+strlen(buf),13+offset,vbuf);

  wrefresh(wstatus);

}

void vcopy(chtype *vbuf, char *buf, chtype attrib)
{
  int i;

  for (i=0;i<strlen(buf);i++) {
    vbuf[i] = buf[i]|attrib;
  }
}
