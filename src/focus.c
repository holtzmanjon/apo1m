#include <math.h>
#include <stdio.h>
#include "mytype.h"
#include "slamac.h"

/* 
  First guess secondary position
     motor: -3940  -1720  -6595
     tilts: 4085.333 1.493843 -0.077137
*/

/* Radius location of each secondary actuator, in inches */
#define RFOC 6.385

/* Encoder steps per inch (1000 steps/1.5mm) */
/* #define FSCALE 16933.333 */
#define FSCALE 16000.000

int setup_focus(double **foc)
{
  int i,j;

  for (i=0; i<3; i++) {
    foc[i] = (double *)malloc(3*sizeof(double));
  }
  foc[0][0] = 0.866025;
  foc[0][1] = 0.5;
  foc[0][2] = 1.;
  foc[1][0] = 0.;
  foc[1][1] = -1.;
  foc[1][2] = 1.;
  foc[2][0] = -0.866025;
  foc[2][1] = 0.5;
  foc[2][2] = 1.;

  for (i=0; i<3; i++)
    for (j=0; j<2; j++)
      foc[i][j] *= RFOC*FSCALE;

  return(0);

}

int calc_focus(double **ifoc, struct STATUS *G)
{
  double vec[3];
  double out[3];

  // Compute focus positions
  vec[0] = G->t_step_pos;
  vec[1] = G->u_step_pos;
  vec[2] = G->v_step_pos;
  vmul(ifoc,3,3,vec,out);
  G->foc_theta = atan(out[0])*DR2D;
  G->foc_phi = atan(out[1])*DR2D;
  G->foc = out[2];

  return(0);
}

int calc_focus_steps(double **foc, double *out, struct STATUS *G)
{
    double vec[3];

    vec[0] = tan(G->foc_theta*DD2R);
    vec[1] = tan(G->foc_phi*DD2R);
    vec[2] = G->foc;
    vmul(foc,3,3,vec,out);
    // Need to send differences, not absolute values
    out[0] -= G->t_step_pos;
    out[1] -= G->u_step_pos;
    out[2] -= G->v_step_pos;
}


int print_focus(double **a)
{
  int i,j;

  for (i=0;i<3;i++) {
    fprintf(stderr,"%d\n",a[i]);
    for (j=0;j<3;j++)
      printf("%d %d %f\n",i,j,a[i][j]);
  }
}

