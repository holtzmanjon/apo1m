#!/usr/bin/env python

import subprocess
import datetime
import time

out=open('gettemp.dat','a')

def snmpget(address, target) :
    return int(subprocess.check_output(['snmpget','-v','1','-c','public',address,target]).split()[3])/100.

while True :
    date=datetime.datetime.now()

    t1=snmpget('10.75.0.18','1.3.6.1.4.1.20916.1.7.1.1.1.2.0')
    t2=snmpget('10.75.0.18','1.3.6.1.4.1.20916.1.7.1.2.1.2.0')
    t3=snmpget('10.75.0.18','1.3.6.1.4.1.20916.1.7.1.2.2.2.0')
    t4=snmpget('10.75.0.25','1.3.6.1.4.1.20916.1.7.1.1.1.2.0')
    t5=snmpget('10.75.0.25','1.3.6.1.4.1.20916.1.7.1.2.1.2.0')
    t6=snmpget('10.75.0.25','1.3.6.1.4.1.20916.1.7.1.2.2.2.0')

    print(('{:8.2f}'*6).format(t1,t2,t3,t4,t5,t6))
    out.write(('{:8.2f}'*6+'\n').format(t1,t2,t3,t4,t5,t6))
    time.sleep(60)

'''
whil
while (1) 
 set date = `date +%s`
 @ date = $date - 1224000000
 set t1 = `snmpget -v 1 -c public 10.75.0.18 1.3.6.1.4.1.20916.1.7.1.1.1.2.0 | awk '{print $4}'`
 set t2 = `snmpget -v 1 -c public 10.75.0.18 1.3.6.1.4.1.20916.1.7.1.2.1.2.0 | awk '{print $4}'`
 set t3 = `snmpget -v 1 -c public 10.75.0.18 1.3.6.1.4.1.20916.1.7.1.2.2.2.0 | awk '{print $4}'`
 set t4 = `snmpget -v 1 -c public 10.75.0.25 1.3.6.1.4.1.20916.1.7.1.1.1.2.0 | awk '{print $4}'`
 set t5 = `snmpget -v 1 -c public 10.75.0.25 1.3.6.1.4.1.20916.1.7.1.2.1.2.0 | awk '{print $4}'`
 set t6 = `snmpget -v 1 -c public 10.75.0.25 1.3.6.1.4.1.20916.1.7.1.2.2.2.0 | awk '{print $4}'`
 
 echo $date $t1 $t2 $t3 $t4 $t5 $t6 >> gettemp.dat
 gettemp
 showtemp
 sleep 60
end
'''
