#include <stdio.h>

/* routines to insert values into FITS headers */
void inheadset(char *card, int val, char *header)
{
   int i, j;
   /* Find the proper location for this card, either an existing location, or
      the first free slot */
   i = getspace(card, header);
   /* Blank out the record */
   for (j=i;j<i+80;j++) header[j] = ' ';
   /* Put in the card name. Since sprintf puts in a terminating null, 
      remove it*/
   sprintf(header+i,"%-8s",card);
   header[i+8] = '=';
   header[i+9] = ' ';
   /* Put in the card value */
   sprintf(header+i+10,"%20d",val);
   header[i+30] = ' ';
}

void fheadset(char *card, double val, char *header)
{
   int i, j;
   /* Find the proper location for this card, either an existing location, or
      the first free slot */
   i = getspace(card, header);
   /* Blank out the record */
   for (j=i;j<i+80;j++) header[j] = ' ';
   /* Put in the card name. Since sprintf puts in a terminating null, 
      remove it*/
   sprintf(header+i,"%-8s",card);
   header[i+8] = '=';
   header[i+9] = ' ';
   /* Put in the card value */
   sprintf(header+i+10,"%20.13f",val);
   header[i+30] = ' ';
}

void lheadset(char *card, int val, char *header)
{
   int i, j;
   /* Find the proper location for this card, either an existing location, or
      the first free slot */
   i = getspace(card, header);
   /* Blank out the record */
   for (j=i;j<i+80;j++) header[j] = ' ';
   /* Put in the card name. Since sprintf puts in a terminating null, 
      remove it*/
   sprintf(header+i,"%-8s",card);
   header[i+8] = '=';
   header[i+9] = ' ';
   /* Put in the card value */
   if (val == 0)
     sprintf(header+i+29,"F");
   else
     sprintf(header+i+29,"T");
   header[i+30] = ' ';
}

void cheadset(char *card, char *val, char *header)
{
   int i, j, n;
   /* Find the proper location for this card, either an existing location, or
      the first free slot */
   i = getspace(card, header);
   /* Blank out the record */
   for (j=i;j<i+80;j++) header[j] = ' ';
   /* Put in the card name. Since sprintf puts in a terminating null, 
      remove it*/
   sprintf(header+i,"%-8s",card);
   header[i+8] = '=';
   header[i+9] = ' ';
   /* Put in the card value */
   n = strlen(val);
   if (n > 64) {
     val[64] = '\0';
     n = 64;
   }
   if (strcmp(card,"COMMENT") == 0 || strcmp(card,"HISTORY") == 0) 
     sprintf(header+i+10,"%-s",val);
   else {
     sprintf(header+i+10,"'%-s'",val);
     n += 2;
   }
   header[i+10+n] = ' ';
}

/* routine to find the location of a card. If card not found in header, return
   location of current END card, and copy END card up one record */
int getspace(char *card, char *header)
{
   int ncard, endcard, found, endfound;
   int icard, i, j,k;
  
   ncard = strlen(header) / 80;
   found = 0;
   endfound = 0;

   icard = 0; 
   while (found == 0 && endfound == 0 && ++icard <= ncard) {
     j = (icard-1)*80;
     if (strncmp(header+j,card,8) == 0) {
       found = 1;
     } else if (strncmp(header+j,"END ",4) == 0) {
       endfound = 1;
       endcard = icard;
     }
   }
   /* If we found the card, return its location */
   if (found == 1) {
     return(j);
   }

   /* If we found the end card, return its location and move the end card up */
   if (endfound == 1) {
     if (endcard < ncard) {
       i = (endcard-1)*80; 
       for (j=i;j<i+80;j++) header[j] = ' ';
       sprintf(header+i+80,"END ");
       header[i+84] = ' ';
       return(i);
     } 
   }

   /* if we get here then weve got a problem - no room in header */
   fprintf(stderr,"No room in FITS header for parameter: %s\n",card);
   return(0);
}

packfit(short *c, short *ibuf,int nbyte)
{
        vswab((char *)c, (char *)ibuf, nbyte);
        return (0);
}

packfit4(short *c, short *ibuf,int nbyte)
/*      This routine copies 4-byte integers from c to ibuf              */
/*      As it copies, it swaps bytes and shortints                      */
/*      This is the necessary conversion from VAX to Sun and back       */
/*      The copying can be done in place                                */
{
        int i;
        short temp;

        /* move the bytes most of the way to their new home, and swap   */
        vswab((char *)c, (char *)ibuf, nbyte);

        /* now swap the words */
        for (i = 0; i < nbyte/4; i++) {
                temp = *ibuf++;
                *(ibuf-1) = *ibuf;
                *(ibuf++) = temp;
        }
        return (0);
}


vswab(from, to, nbytes)
/* This is an implementation of the Unix swab routine to swap bytes     */
/* It is permissable for the source and destination to overlap          */
/*      The following is from the SunOS man page:                       */
/*   nbytes should be even and positive.  If nbytes  is  odd  and       */
/*   positive,  swab()  uses  nbytes  -  1 instead.  If nbytes is       */
/*   negative, swab() does nothing.                                     */
char    *from, *to;
int     nbytes;
{
        int i;
        char temp;

        /* move the bytes most of the way to their new home */
        vbcopy(from, to, nbytes);

        /* now swap them */
        nbytes /= 2;
        for (i = 0; i < nbytes; i++) {
                temp = *to++;
                *(to-1) = *to;
                *(to++) = temp;
        }
}

vbcopy(b1, b2, length)
/* This is an implementation of the Unix bcopy routine to copy strings  */
/* of bytes around in memory.  It is permissable for the source and     */
/* destination to overlap.                                              */
char    *b1, *b2;
int     length;
{
        int i;

        if (b1 == b2) {
                return;
        } else if (b1 > b2) {
                for (i = 0; i < length; i++) {
                        b2[i] = b1[i];
                }
        } else if (b1 < b2) {
                for (i = length - 1; i >= 0; i--) {
                        b2[i] = b1[i];
                }
        }
}
