#include <stdio.h>
#include <string.h>
#include "slamac.h"

double tpoint_npae, tpoint_nrx, tpoint_nry, tpoint_tf, tpoint_ih, tpoint_id;
double tpoint_np, tpoint_ch, tpoint_me, tpoint_ma, tpoint_fo, tpoint_ie;
double tpoint_ia, tpoint_ca, tpoint_an, tpoint_aw;

unsigned read_mount_correction_model(char *);

main()
{

   read_mount_correction_model("000425.mod");

   printf("%12.4f %12.4f%12.4f %12.4f%12.4f %12.4f%12.4f %12.4f %12.4f\n",
       tpoint_ia, tpoint_ie, tpoint_npae, tpoint_ca, tpoint_an, tpoint_aw,
       tpoint_nrx, tpoint_nry);
}


unsigned read_mount_correction_model(char *filename)
{
  FILE *mcfile = fopen(filename, "r");
  char buffer[256];
  char *label;
  char *term;
  int terms;

  if (!mcfile)
      return -1;


  tpoint_npae = 0;
  tpoint_nrx = 0;
  tpoint_nry = 0;
  tpoint_tf = 0;
  tpoint_ih = 0;
  tpoint_id = 0;
  tpoint_np = 0;
  tpoint_ch = 0;
  tpoint_me = 0;
  tpoint_ma = 0;
  tpoint_fo = 0;
  tpoint_ie = 0;
  tpoint_ia = 0;
  tpoint_ca = 0;
  tpoint_an = 0;
  tpoint_aw = 0;
  while (fgets(buffer, 255, mcfile))
                        {
                                label = strtok(buffer, " ");
                                term = strtok(NULL, " ");

                // compare for terms whose names are more than 2 characters
                                if (strcasecmp(label, "npae") == 0)
                                {
                                        tpoint_npae = atof(term)*DAS2R;
                                        terms--;
                                }
                                else if (strcasecmp(label, "nrx") == 0)
                                {
                                        tpoint_nrx = atof(term)*DAS2R;
                                        terms--;
                                }
                                else if (strcasecmp(label, "nry") == 0)
                                {
                                        tpoint_nry = atof(term)*DAS2R;
                                        terms--;
                                }
                                else
                                {
                                        unsigned i = (label[1] << 8) + label[0];
                                        terms--;
                                        switch (i)
                                        {
                                case 'TF': tpoint_tf = atof(term)*DAS2R; break;
                                case 'IH': tpoint_ih = atof(term)*DAS2R; break;
                                case 'ID': tpoint_id = atof(term)*DAS2R; break;
                                case 'NP': tpoint_np = atof(term)*DAS2R; break;
                                case 'CH': tpoint_ch = atof(term)*DAS2R; break;
                                case 'ME': tpoint_me = atof(term)*DAS2R; break;
                                case 'MA': tpoint_ma = atof(term)*DAS2R; break;
                                case 'FO': tpoint_fo = atof(term)*DAS2R; break;
                                case 'IE': tpoint_ie = atof(term)*DAS2R; break;
                                case 'IA': tpoint_ia = atof(term)*DAS2R; break;
                                case 'CA': tpoint_ca = atof(term)*DAS2R; break;
                                case 'AN': tpoint_an = atof(term)*DAS2R; break;
                                case 'AW': tpoint_aw = atof(term)*DAS2R; break;
                                default: terms++;
                                        }
                                }
                        }

                // close the file
                fclose(mcfile);
                // flip the signs of the coefficients
                tpoint_npae = -1 * tpoint_npae ;
                tpoint_nrx = -1 * tpoint_nrx ;
                tpoint_nry = -1 * tpoint_nry ;
                tpoint_tf = -1 * tpoint_tf ;
                tpoint_ih = -1 * tpoint_ih ;
                tpoint_id = -1 * tpoint_id ;
                tpoint_np = -1 * tpoint_np ;
                tpoint_ch = -1 * tpoint_ch ;
                tpoint_me = -1 * tpoint_me ;
                tpoint_ma = -1 * tpoint_ma ;
                tpoint_fo = -1 * tpoint_fo ;
                tpoint_ie = -1 * tpoint_ie ;
                tpoint_ia = -1 * tpoint_ia ;
                tpoint_ca = -1 * tpoint_ca ;
                tpoint_an = -1 * tpoint_an ;
                tpoint_aw = -1 * tpoint_aw ;
}
