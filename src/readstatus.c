#include "mytype.h"
#include "slamac.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* 
   readstatus routine attempts to read the latest telescope status file
   into the STATUS structure which is passed to the routine from the status
   files which are passed to the routine. If it fails (because the status
   file is not ready for reading), the routine returns -1. If it succeeds, the
   routine returns 0
*/

FILE *tfile;
int topen = 0;
char *getresp(FILE *);

#include <sys/timeb.h>

#ifdef SOCKET
readstatus(int sock,int port, char *server, struct STATUS *G)
{
  char statusinfo[500];
  int client;
  FILE *fp;

  read_server(&sock, &client, port, server, statusinfo, 500);
  close(client);
  fp = fopen("/home/export/tocc/statr.doc","w");
  fprintf(fp,"%s",statusinfo);
  fclose(fp);
  return(parsestatus(statusinfo, G));

}
getstatus(int port, char *server, struct STATUS *G)
{
  char line[500];
  int sock, nread;
  int status;

#define NEW
#ifdef NEW
  status= sendport(port, server, "STATUS\n", line, 500);
  if (strstr(line,"STATUS:") != NULL)
    parsestatus(strstr(line,"STATUS:"),G);
#else
//fprintf(stderr,"asking for status %d %s\n",port, server);
  setup_client(&sock,port,server,1);
//fprintf(stderr,"sock: %d\n",sock);
  if (sock>0) {
     write(sock,"STATUS\n",7);

     line[0] = 0;
     while (strstr(line,"DONE") == NULL) {
       nread=0;
       memset(line,0,500);
       while (strstr(line,"\n") == NULL) {
         nread += read(sock,line+nread,1);
       }
       parsestatus(line,G);
     }
     close(sock);
     status=0;
   } else {
     fprintf(stderr,"no server available!\n");
     status=-1;
   }
#endif
  //fprintf(stderr,"getstatus time: %f\n",G->current_utc);
  return(status);
}
parsestatus(char *statusinfo, struct STATUS *G) 
{
  int n;

  sprintf(G->mc_file,"\0");
  n=sscanf(statusinfo,
  "STATUS: %lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%d%d%d%d%d%d%d%d%d%lf%lf%lf%d%lf%d%d%d%d%d%d%lf%lf%lf%d%d%d%lf%lf%lf%d%d%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%ld%ld%ld%d%d%s",
       &G->current_obs_ra, &G->current_obs_dec, &G->current_pa,
       &G->current_mean_ra, &G->current_mean_dec, 
       &G->current_mean_epoch, &G->current_utc, &G->current_lasth,
       &G->current_obs_az, &G->current_obs_alt, &G->current_obs_rot,
       &G->tertiary_port,
       &G->telescope_initialized, &G->telescope_at_home,
       &G->tracking_on, &G->telescope_is_slewing, &G->use_encoders,
       &G->x_encoder_tracking,&G->y_encoder_tracking, &G->z_encoder_tracking,
       &G->last_x_rate,&G->last_y_rate, &G->last_z_rate,
//       &G->x_encoder_installed, &G->y_encoder_installed,&G->z_encoder_installed,
       &G->nmove,
       &G->ut1_minus_utc,&G->dome_initialized,&G->dome_slaved,
       &G->dome_open,&G->lower_dome_open,&G->dome_azimuth,
       &G->mirror_covers_open,
       &G->last_az_error,&G->last_alt_error,&G->last_rot_error,
       &G->t_step_pos,&G->u_step_pos,&G->v_step_pos,
       &G->current_out_temp,&G->current_cab_temp,&G->current_aux_temp,
       &G->current_winddir,&G->current_windspeed,&G->check_35m_closed,
       &G->shutdown_state,
       G->sx+1,G->sy+1,G->cx+1,G->cy+1,G->theta+1,
       G->sx+2,G->sy+2,G->cx+2,G->cy+2,G->theta+2,
       &G->ccd_temp, 
       &G->guider_x_pos, &G->guider_y_pos, &G->guider_z_pos,
       &G->guider_filtpos,
       &G->mc_enabled,G->mc_file); 
  G->theta[1]*=DR2D; 
  G->theta[2]*=DR2D; 
  if (n==61) 
    return(0);
  else
    return(-1);
}

