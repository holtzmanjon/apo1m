#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

main()
{
 char commandline[80];
 int da_year;
 char da_mon, da_day;
 unsigned char ti_hour, ti_min, ti_sec, ti_hund;

 sprintf(commandline,"SETTIME 18 11 10 17 50 23");

 sscanf(commandline+8),
         sscanf(commandline+8,"%d%d%d%d%d%d",
           &da_year, &da_mon, &da_day, &ti_hour, &ti_min, &ti_sec);

 ti_hund = 0;
 fprintf(stderr,"setting date/time: %d %d %d %d %d %d %d\n",
            da_year, da_mon, da_day,
            ti_hour, ti_min, ti_sec, ti_hund);

}

