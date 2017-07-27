#include "globals.h"
#include "guiding.h"
#include "mytype.h"
#include "slalib.h"
#include "slamac.h"
#include "io.h"

extern void update_telescope_position();

void update_guide_correction_inst(int cur_inst, double dx, double dy)
{
  double theta, dra, ddec;

  // Correct ra and dec to move object dx, dy on current instrument
  theta = G->current_pa + sysGlobal->rot[cur_inst];

  if (sysGlobal->fixed[cur_inst]) {
    update_telescope_position();
    double desired_rot;
    desired_rot = G->current_obs_rot - G->last_rot_error;
    theta += (sysGlobal->rot0[cur_inst] - desired_rot)*DD2R;
  }

  dra  =  -dx*sysGlobal->sx[cur_inst]*cos(theta)
          -dy*sysGlobal->sy[cur_inst]*sin(theta);
  ddec  = +dx*sysGlobal->sx[cur_inst]*sin(theta)
          -dy*sysGlobal->sy[cur_inst]*cos(theta);
  dra*=G->guide_factor;
  ddec*=G->guide_factor;

  G->guide_err_ra += dra*DAS2R/ cos(G->current_mean_dec);
  G->guide_err_dec += ddec*DAS2R;

  sprintf(outbuf,"dx: %lf dy: %lf  dra: %lf  ddec: %lf"
                 "  guide_err_ra: %lf guide_err_dec: %lf\n",
	    dx, dy, dra, ddec,
            G->guide_err_ra*DR2AS,G->guide_err_dec*DR2AS);
  writeline(outbuf,1);
  writeline(outbuf,3);
}

void update_guide_correction(double dra, double ddec)
{
    // arguments should be in radians. They give the current location reported 
    //   by the guider relative to a reference starting location

    double guide_const = 1.00;
    double az, alt, azobs, altobs;
    struct mytime t;
    struct date d;
    struct ALLTIMES timeRec;

    // Store the current time while we're at it
    mygettime(&d,&t);
    // getdate(&d);
    get_all_times_at(d, t, timeRec);

  // Convert current desired position to alt-az. Since we're just going to
  //   difference this with current observed position, don't worry about
  //   pointing model, mean_to_app, etc.
    slaDe2h(slaDrange(timeRec.last-G->current_mean_ra), G->current_mean_dec,
            G->latitude, &az, &alt);

  // Convert current observed guider position to alt-az
    slaDe2h(slaDrange(timeRec.last-(G->current_mean_ra+dra)), 
                      G->current_mean_dec+ddec,
                      G->latitude, &azobs, &altobs);

  // Accumulate new corrections on top of old ones
    G->guide_err_az += (az - azobs)*guide_const;
    G->guide_err_alt += (alt - altobs)*guide_const;

/*
    sprintf(outbuf,"dra ddec: %lf %lf\r\n"
                    "az alt: %lf %lf\r\n"
                    "azobs altobs: %lf %lf\r\n"
                    "ra dec: %lf %lf\r\n"
                    "ra dec: %lf %lf\r\n",
            dra, ddec, az, alt, azobs, altobs,
            G->current_mean_ra,G->current_mean_dec,
            G->current_mean_ra+dra,G->current_mean_dec+ddec);
    writeline(outbuf,1);
*/
    sprintf(outbuf,"guide_errs: %lf %lf %lf %lf\n",
            G->guide_err_az,G->guide_err_alt,
            G->guide_err_az*DR2AS,G->guide_err_alt*DR2AS);
    writeline(outbuf,1);
    writeline(outbuf,3);
}

void update_guide_correction_altaz(double daz, double dalt)
{
    G->guide_err_az += daz;
    G->guide_err_alt += dalt;
}

void zero_guide_correction()
{
    G->guide_err_az = 0.;
    G->guide_err_alt = 0.;
    G->guide_err_ra = 0.;
    G->guide_err_dec = 0.;
    G->apply_rates = 0;
    G->dra = 0;
    G->ddec = 0;
}

void apply_guide_correction_altaz(double &az, double &alt)
{
    az += G->guide_err_az;
    alt += G->guide_err_alt;
//    fprintf(G->move_file,"applying guide_errs: %lf %lf %lf %lf\n",
//            G->guide_err_az,G->guide_err_alt,
//            G->guide_err_az*DR2AS,G->guide_err_alt*DR2AS);
}

void apply_guide_correction_radec(double *ra, double *dec)
{
    *ra += G->guide_err_ra;
    *dec += G->guide_err_dec;
//    fprintf(G->move_file,"applying radec guide_errs: %lf %lf %lf %lf\n",
//            G->guide_err_ra,G->guide_err_dec,
//            G->guide_err_ra*DR2AS,G->guide_err_dec*DR2AS);
}
