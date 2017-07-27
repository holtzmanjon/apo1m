#include <dos.h>
#include <stdio.h>
#include "mytype.h"

void mygettime(struct date *myd, struct mytime *myt)
{
  struct time t;
  struct date d;
  unsigned long tics, secs;
  unsigned int counter, counter1, counter2;
  long double fsec;
  unsigned char mon_arr[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  // data area for BIOS tics value
  const long far *BiosTics = (long far *)MK_FP(0x0040, 0x006c);

  // Get system date 
  getdate(&d);
  
  // Latch value to insure accurate reads of 16-bit timer
  outportb(0x0043,0x0006);

  // Read 16-bit times with two reads, for LSB and MSB
  counter1 = inportb(0x0040);
  counter2 = inportb(0x0040);

  // Construct 16-bit timer value
  counter = (counter2<<8) + counter1;

  // Calculate time since midnight in 32-bit tics
  tics = *BiosTics;
 
  // Convert tics into hrs, mins, secs, hundredths
  fsec = (long double) tics * 0.054925491;
  secs = (unsigned long) (fsec);// seconds since midnight rounded down
  fsec -= secs;
  myt->ti_hour = secs / 3600; 
  secs -= myt->ti_hour*3600;
  myt->ti_min =  (int) secs / 60;
  secs -= myt->ti_min*60;
  myt->ti_sec =  (int) secs;
  myt->ti_hund = (double)fsec*100 + 5.4925491-counter*0.000083810;

  // Handle possible out of range values
  if (myt->ti_hund >= 100.) {
    myt->ti_hund -= 100;
    myt->ti_sec++;
    if (myt->ti_sec>=60) {
      myt->ti_sec -= 60;
      myt->ti_min++;
      if (myt->ti_min>=60) {
        myt->ti_min -= 60;
        myt->ti_hour++;
      }
      if (myt->ti_hour>=24) {
        myt->ti_hour -= 24;
        d.da_day++;
        if (d.da_day > mon_arr[d.da_mon - 1])
        {
           d.da_day = 1;
           d.da_mon++;

           if (d.da_mon == 13) {
             d.da_mon = 1;
             d.da_year++;
           }
        }
      }
    }
  }
  myd->da_day = d.da_day;
  myd->da_mon = d.da_mon;
  myd->da_year = d.da_year;

//  compare with system clock for debugging
/*
gettime(&t);
fprintf(stderr,"%u %Lf %d %d %d %d %d %d %d %f\n",
       counter,fsec,
       t.ti_hour, t.ti_min, t.ti_sec, t.ti_hund,
       myt->ti_hour, myt->ti_min, myt->ti_sec, myt->ti_hund);
*/
}
