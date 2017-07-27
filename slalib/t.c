#include "slalib.h"
#include "slamac.h"
main()
{
  double az, alt, azobs, altobs, dra, ddec;
  double latitude = 31.*DD2R;
  double ha0, ha, dec0, dec;


  ha0 = 0.;
  dec0 = 0.;
  ddec= 0.;

  dra=10./15.*DS2R;
  slaDe2h(ha0,dec0,latitude,&az,&alt);
  slaDe2h(ha0+dra,dec0,latitude,&azobs,&altobs);
  printf ("%f8.3 %f8.3\n %f8.3 %f8.3\n",az, alt, azobs, altobs);
  printf ("%f8.3 %f8.3\n",slaDrange(az-azobs), alt-altobs);
  printf ("%f8.3\n\n",dra);

  dra=10./15.*DAS2R;
  slaDe2h(ha0,dec0,latitude,&az,&alt);
  slaDe2h(ha0+dra,dec0,latitude,&azobs,&altobs);
  printf ("%f8.3 %f8.3\n %f8.3 %f8.3\n",az, alt, azobs, altobs);
  printf ("%f8.3 %f8.3\n",slaDrange(az-azobs), alt-altobs);
  printf ("%f8.3\n\n",dra);

}