#else
readstatus(char *tstatusfile,char *tstatusreadyfile,struct STATUS *G)
{
  char com[100],tmpfile[80];
  int iret;
  FILE *fp;

  fp = fopen(tstatusfile,"r");
  if (fp == NULL) {
    //fprintf(stderr,"fopen fails: %s\n",tstatusfile);
    //perror("open error: ");
    return(-1);
  }
  fclose(fp);
  
  sprintf(tmpfile,"/tmp/%10.10d",random());
  sprintf(com,"tail -2 %s >%s 2>/dev/null",tstatusfile,tmpfile);
  iret=system(com); 
  iret = areadstatus(tmpfile,tmpfile,G); 
  if (tfile != NULL) fclose(tfile);
  topen = 0;
  remove(tmpfile);
  if (iret==60) {
    return(0);
  } else {
//    fprintf(stderr,"iret: %d %s\n", iret, tmpfile);
    return(-1);
  }
  //  return(iret);
}

areadstatus(char *tstatusfile, char *tstatusreadyfile, struct STATUS *G)
{
  int read, ifile, n;
  char *statusinfo;
  char cpfile[500];
  double x;
  char *l;

    if (topen == 0) {
      tfile = fopen (tstatusfile,"r");
      if (tfile != NULL) 
        topen = 1;
      else
        return(-1);
    }
 
/*  Read telescope status file up to latest entry */
    read=-1;
    while ( (statusinfo=getresp(tfile)) != NULL ) {
//fprintf(stderr,"statusinfo: %s\n",statusinfo);
      read = 0;
      sprintf(G->mc_file,"\0");
//strcpy(cpfile,statusinfo);
//      n=sscanf(cpfile,
      n=sscanf(statusinfo,
  "STATUS: %lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%d%d%d%d%d%d%d%d%d%d%d%d%lf%d%d%d%d%d%d%lf%lf%lf%d%d%d%lf%lf%lf%d%d%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%ld%ld%ld%d%d%s",
       &G->current_obs_ra, &G->current_obs_dec, &G->current_pa,
       &G->current_mean_ra, &G->current_mean_dec, 
       &G->current_mean_epoch, &G->current_utc, &G->current_lasth,
       &G->current_obs_az, &G->current_obs_alt, &G->current_obs_rot,
       &G->telescope_initialized, &G->telescope_at_home,
       &G->tracking_on, &G->telescope_is_slewing, &G->use_encoders,
       &G->x_encoder_tracking,&G->y_encoder_tracking, &G->z_encoder_tracking,
       &G->x_encoder_installed, &G->y_encoder_installed,&G->z_encoder_installed,
       &G->nmove,
       &G->ut1_minus_utc,&G->dome_initialized,&G->dome_slaved,
       &G->dome_open,&G->lower_dome_open,&G->dome_azimuth,
       &G->mirror_covers_open,
       &G->last_az_error,&G->last_alt_error,&G->last_rot_error,
       &G->t_step_pos,&G->u_step_pos,&G->v_step_pos,
       &G->current_out_temp,&G->current_cab_temp,&G->current_aux_temp,
       &G->current_winddir,&G->current_windspeed,&G->check_35m_closed,
       &G->shutdown_state,
       G->sx+1,G->sy+1,G->cx+1,G->cy+1,G->theta+1,
       G->sx+2,G->sy+2,G->cx+2,G->cy+2,G->theta+2,
       &G->ccd_temp, 
       &G->guider_x_pos, &G->guider_y_pos, &G->guider_z_pos,
       &G->guider_filtpos,
       &G->mc_enabled,G->mc_file); 
     G->theta[1]*=DR2D; 
     G->theta[2]*=DR2D; 
/*
fprintf(stderr,
  "%lf %lf %lf %lf %lf %lf %lf %lf %lf %d %d %d %d %d %d %d %d %d %d %d %d %lf %d %d %d %d %d %d %lf %lf %lf %d %d %d %lf %lf %lf %d %d %d %lf %d %s\n",
       G->current_obs_ra, G->current_obs_dec, G->current_pa,
       G->current_mean_epoch, G->current_utc, G->current_lasth,
       G->current_obs_az, G->current_obs_alt, G->current_obs_rot,
       G->telescope_initialized, G->telescope_at_home,
       G->tracking_on, G->telescope_is_slewing, G->use_encoders,
       G->x_encoder_tracking_enabled,G->y_encoder_tracking_enabled,
       G->z_encoder_tracking_enabled,
       G->x_encoder_installed, G->y_encoder_installed,G->z_encoder_installed,
       G->nmove, G->ut1_minus_utc,G->dome_initialized,G->dome_slaved,
       G->dome_open,G->lower_dome_open,G->dome_azimuth,G->mirror_covers_open,
       G->last_az_error,G->last_alt_error,G->last_rot_error,
       G->t_step_pos,G->u_step_pos,G->v_step_pos,
       G->current_out_temp,G->current_cab_temp,G->current_aux_temp,
       G->current_winddir,G->current_windspeed,G->check_35m_closed,G->ccd_temp,
       G->mc_enabled,G->mc_file); 
*/
    }
    //return(read);
    return(n);
}
#endif

