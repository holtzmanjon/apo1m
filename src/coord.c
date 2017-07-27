#include <stdio.h>
#include <time.h>
#include "../slalib/slamac.h"
#include "../slalib/slalib.h"

double aoprms[14], amprms[21];

void coord_init(double, double, double);
void coord_altaz(double, double, double, double, double *, double *, double *);
void coord_sun(double, double *, double *, double *);
void coord_moon(double, double *, double *, double *);
#ifdef MAIN
main()
{
  double longitude, latitude, altitude;
  double mjd_utc, ra, dec, equinox, az, alt, rot, diam;
  time_t *t;

  latitude =32.78*DD2R;
  longitude=-105.8208333*DD2R;
  altitude=2798.0;
  coord_init(longitude, latitude, altitude);
fprintf(stderr,"aoprms: %f %f\n",aoprms[12]*DR2D,aoprms[0]*DR2D);

  mjd_utc = time(t)/3600./24. + 40587.0;
  equinox = 2000.;
  ra = (18.+48./60.+11/3600.)*15.*DD2R;
  dec = (18.+7/60.+35/3600.)*DD2R;
  fprintf(stderr,"mjd: %f\n",mjd_utc);
  coord_altaz(mjd_utc,ra,dec,equinox,&az,&alt,&rot);
  fprintf(stderr,"ra: %f dec: %f az: %f alt: %f rot: %f\n",ra*DR2D, dec*DR2D, az*DR2D, alt*DR2D, rot*DR2D);
  coord_moon(mjd_utc,&ra,&dec,&diam);
  coord_altaz(mjd_utc,ra,dec,equinox,&az,&alt,&rot);
  fprintf(stderr,"moon ra: %f dec: %f az: %f alt: %f rot: %f diam: %f\n",ra*DR2D, dec*DR2D, az*DR2D, alt*DR2D, rot*DR2D, diam*DR2D);
  coord_sun(mjd_utc,&ra,&dec,&diam);
  coord_altaz(mjd_utc,ra,dec,equinox,&az,&alt,&rot);
  fprintf(stderr,"sun ra: %f dec: %f az: %f alt: %f rot: %f diam: %f\n",ra*DR2D, dec*DR2D, az*DR2D, alt*DR2D, rot*DR2D, diam*DR2D);
}
#endif
void coord_init(double longitude, double latitude, double altitude)
{
  double polor_x = 0.;
  double polor_y = 0.;
  double dut = 0.;
  double pressure;
  double temp = 273.155;
  double humidity = 0.3;
  double lambda = 0.55;
  double lapse = 0.0065;
  double mjd_utc = 0;

  pressure = 1013.25 * exp(-altitude/8149.9415);

  slaAoppa(mjd_utc,dut,longitude,latitude,altitude,polor_x,polor_y,temp,pressure,humidity,lambda,lapse,aoprms);
}

void coord_altaz(double mjd_utc, double ra, double dec, double equinox, double *az, double *alt, double *rot)
{
  double app_ra, app_dec, pa, ha;
  double obs_ra, obs_dec, obs_ha, obs_az, obs_zd;

  slaMappa(equinox,mjd_utc,amprms);
  slaMapqkz(ra,dec,amprms,&app_ra,&app_dec); 
  slaAoppat(mjd_utc,aoprms);
  slaAopqk(app_ra,app_dec,aoprms,&obs_az,&obs_zd,&obs_ha,&obs_dec,&obs_ra);
  *az = slaDranrm(obs_az);
  *alt = DPIBY2 - obs_zd;
  ha = aoprms[13] - app_ra;
  pa = slaPa(slaDrange(ha), app_dec, aoprms[0]);
  *rot = -1 * *alt - pa;
}

void coord_moon(double mjd_utc, double *ra, double *dec, double *diam)
{
  slaRdplan(mjd_utc,3,aoprms[12],aoprms[0],ra,dec,diam);
}

void coord_sun(double mjd_utc, double *ra, double *dec, double *diam)
{
  slaRdplan(mjd_utc,0,aoprms[12],aoprms[0],ra,dec,diam);
}

