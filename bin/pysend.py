#!/usr/bin/env python
from socket import *
s=socket(AF_INET,SOCK_DGRAM)
s.setsockopt(SOL_SOCKET,SO_BROADCAST,1)
port=19999
host="192.41.211.255"
addr=(host,port)
command="command=qu"
s.sendto(command,addr)