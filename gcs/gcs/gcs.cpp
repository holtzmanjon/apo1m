/****************************************************************************/
/*                                                        */
/*  Module:    main.cpp                               */
/*                                                        */
/*  Purpose:  command program loop for guider computer  */
/*                                                        */
/****************************************************************************/

#define no_filter
#define no_guider

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <mem.h>
#include <alloc.h>
#include <float.h>
#include <math.h>
#include <dos.h>
#include "error.h"
#include "io.h"
#include "systimer.h"
#include "mytype.h"
#include "filter.h"
#include "guider.h"
#include "ccd.h"
#include "scf.h"

#define NEWMEM

#ifndef no_pc38
#include "pcx.h"
#endif

long return_step_position(char);

void command_help();
void check_restart();

FILE *guide_file;
FILE *debug_file;

BOOL guiding = FALSE;
BOOL writeguide = FALSE;
BOOL offguiding = FALSE;
BOOL remote_on = TRUE;
BOOL remote_command = FALSE;
BOOL use_shutter = TRUE;
BOOL do_disable = FALSE;
BOOL lowres = TRUE;

extern double x0, y0, ax, bx, ay, by, badthresh=-100;
extern long nguide;
extern int size, update, nupdate;
extern unsigned int exptime;    
extern int nrow, ncol;
extern int verbose;
extern int domedian;
extern long iseq;
extern WORD **buf;
#ifdef NEWMEM
#define MAXSIZE 25
#endif

BOOL command_mode = TRUE;
int adj_nupdate = 0;
int adj_update = 0;
int adj_offset = 0;
int adj_foc = 0;
int cur_foc = 0;
BOOL fochold = TRUE;

