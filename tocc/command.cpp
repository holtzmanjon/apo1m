//****************************************************************************/
/*                                                        */
/*  Module:    command.cpp                               */
/*                                                        */
/*  Purpose:  command program loop                           */
/*                                                        */
/****************************************************************************/
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <dos.h>
#include "key_util.h"
#include "guiding.h"
#include "inst.h"
#include "tracking.h"
#include "mytype.h"
#include "pcx.h"
#include "cp4016.h"
#include "tcs.h"
#include "tcs_math.h"
#include "ocs.h"
#include "globals.h"
#include "slamac.h"
#include "slalib.h"
#include "io.h"
#include "shutdown.h"
#include "status.h"
#include "systimer.h"
#include "weather.h"
#include "filter.h"
#include "tertiary.h"
#include "guider.h"

// Function prototypes
int getsao(double,double,double,double,double,
  double *,double *,long *,double *,double,long *,int);
int getsaved(double &, double &, double &);
void gethms(double, int &, int &, double &, int &);
void display_menu();
int telescope_startup();
void vcopy(char *, char *, unsigned char);
BOOL illegal_ra(double);
BOOL illegal_dec(double);
BOOL illegal_az(double);
BOOL illegal_alt(double);

BOOL command_mode = TRUE;
BOOL remote_on = TRUE;
BOOL remote_command = FALSE;

// Global variables
struct text_info tinfo;
int status_update_rate = SYSTMR_STATUS_INC;
#define DISPHEIGHT 9

// saved coordinate list
#define MAXSAVE 5
int isave;
double savedra[MAXSAVE] = {0.,0.,0.,0.,0.};
double saveddec[MAXSAVE] = {0.,0.,0.,0.,0.};
double savedepoch[MAXSAVE] = {0.,0.,0.,0.,0.};

