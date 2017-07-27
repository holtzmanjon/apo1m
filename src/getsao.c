#include "slamac.h"
#include <stdio.h>
#include <math.h>

main()
{
 double ra,dec,outra,outdec,outv;
 int outid,oldid,istar,getsao();

 while(1) {
  printf("enter ra, dec: ");
  scanf("%lf%lf",&ra,&dec);
  istar = getsao(ra,dec,1950.,0.,20.,&outra,&outdec,&outid,&outv,5.,&oldid,0,1);

  printf("istar: %d, outid: %d\n",istar, outid);
 }
}

int getsao(double saora, double saodec, double epoch, double vmin,
double vmax, double *outra, double *outdec, long *outid, double *outv,
double radius, long *oldid, int nold, int faint)
{
   int i, istart, iend, ifile;
   char inputfile[60], line[80];
   int rah,ram,decd,decm;
   long id;
   char sign;
   double ra,dec,ras,rap,decs,decp,vmag,cosdec,ddec,dra,dmin,dist;
   FILE *fp;

   *outid = -1;
   cosdec = cos(saodec*DD2R);
   dra = saora - radius/15/cosdec;
   istart = ( dra < 0 ? (int)dra - 1 : (int)dra );
   iend = (int) (saora + radius/15/cosdec);
   dmin = radius*radius;
   for (i=istart; i<=iend; i++) {
     ifile= (i< 0 ? i+24 : i);
     ifile= (ifile> 23 ? ifile-24 : ifile);
     if (faint) 
       sprintf(inputfile,"/home/export/sao/faint/sao%02d.dat\0",ifile+1);
     else
       sprintf(inputfile,"/home/export/sao/sao%02d.dat\0",ifile+1);
     fp = fopen(inputfile,"r");
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

int samestar(long id,long *oldid,int nold)
{
  int i;

  for (i=0; i<nold; i++) {
    if (id == *(oldid+i)) return(TRUE);
  }

  return(FALSE);
}

