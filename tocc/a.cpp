#include <stdio.h>

main()
{
 char commandline[80];
 int year, mon, day, hour, min, sec;
 int da_year;
 char da_mon, da_day;
 unsigned char ti_hour, ti_min, ti_sec, ti_hund;

 sprintf(commandline,"SETTIME 18 11 10 17 50 23");

 sscanf(commandline+8,"%d%d%d%d%d%d",
           &year, &mon, &day, &hour, &min, &sec);

 da_year = year;
 da_mon = mon;
 da_day = day;
 ti_hour = hour;
 ti_min = min;
 ti_sec = sec;
 ti_hund = 0;
 fprintf(stderr,"setting date/time: %d %d %d %d %d %d %d\n",
            da_year, da_mon, da_day,
            ti_hour, ti_min, ti_sec, ti_hund);

}