void process_command()
{
// local variables
  double current_epoch = 1950.;
  unsigned char ans[2],command[MAXCMD];
  unsigned char inputline[MAXCMD],commandline[MAXCMD];
  double coord, ra, dec, pmra, pmdec, epoch;
  double parallax, eff_wavelength, radial_velocity, tlr, newpa;
  int status, nread;
  double saora, saodec; 
  struct WEATHERPACK weather;

  BOOL pointing_output = FALSE;
  FILE *pointing_file;

  BOOL priv = TRUE;
  UPSPACK ups;

// Initialize
  clrscr();
  status = telescope_startup();
  error_code(status);
 
// set the terminal window
  gettextinfo(&tinfo);
  window(1,DISPHEIGHT+1,tinfo.screenwidth,tinfo.screenheight);
  clrscr();

// start up the display updating
  do_display = TRUE;

#ifdef NOREMOTE
// print a help menu
  command_help();
#endif

  while (command_mode) {

//  Default status is OK
    status = TCSERR_OK ;

    sprintf(outbuf,"Command: ");
    writeline(outbuf,0);
//  Command-mode commands: these are not acted upon until a CR is received
    if ( getline(commandline,sizeof(commandline)) != 0) {
      status = TCSERR_COMIO;
    } else
    {

//  Parse out the first word in case parameters are on the input line
    command[0] = '\0';
    sscanf(commandline,"%s",command);

    if (strlen(command) > 1) {

//  Log the command to the moves file if we're in debug option
    writeline(command,2);
    writelog(command,15);

//  Convert to upper case for command 
    strupr(command);

    // Help list
    if (strcmp(command,"HP")==0) {
      command_help();
    }

    else if (strcmp(command,"RT")==0) {
      remote_on = !remote_on ;
    }

    else if (strcmp(command,"VERBOSE")==0) {
      G->verbose = !G->verbose ;
    }

    // Coordinate move to a desired RA/DEC
    else if (strcmp(command,"CO")==0 || strcmp(command,"HA") ==0) {
      sprintf(outbuf,"  Using epoch : %.1f\r\n",current_epoch);
      writeline(outbuf,0);
      sprintf(outbuf,"    (Use NE command to change)\r\n\r\n");
      writeline(outbuf,0);
      if (strcmp(command,"HA")==0) {
        do {
		      getcoord("    HA\0",&ra);
        } while (0);
        ra = G->current_lasth - ra;
      } else {
        do {
          getcoord("    RA\0",&ra);
	} while (illegal_ra(ra));
      } 

      do {
        getcoord("    DEC\0",&dec);
      } while (illegal_dec(dec));

      sprintf(outbuf,
		  "  Proposed move to: \r\n"
		  "     RA:  %lf \r\n"
		  "     DEC: %lf \r\n",ra,dec);
      writeline(outbuf,0);
      sprintf(outbuf,"  Hit <CR> to move, anything else to abort: ");
      writeline(outbuf,0);
      if (getline(ans,sizeof(ans))>=0 && ans[0]==0) {
        zero_guide_correction();
        ra *= DH2R;
        dec *= DD2R;
        pmra = 0;
        pmdec = 0;
        status = tcs_move_to_coordinates(
			ra,dec,current_epoch,pmra,pmdec,0.,0.,0.55,0.0065,0.);
        // store this position in list of saved positions
        if (status==TCSERR_OK) {
          savedra[isave] = ra * DR2H;
          saveddec[isave] = dec * DR2D;
          savedepoch[isave] = current_epoch;
          isave = (++isave) % MAXSAVE;
        }
      }
    }

    // Change the current instrument position angle
    else if (strcmp(command,"PA")==0) {
      sprintf(outbuf,"Enter desired PA: ");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      double pa;
      sscanf(inputline,"%lf",&pa);
      // Now move the telescope
      status = tcs_move_to_coordinates(
			   G->current_mean_ra,G->current_mean_dec,
			   G->current_mean_epoch,G->current_mean_pmra,
			   G->current_mean_pmdec,G->current_mean_parallax,
			   G->current_mean_radial_velocity,
			   G->current_mean_eff_wavelength,G->current_tlr,pa);
    }

    // Change the current instrument rotator angle
    else if (strcmp(command,"ROT")==0) {
      double desired_rot, az, alt, rot;

      sprintf(outbuf,"Enter desired rotator angle: ");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%lf",&desired_rot);
      desired_rot *= DD2R ;
      struct mytime t;
      struct date d;
      struct ALLTIMES timeRec;
      mygettime(&d, &t);
      get_all_times_at(d, t, timeRec);
      // Now get the rotator angle for current coords
      double pa, parang;
      mean_to_mount_corrected_sub(timeRec,
			   G->current_mean_ra,G->current_mean_dec,
			   G->current_mean_epoch,G->current_mean_pmra,
			   G->current_mean_pmdec,G->current_mean_parallax,
			   G->current_mean_radial_velocity,
			   az,alt,rot,parang);
      pa = G->current_pa + (desired_rot - rot);
      pa *= DR2D;
      // Now move the telescope
      status = tcs_move_to_coordinates(
			   G->current_mean_ra,G->current_mean_dec,
			   G->current_mean_epoch,G->current_mean_pmra,
			   G->current_mean_pmdec,G->current_mean_parallax,
			   G->current_mean_radial_velocity,
			   G->current_mean_eff_wavelength,G->current_tlr,pa);
    }

    // Get SAO star and move to it
    else if (strcmp(command,"SA")==0 || strcmp(command,"ALTAZ")==0 ) {

      double outra, outdec, outv, vmin, vmax, epoch;
      long outid, oldid[10];
      int istat, nold;

      if (strcmp(command,"SA")==0) {
	sprintf(outbuf, "Find star near current telescope position (T),"
		    " other ra/dec (O), other alt/az (A): ");
        writeline(outbuf,0);
        getline(ans,sizeof(ans));
        sprintf(outbuf,"\r\n");
        writeline(outbuf,0);
      }
      if (strcmp(command,"ALTAZ")==0 || ans[0]=='A' || ans[0]=='a'){
        double alt, az;
        do {
          getcoord("  Desired AZ\0",&az);
        } while (illegal_az(az));
 
        do {
	  getcoord("  Desired ALT\0",&alt);
        } while (illegal_alt(alt));

    // Convert the current alt-az to ra and dec and load into variables
        struct mytime t;
        struct date d;
        struct ALLTIMES timeRec;
        mygettime(&d, &t);
        // getdate(&d);
        get_all_times_at(d, t, timeRec);

        slaDh2e(az*DD2R, alt*DD2R, G->latitude, &ra, &dec);
        ra = slaDranrm(timeRec.last - ra);
        slaPreces("FK4",timeRec.equinox,1950.,&ra,&dec);
        ra *= DR2H ;
		      dec *= DR2D ;
      } else if (ans[0]=='T' || ans[0]=='t') {
        ra = G->current_mean_ra;
        dec = G->current_mean_dec;
        ra *= DR2H ;
        dec *= DR2D ;
      } else {
        do {
          getcoord("  Desired RA\0",&ra);
        } while (illegal_ra(ra));

	do {
	  getcoord("  Desired DEC\0",&dec);
	} while (illegal_dec(dec));
      }
      if (strcmp(command,"ALTAZ")==0)  {
	ra *= DH2R;
        dec *= DD2R;
        pmra = 0;
        pmdec = 0;
        zero_guide_correction();
        status = tcs_move_to_coordinates(
		ra, dec, 1950., pmra, pmdec, 0., 0., 0.55, 0.0065, 0.);
      } else {
        sprintf(outbuf, "    Enter min, max magnitude (<cr> for all): ");
        writeline(outbuf,0);
        getline(inputline,sizeof(inputline));
        vmin = -1;
        vmax = 99;
        sscanf(inputline,"%lf %lf",&vmin,&vmax);

        ans[0] = 'a';
        nold = 0;
        while (ans[0]=='a' || ans[0]=='A' && ans[0] != 'q' && ans[0] != 'Q') {
          G->current_mean_epoch = 1950.;
          istat=getsao(ra,dec,G->current_mean_epoch,vmin,vmax,
			  &outra,&outdec,&outid,&outv,10.0,oldid,nold);
          if (istat != 0) {
	    sprintf(outbuf,"error opening sao file\r\n");
	    writeline(outbuf,0);
	    ans[0] = 'q';
	  } else if (outid<0) {
	    sprintf(outbuf,"No star found in specified range\r\n");
	    writeline(outbuf,0);
	    ans[0] = 'q';
	  } else {
	    sprintf(outbuf,
	      "  Proposed move to SAO star: %ld \r\n"
	      "      RA: %lf \r\n"
	      "     DEC: %lf \r\n"
	      "       V: %lf \r\n",outid,outra,outdec,outv);
	    writeline(outbuf,0);
	    sprintf(outbuf,"\n Hit <CR> to move, " "A to select another star, "
			     "anything else to abort: ");
            writeline(outbuf,0);
	    if (getline(ans,sizeof(ans))>=0 && ans[0]==0) {
	      zero_guide_correction();
	      outra *= DH2R;
	      outdec *= DD2R;
	      pmra = 0;
	      pmdec = 0;
	      status = tcs_move_to_coordinates(
		outra, outdec, 1950., pmra, pmdec, 0., 0., 0.55, 0.0065, 0.);
              // store this position in list of saved positions
              if (status==TCSERR_OK) {
		savedra[isave] = outra * DR2H;
                saveddec[isave] = outdec * DR2D;
                savedepoch[isave] = 1950.;
                isave = (++isave) % MAXSAVE;
              }
            } else if (ans[0]=='A' || ans[0] == 'a' && nold<9) {
	      oldid[nold++] = outid;
            } else
	      ans[0] = 'q'; 
          }
	}
     }
   }                      


    // Previous move
    else if (strcmp(command,"PM")==0) {
      if (getsaved(ra,dec,epoch) > 0) {
        sprintf(outbuf,
		    "  Proposed move to: \r\n"
		    "     RA:  %f \r\n"
		    "     DEC: %f \r\n"
		    "     EPOCH: %f \r\n",ra,dec,epoch);
        writeline(outbuf,0);
        sprintf(outbuf, "  Hit <CR> to move, anything else to abort: ");
        writeline(outbuf,0);
        if (getline(ans,sizeof(ans))>=0 && ans[0]==0) {
          zero_guide_correction();
          ra *= DH2R;
          dec *= DD2R;
          pmra = 0;
          pmdec = 0;
          status = tcs_move_to_coordinates(
  			  ra, dec, epoch, pmra, pmdec, 0., 0., 0.55, 0.0065, 0.);
          if (status==TCSERR_OK) {
	    savedra[isave] = ra * DR2H;
	    saveddec[isave] = dec * DR2D;
	    savedepoch[isave] = epoch;
	    isave = (++isave) % MAXSAVE;
          }
        }
      }
    }

    else if (strcmp(command,"RATES")==0) {
      double rarate, decrate;
      // Enter rates in arcsec/hr
      sscanf(commandline+5,"%lf %lf",&rarate,&decrate);
      G->dra = rarate*DAS2R/cos(G->current_mean_dec)*24;
      G->ddec = decrate*DAS2R*24;
      struct mytime t;
      struct date d;
      struct ALLTIMES timeRec;
      mygettime(&d, &t);
      get_all_times_at(d, t, timeRec);
      G->epoch0 = timeRec.mjd_utc;
      sprintf(outbuf,"rarate: %f decrate %f G->dra: %f G->ddec: %f G->epoch0: %f",rarate,decrate,G->dra, G->ddec, G->epoch0);
      writeline(outbuf,1);
      G->apply_rates = TRUE;
    }

    // Guide parameters
    else if (strcmp(command,"GUIDE")==0) {
      double rasec, decsec, dra, ddec;
      sscanf(commandline+5,"%lf %lf",&rasec,&decsec);

      dra = rasec / 15. / cos(G->current_mean_dec);
      dra *= DS2R;
      ddec = decsec*DAS2R;
      update_guide_correction(dra, ddec);
      status = TCSERR_OK;
    }

    else if (strcmp(command,"GUIDEINST")==0) {
      int inst;
      double dx, dy;
      sscanf(commandline+9,"%d %lf %lf",&inst, &dx,&dy);
      update_guide_correction_inst(inst, dx, dy);
      status = TCSERR_OK;
    }

    else if (strcmp(command,"GUIDEAA")==0 || strcmp(command,"DAA")==0) {
      double azsec, altsec, daz, dalt;
      if (strcmp(command,"GUIDEAA") == 0 )
        sscanf(commandline+7,"%lf %lf",&azsec,&altsec);
      else
        sscanf(commandline+3,"%lf %lf",&azsec,&altsec);

      daz = azsec / cos(G->current_obs_alt*DD2R);
      daz *= DAS2R;
      dalt = altsec*DAS2R;
      update_guide_correction_altaz(daz, dalt);
      status = TCSERR_OK;
    }

    else if (strcmp(command,"SETINST")==0) {
      int inst;
      double xc, yc, sx, sy, rot,rot0;
      BOOL fixed;
      sscanf(commandline+7,"%d %lf %lf %lf %lf %lf %lf %u", &inst, &sx, &sy, &xc, &yc, &rot,&rot0,&fixed);
      update_instrument_correction(inst, sx, sy, xc, yc, rot, rot0, fixed);
    }

    else if (strcmp(command,"INST")==0) {
		sscanf(commandline+4,"%d",&G->current_inst);
      int cur_inst = G->current_inst;
      double theta = G->current_pa;
      double dra =  
        +sysGlobal->xc[cur_inst]*cos(theta) -sysGlobal->yc[cur_inst]*sin(theta);
      double ddec = 
        -sysGlobal->xc[cur_inst]*sin(theta) -sysGlobal->yc[cur_inst]*cos(theta);
      sprintf(outbuf,"inst cneter: %d %12.3f %12.3f %12.3f %12.3f",
	  cur_inst,sysGlobal->xc[cur_inst], sysGlobal->yc[cur_inst], dra, ddec);
      writeline(outbuf,1);

    }

    // Instrument plane offset
    else if (strcmp(command,"OFFSET")==0) {
      int inst;
      double dx, dy;
      sscanf(commandline+6,"%d %lf %lf", &inst, &dx, &dy);
      if (inst<0 || inst>MAXINST) {
        sprintf(outbuf,"Instrument out of range");
        writeline(outbuf,1);
        status = -1;
      } else if (fabs(dx) > 5000 || fabs(dy) > 5000) {
        sprintf(outbuf,"Offset out of range");
        writeline(outbuf,1);
        status = -1;
      } else
        apply_instrument_offset(inst,dx,dy); 
    }

    // Relative focus command
    else if (strcmp(command,"DF")==0) {
      sprintf(outbuf,"Enter delta focus to apply (steps): ");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      long dfoc;
      sscanf(inputline,"%ld",&dfoc);
      status = tcs_move_secondary_steps(dfoc,dfoc,dfoc);
    }

    // Move secondary to home position
    else if (strcmp(command,"SHO")==0 && priv) {
      status = tcs_home_secondary();
    }

    // New equinox
    else if (strcmp(command,"NE")==0) {
      sprintf(outbuf,
		 "Current epoch for coordinates: %f\r\n"
		 "Enter new epoch for coordinates: ",current_epoch);
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%lf",&current_epoch);
      status = TCSERR_OK ;
    }

    // Update coordinates
    else if (strcmp(command,"UC")==0 || (strcmp(command,"UH")==0 && priv) ) {
      if (!G->tracking_on) {
        sprintf(outbuf,"Tracking is OFF!"
			    " Turn it on before updating coords");
        writeline(outbuf,0);
      } else {
        sprintf(outbuf,
	" Enter desired position to update to from %d last moves,\r\n"
	" or 0 to enter a new position manually\r\n\n", MAXSAVE);
        writeline(outbuf,0);
        int iret;
        if ( (iret = getsaved(ra,dec,epoch)) >= 0) {
          if (iret > 0) {
            ra *= DH2R;
            dec *= DD2R;
          } else {
            sprintf(outbuf, "  Using epoch : %.1f\r\n"
			"    (Use NE command to change)\r\n\r\n",current_epoch);
            writeline(outbuf,0);
            epoch = current_epoch;
            do   {
              getcoord(" Current RA\0",&ra);
            } while (illegal_ra(ra));
  
            do {
              getcoord(" Current DEC\0",&dec);
            } while (illegal_dec(dec));
            ra *= DH2R;
            dec *= DD2R;
          }
          pmra = 0;
          pmdec = 0;
          // Set the current coordinates
          zero_guide_correction();
          if (strcmp(command,"UC") == 0) 
	    status = tcs_mark_coordinates(
			   ra,dec,epoch,pmra,pmdec,0.,0.,0.55,0.0065,TRUE);
          else {
	    sprintf(outbuf, "\r\nEnter (Y) to confirm that you really want "
			 "to change the home positions!"); 
            writeline(outbuf,0);
            getline(ans,sizeof(ans));
            if (ans[0]=='Y' || ans[0] == 'y') {
              status = tcs_mark_coordinates(
			   ra,dec,epoch,pmra,pmdec,0.,0.,0.55,0.0065,FALSE);
            } else {
	      sprintf(outbuf,"\r\nTelescope home positions have NOT been changed");
              writeline(outbuf,0);
            }
          }
          update_tracking_rates(TRUE);
        }
      } 
    }

    // Update the current position angle
    else if (strcmp(command,"UR")==0) {
      double drot;
      sscanf(commandline+2,"%lf",&drot);
      G->ref_rot += drot ;
    }

    else if (strcmp(command,"UP")==0) {
      sprintf(outbuf,"Current PA reading: %f\r\n",G->current_pa*DR2D);
      writeline(outbuf,0);
      double old_pa = G->current_pa;
      sprintf(outbuf,"Enter correct PA: ");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%lf",&G->current_pa);
      G->current_pa *= DD2R ;
      // Update the reference rotation angle to correct for
      //   the proper PA
      G->ref_rot = G->ref_rot + (G->current_pa - old_pa)*DR2D ; 
      status = TCSERR_OK;
    }

    // Toggle tracking
    else if (strcmp(command,"TR")==0) {
      if (G->tracking_on) {
		     tcs_telescope_stop();
		     G->tracking_on = !G->tracking_on;
		     sprintf(outbuf,"Tracking now OFF\r\n");
      } else {
		     // Set current position to be position to track to
		     update_telescope_position();
		     G->current_mean_ra = G->current_obs_ra;
		     G->current_mean_dec = G->current_obs_dec;
		     G->tracking_on = !G->tracking_on;
		     update_tracking_rates(TRUE);
		     sprintf(outbuf,"Tracking now ON\r\n");
      }
      writeline(outbuf,0);
      status = TCSERR_OK;
    }

    else if (strcmp(command,"YSEP") == 0) {
      G->y_axis_separate = !G->y_axis_separate;
      sprintf(outbuf,"y_axis_separate: %d\n",G->y_axis_separate);
      writeline(outbuf,1);
    }

    else if (strcmp(command,"ZENABLE") == 0) {
      sysGlobal->z_axis_enabled = 1;
      writesysscf(sysGlobal);
    }

    else if (strcmp(command,"ZDISABLE") == 0) {
      sysGlobal->z_axis_enabled = 0;
      writesysscf(sysGlobal);
    }

    else if (strcmp(command,"+ZENCODER") == 0) {
      sysGlobal->z_encoder_installed = 1;
    }

    else if (strcmp(command,"-ZENCODER") == 0) {
      sysGlobal->z_encoder_installed = 0;
    }

    // Toggle full use of encoders for pointing
    else if (strcmp(command,"EP")==0 && priv) {
      G->use_encoders = !G->use_encoders;
      status = TCSERR_OK;
    }

    // Set use of encoders for tracking
    else if (strcmp(command,"ET")==0) {
      int axis, npts;
      double error;
      if (sscanf(commandline+2,"%d %d %lf",&axis,&npts,&error) == 3) {
	if (axis == 1) {
          G->x_encoder_tracking = npts;
          G->x_encoder_error = error;
        } else if (axis==2) {
          G->y_encoder_tracking = npts;
          G->y_encoder_error = error;
        } else if (axis==3) {
          G->z_encoder_tracking = npts;
          G->z_encoder_error = error;
        } 
      } else if (sscanf(commandline+2,"%d %d",&axis,&npts) == 2) {
	if (axis == 1) G->x_encoder_tracking = npts;
	else if (axis == 2) G->y_encoder_tracking = npts;
	else if (axis == 3) G->z_encoder_tracking = npts;
      } else if (sscanf(commandline+2,"%d",&npts) == 1) {
	G->x_encoder_tracking = npts;
	G->y_encoder_tracking = npts;
	G->z_encoder_tracking = npts;
      } else {
        sprintf(outbuf,"Usage: ET [axis] npts");
        writeline(outbuf,1);
      }
      status = TCSERR_OK;
    }

    // Adjust the tracking "delta" time
    else if (strcmp(command,"TD")==0 && priv) {
      sprintf(outbuf,"Enter new tracking dtime :");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%d",&G->tracking_dtime);
    }
    else if (strcmp(command,"MAXRATE")==0 && priv) {
      sprintf(outbuf,"Enter new maximum fractional rate change :");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%lf",&G->max_rate_change);
    }

    else if (strcmp(command,"TC")==0 && priv) {
      G->const_tracking = !G->const_tracking;
    }

    // Adjust the tracking rate factors
    else if (strcmp(command,"TF")==0 && priv) {
      sprintf(outbuf,"Enter new tracking factor :");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%d",&G->x_tracking_factor);
      G->y_tracking_factor = G->x_tracking_factor;
      G->z_tracking_factor = G->x_tracking_factor;
    }

    else if (strcmp(command,"IOSERVER")==0) {
      strcpy(ioserver,commandline+9);
    }

    else if (strcmp(command,"SERVER")==0) {
      int ret;
      ret = setup_server(&command_serv, 1050);
      if (ret!=0) {
        sprintf(outbuf,"setup_server returns: %d",ret); 
        writeline(outbuf,1);
      }
    }

    else if (strcmp(command,"ALIVE")==0) {
        SysTimer[SYSTMR_NETWORK_LOST].NewTimerSecs(30);
    }

    // Send a command directly to the PC38
    else if (strcmp(command,"PC")==0 && priv) {
#ifdef SOCKET
      strcpy(inputline,commandline+3);
      writeline(inputline,1);
#else
      sprintf(outbuf,"Enter command to send to PC38:");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
#endif
      return_position(PC38,'a',inputline,1);
      //pc38_send_commands(inputline);
      status = TCSERR_OK;
    }

    else if (strcmp(command,"PC34")==0 && priv) {
#ifdef SOCKET
      strcpy(inputline,commandline+5);
      writeline(inputline,1);
#else
      sprintf(outbuf,"Enter command to send to PC34:");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
#endif
      pc34_send_commands(inputline);
      status = TCSERR_OK;
    }

    else if ((strcmp(command,"GUIDERPC38")==0 || strcmp(command,"GPC")==0) && priv) {
#ifdef SOCKET
      if (strcmp(command,"GPC")==0)
        strcpy(inputline,commandline+4);
      else
        strcpy(inputline,commandline+11);
      writeline(inputline,1);
#else
      sprintf(outbuf,"Enter command to send to guider PC38:");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
#endif
      pcx_send_commands(GUIDERPC38,inputline);
      status = TCSERR_OK;
    }

    // Read guider PC38 status
    else if (strcmp(command,"PCREAD")==0) {
      AXISPACK axisRec;

      sprintf(outbuf,"telescope PC38:");
      sprintf(outbuf+strlen(outbuf),"x returns: %ld\n", 
                          return_position(PC38,'x',"re\0",0));
      sprintf(outbuf+strlen(outbuf),"y returns: %ld\n",
                          return_position(PC38,'y',"re\0",0));
      sprintf(outbuf+strlen(outbuf),"z returns: %ld\n",
                          return_position(PC38,'z',"rp\0",0));
      sprintf(outbuf+strlen(outbuf),"t returns: %ld\n",
                          return_position(PC38,'t',"rp\0",0));
      sprintf(outbuf+strlen(outbuf),"u returns: %ld\n",
                          return_position(PC38,'u',"rp\0",0));
      sprintf(outbuf+strlen(outbuf),"v returns: %ld\n",
                          return_position(PC38,'v',"rp\0",0));
      sprintf(outbuf+strlen(outbuf),"output bits returns: %x\n",
                          pcx_read_inputs(PC38));
      pcx_get_axis_status(PC38,'x', axisRec);
      sprintf(outbuf+strlen(outbuf),"x limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(PC38,'y', axisRec);
      sprintf(outbuf+strlen(outbuf),"y limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(PC38,'z', axisRec);
      sprintf(outbuf+strlen(outbuf),"z limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(PC38,'t', axisRec);
      sprintf(outbuf+strlen(outbuf),"t limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(PC38,'u', axisRec);
      sprintf(outbuf+strlen(outbuf),"u limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(PC38,'v', axisRec);
      sprintf(outbuf+strlen(outbuf),"v limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      writeline(outbuf,0);
      sprintf(outbuf,"guider PC38:");
      sprintf(outbuf+strlen(outbuf),"x returns: %ld\n", 
                          return_position(GUIDERPC38,'x',"rp",0));
      sprintf(outbuf+strlen(outbuf),"y returns: %ld\n",
                          return_position(GUIDERPC38,'y',"rp",0));
      sprintf(outbuf+strlen(outbuf),"z returns: %ld\n",
                          return_position(GUIDERPC38,'z',"rp",0));
      sprintf(outbuf+strlen(outbuf),"t returns: %ld\n",
                          return_position(GUIDERPC38,'t',"rp",0));
      sprintf(outbuf+strlen(outbuf),"u returns: %ld\n",
                          return_position(GUIDERPC38,'u',"rp",0));
      sprintf(outbuf+strlen(outbuf),"v returns: %ld\n",
                          return_position(GUIDERPC38,'v',"rp",0));
      sprintf(outbuf+strlen(outbuf),"output bits returns: %x\n",
                          pcx_read_inputs(GUIDERPC38));
      sprintf(outbuf+strlen(outbuf),"output bits returns: %u\n",
                          pcx_read_inputs(GUIDERPC38));
      pcx_get_axis_status(GUIDERPC38,'x', axisRec);
      sprintf(outbuf+strlen(outbuf),"x limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(GUIDERPC38,'y', axisRec);
      sprintf(outbuf+strlen(outbuf),"y limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(GUIDERPC38,'z', axisRec);
      sprintf(outbuf+strlen(outbuf),"z limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      pcx_get_axis_status(GUIDERPC38,'t', axisRec);
      sprintf(outbuf+strlen(outbuf),"t limit status: %d   home: %d\n",
                          axisRec.limit,axisRec.home);
      sprintf(outbuf+strlen(outbuf),"is_detent_in: %u\n",
                          is_detent_in());
      sprintf(outbuf+strlen(outbuf),"is_detent_out: %u\n",
                          is_detent_out());
      sprintf(outbuf+strlen(outbuf),"is_brake_in: %u\n",
                          is_brake_in());
      sprintf(outbuf+strlen(outbuf),"is_brake_out: %u\n",
                          is_brake_out());
      sprintf(outbuf+strlen(outbuf),"filter position: %d\n",
                          get_filter_pos());
      sprintf(outbuf+strlen(outbuf),"PC34 output bits returns: %x\n",
                          pcx_read_inputs(PC34));
      writeline(outbuf,0);

     // sprintf(outbuf,"t encoder: %d\n",return_encoder_position('t'));
     // writeline(outbuf,0);
    }


    else if (strcmp(command,"DAZ")==0 && priv) {
      double degrees;
#ifdef SOCKET
      sscanf(commandline+3,"%lf",&degrees);
#else
      sprintf(outbuf, 
        "Enter amount to move in az (in degrees, positive = CW from above):");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%lf",&degrees);
#endif
      sprintf(inputline,"AX MR%ld GO;", (long)(-degrees*sysGlobal->x_steps_degree));
      writeline(inputline,1);
#ifdef SOCKET
      pc38_send_commands(inputline);
#else
      sprintf(outbuf,"Proceed to move (Y/N)?");
      writeline(outbuf,0);
      getline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0] == 'y') pc38_send_commands(inputline);
#endif
      status = TCSERR_OK;
    }

    else if (strcmp(command,"DALT")==0 && priv) {
      double degrees;
#ifdef SOCKET
      sscanf(commandline+4,"%lf",&degrees);
#else
      sprintf(outbuf, "Enter amount to move in alt (in degrees)");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%lf",&degrees);
#endif
      sprintf(inputline,"AY MR%ld GO;", (long)(degrees*sysGlobal->y_steps_degree));
      writeline(inputline,1);
#ifdef SOCKET
      pc38_send_commands(inputline);
#else
      sprintf(outbuf,"Proceed to move (Y/N)?");
      writeline(outbuf,0);
      getline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0] == 'y') pc38_send_commands(inputline);
#endif
      status = TCSERR_OK;
    }

    else if (strcmp(command,"DROT")==0 && priv) {
      double degrees;
#ifdef SOCKET
      sscanf(commandline+4,"%lf",&degrees);
#else
      sprintf(outbuf, "Enter amount to move in rot (in degrees, positive = CW):");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      sscanf(inputline,"%lf",&degrees);
#endif
      sprintf(inputline,"AZ MR%ld GO;", (long)(-degrees*sysGlobal->z_steps_degree));
      writeline(inputline,1);
#ifdef SOCKET
      pc38_send_commands(inputline);
#else
      sprintf(outbuf,"Proceed to move (Y/N)?");
      writeline(outbuf,0);
      getline(ans,sizeof(ans));
      if (ans[0]=='Y' || ans[0] == 'y') pc38_send_commands(inputline);
#endif
      status = TCSERR_OK;
    }

    // Open the debug output files with append status
    else if (strcmp(command,"OF")==0) {
      #ifdef debug_move_file
      sprintf(outbuf,"Opening e:\\tocc\\moves.dbx ...");
      writeline(outbuf,0);
      G->move_file = fopen("e:\\tocc\\moves.dbx", "a");
      if (G->move_file == NULL) {
        sprintf(outbuf,"Error opening moves.dbx");
        writeline(outbuf,0);
      }
      #endif
      #ifdef debug_tracking_file
      G->track_file = fopen("e:\\tocc\\track.dbx", "a");
      #endif
      #ifdef debug_pcx_file
      G->pcx_file = fopen("e:\\tocc\\pcx.dbx", "a");
      #endif
      #ifdef have_debug_file
      G->debug_file = fopen("e:\\tocc\\debug.dbx", "a");
      #endif
      
      status = TCSERR_OK;
    }

    // Close the debug output files so they can be read
    else if (strcmp(command,"CF")==0) {
      #ifdef debug_move_file
      fclose(G->move_file);
      #endif
      #ifdef debug_tracking_file
      fclose(G->track_file);
      #endif
      #ifdef debug_pcx_file
      fclose(G->pcx_file);
      #endif
      #ifdef have_debug_file
      fclose(G->debug_file);
      G->debug_file = NULL;
      #endif
      status = TCSERR_OK;
    }

    // Toggle pointing output
    else if (strcmp(command,"PO")==0) {
      if (pointing_output)
        fclose(pointing_file);
      else
        pointing_file = fopen("pointing.dat", "a");

      pointing_output = !pointing_output;
    }
	 
    // calibrate azimuth axis
    else if (strcmp(command,"CAZ")==0) {
      status = tcs_calibrate_steps_degree('x');
    }
    else if (strcmp(command,"AZSCALE")==0) {
      double steps_deg;
      sscanf(commandline+7,"%lf",&steps_deg);
      sysGlobal->x_encoder_encoder_steps_deg = steps_deg;
      sysGlobal->x_steps_degree = sysGlobal->x_encoder_encoder_steps_deg *
                   sysGlobal->x_encoder_motor_steps_rev/
                   sysGlobal->x_encoder_encoder_steps_rev;
      writesysscf(sysGlobal);
      sprintf(outbuf,"Calibrate azimuth: %lf %lf %lf %lf %lf",
             sysGlobal->x_encoder_encoder_steps_deg, steps_deg,
             G->current_out_temp,G->current_cab_temp,G->current_aux_temp);
      writelog(outbuf,8);
      status = TCSERR_OK;
    }

    else if (strcmp(command,"ALTSCALE")==0) {
      double steps_deg;
      sscanf(commandline+8,"%lf",&steps_deg);
      sysGlobal->y_encoder_encoder_steps_deg = steps_deg;
      sysGlobal->y_steps_degree = sysGlobal->y_encoder_encoder_steps_deg *
                   sysGlobal->y_encoder_motor_steps_rev/
                   sysGlobal->y_encoder_encoder_steps_rev;
      writesysscf(sysGlobal);
      status = TCSERR_OK;
    }
    else if (strcmp(command,"SCALE")==0) {
      double scale;
      sscanf(commandline+5,"%lf",&scale);
      sysGlobal->x_encoder_encoder_steps_deg *= scale;
      sysGlobal->y_encoder_encoder_steps_deg *= scale;
      sysGlobal->x_steps_degree = sysGlobal->x_encoder_encoder_steps_deg *
                   sysGlobal->x_encoder_motor_steps_rev/
                   sysGlobal->x_encoder_encoder_steps_rev;
      sysGlobal->y_steps_degree = sysGlobal->y_encoder_encoder_steps_deg *
                   sysGlobal->y_encoder_motor_steps_rev/
                   sysGlobal->y_encoder_encoder_steps_rev;
      writesysscf(sysGlobal);
      sprintf(outbuf,"New scales: az: %lf  alt: %lf \n",
           sysGlobal->x_encoder_encoder_steps_deg,
           sysGlobal->y_encoder_encoder_steps_deg);
      writeline(outbuf,1);
      status = TCSERR_OK;
    }
    // calibrate rotator axis
    else if (strcmp(command,"CRO")==0) {
      status = tcs_calibrate_steps_degree('z');
    }
    else if (strcmp(command,"ROTSCALE")==0) {
      double steps_deg;
      sscanf(commandline+8,"%lf",&steps_deg);
      sprintf(outbuf,"Calibrate rotator: %lf %lf %lf %lf %lf",
             sysGlobal->z_steps_degree, steps_deg,
             G->current_out_temp,G->current_cab_temp,G->current_aux_temp);
      writelog(outbuf,9);
      sysGlobal->z_steps_degree = steps_deg;
      writesysscf(sysGlobal);
      status = TCSERR_OK;
    }

    // setup various variables
    else if (strcmp(command,"SETUP")==0 && priv) {
      status = setup_scf();
    }

    else if (strcmp(command,"READSCF")==0 && priv) {
      fprintf(stderr,"readtoccscf...\n");
      readtoccscf(&autoGlobalNew);
      G->ref_alt = autoGlobal->home_alt;
      G->ref_az = autoGlobal->home_az;
      G->ref_rot = autoGlobal->home_rot;
      G->mc_enabled = autoGlobal->mc_enabled;
      if (autoGlobal->mc_enabled) 
        status = read_mount_correction_model(autoGlobal->mc_file);
      fprintf(stderr,"readsysscf...\n");
      readsysscf(&sysGlobalNew);
      fprintf(stderr,"readgcsscf...\n");
      readgcsscf(&gcsGlobalNew);
    }

#ifdef NOTDEF
    // edit autoGlobal variable
    else if (strcmp(command,"TOCCSCF")==0 && priv) {
      status = edit_toccscf();
    }

    // edit sysGlobal variable
    else if (strcmp(command,"SYSSCF3")==0 && priv) {
      status = edit_sysscf3();
    }

    // edit sysGlobal variable
    else if (strcmp(command,"SYSSCF2")==0 && priv) {
      status = edit_sysscf2();
    }

    // edit sysGlobal variable
    else if (strcmp(command,"SYSSCF")==0 && priv) {
      status = edit_sysscf();
    }
#endif
    // initialize the telescope if we already have known position
    else if (strcmp(command,"SAVED")==0) {
      sprintf(outbuf,"saved positions: %lf %lf %lf",
         autoGlobal->saved_az, autoGlobal->saved_alt,autoGlobal->saved_rot);
      writeline(outbuf,1);
      sscanf(commandline+5,"%lf%lf%lf",
         &autoGlobal->saved_az, &autoGlobal->saved_alt,&autoGlobal->saved_rot);
      sprintf(outbuf,"saved positions: %lf %lf %lf",
         autoGlobal->saved_az, autoGlobal->saved_alt,autoGlobal->saved_rot);
      writeline(outbuf,1);
    }

    else if (strcmp(command,"IN")==0) {
      status = tcs_home_telescope();
    }

    // home (initialize) the telescope by moving to home switches
    else if (strcmp(command,"HO")==0) {
      autoGlobal->saved_az = -9999;
      autoGlobal->saved_alt = -9999;
      status = tcs_home_telescope();
    }

    else if (strcmp(command,"DEINIT")==0 || strcmp(command,"REINIT")==0 ) {
      G->telescope_session_init = FALSE;
      autoGlobal->saved_az = -9999;
      autoGlobal->saved_alt = -9999;
      autoGlobal->saved_rot = -9999;
      autoGlobal->z_axis_pos = 0;
      status = tcs_home_telescope();
    }

    else if (strcmp(command,"+LIMITS")==0 || strcmp(command,"-LIMITS")==0 ) {
      if (strcmp(command,"+LIMITS")==0)
       G->soft_limits_disabled=0;
      else
       G->soft_limits_disabled=1;
    }
/*
    else if (strcmp(command,"HOMESTATUS")==0) {
      HOMESTATUSPACK homeRec;
      tcs_return_home_status(homeRec);
      sprintf(outbuf,"Home status: %d %d %d %d %d %d",
        homeRec.x_axis,homeRec.y_axis,homeRec.z_axis,
        homeRec.t_axis,homeRec.u_axis,homeRec.v_axis);
      writeline(outbuf,1);
      LIMITSTATUSPACK limStatus;
      tcs_return_limit_status(limStatus);
      sprintf(outbuf,"Limit status: %d %d %d %d %d %d",
        limStatus.x_axis,limStatus.y_axis,limStatus.z_axis,
        limStatus.t_axis,limStatus.u_axis,limStatus.v_axis);
      writeline(outbuf,1);
    }
*/
/*  No Z-encoder currently installed !!
    else if (strcmp(command,"ZREAD")==0) {
      sprintf(outbuf,"encoder: %ld\n",cp4016_read_pos());
      writeline(outbuf,1);
      sprintf(outbuf,"motor: %ld\n",tcs_return_step_position('z'));
      writeline(outbuf,1);
    }

    else if (strcmp(command,"ZRESET")==0) {
      cp4016_reset();
    }
    else if (strcmp(command,"ZRESET2")==0) {
      cp4016_reset2();
    }
    else if (strcmp(command,"ZINIT")==0) {
      cp4016_init();
    }
*/

    // stow the telescope in the parking position
    else if (strcmp(command,"ST")==0) {
      status = tcs_telescope_park(0);
    }

    else if (strcmp(command,"STUP")==0) {
      status = tcs_telescope_park(1);
    }

    // put the rotator in the park/fill position
    else if (strcmp(command,"FI")==0) {
      int narg;
      double fill_time ;
      narg = sscanf(commandline+3,"%lf",&fill_time);
      if (narg ==  0) {
        sprintf(outbuf, "Enter desired fill time (in minutes)");
        writeline(outbuf,0);
        getline(inputline,sizeof(inputline));
        sscanf(inputline,"%lf",&fill_time);
      }
      if (fill_time>30) {
        sprintf(outbuf, "Limiting to maximum fill time of 30 minutes");
        writeline(outbuf,0);
        fill_time = 30;
      }
      status = tcs_telescope_fill(fill_time);
    }

    else if (strcmp(command,"UPS") == 0) {
      ocs_ups_status(ups);
    }  
    else if (strcmp(command,"FILLSTATUS") == 0) {
      if (ocs_ccd_fill_open() == TRUE) 
		  writeline("Filler valve is OPEN\n",1);
      else
		  writeline("Filler valve is CLOSED\n",1);
    }

/*  No CCD control through TOCC!!
    else if (strcmp(command,"CCDTEMP") == 0) {
      check_ccd_temp(1);
    }
 
    else if (strcmp(command,"CCDFAST") == 0) {
      set_ccd_speed(1);
    }
 
    else if (strcmp(command,"CCDSLOW") == 0) {
      set_ccd_speed(0);
    }
*/
 
    else if (strcmp(command,"SERVICE")==0) {
      status = tcs_telescope_service();
      sprintf(outbuf, "Rotator is at service position - TRACKING HAS BEEN TURNED OFF");
      writeline(outbuf,0);
      sprintf(outbuf,
	"Dont forget to turn tracking on using TR command AFTER servicing");
      writeline(outbuf,0);
    }

    // Toggle dome slaving
    else if (strcmp(command,"DM")==0) {
      autoGlobal->dome_slaved = !autoGlobal->dome_slaved;
      status = TCSERR_OK;
    }

    // Initialize dome
    else if (strcmp(command,"DI")==0) {
      status = ocs_home_dome(FALSE,TRUE);
    }

    else if (strcmp(command,"DIFORCE")==0) {
      double daz;
      sscanf(commandline+7,"%lf",&daz);
      status = ocs_home_dome_force(daz);
      G->dome_initialized = TRUE;
    }

    else if (strcmp(command,"OCSINIT")==0) {
      ocs_initialize();
    }

    else if (strcmp(command,"CDO")==0 && priv) {
      double newValue;
      status = ocs_calibrate_dome_steps(newValue);
    }

    else if (strcmp(command,"HOMEDOME")==0 && priv) {
      double home_az;
      sscanf(commandline+8,"%lf",&home_az);
      sysGlobal->dome_home_azimuth = home_az;
      writesysscf(sysGlobal);
      status = TCSERR_OK;
    }

    // Open main dome slit
    else if (strcmp(command,"OD")==0 || strcmp(command,"XOD") == 0) {
      int opentime = 0;
      sscanf(commandline+3,"%d",&opentime);
      if (strcmp(command,"OD") == 0) {
        if (G->mirror_covers_open) {
          sprintf(outbuf,"WARNING: mirror covers open");
          writeline(outbuf,0); 
          sprintf(outbuf,"Do you wish to close them (Y/N)");
          writeline(outbuf,0); 
          getline(inputline,sizeof(inputline));
          if (strncmp(inputline,"Y",1) == 0 || strncmp(inputline,"y",1)== 0) 
	    tcs_dust(0);
        }
      }
      status = ocs_open_shutter(opentime);
    }
    else if (strcmp(command,"ODS")==0 || strcmp(command,"XODS") == 0) {
      status = ocs_open_shutters();
    }
    else if (strcmp(command,"CDS")==0 || strcmp(command,"XCDS") == 0) {
      status = ocs_close_shutters();
    }

    // Close main dome slit
    else if (strcmp(command,"CD")==0 || strcmp(command,"XCD") == 0) {
      if (strcmp(command,"CD") == 0) {
        if (G->mirror_covers_open) {
          sprintf(outbuf,"WARNING: mirror covers open!! ");
          writeline(outbuf,0); 
          sprintf(outbuf,"Do you wish to close them (Y/N)");
          writeline(outbuf,0); 
          getline(inputline,sizeof(inputline));
          if (strncmp(inputline,"Y",1) == 0 || strncmp(inputline,"y",1)== 0) 
	    tcs_dust(0);
        }
	if (G->lower_dome_open) {
          sprintf(outbuf,"WARNING: lower dome open");
          writeline(outbuf,0); 
          sprintf(outbuf,"Do you wish to close it (Y/N)");
          writeline(outbuf,0); 
          getline(inputline,sizeof(inputline));
          if (strncmp(inputline,"Y",1) == 0 || strncmp(inputline,"y",1)== 0) 
            status = ocs_close_lower_shutter();
        }
      }
      status = ocs_close_shutter();
    }

    // Open lower dome slit
    else if (strcmp(command,"OLD") == 0 || strcmp(command,"XOLD") == 0) {
      int opentime = 0;
      sscanf(commandline+4,"%d",&opentime);
      status = ocs_open_lower_shutter(opentime);
    }

    else if (strcmp(command,"CLD") == 0 || strcmp(command,"XCLD") == 0) {
      status = ocs_close_lower_shutter();
    }

/* No mirror covers installed !!
    // Open mirror covers
    else if (strcmp(command,"OM")==0) {
      status = tcs_dust(1);
    }

    // Close mirror covers
    else if (strcmp(command,"CM")==0) {
      status = tcs_dust(0);
    }
*/

    // Read weather station
    else if (strcmp(command,"WE")==0) {
      status = read_weather_station(weather);
    }

/* No WWV clock installed!!
    // Read WWV clock
    else if (strcmp(command,"WWV")==0) {
      status = ocs_get_set_date_time();
    }
*/

    // Set the clock from an external time
    else if (strcmp(command,"SETTIME")==0) {
      struct time t;
      struct date d;
      int year, mon, day, hour, min, sec;
      sscanf(commandline+8,"%d%d%d%d%d%d", &year, &mon, &day, &hour, &min, &sec);
      d.da_year = year + 1980;
      d.da_mon = mon;
      d.da_day = day;
      t.ti_hour = hour;
      t.ti_min = min;
      t.ti_sec = sec;
      t.ti_hund = 99;
      sprintf(outbuf,"Remaining time in network timer: %d\n",
	   SysTimer[SYSTMR_NETWORK_LOST].RemainingTimeInSecs());
      writeline(outbuf,1);
      sprintf(outbuf,"commandline: %s\n",commandline);
      writeline(outbuf,1);
      sprintf(outbuf,"setting date/time: %d %d %d %d %d %d %d\n",
	    d.da_year, d.da_mon, d.da_day,
	    t.ti_hour, t.ti_min, t.ti_sec, t.ti_hund);
      writeline(outbuf,1);
      setdate(&d);
      settime(&t);

      getdate(&d);
      gettime(&t);
      sprintf(outbuf,"Set system date/time to: %d %d %d %d %d %d %d\n",
	    d.da_year, d.da_mon, d.da_day,
	    t.ti_hour, t.ti_min, t.ti_sec, t.ti_hund);
      writelog(outbuf,13);
      writeline(outbuf,1);


      sprintf(outbuf,"Remaining time in network timer: %d\n",
	   SysTimer[SYSTMR_NETWORK_LOST].RemainingTimeInSecs());
      writeline(outbuf,1);
      init_system_timers();
      sprintf(outbuf,"Remaining time in network timer: %d\n",
	   SysTimer[SYSTMR_NETWORK_LOST].RemainingTimeInSecs());
      writeline(outbuf,1);
    }

    // Toggle mount correction on/off
    else if (strcmp(command,"MC")==0) {
      G->mc_enabled = !G->mc_enabled;
    }
    // Pointing model by command
    else if (strcmp(command,"TPOINT")==0) {
      sscanf(commandline+6,"%lf%lf%lf%lf%lf%lf%lf%lf", 
          &autoGlobal->tpoint_ia,
          &autoGlobal->tpoint_ie,
          &autoGlobal->tpoint_npae,
          &autoGlobal->tpoint_ca,
          &autoGlobal->tpoint_an,
          &autoGlobal->tpoint_aw,
          &autoGlobal->tpoint_nrx,
          &autoGlobal->tpoint_nry);
      writetoccscf(autoGlobal);
      status = set_mount_correction_model();
    }

    // Read mount correction file
    else if (strcmp(command,"RMC")==0) {
      sprintf(outbuf,"Enter mount correction file name: ");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      status = read_mount_correction_model(inputline);
    }

    // Toggle display on/off
    else if (strcmp(command,"-DISPLAY")==0) {
      do_display = FALSE;
    }
    else if (strcmp(command,"+DISPLAY")==0) {
      do_display = TRUE;
    }

    else if (strcmp(command,"35M")==0) {
      check_weather();
    }

    else if (strcmp(command,"+35M")==0) {
      G->check_35m_closed = TRUE;
    }

    else if (strcmp(command,"-35M")==0) {
      G->check_35m_closed = FALSE;
    }

    else if (strcmp(command,"CLEAR")==0) {
      G->shutdown_state = 0;
           SysTimerActive[SYSTMR_35M_OPEN] = FALSE;
    }
#ifndef no_filter
    else if (strcmp(command,"FILTER")==0) {
      int pos, ntry;
      sscanf(commandline+6,"%d",&pos);
      ntry = 0;
      while ((status = move_filter(pos)) != 0 && ntry< 5) ntry++;
    }

    else if (strcmp(command,"FILTTEST")==0) {
      int pos;
      sscanf(commandline+8,"%d",&pos);
      int ntest;
      ntest=100;
      int n=0;
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
/*
    else if (strcmp(command,"FILTINIT1")==0) {
      status = filter_initialize(1);
    }
*/
    else if (strcmp(command,"FILTPOS")==0) {
      int pos;
      pos = get_filter_pos();
      sprintf(outbuf,"filter pos: %d",pos);
      writeline(outbuf,0);
    }

    else if (strcmp(command,"FILTSTEPS")==0) {
      long steps;
      sscanf(commandline+9,"%ld",&steps);
      new_steps_per_rot(steps);
    }

    else if (strcmp(command,"DETENT")==0) {
      int det;
      sscanf(commandline+6,"%d",&det);
      if (det==0)
        status = move_detent_out();
      else
        status = move_detent_in();
    }

// Tertiary control commands
    else if (strcmp(command,"TINIT")==0) {
      status = tertiary_initialize();
    }

    else if (strcmp(command,"THOME")==0) {
      status = tertiary_home(0);
    }

    else if (strcmp(command,"THOMER")==0) {
      status = tertiary_home(1);
    }

    else if (strcmp(command,"TSETNA1")==0) {
      long pos;
      sscanf(commandline+7,"%ld",&pos);
      gcsGlobal->t_pos_na1 = pos;
    }

    else if (strcmp(command,"TSETNA2")==0) {
      long pos;
      sscanf(commandline+7,"%ld",&pos);
      gcsGlobal->t_pos_na2 = pos;
    }

    else if (strcmp(command,"TMOVE")==0) {
      long pos;
      sscanf(commandline+5,"%ld",&pos);
      status = tertiary_move(pos,1);
    }

    else if (strcmp(command,"QTMOVE")==0) {
      long pos;
      sscanf(commandline+6,"%ld",&pos);
      status = tertiary_move(pos,-1);
    }

    else if (strcmp(command,"NA1")==0) {
      status = tertiary_move(gcsGlobal->t_pos_na1,0);
      if (status==0)
        autoGlobal->tertiary_port = 1;
      else
        autoGlobal->tertiary_port = 0;
      writetoccscf(autoGlobal);
      sprintf(outbuf,"autoGlobal->tertiary_port: %d",autoGlobal->tertiary_port);
      writeline(outbuf,1);
      sysGlobal->z_axis_enabled = 1;
      writesysscf(sysGlobal);
    }

    else if (strcmp(command,"NA2")==0) {
      status = tertiary_move(gcsGlobal->t_pos_na2,0);
      if (status==0)
        autoGlobal->tertiary_port = 2;
      else
        autoGlobal->tertiary_port = 0;
      writetoccscf(autoGlobal);
      sprintf(outbuf,"autoGlobal->tertiary_port: %d",autoGlobal->tertiary_port);
      writeline(outbuf,1);
      // Send rotator home and disable rotator
      sysGlobal->z_axis_enabled = 0;
      writesysscf(sysGlobal);
      sprintf(inputline,"AZ MA0 GO;");
      writeline(inputline,1);
      pc38_send_commands(inputline);
    }

    else if (strcmp(command,"BRAKE")==0) {
      int det;
      sscanf(commandline+5,"%d",&det);
      if (det==0)
        status = move_brake_out();
      else
        status = move_brake_in();
    }
#endif

//  Guider control commands
#ifndef no_guider
    else if (strcmp(command,"GUIDEINIT")==0) {
      status = guider_initialize(1,0,0);
    }

    else if (strcmp(command,"GUIDESET")==0) {
      long xpos, ypos;
      sscanf(commandline+8,"%ld %ld",&xpos,&ypos);
      status = guider_initialize(0,xpos,ypos);
    }

    else if (strcmp(command,"GUIDEHOME")==0) {
      status = guider_move_steps(
      gcsGlobal->x_home_pos,gcsGlobal->y_home_pos,1);
    }

    else if (strcmp(command,"GUIDEMOVE")==0) {
      long dx, dy;
      sscanf(commandline+9,"%ld %ld",&dx,&dy);
      status = guider_move_steps(dx,dy,0);
    }

    else if (strcmp(command,"GUIDELOC")==0) {
      double skypos;
      sscanf(commandline+8,"%lf",&skypos);
      status = guider_skypos(skypos);
    }

    else if (strcmp(command,"GSETHOME")==0) {
      long xpos,ypos;
      double skypos;
      sscanf(commandline+8,"%ld%ld%lf",&xpos,&ypos,&skypos);
      status = guider_set_home(xpos,ypos,skypos);
    }

    else if (strcmp(command,"GUIDEFOC")==0 || strcmp(command,"GDF")==0 ) {
      int dfoc;
      if (strcmp(command,"GUIDEFOC")==0)
        sscanf(commandline+8,"%d",&dfoc);
      else
        sscanf(commandline+3,"%d",&dfoc);
      status = guider_focus(dfoc,0);
    }

    else if (strcmp(command,"GFO")==0 ) {
      int dfoc;
      sscanf(commandline+3,"%d",&dfoc);
      status = guider_focus(dfoc,1);
    }

    else if (strcmp(command,"GUIDEPOS")==0) {
      long dx, dy;
      sscanf(commandline+8,"%ld %ld",&dx,&dy);
      status = guider_move_steps(dx,dy,1);
    }
/*
    else if (strcmp(command,"GUIDEADJ")==0) {
      sscanf(commandline+8,"%d %d",&adj_nupdate,&adj_foc);
      adj_update = 0;
      adj_offset = 0;
      cur_foc = 0;
    }
*/
#endif

    // Make yourself a privilidged user
    else if (strcmp(command,"PRIV")==0) {
      sprintf(outbuf,"Enter password :");
      writeline(outbuf,0);
      getline(inputline,sizeof(inputline));
      if (strcmp(inputline,"clyde") == 0) priv = TRUE;
    }

    // If command requires privilige, notify user
    else if (strcmp(command,"UH")==0 ||
	     strcmp(command,"SHO")==0 ||
	     strcmp(command,"XTILT")==0 ||
	     strcmp(command,"YTILT")==0 ||
	     strcmp(command,"ET")==0 ||
	     strcmp(command,"EP")==0 ||
	     strcmp(command,"ETX")==0 ||
	     strcmp(command,"ETY")==0 ||
	     strcmp(command,"ETZ")==0 ||
	     strcmp(command,"TD")==0 ||
	     strcmp(command,"TF")==0 ||
	     strcmp(command,"SETUP")==0 ||
	     strcmp(command,"PC")==0 ||
	     strcmp(command,"CRO")==0 ||
	     strcmp(command,"CAZ")==0 ||
	     strcmp(command,"CDO")==0 ||
	     strcmp(command,"TFX")==0 ||
	     strcmp(command,"TFY")==0 ||
	     strcmp(command,"TFZ")==0 ) {
	  printf("Sorry - you must be a privilidged user to use command %s\n",
		     command);
	  printf("Use the PRIV command (with password) to enable yourself\n");
    }


    // quit command mode
    else if (strcmp(command,"QU")==0) {
      command_mode = !command_mode;
      sprintf(outbuf,"Leaving command mode.");
      writeline(outbuf,0);
    }

    // special remote commands
   // else if (remote_on) {
    else if (TRUE) {
      // Remote move command
      if (strcmp(command,"XMOVE")==0) {
         newpa = 0;
	 nread=sscanf(commandline+5,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&ra, &dec, &epoch, &pmra, &pmdec, &parallax, &radial_velocity,
		&eff_wavelength, &tlr, &newpa); 
	 zero_guide_correction();
	 status = tcs_move_to_coordinates(
		   ra,dec,epoch,pmra,pmdec,parallax,
		   radial_velocity,eff_wavelength,tlr,newpa);
      }
     
      // Remote PA
      else if (strcmp(command,"XMOVEPA")==0) {
         double pa;
	 sscanf(commandline+7,"%lf",&pa);
	 // Now move the telescope
	 status = tcs_move_to_coordinates(
		   G->current_mean_ra,G->current_mean_dec,
		   G->current_mean_epoch,G->current_mean_pmra,
		   G->current_mean_pmdec,G->current_mean_parallax,
		   G->current_mean_radial_velocity,
		   G->current_mean_eff_wavelength,G->current_tlr,pa);
      }

      // Remote relative move
      else if (strcmp(command,"XMOVREL")==0) {
	 G->handpaddle = 2;
	 double rasec, decsec;
	 sscanf(commandline+7,"%lf %lf",&rasec,&decsec);
	 rasec = rasec / 15. / cos(G->current_mean_dec);
	 status = tcs_move_noupdate(rasec, decsec);
      }

      else if (strcmp(command,"XQM")==0) {
	 double rasec, decsec;
	 sscanf(commandline+3,"%lf %lf",&rasec,&decsec);
	 rasec = rasec / 15. / cos(G->current_mean_dec);
	 status = tcs_quick_move(rasec, decsec);
      }

      // Remote dome move
      else if (strcmp(command,"XDOME")==0) {
	 double az;
	 sscanf(commandline+5,"%lf",&az);
	 status = ocs_rotate_dome(az);
      }

      // Remote dome move
      else if (strcmp(command,"XDOMELOC")==0) {
         double dome_enc;
         dome_enc = ocs_return_dome_azimuth();
	 writeline(outbuf,1);
         status = 0;

      }

      // Remote focus
      else if (strcmp(command,"XFOCUS")==0) {
	 long dt, du, dv;
	 sscanf(commandline+6,"%ld %ld %ld",&dt,&du,&dv);
	 status = tcs_move_secondary_steps(dt,du,dv);
      }

      // Remote change coordinates
      else if (strcmp(command,"XMARK")==0) {
	 int sessiononly;
	 sscanf(commandline+5,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %d",
		&ra, &dec, &epoch, &pmra, &pmdec, &parallax, &radial_velocity,
		&eff_wavelength, &tlr, &sessiononly); 
	 zero_guide_correction();
	 if (sessiononly==1)
	   status = tcs_mark_coordinates(
		   ra,dec,epoch,pmra,pmdec,parallax,
		   radial_velocity,eff_wavelength,tlr,TRUE);
	 else
	   status = tcs_mark_coordinates(
		   ra,dec,epoch,pmra,pmdec,parallax,
		   radial_velocity,eff_wavelength,tlr,FALSE);
      }

      else if (strcmp(command,"XTD")==0) {
	 sscanf(commandline+3,"%d",&G->tracking_dtime);
	 status = TCSERR_OK;
      }

      else if (strcmp(command,"XGF")==0) {
	   double guide_factor;
	   sscanf(commandline+3,"%lf",&guide_factor);
	   G->guide_factor = guide_factor;
	 status = TCSERR_OK;
      }

      else if (strcmp(command,"XTF")==0) {
	 int axis, timer;
	 double tracking_factor;
	 sscanf(commandline+3,"%d %lf %d",&axis,&tracking_factor,&timer);
	 G->x_tracking_factor_new = G->x_tracking_factor;
	 G->y_tracking_factor_new = G->y_tracking_factor;
	 G->z_tracking_factor_new = G->z_tracking_factor;
	 G->tracking_factor_mod = TRUE;
	 SysTimer[SYSTMR_TF].NewTimerSecs(timer);
	 if (axis == 1) {
	   G->x_tracking_factor_new = tracking_factor;
	   if (tracking_factor < 0) G->x_tracking_rate = tracking_factor;
	 }
	 else if (axis == 2){
	   G->y_tracking_factor_new = tracking_factor;
	   if (tracking_factor < 0) G->y_tracking_rate = tracking_factor;
	 }
	 else if (axis == 3){
	   G->z_tracking_factor_new = tracking_factor;
	   if (tracking_factor < 0) G->z_tracking_rate = tracking_factor;
	 }
	 else {
	   G->x_tracking_factor_new = tracking_factor;
	   G->y_tracking_factor_new = tracking_factor;
	   G->z_tracking_factor_new = tracking_factor;
	 }
	 status = TCSERR_OK;
      }

      // Read mount correction file
      else if (strcmp(command,"XRMC")==0) {
        sscanf(commandline+5,"%s",inputline);
        status = read_mount_correction_model(inputline);
      }

      else if (strcmp(command,"STATUS")==0) {
        update_status(1);
      }

      else if (strcmp(command,"UPDATE")==0) {
	sscanf(commandline+6,"%d",&status_update_rate);
	status = TCSERR_OK;
      }

      else if (strlen(command)>0){
        sprintf(outbuf,"Unknown command!!%d %s\r\n",strlen(command),command);
        writeline(outbuf,0);
        sprintf(outbuf,"Commandline:!!%d %s\r\n",strlen(commandline),commandline);
        writeline(outbuf,0);
      }
      
    }

    // Unknown command
    else if (strlen(command)>0){
        sprintf(outbuf,"Unknown command!!\r\n");
        writeline(outbuf,0);

    }  // end big if block for different commands

    } // end if strlen(command > 1)

    } // end if (getline != TCSERR_OK)

    // Now process the returned status. This will also take care of
    //   sending a verification of returned status for remote operation
    sprintf(outbuf,"status: %d\n",status);
    writelog(outbuf,16);
    error_code(status);
	       
  }  // end while (command_mode)

// If we're finished with command mode, turn off display and return to
//     full screen
  do_display = FALSE;
  window(1,1,tinfo.screenwidth,tinfo.screenheight);
  clrscr();
  display_menu();
}

int getsao(double saora, double saodec, double epoch, double vmin,
double vmax, double *outra, double *outdec, long *outid, double *outv,
double radius, long *oldid, int nold)
{
   int i, istart, iend, ifile;
   char infile[20], line[80];
   int rah,ram,decd,decm;
   long id;
   char sign;
   double ra,dec,ras,rap,decs,decp,vmag,cosdec,ddec,dra,dmin,dist;
   FILE *fp;
   BOOL samestar(long,long *, int);

   *outid = -1;
   cosdec = cos(saodec*DD2R);
   dra = saora - radius/15/cosdec;
   istart = ( dra < 0 ? (int)dra - 1 : (int)dra );
   iend = (int) (saora + radius/15/cosdec);
   dmin = radius*radius;
   for (i=istart; i<=iend; i++) {
     ifile= (i< 0 ? i+24 : i);
     ifile= (ifile> 23 ? ifile-24 : ifile);
     sprintf(infile,"sao\\sao%02d.dat\0",ifile+1);
     fp = fopen(infile,"r");
     if (fp==NULL) return(1);
     while (fgets(line,80,fp) != NULL) {
	 sscanf(line,"%6ld%2d%2d%6lf%7lf%1c%2d%2d%5lf%6lf%4lf",
	      &id,&rah,&ram,&ras,&rap,&sign,&decd,&decm,&decs,&decp,&vmag);
	 if (!samestar(id,oldid,nold) && vmag>vmin && vmag<vmax) {
	   dec=decd+decm/60.+decs/3600.;
	   if (sign == '-') dec *= -1;
	   dec += decp*(epoch-1950.)/3600.;
	   ddec=saodec-dec;
	   if (fabs(ddec) < radius) {
	     ra=rah+ram/60.+ras/3600.;
	     ra += rap*(epoch-1950.)/3600.;
	     dra=(saora-ra)*15.*cosdec;
	     dra = (dra>180 ? dra-360 : dra);
	     dra = (dra<-180 ? dra+360 : dra);
	     if (fabs(dra) < radius) {
		   dist = dra*dra + ddec*ddec;
		   if (dist<dmin) {
		     *outra=ra;
		     *outdec=dec;
		     *outid=id;
		     *outv=vmag;
		     dmin = dist;
		   }
	     }
	   }
	}
     }
     fclose(fp);
  }
  return(0);

}
BOOL samestar(long id,long *oldid,int nold)
{
  int i;

  for (i=0; i<nold; i++) {
    if (id == *(oldid+i)) return(TRUE);
  }

  return(FALSE);
}

void command_help()
{
   char ans;

// Temporarily suspend the screen updating
   do_display = FALSE;

// Switch to full screen to see all commands
   window(1,1,tinfo.screenwidth,tinfo.screenheight);
   clrscr();

   sprintf(outbuf,
   "General commands: \r\n"
   " HP: print help menu\r\n"
   "QU: quit command mode\r\n"
   "Commands to move/initialize telescope: \r\n"
   " CO: move to specified coordinates\r\n"
   " SA: move to SAO star\r\n"
   " PM: move to a previous position\r\n"
   " RM: relative move from current position\r\n"
   " HO: initialize telescope by moving to home\r\n"
   " IN: initialize telescope with saved coords if available\r\n"
   " ST: stow telescope in parked position\r\n"
   "Commands to alter equinox, current position\r\n"
   " NE: change to new equinox\r\n"
   " UC: update coordinates at current position\r\n"
   " UP: update current instrument position angle\r\n"
   " UH: update home coordinates (beware!)\r\n"
   "Tracking/moving commands: \r\n"
   " TR: toggle tracking on/off\r\n"
   " EP: toggle use of encoders for pointing on/off\r\n"
   " ET: toggle use of encoders for tracking on/off\r\n"
   "Engineering commands: \r\n"
   " PC: issue a PC38 command directly\r\n"
   " CAZ: calibrate azimuth axis\r\n"
   " CRO: calibrate rotator axis\r\n");
   writeline(outbuf,0);

   sprintf(outbuf,"\r\nHit <CR> to continue: ");
   writeline(outbuf,0);
   getline(&ans,sizeof(ans));

   sprintf(outbuf,
   "Dome commands: \r\n"
   " DI: initialize the dome\r\n"
   " DM: toggle dome slaving on/off\r\n"
   " OD: open dome \r\n"
   " CD: close dome \r\n");
   writeline(outbuf,0);

   sprintf(outbuf,"\r\nHit <CR> to continue: ");
   writeline(outbuf,0);
   getline(&ans,sizeof(ans));

// Clear the screen
   window(1,1,tinfo.screenwidth,tinfo.screenheight);
   clrscr();

// Go back to dual mode screen 
   do_display = TRUE;
   window(1,DISPHEIGHT+1,tinfo.screenwidth,tinfo.screenheight);
   clrscr();
}

void update_display()
{
   int h,m,sign;
   double ha, s;

   char buf[80], vbuf[160];

   if (!SysTimer[SYSTMR_UPDATE_DISPLAY].Expired()) return;

   // get the observed telescope position and time into global variables
   update_telescope_position();
   G->dome_open = autoGlobal->shutter_opened;
   G->lower_dome_open = autoGlobal->lower_shutter_opened;
   G->dome_slaved = autoGlobal->dome_slaved;
   G->mirror_covers_open = autoGlobal->mirror_covers_open;

   gethms(G->current_obs_ra*DR2H,h,m,s,sign);
   sprintf(buf,"RA:  %2d %2d %4.1f\0",h,m,s);
   vcopy(vbuf,buf,WHITE);
   puttext(1,1,15,1,vbuf);

   gethms(fabs(G->current_obs_dec*DR2D),h,m,s,sign);
   if (G->current_mean_dec >= 0)
     sprintf(buf,"DEC: %2d %2d %4.1f\0",h,m,s);
   else
     sprintf(buf,"DEC:-%2d %2d %4.1f\0",h,m,s);
   vcopy(vbuf,buf,WHITE);
   puttext(1,2,15,2,vbuf);

   ha = slaDrange(G->current_lasth*DH2R - G->current_obs_ra)*DR2H;
   gethms(fabs(ha),h,m,s,sign);
   if (ha >= 0)
     sprintf(buf,"HA:  %2d %2d %4.1fW\0",h,m,s);
   else
     sprintf(buf,"HA:  %2d %2d %4.1fE\0",h,m,s);
   vcopy(vbuf,buf,WHITE);
   puttext(1,3,16,3,vbuf);

   sprintf(buf,"EPOCH:   %6.1f",G->current_mean_epoch);
   vcopy(vbuf,buf,WHITE);
   puttext(1,4,15,4,vbuf);

   sprintf(buf,"PA:      %6.1f",G->current_pa*DR2D);
   vcopy(vbuf,buf,WHITE);
   puttext(1,5,15,5,vbuf);

   if (G->x_encoder_installed) {
     sprintf(buf,"Encoder AZ:   %7.3f",G->current_obs_az);
   }
   else {
     sprintf(buf,"Motor AZ:     %7.3f",G->current_obs_az);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(20,1,40,1,vbuf);

   if (G->y_encoder_installed) {
     sprintf(buf,"Encoder ALT:  %7.3f",G->current_obs_alt);
   }
   else {
     sprintf(buf,"Motor ALT:    %7.3f",G->current_obs_alt);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(20,2,40,2,vbuf);

   if (G->z_encoder_installed) {
     sprintf(buf,"Encoder ROT:  %7.3f",G->current_obs_rot);
   }
   else {
     sprintf(buf,"Motor ROT:    %7.3f",G->current_obs_rot);
   }
   vcopy(vbuf,buf,WHITE);
   puttext(20,3,40,3,vbuf);

   sprintf(buf,"Motor FOC: %6ld %6ld %6ld",
     autoGlobal->t_step_pos,autoGlobal->u_step_pos,autoGlobal->v_step_pos);
   vcopy(vbuf,buf,WHITE);
   puttext(20,4,50,4,vbuf);

   gethms((G->current_utc - (long)G->current_utc)*24,h,m,s,sign);
   sprintf(buf,"UT:  %2d %2d %2d",h,m,(int)s);
   vcopy(vbuf,buf,WHITE);
   puttext(45,1,57,1,vbuf);

   gethms(G->current_lasth,h,m,s,sign);
   sprintf(buf,"LST: %2d %2d %2d",h,m,(int)s);
   vcopy(vbuf,buf,WHITE);
   puttext(45,2,57,2,vbuf);

   sprintf(buf,"TELESCOPE STATUS:");
   vcopy(vbuf,buf,WHITE);
   puttext(1,6,17,6,vbuf);
   if (G->telescope_at_home) {
     sprintf(buf,"HOME        ");
     vcopy(vbuf,buf,WHITE);
     puttext(19,6,30,6,vbuf);
   }
   else if (G->tracking_on) {
     sprintf(buf,"TRACKING ON ");
     vcopy(vbuf,buf,WHITE);
     puttext(19,6,30,6,vbuf);
   } else if (G->telescope_is_slewing) {
     sprintf(buf,"SLEWING     ");
     vcopy(vbuf,buf,WHITE);
     puttext(19,6,30,6,vbuf);
   } 
   else {
     sprintf(buf,"TRACKING OFF");
     vcopy(vbuf,buf,WHITE);
     puttext(19,6,30,6,vbuf);
   }

   sprintf(buf,"ENCODER STATUS: ");
   vcopy(vbuf,buf,WHITE);
   puttext(1,7,16,7,vbuf);
   if (G->use_encoders) {
     sprintf(buf," ON FOR POINTING");
     vcopy(vbuf,buf,WHITE);
     puttext(19,7,34,7,vbuf);
   } else {
     sprintf(buf,"OFF FOR POINTING");
     vcopy(vbuf,buf,WHITE);
     puttext(19,7,34,7,vbuf);
   }
   if (G->x_encoder_installed && G->y_encoder_installed) {
     sprintf(buf,"    ON FOR MOVES");
     vcopy(vbuf,buf,WHITE);
     puttext(35,7,50,7,vbuf);
   } else {
     sprintf(buf,"   OFF FOR MOVES");
     vcopy(vbuf,buf,WHITE);
     puttext(35,7,50,7,vbuf);
   }
   if (G->x_encoder_tracking || 
       G->y_encoder_tracking ||
       G->z_encoder_tracking) {
     sprintf(buf," ON FOR     TRACKING");
     vcopy(vbuf,buf,WHITE);
     puttext(52,7,71,7,vbuf);
     if (G->x_encoder_tracking) {
       sprintf(buf,"X");
       vcopy(vbuf,buf,WHITE);
       puttext(60,7,60,7,vbuf);
     }
     if (G->y_encoder_tracking) {
       sprintf(buf,"Y");
       vcopy(vbuf,buf,WHITE);
       puttext(61,7,61,7,vbuf);
     }
     if (G->z_encoder_tracking) {
       sprintf(buf,"Z");
       vcopy(vbuf,buf,WHITE);
       puttext(62,7,62,7,vbuf);
     }
   } else {
     sprintf(buf,"    OFF FOR TRACKING");
     vcopy(vbuf,buf,WHITE);
     puttext(52,7,71,7,vbuf);
   }

   sprintf(buf,
"                                                                      ");
   vcopy(vbuf,buf,WHITE);
   puttext(1,8,strlen(buf),8,vbuf);
   if (G->mc_enabled) 
     sprintf(buf,"MC:  ENABLED, FILE: %s",autoGlobal->mc_file);
   else 
     sprintf(buf,"MC: DISABLED");
   vcopy(vbuf,buf,WHITE);
   puttext(1,8,strlen(buf),8,vbuf);

   if (remote_on) 
     sprintf(buf,"REMOTE:  ON");
   else
     sprintf(buf,"REMOTE: OFF");
   vcopy(vbuf,buf,WHITE);
   puttext(1,9,11,9,vbuf);
   
   sprintf(buf,"DOME AZ: %3d",G->dome_azimuth);
   vcopy(vbuf,buf,WHITE);
   puttext(36,5,47,5,vbuf);
   
   sprintf(buf,"DOME STATUS:");
   vcopy(vbuf,buf,WHITE);
   puttext(36,6,47,6,vbuf);
   if (G->dome_initialized) {
     sprintf(buf,"INITIALIZED  , ");
     vcopy(vbuf,buf,WHITE);
     puttext(48,6,62,6,vbuf);
   } else {
     sprintf(buf,"UNINITIALIZED, ");
     vcopy(vbuf,buf,WHITE);
     puttext(48,6,62,6,vbuf);
   } 
   if (G->dome_slaved) {
     sprintf(buf,"SLAVED    ,");
     vcopy(vbuf,buf,WHITE);
     puttext(63,6,73,6,vbuf);
   } else {
     sprintf(buf,"NOT SLAVED,");
     vcopy(vbuf,buf,WHITE);
     puttext(63,6,73,6,vbuf);
   } 
   if (G->dome_open) {
     sprintf(buf,"OPEN  ");
     vcopy(vbuf,buf,WHITE);
     puttext(74,6,79,6,vbuf);
   } else {
     sprintf(buf,"CLOSED");
     vcopy(vbuf,buf,WHITE);
     puttext(74,6,79,6,vbuf);
   }

   SysTimer[SYSTMR_UPDATE_DISPLAY].NewTimer(SYSTMR_DISPLAY_INC);
}

void update_status(int out)
{
     int iout;

     if (out==0 && 
       (status_update_rate<0 || !SysTimer[SYSTMR_UPDATE_STATUS].Expired()))return;

     G->dome_open = autoGlobal->shutter_opened;
     G->lower_dome_open = autoGlobal->lower_shutter_opened;
     G->mirror_covers_open = autoGlobal->mirror_covers_open;
     sprintf(outbuf,
     "STATUS: %10.7f %10.7f %f %10.7f %10.7f %f %f %f %f %f %f %d %d %d %d %d %d %d %d %d %f %f %f %d" 
     " %f %d %d %d %d %d %d %f %f %f %ld %ld %ld %f %f %f %d %d %d %d %f"
     " %f %f %f %f %f %f %f %f %f %f %ld %ld %ld %d %d %s",
       G->current_obs_ra, G->current_obs_dec, G->current_pa,
       G->current_mean_ra, G->current_mean_dec, 
       G->current_mean_epoch, G->current_utc, G->current_lasth,
       G->current_obs_az, G->current_obs_alt, G->current_obs_rot,
       autoGlobal->tertiary_port, G->telescope_initialized, G->telescope_at_home,
       G->tracking_on, G->telescope_is_slewing, G->use_encoders,
       G->x_encoder_tracking, G->y_encoder_tracking, G->z_encoder_tracking,
       G->last_x_rate, G->last_y_rate, G->last_z_rate,
//       G->x_encoder_installed, G->y_encoder_installed,G->z_encoder_installed,
       G->nmove, G->ut1_minus_utc, G->dome_initialized, G->dome_slaved,
       G->dome_open,G->lower_dome_open,G->dome_azimuth,G->mirror_covers_open,
       G->last_az_error, G->last_alt_error, G->last_rot_error,
       autoGlobal->t_step_pos,autoGlobal->u_step_pos, autoGlobal->v_step_pos,
       G->current_out_temp,G->current_cab_temp,G->current_aux_temp,
       G->current_winddir, G->current_windspeed,G->check_35m_closed,
       G->shutdown_state,
       sysGlobal->sx[1],sysGlobal->sy[1],sysGlobal->xc[1],
         sysGlobal->yc[1],sysGlobal->rot[1],
       sysGlobal->sx[2],sysGlobal->sy[2],sysGlobal->xc[2],
         sysGlobal->yc[2],sysGlobal->rot[2],
       G->ccd_temp,
       gcsGlobal->x_step_pos,gcsGlobal->y_step_pos,gcsGlobal->z_step_pos,   
       gcsGlobal->current_filter_pos,
       G->mc_enabled,autoGlobal->mc_file);
#ifdef SOCKET
     strcat(outbuf,"\n");
#endif
     if (out==0) {
       iout = 6;
       writeline(outbuf,iout);
     } else {
       iout = 1;
       writeline(outbuf,iout);
     }
     SysTimer[SYSTMR_UPDATE_STATUS].NewTimer(status_update_rate);
}

void vcopy(char *vbuf, char *buf, unsigned char attrib)
{
  int ii, i;

  ii = 0;
  for (i=0;i<strlen(buf);i++) {
    vbuf[ii++] = buf[i];
	vbuf[ii++] = attrib;
  }
}

void gethms(double in,int &h, int &m, double &s,int &sign)
{
  double tmpin, tmp;

  if (in < 0) {
    sign = -1;
    tmpin = -1 * in;
  }
  else {
    sign = 1;
    tmpin = in;
  }

  h = (int) tmpin;
  tmp = (tmpin-h) * 60.;
  m = (int) tmp;
  s = (tmp-m) * 60.;
  if (s>59.95) {
    m++;
    s = 0.;
    if (m>=60) {
      h++;
      m -= 60;
    }
    if (h>23) h-=24;
  }
}

BOOL illegal_ra(double ra) 
{
  if (ra < 0 | ra >= 24) {
    sprintf(outbuf,"Illegal value of RA\r\n");
    writeline(outbuf,0);
    return(TRUE);
  }
  else
    return(FALSE);
}

BOOL illegal_dec(double dec) 
{
  if (dec < -90 | dec > 90) {
    sprintf(outbuf,"Illegal value of DEC\r\n");
    writeline(outbuf,0);
    return(TRUE);
  }
  else
    return(FALSE);
}

BOOL illegal_az(double az) 
{
  if (az < 0 | az > 360) {
    sprintf(outbuf,"Illegal value of AZ\r\n");
    writeline(outbuf,0);
    return(TRUE);
  }
  else
    return(FALSE);
}

BOOL illegal_alt(double alt) 
{
  if (alt < 0 | alt > 90) {
    sprintf(outbuf,"Illegal value of ALT\r\n");
    writeline(outbuf,0);
    return(TRUE);
  }
  else
    return(FALSE);
}

int getsaved(double &ra, double &dec, double &epoch)
{
   int i, j, rah, ram, decd, decm, sign;
   double ras, decs;
   char inputline[10];

   sprintf(outbuf,"  STAR     RA          DEC         EPOCH");
   writeline(outbuf,0);
   j=0;
   for (i=MAXSAVE+isave-1; i>isave-1; i--)
   {
     j++;
     gethms(savedra[i%MAXSAVE],rah,ram,ras,sign);
     gethms(saveddec[i%MAXSAVE],decd,decm,decs,sign);
     if (sign < 0) 
       sprintf(outbuf,"  %d: %2d %2d %4.1f -%2d %2d %4.1f  %6.1f",
	 j, rah, ram, ras, decd, decm, decs, 
	 savedepoch[i%MAXSAVE]);
     else
       sprintf(outbuf,"  %d: %2d %2d %4.1f  %2d %2d %4.1f  %6.1f",
	 j, rah, ram, ras, decd, decm, decs, 
	 savedepoch[i%MAXSAVE]);
     writeline(outbuf,0);
   }
   sprintf(outbuf,"\r\n Enter star number (0 for none): ");
   writeline(outbuf,0);
   getline(inputline,sizeof(inputline));
   sscanf(inputline,"%d",&i);

   if (i <=0 ) 
     return(i);
   else {
     i = 2*MAXSAVE + isave-i;
     ra = savedra[i%MAXSAVE];
     dec = saveddec[i%MAXSAVE];
     epoch = savedepoch[i%MAXSAVE];
     return(i);
   }
     
}

int telescope_startup()
{ 
  char ans[2];
  int status;

  // Default is to use encoder pointing and encoder tracking
  G->use_encoders = TRUE ;
  G->x_encoder_tracking = sysGlobal->encoder_tracking;
  G->y_encoder_tracking = sysGlobal->encoder_tracking;
  G->z_encoder_tracking = sysGlobal->encoder_tracking;
 
  // Turn off dome slaving
  autoGlobal->dome_slaved = FALSE;

  // Enable the pointing model
  G->mc_enabled = autoGlobal->mc_enabled;
  //if (autoGlobal->mc_enabled) 
  //   status = read_mount_correction_model(autoGlobal->mc_file);
  status = set_mount_correction_model();

#ifdef NOREMOTE
  // Initialize the telescope
  sprintf(outbuf,"Do you wish to initialize the telescope with a :\r\n"
" Q: very quick init (stored coords - assumes telescope hasn't been moved!)\r\n"
" I: normal init (uses stored coords to find home positions - will be very\r\n"
"    slow if stored coords are wrong)\r\n"
" F: full init - finds home positions without any previous knowledge\r\n"
" S: skip initialization\r\n"
"Enter your choice: ");
  writeline(outbuf,0);
  getline(ans,sizeof(ans));
  if (ans[0]=='Q' || ans[0]=='q') {
	status = tcs_home_telescope();
  } else if (ans[0] == 'F' || ans[0] == 'f') {
	autoGlobal->saved_az = -9999;
	autoGlobal->saved_alt = -9999;
	status = tcs_home_telescope();
  } else if (ans[0] == 'S' || ans[0] == 's') {
    ;
  } else {
	status = tcs_home_telescope();
	autoGlobal->saved_az = -9999;
	autoGlobal->saved_alt = -9999;
	status = tcs_home_telescope();
  }
  if (status != TCSERR_OK) return(status);

  // Initialize the dome?
  sprintf(outbuf,"Do you wish to initialize the dome (Y or N)? ");
  writeline(outbuf,0);
  getline(ans,sizeof(ans));
  if (ans[0]=='Y' || ans[0]=='y') {
    status = ocs_home_dome(FALSE,TRUE);
    if (status != TCSERR_OK) return(status);
    sprintf(outbuf,"Do you wish to slave the dome (Y or N)? ");
    writeline(outbuf,0);
    getline(ans,sizeof(ans));
    if (ans[0]=='Y' || ans[0]=='y') {
	autoGlobal->dome_slaved = TRUE;
	G->dome_slaved = autoGlobal->dome_slaved;
    }
  }
#endif

  return(status);
}
