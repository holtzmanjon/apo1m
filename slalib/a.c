/* pi */
#define DPI 3.1415926535897932384626433832795028841971693993751

/* 2pi */
#define D2PI 6.2831853071795864769252867665590057683943387987502

/* 1/(2pi) */
#define D1B2PI 0.15915494309189533576888376337251436203445964574046

/* 4pi */
#define D4PI 12.566370614359172953850573533118011536788677597500

/* 1/(4pi) */
#define D1B4PI 0.079577471545947667884441881686257181017229822870228

/* pi^2 */
#define DPISQ 9.8696044010893586188344909998761511353136994072408

/* sqrt(pi) */
#define DSQRPI 1.7724538509055160272981674833411451827975494561224

/* pi/2:  90 degrees in radians */
#define DPIBY2 1.5707963267948966192313216916397514420985846996876

/* pi/180:  degrees to radians */
#define DD2R 0.017453292519943295769236907684886127134428718885417

/* 180/pi:  radians to degrees */
#define DR2D 57.295779513082320876798154814105170332405472466564

/* pi/(180*3600):  arcseconds to radians */
#define DAS2R 4.8481368110953599358991410235794797595635330237270e-6

/* 180*3600/pi :  radians to arcseconds */
#define DR2AS 2.0626480624709635515647335733077861319665970087963e5

/* pi/12:  hours to radians */
#define DH2R 0.26179938779914943653855361527329190701643078328126

/* 12/pi:  radians to hours */
#define DR2H 3.8197186342054880584532103209403446888270314977709

/* pi/(12*3600):  seconds of time to radians */
#define DS2R 7.2722052166430399038487115353692196393452995355905e-5

/* 12*3600/pi:  radians to seconds of time */
#define DR2S 1.3750987083139757010431557155385240879777313391975e4

/* 15/(2pi):  hours to degrees x radians to turns */
#define D15B2P 2.3873241463784300365332564505877154305168946861068


main()
{
  double ha, dec, phi, slaPa(), del;
  double az, azd, azdd, el, eld, eldd, pa, pad, padd;
  double az2, azd2, azdd2, el2, eld2, eldd2, pa2, pad2, padd2;
  void slaAltaz();
  int i;

  phi=32;

/*
  for (i=-3600*3;i<3600*3;i=i+10)
*/
  while(1)
  {

    dec = 38.922222;
    ha = i/3600.;
  
    printf("Enter ha, dec: ");
    scanf("%lf %lf",&ha,&dec);

    slaAltaz ( ha*DH2R, dec*DD2R, phi*DD2R,
               &az, &azd, &azdd,
               &el, &eld, &eldd,
               &pa, &pad, &padd );

    del = 2;
    slaAltaz ( (ha+del/3600.)*DH2R, dec*DD2R, phi*DD2R,
               &az2, &azd2, &azdd2,
               &el2, &eld2, &eldd2,
               &pa2, &pad2, &padd2 );

/***        angles * 360/2pi -> degrees */
/**        velocities * (2pi/86400)*(360/2pi) -> degree/sec */
/**        accelerations * ((2pi/86400)**2)*(360/2pi) -> degree/sec/sec */

    printf("%f %f \n",az*DR2D,el*DR2D);
    printf("%f %f %f %f %f %f %f %f %f %f\n",ha,pa,(pa2-pa+el2-el)/del*DR2D,
                    (pad+eld)*360/86400,
                    (pa2-pa+el2-el)/del*DR2D-(pad+eld)*360/86400,
                    azd*360/86400.*3600, 
                    eld*360/86400.*3600, 
                    (pad+eld)*360/86400*3600,
                    azdd*((DPI/86400)*(DPI/86400)*(360/DPI)*3600),
                    eldd*((DPI/86400)*(DPI/86400)*(360/DPI)*3600)
                    );

  }

}


