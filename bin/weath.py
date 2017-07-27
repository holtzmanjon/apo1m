#!/usr/bin/env python
from socket import *    # import *, but we'll avoid name conflict

sock = socket(AF_INET, SOCK_DGRAM)
messout = "all"
sock.sendto(messout, ('192.41.211.39', 6251))
messin, server = sock.recvfrom(1024)
sock.close()

# print received string
print messin

# strip off beginning.
# replace = with =', and replace space with space'
# for example dewPoint=14.0 becomes dewPoint='14.0'
start = messin.find('timeStamp')
stop = messin.find('end')
stuff = messin[start:stop].replace ("=","='").replace (" ","'; ")

# exec - causes the pieces to become global variables
print '%s' % (messin)
print '%s' % (stuff)
exec(stuff)

#print 'pressure=%s, airtemp=%s, humidity=%s' % (pressure,airtemp,humidity)
#print '%s' % (airtemp)
print '%s' % (encl25m)


# To broadcast status via udp, uncomment last line
s=socket(AF_INET,SOCK_DGRAM)
s.setsockopt(SOL_SOCKET,SO_BROADCAST,1)
port=19999
host="192.41.211.255"
addr=(host,port)
command='shutters=%s' % (encl25m)
if ( encl25m == "1" ) :
  command='shutters=40' 
else:
  command='shutters=0' 
s.sendto(command,addr)