main()
{
  char command[MAXCMD], inputline[MAXCMD], inputline2[MAXCMD];
  int status, n, idark, ndark;
  struct time t;
  #ifndef no_pc38
  AXISPACK axisRec;
  #endif

  // Check to see if communications program has been started, and open
  //   communications files if so.
  writeline("check_restart\n",1);
  check_restart();
  writeline("opening files\n",1);
//  guide_file = fopen("d:\\spec\\guide.log","a");
  debug_file = fopen("debug.log","w");

  // Read in configuration parameters
  //initgcsscf(&gcsGlobalNew);
  //printgcsscf(&gcsGlobalNew);
  //writegcsscf(&gcsGlobalNew);
  readgcsscf(&gcsGlobalNew);
  gcsGlobal = &gcsGlobalNew;

// Initialize SpectraSource camera
#ifndef no_ccd
  ccd_initialize();
#endif

// Initialize filter wheel
#ifndef no_filter
  status = filter_initialize(0);
#endif

// Quick init on guider
#ifndef no_guider
  guider_initialize(0,gcsGlobal->x_step_pos,gcsGlobal->y_step_pos);
#endif
  printgcsscf(&gcsGlobalNew);

  while (command_mode) {

    status = GCSERR_OK;

    writeline("Command: ",0);
//  Command-mode commands: these are not acted upon until a CR is received
    if ( getline(inputline2,sizeof(inputline2)) != 0) {
      status = GCSERR_COMIO;
    } else {

//    Parse out the first word in case more junk is on the input line
      command[0] = '\0';
      sscanf(inputline2,"%s",command);
      strupr(inputline2);
    sprintf(outbuf,"got command %d %s",strlen(command),command);
    writeline(outbuf,0);
      if (strlen(command) > 1) {

//      Convert to upper case
	strupr(command);

	// Help list
	if (strcmp(command,"HP")==0) {
	      command_help();
	}
 
	else if (strcmp(command,"RT")==0) {
	      remote_on = !remote_on;
	      sprintf(outbuf,"remote_on: %d",remote_on);
	      writeline(outbuf,0);
	}

 #ifndef no_ccd
	// Exposure
	else if (strcmp(command,"EXP")==0) {
	      if ((n=sscanf(inputline2,"EXP %u",&exptime)) < 1) {
		writeline("Enter exposure time (ms): ",0);
		if (getline(inputline,sizeof(inputline)) ==0) {
		  sscanf(inputline,"%u",&exptime);
		} else
		status = GCSERR_COMIO;
	      }
	      if (status == GCSERR_OK) {
		gettime(&t);
		sprintf(outbuf,"%d:%d:%d.%d   calling write\n",
			t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
		status = ccd_write(buf,exptime,ncol,nrow,1,1,-1);
		gettime(&t);
		sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    calling expose\n",
			t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
		status = ccd_expose(buf,exptime,use_shutter,0,ncol-1,0,nrow-1,1);
		gettime(&t);
		sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    done\n",
			t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
		writeline(outbuf,1);
	      }
	}

	// Dark Exposure
	else if (strcmp(command,"DARK")==0) {
	      if ((n=sscanf(inputline2,"DARK %u%d",&exptime,&ndark)) < 2) {
			    sprintf(outbuf,"n: %d %s",n,inputline2);
				writeline(outbuf,1);
		writeline("Enter exposure time (ms): ",0);
		if (getline(inputline,sizeof(inputline)) ==0) {
		  sscanf(inputline,"%u",&exptime);
		} else
		status = GCSERR_COMIO;
		writeline("Enter number of darks to take: ",0);
		if (getline(inputline,sizeof(inputline)) ==0) {
		  sscanf(inputline,"%d",&ndark);
		} else
		status = GCSERR_COMIO;
	      }
	      if (status == GCSERR_OK) {
		for (idark=0 ; idark<ndark ; idark++) {
		 gettime(&t);
		 sprintf(outbuf,"%d:%d:%d.%d    calling write\n",
			t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
		 status = ccd_write(buf,exptime,ncol,nrow,1,1,idark);
		 gettime(&t);
		 sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    calling dark expose\n",
			t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
		 status = ccd_expose(buf,exptime,FALSE,0,ncol-1,0,nrow-1,1);
		 gettime(&t);
		 sprintf(outbuf+strlen(outbuf),"%d:%d:%d.%d    done\n",
			t.ti_hour,t.ti_min,t.ti_sec,t.ti_hund);
		 writeline(outbuf,1);
		}
	      }
	}

	// Start guiding at specified position
	else if (strcmp(command,"OLDGUIDE")==0) {
	     sscanf(inputline2,"OLDGUIDE %lf %lf %d %u %d %lf %lf %lf %lf",
		    &x0,&y0,&size,&exptime,&update,&ax,&bx,&ay,&by);
	     nupdate = 0;
	     guiding = TRUE;
	     offguiding = FALSE;
	}

	else if (strcmp(command,"NEWGUIDE")==0) {
	     sscanf(inputline2,"NEWGUIDE %lf %lf %d %u %d",
		    &x0,&y0,&size,&exptime,&update);
#ifdef NEWMEM
	     size = (size+4>MAXSIZE ? MAXSIZE-4 : size);
#endif
	     nupdate = 0;
	     nguide = 0;
	     offguiding = TRUE;
	     guiding = TRUE;
	}
#endif
	else if (strcmp(command,"+VERBOSE")==0) {
		     verbose = 1;
	}

	else if (strcmp(command,"-VERBOSE")==0) {
		     verbose = 0;
	}

	else if (strcmp(command,"MEDIAN")==0) {
	     sscanf(inputline2,"MEDIAN %d",&domedian);
	}

	else if (strcmp(command,"THRESH")==0) {
	     sscanf(inputline2,"THRESH %lf",&badthresh);
	}

	// Write guide images
	else if (strcmp(command,"WRITE")==0) {
	     writeguide = TRUE;
	     sscanf(inputline2,"WRITE %ld",&iseq);
	}

	else if (strcmp(command,"NOWRITE")==0) {
	     writeguide = FALSE;
	}

	// low/high res mode
	else if (strcmp(command,"LOWRES")==0) {
	     lowres = TRUE;
	}

	else if (strcmp(command,"HIGHRES")==0) {
	     lowres = FALSE;
	}

	else if (strcmp(command,"+FOCHOLD")==0) {
	     fochold = TRUE;
	}

	else if (strcmp(command,"-FOCHOLD")==0) {
	     fochold = FALSE;
	}

	// Stop guiding
	else if (strcmp(command,"GUIDEOFF")==0) {
             adj_nupdate = 0;
             adj_update = 0;
             adj_offset = 0;
             adj_foc = 0;
             cur_foc = 0;
	     guiding = FALSE;
	}

	else if (strcmp(command,"UPDATE")==0) {
	     sscanf(inputline2,"UPDATE %d",&update);
	     nupdate = 0;
	}

	else if (strcmp(command,"SIZE")==0) {
	     sscanf(inputline2,"SIZE %d",&size);
#ifdef NEWMEM
	     size = (size+4>MAXSIZE ? MAXSIZE-4 : size);
#endif
	}

	// Set default exposure time
	else if (strcmp(command,"EXPOSURE")==0) {
	      double dexptime;
	      sscanf(inputline2,"EXPOSURE %lf",&dexptime);
	      exptime = (unsigned int)dexptime*1000;
	}

	// Dummy commands to keep things compatible with PI commands
	else if (strcmp(command,"NUMSEQ")==0 ||
		 strcmp(command,"SETINCVAL")==0 ||
		 strcmp(command,"SHUTTERCMD")==0 ||
		 strcmp(command,"NAME")==0 ||
		 strcmp(command,"CLEANS")==0) {}

#ifndef no_ccd
	else if (strcmp(command,"TECON")==0) {
	     ccd_cooler(TRUE);
	}

	else if (strcmp(command,"TECOFF")==0) {
	     ccd_cooler(FALSE);
	}

	else if (strcmp(command,"OPEN")==0) {
	     ccd_shutter(TRUE);
	}

	else if (strcmp(command,"CLOSE")==0) {
	     ccd_shutter(FALSE);
	}

	else if (strcmp(command,"CCDINIT")==0) {
	     ccd_initialize();
	}
#endif

	else if (strcmp(command,"NOSHUTTER")==0) {
	     use_shutter = -1;
	}

	else if (strcmp(command,"SHUTTER")==0) {
	     use_shutter = TRUE;
	}

	else if (strcmp(command,"ENABLE")==0) {
	     do_disable = FALSE;
	}

	else if (strcmp(command,"DISABLE")==0) {
	     do_disable = TRUE;
	}

	else if (strcmp(command,"SETTIME")==0) {
	 struct time t;
	 struct date d;
	 int year, mon, day, hour, min, sec;
	 sscanf(inputline2+8,"%d%d%d%d%d%d",
	   &year, &mon, &day, &hour, &min, &sec);
	 d.da_year = year;
	 d.da_mon = mon;
	 d.da_day = day;
	 t.ti_hour = hour;
	 t.ti_min = min;
	 t.ti_sec = sec;
	 t.ti_hund = 99;
	 sprintf(outbuf,"inputline2: %s\n",inputline2);
	 writeline(outbuf,1);
	 sprintf(outbuf,"setting date/time: %d %d %d %d %d %d %d\n",
	    d.da_year, d.da_mon, d.da_day,
	    t.ti_hour, t.ti_min, t.ti_sec, t.ti_hund);
	 writeline(outbuf,1);
	 setdate(&d);
	 settime(&t);
	}

#ifndef no_pc38
	else if (strcmp(command,"PC")==0) {
		  sprintf(outbuf,"Enter command to send to PC38: ");
		  writeline(outbuf,0);
		  getline(inputline,sizeof(inputline));
		  pc38_send_commands(inputline);
		}

	else if (strcmp(command,"PCCHECK")==0) {
		  printf("status: %x",pcx_check_status(1));
		}

	else if (strcmp(command,"PCREAD")==0) {
		  sprintf(outbuf,"x returns: %ld\n",
			  return_step_position('x'));
		  sprintf(outbuf+strlen(outbuf),"y returns: %ld\n",
			  return_step_position('y'));
		  sprintf(outbuf+strlen(outbuf),"z returns: %ld\n",
			  return_step_position('z'));
		  sprintf(outbuf+strlen(outbuf),"t returns: %ld\n",
			  return_step_position('t'));
		  sprintf(outbuf+strlen(outbuf),"u returns: %ld\n",
			  return_step_position('u'));
		  sprintf(outbuf+strlen(outbuf),"v returns: %ld\n",
			  return_step_position('v'));
		  sprintf(outbuf+strlen(outbuf),"output bits returns: %x\n",
			  pc38_read_inputs());
		  sprintf(outbuf+strlen(outbuf),"output bits returns: %u\n",
			  pc38_read_inputs());
		  pc38_get_axis_status('x', axisRec);
		  sprintf(outbuf+strlen(outbuf),"x limit status: %d\n",
			  axisRec.limit);
		  pc38_get_axis_status('y', axisRec);
		  sprintf(outbuf+strlen(outbuf),"y limit status: %d\n",
			  axisRec.limit);
		  sprintf(outbuf+strlen(outbuf),"is_detent_in: %u\n",
			  is_detent_in());
		  sprintf(outbuf+strlen(outbuf),"is_detent_out: %u\n",
			  is_detent_out());

		  writeline(outbuf,0);
		}
#endif

#ifndef no_guider
	else if (strcmp(command,"GUIDEINIT")==0) {
		  status = guider_initialize(1,0,0);
		}

	else if (strcmp(command,"GUIDEHOME")==0) {
		  status = guider_move_steps(
		   gcsGlobal->x_home_pos,gcsGlobal->y_home_pos,1);
		}

	else if (strcmp(command,"GUIDEMOVE")==0) {
		  long dx, dy;
		  sscanf(inputline2,"GUIDEMOVE %ld %ld",&dx,&dy);
		  status = guider_move_steps(dx,dy,0);
		}

	else if (strcmp(command,"GUIDELOC")==0) {
		  double skypos;
		  sscanf(inputline2,"GUIDELOC %lf",&skypos);
		  status = guider_skypos(skypos);
		}

	else if (strcmp(command,"GUIDEFOC")==0 || strcmp(command,"DF")==0 ) {
		  int dfoc;
                  if (strcmp(command,"GUIDEFOC")==0)
		    sscanf(inputline2,"GUIDEFOC %d",&dfoc);
                  else
		    sscanf(inputline2,"DF %d",&dfoc);
		  status = guider_focus(dfoc,0);
		}

	else if (strcmp(command,"FO")==0 ) {
		  int dfoc;
		  sscanf(inputline2,"FO %d",&dfoc);
		  status = guider_focus(dfoc,1);
		}

	else if (strcmp(command,"GUIDEPOS")==0) {
		  long dx, dy;
		  sscanf(inputline2,"GUIDEPOS %ld %ld",&dx,&dy);
		  status = guider_move_steps(dx,dy,1);
		}

	else if (strcmp(command,"GUIDEADJ")==0) {
		  sscanf(inputline2,"GUIDEADJ %d %d",&adj_nupdate,&adj_foc);
                  adj_update = 0;
                  adj_offset = 0;
                  cur_foc = 0;
		}
#endif

#ifndef no_filter
	else if (strcmp(command,"FILTER")==0) {
		  int pos, ntry;
		  sscanf(inputline2,"FILTER %d",&pos);
		  ntry = 0;
		  while ((status = move_filter(pos)) != 0 && ntry< 5) ntry++;
		}

	else if (strcmp(command,"FILTTEST")==0) {
		  int pos;
		  sscanf(inputline2,"FILTTEST %d",&pos);
		  int ntest;
		  ntest=100;
		  n=0;
		  while (n<ntest) {
		    if (move_filter(pos) !=0) break;
		    if (move_filter(pos+5) !=0) break;
			n++;
		  }
		  sprintf(outbuf,"%d-%d succeeded %d/%d times\n",pos,pos+5,n,ntest);
		  writeline(outbuf,1);
	}

	else if (strcmp(command,"FILTINIT")==0) {
		  status = filter_initialize(0);
		}

	else if (strcmp(command,"FILTINIT1")==0) {
		  status = filter_initialize(1);
		}

	else if (strcmp(command,"FILTPOS")==0) {
		  int pos;
		  pos = get_filter_pos();
		  sprintf(outbuf,"filter pos: %d",pos);
		  writeline(outbuf,0);
		}

	else if (strcmp(command,"FILTSTEPS")==0) {
		  long steps;
		  sscanf(inputline2,"FILTSTEPS%ld",&steps);
				  new_steps_per_rot(steps);
		}

	else if (strcmp(command,"DETENT")==0) {
		  int det;
		  sscanf(inputline2,"DETENT%d",&det);
		  fprintf(stderr,"inputline2: %s\n",inputline2);
		  fprintf(stderr,"det: %d\n",det);
		  if (det==0)
		    status = move_detent_out();
	      else
		    status = move_detent_in();
		}
#endif

	else if (strcmp(command,"READSCF")==0) {
		  readgcsscf(&gcsGlobalNew);
	 }

	// quit command mode
	else if (strcmp(command,"QU")==0) {
	      command_mode = !command_mode;
	      sprintf(outbuf,"Leaving command mode and exiting program");
	      writeline(outbuf,0);
	}
  
	else if (strlen(command)>0){
	   writeline("Unknown command!!\r\n",0);
	}
	
      } // end if strlen(command > 1)

    } // end if (getline != GCSERR_OK)

    // Now process the returned status. This will also take care of
    //   sending a verification of returned status for remote operation
    error_code(status);
	       
  }  // end while (command_mode)

  // Write out current configuration
  writegcsscf(gcsGlobal);

  system("copy debug.log d:\\spec\\debug.log");

#ifndef no_hardware
  CameraCooler(FALSE);
#endif
  return(0);
}

void command_help()
{
   char ans;

   sprintf(outbuf,"General commands: \r\n"
		  " HP: print help menu\r\n"
		  " QU: quit command mode\r\n");
   writeline(outbuf,0);
}

#ifdef no_hardware
  #define SPEC "d:"
#else
  #define SPEC "e:"
#endif
char *holdfile = SPEC"\\spec\\fochold.doc";

void update_all()
{
  int status;
  static int adj_sign = 1;
  static double ratsum = 0;
  static double ratsum2 = 0;
  float peak, tot;
  double avgrat, avgsig;
  static double avgold = 0;
  BOOL wait = FALSE;
  FILE *fp;

  check_restart();

  if (guiding) {
    status=guide(buf,x0,y0,size,exptime,update,ax,bx,ay,by,&peak,&tot);
    if (status != GCSERR_OK) {
      error_code(status);
    } else if (adj_nupdate > 0) {
      /* Do we have focus update turned on? */
      /* Add one to update counter, reset sum to zero if this is first cycle */
      if (adj_update++ == 0) ratsum = ratsum2 = 0;
      ratsum += peak/tot;
      ratsum2 += (peak/tot)*(peak/tot);
      sprintf(outbuf,"update: %d  nupdate: %d   rat: %f  peak: %f   tot: %f",
              adj_update,adj_nupdate,peak/tot,peak, tot);
      if (verbose) writeline(outbuf,1);
      /* Do we have focus update turned on? */
      if (adj_update >= adj_nupdate) {
       wait = FALSE;
       if (fochold) {
          fp = fopen(holdfile,"r");
          if (fp != NULL) {
            wait = TRUE;
            fclose(fp);
          }
       }
       if ( !wait ) {
        avgrat = ratsum/adj_update;
        avgsig = (ratsum2 - ratsum*ratsum/adj_update);
        avgsig = 
          (avgsig>0&&adj_update>1 ? sqrt(avgsig/adj_update/(adj_update-1)): 0.05 );
        sprintf(outbuf,"offset: %d cur_foc: %d avgrat: %f  avgold: %f  avgsig: %f",
                adj_offset, cur_foc, avgrat, avgold, avgsig);
        writeline(outbuf,1);
        if (adj_offset == 0) {
          /* If this is first focus set, just do a guider focus offset */
          avgold = avgrat;
          guider_focus(adj_sign*adj_foc,0);
          cur_foc += adj_sign*adj_foc;
          sprintf(outbuf,"offsetting guider: %d",adj_sign*adj_foc);
          writeline(outbuf,1);
          adj_offset = 1;
        } else if (avgrat > (avgold+avgsig) ) {
          /* If ratio is better then previous, change telescope focus
              and zero out focus offset. Keep guider offset in same direction */
          sprintf(outbuf,"offsetting telescope: %d",-adj_sign*adj_foc);
          writeline(outbuf,1);
          sprintf(outbuf,"XFOCUS %d %d %d",-adj_sign*adj_foc,-adj_sign*adj_foc,-adj_sign*adj_foc);
          writeline(outbuf,6);
          writeline(outbuf,1);
          EventTimer PauseTimer;
          PauseTimer.NewTimer(2);
          while (!PauseTimer.Expired()) {}
          avgold = avgrat;
          adj_offset = 1;
        } else {
          /* If ratio is worse then previous, retake zero position, then try other direction */
          guider_focus(-adj_sign*adj_foc,0);
          cur_foc -= adj_sign*adj_foc;
          sprintf(outbuf,"offsetting guider: %d",-adj_sign*adj_foc);
          writeline(outbuf,1);
          adj_sign *= -1;
          adj_offset = 0;
        }
        adj_update = 0;
       }
      }
    }
  } else {
   EventTimer PauseTimer;
   PauseTimer.NewTimer(1);
   while (!PauseTimer.Expired()) {}
  }
}

void check_restart()
{
	int ifile;
    // Does RESTART exist?  If it does, then tcomm has been restarted, and
    //  we need to open/reopen files
	ifile = open(restart,O_RDONLY);
	if (ifile >= 0){
	  close(ifile);
fprintf(stderr,"closing files\n");
	  if (rfile != NULL) fclose(rfile);
	  if (sfile != NULL) fclose(sfile);
	  if (cfile != NULL) fclose(cfile);
	  if (ffile != NULL) fclose(ffile);
	  ropen = FALSE;
	  statopen = FALSE;
	  copen = FALSE;
	  flushopen = FALSE;
	  remove(restart);
	}
}

