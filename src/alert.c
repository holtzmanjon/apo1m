
remove("/home/tcomm/alert/alert.int");
fprintf(stderr,"%d %f %f %d %d\n",type, ra, dec, tjd, tsod);

if (type != 82) status = sendccd(toccd.port, toccd.server,"-ALERT",default_timeout);

// How long since alert was issued?
time(&t);
tm = gmtime(&t);
get_all_times_at(*tm, &timeRec);
double mjd;
mjd = tjd + tsod/100./86400.;
fprintf(stderr,"%f %f\n",timeRec.mjd_utc, mjd);
if ( (timeRec.mjd_utc - (mjd+40000)) < 1./24.)  {
  
  ra = slaDranrm(ra/15.*DH2R);
  dec *= DD2R;
  epoch = 2000;
  pmra = pmdec = 0.;
  newpa = -999.;

  sprintf(command,"GUIDEOFF");
  status = sendccd(togccd.port, togccd.server, command,default_timeout);
  guiding = FALSE;
  sprintf(input,"OBJECT GRB%5d.%3d",tjd,(int)(tsod/86400.*10.));
  status = sendccd(toccd.port, toccd.server, input,default_timeout);
  sprintf(command,"OBJNUM 0");
  status = sendccd(toccd.port, toccd.server, command,default_timeout);
  sprintf(command, "XMOVE %10.7f %10.7f %f %f %f %f %f %f %f %f",
                     ra,dec,epoch,pmra,pmdec,0.,0.,0.55,0.0065,newpa);
  status = sendport(toport.port,toport.server,command,ret,MAXCMD);
  fprintf(stderr,"Xmove status: %d\n",status);
  // Make sure we have full window
  sprintf(command,"WINDOW FULL");
  status = sendccd(toccd.port,toccd.server,command,default_timeout);
  // Turn off display to save a few seconds per image
  sprintf(command,"-DISPLAY");
  status = sendccd(toccd.port, toccd.server,command,default_timeout);

  sprintf(input,"/home/tcomm/alert/alert.%3.3d.seq",type);
  if ( (falert = fopen(input,"r")) == NULL ) falert = fopen("/home/tcomm/alert/alert.seq","r");

  while ( status==0 && !ctrlc && fgets(line, MAXCMD-1, falert) != NULL) {
    sscanf(line,"%s %d %lf",fname,&nexp,&exptime);
    fprintf(stderr,"%s %d %lf\n",fname, nexp, exptime);
    if (type != 82) status = sendccd(toccd.port, toccd.server,"-ALERT",default_timeout);

    filter = getfilt(fname);
    if (filter > 0 && filter <= MAXFILT) {
      dofilter(filter,toport.port, toport.server, toccd.port, toccd.server, togccd.port, togccd.server);
      dfoc = focoff[filter] - foc0;
      fprintf(stderr,"Adjusting telescope focus: %d\n",dfoc);
      if (dfoc != 0) {
        sprintf(command,"XFOCUS %ld %ld %ld",dfoc,dfoc,dfoc);
        status = sendport(toport.port,toport.server,command,ret,MAXCMD);
        sprintf(command,"GUIDEFOC %ld",dfoc);
        fprintf(stderr,"Compensating guider focus...\n");
        status = sendccd(togccd.port, togccd.server,command);
      }
      foc0 = focoff[filter];
    }
    for (i=0 ; i<nexp ; i++) {
      status = do_exposure(toport.port,toport.server,toccd.port,toccd.server,
                                 exptime,i);
    if (ctrlc) break;
    }
  }
  fclose(falert);
  donealert = TRUE;
  status = sendccd(toccd.port, toccd.server,"+ALERT",default_timeout);
}
