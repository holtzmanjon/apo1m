#include "globals.h"
#include "inst.h"
#include "mytype.h"
#include "io.h"
#include "slamac.h"

#undef OLD

extern void update_telescope_position();

void apply_instrument_correction(double ra, double dec, 
                                 double &inst_ra, double &inst_dec)
{
  double theta, dra, ddec;
  int cur_inst;
 
  cur_inst = G->current_inst; 
#ifdef OLD
  theta = G->current_pa + sysGlobal->rot[cur_inst];
  dra =  -sysGlobal->xc[cur_inst]*sysGlobal->sx[cur_inst]*cos(theta)  
         -sysGlobal->yc[cur_inst]*sysGlobal->sy[cur_inst]*sin(theta);
  ddec = -sysGlobal->yc[cur_inst]*sysGlobal->sy[cur_inst]*cos(theta)  
         +sysGlobal->xc[cur_inst]*sysGlobal->sx[cur_inst]*sin(theta);
#else
  theta = G->current_pa;
  dra =  +sysGlobal->xc[cur_inst]*cos(theta)  
         -sysGlobal->yc[cur_inst]*sin(theta);
  ddec = -sysGlobal->xc[cur_inst]*sin(theta)
         -sysGlobal->yc[cur_inst]*cos(theta); 
#endif

  inst_ra = ra + dra * DAS2R / cos(G->current_mean_dec);
  inst_dec = dec + ddec * DAS2R;
  sprintf(outbuf,"dra, ddec: %lf %lf %lf %lf %lf %lf",
            dra, ddec, ra, inst_ra, dec, inst_dec);
  writeline(outbuf,3);
}

void update_instrument_correction(int inst, double sx, double sy, double xc,
    double yc, double rot, double rot0, BOOL fixed)
{
// sx, sy are scales of instrument in "/pixel
// xc, yc are the pixel positions of the instrument center relative to
//   to rotator center, at PA=0
// rot is the angle between the negative y axis (up) on the chip and N,
//   measured positive as N moves clockwise.
  sysGlobal->sx[inst] = sx;
  sysGlobal->sy[inst] = sy;
  sysGlobal->xc[inst] = xc;
  sysGlobal->yc[inst] = yc;
  sysGlobal->rot[inst] = rot*DD2R;
  sysGlobal->rot0[inst] = rot0;
  sysGlobal->fixed[inst] = fixed;
  writesysscf(sysGlobal);
  sprintf(outbuf,"Set instrument block: %d sx: %lf  sy: %lf xc: %lf  yc: %lf  rot: %lf  rot0: %lf  fixed: %u\n",
         inst, sx, sy, xc, yc, rot, rot0, fixed);
  writeline(outbuf,1);
  writelog(outbuf,14);
}

void apply_instrument_offset(int cur_inst, double dx, double dy)
{
  double theta, dra, ddec;

  // Correct ra and dec to move object dx, dy on desired instrument
  theta = G->current_pa + sysGlobal->rot[cur_inst];
  if (sysGlobal->fixed[cur_inst]) {
    update_telescope_position();
    sprintf(outbuf,"instblock rotation: %f",theta);
    writeline(outbuf,1);
    double desired_rot;
    desired_rot = G->current_obs_rot - G->last_rot_error;
    theta += (sysGlobal->rot0[cur_inst] - desired_rot)*DD2R;
    sprintf(outbuf,"using corrected rotation: %f",theta);
    writeline(outbuf,1);
  }

  // Signs of dx and dy must be flipped to move object by desired amount,
  //   since to do so we must move telescope in opposite direction
  dra  =  -dx*sysGlobal->sx[cur_inst]*cos(theta) 
          -dy*sysGlobal->sy[cur_inst]*sin(theta);
  ddec  = +dx*sysGlobal->sx[cur_inst]*sin(theta)
          -dy*sysGlobal->sy[cur_inst]*cos(theta);

  sprintf(outbuf,"dx: %lf  dy: %lf   dra: %lf  ddec: %lf\n",dx,dy,dra,ddec);
  writeline(outbuf,1);

  G->current_mean_ra += dra*DAS2R/cos(G->current_mean_dec);
  G->current_mean_dec += ddec*DAS2R;
  
}