initstatus(struct STATUS *G)
{
       G->current_obs_ra = 0;
       G->current_obs_dec = 0; 
       G->current_pa = 0;
       G->current_mean_epoch = 0; 
       G->current_utc = 0;
       G->current_lasth = 0;
       G->current_obs_az = 0; 
       G->current_obs_alt = 0; 
       G->current_obs_rot = 0;
       G->tracking_on = 0; 
       G->telescope_is_slewing = 0; 
       G->use_encoders = 0;
       G->encoder_tracking = 0;
       G->x_encoder_installed = 0; 
       G->y_encoder_installed = 0;
       G->z_encoder_installed = 0;
       G->nmove = 0; 
       G->ut1_minus_utc = 0;
       G->dome_initialized = 0;
       G->dome_slaved = 0;
       G->last_az_error = 0;
       G->last_alt_error = 0;
       G->last_rot_error = 0;
}

#ifdef CURSES
readccdstatus(char *cstatusfile, char *cstatusreadyfile, struct CCDSTATUS *G)
{
}
#else
readccdstatus(char *cstatusfile, char *cstatusreadyfile, struct CCDSTATUS *G)
{
  int ready, l;
  char *object;
  FILE *file;
  char statusinfo[300];

/*  If CCD status ready, then read it */
  G->filtfoc = 0;
  ready = open(cstatusreadyfile,O_RDONLY);
  if (ready > 0) {
    close(ready);
    file = fopen(cstatusfile,"r");
    if (file<0) {
      fprintf(stderr,"error opening ccdstatus file\n");
      return(-1);
    }
    fgets(statusinfo,299,file);

    sscanf(statusinfo, "%d%lf%d%lf%s%d%d%d%d%d%d%d%lf%d%d%d%d%lf%lf%lf%lf%lf%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%d%s",
       &G->cleans, &G->exposure, &G->expstatus, &G->end_time,
       &G->filename, &G->filetype, &G->autodisplay, &G->autoxfer, 
       &G->autodark, &G->autoflat, &G->filter, &G->filtfoc, &G->focus,
       &G->numseq, &G->incval, &G->shutter, 
       &G->guiding, &G->guide_x0, &G->guide_y0,
       &G->guide_pa, &G->guide_rad, &G->guide_mag,
       &G->guide_size, &G->guide_update, 
       &G->ax, &G->bx, &G->ay, &G->by, 
       &G->sx, &G->sy, &G->theta, &G->cx, &G->cy,
       &G->ccd_temp,&G->ccd_temp_status,
       &G->object);
    fgets(G->object,NOBJECT-1,file);
    object = G->object;
    l = strlen(object) - 1;
    *(object+l) = 0;

    fclose(file);

    return(0);
  }
  return(-1);
}
#endif
