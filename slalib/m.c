main()
{
  double amprms[21];
  double ra, dec, ora, odec;

  ra=0;
  printf("enter dec: ");
  scanf("%lf",&dec);

  slaMappa( 1950., 40000., amprms);
  slaMapqk ( ra, dec, 0., 0., 0., 0., amprms, &ora, &odec);
  printf("%f %f\n", ora*206265, odec*206265);

  slaMappa( 1950., 40001., amprms);
  slaMapqk ( ra, dec, 0., 0., 0., 0., amprms, &ora, &odec);
  printf("%f %f\n", ora*206265, odec*206265);

}
