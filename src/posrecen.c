#include "../slalib/slalib.h"
#include "../slalib/slamac.h"
#include <stdio.h>

main(int argc, char **argv)
{
  FILE *fp;
  double mean_to_app_parms[21];
  double app_to_obs_parms[14];
  double ra, dec, corrected_ra, corrected_dec, utc, epoch, lst;
  double obs_az, obs_alt, ra0, dec0, ha0;
  double latitude, longitude, altitude, az, zen;
  double rap, dap, roap, doap,high;
  int year, month, day, init=0;

  if (argc != 4) {
    fprintf(stderr,"Usage: posrecen YEAR MONTH DAY\n");
    exit(0);
  }

  latitude = 32.780300*DD2R; 
  longitude = -105.820000*DD2R;
  latitude = 32.7799988*DD2R; 
  longitude = -105.820831*DD2R;
  altitude = 2800.000000;
  utc    =  53137.4391391481477;
  utc    =  55875.4391391481477;

//              0., 273., 1013.25, 0.5, 0.55, 0.0065, app_to_obs_parms);

  //fp = fopen("pm.out","r");
  // Put TPOINT header
  printf("00-00-0000\n\n");
  year=atoi(argv[1]);
  month=atoi(argv[2]);
  day=atoi(argv[3]);
  printf(" 32 46 49.1 %4.4d %2.2d %2.2d 10.0  718.6 2800.0 0.20 0.55 0.0065\n\n",
      year, month, day);

//fprintf(stderr,"fp: %d\n",fp);
  while (scanf("%lf%lf%lf%lf%lf%lf%lf%lf",&ra,&dec,&epoch,&utc,&lst,&obs_az,&obs_alt,&high) != EOF) {
//  fprintf(stderr,"%lf %lf %lf %lf %lf %lf %lf\n",ra,dec,epoch,utc,lst,obs_az,obs_alt);

    if (!init) {
      slaAoppa(utc, 0., longitude, latitude, altitude, 0.,
              0., 273., 720.00, 0.5, 0.55, 0.0065, app_to_obs_parms);
      init=1;
    }

    ra*=DH2R;
    dec*=DD2R;

    slaMappa(epoch, utc, mean_to_app_parms);
    slaMapqk(ra, dec, 0., 0., 0., 0., mean_to_app_parms,
                    &corrected_ra, &corrected_dec);

    // recompute siderial time in app_to_obs_parms
    slaAoppat(utc, app_to_obs_parms);
    // apparent to observed
    slaAopqk(corrected_ra, corrected_dec, app_to_obs_parms,
                        &az, &zen, &ha0, &dec0, &ra0);
/*
    printf("STAR:\n%12.6f %12.6f %12.6f %12.6f\n", utc, epoch,app_to_obs_parms[13]*DR2H,lst);
    printf("%12.6f %12.6f\n", ra*DR2H, dec*DR2D);
    printf("%12.6f %12.6f\n", corrected_ra*DR2H, corrected_dec*DR2D);
    printf("%12.6f %12.6f\n\n", az*DR2D, zen*DR2D);
*/
    printf("%12.6f %12.6f %12.6f %12.6f %12.6f %12.6f\n", az*DR2D, 90-zen*DR2D, obs_az, obs_alt, obs_az, obs_alt);
/*
    printf("%12.6f %12.6f\n", (rap-roap)*15, dap-doap);
    printf("%12.6f %12.6f %12.5f %12.6f\n", rap,roap, dap,doap);
*/

  }

}


