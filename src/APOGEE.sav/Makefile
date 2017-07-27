FC=gfortran
CC=gcc
CFLAGS=

ap7p: ap7p.c
	$(CC) -O2 -g -Wall -fno-strength-reduce -c -DLINUX -D_APG_PPI ap7p.c
libapogee:
	(cd apogee ; make)
alta: alta.cpp
	g++ -c -g -fPIC -I. -I/opt/apogee/include/libapogee-3.0/apogee -I/opt/apogee/include -I/usr/include/tcl -DLINUX -DAPOGEE_ALTAF apgSampleCmn.cpp
	g++ -c -g -fPIC -I. -I/opt/apogee/include -I/opt/apogee/include/libapogee-3.0/apogee -I/opt/apogee/include/libapogee-3.0 alta.cpp
#	cc -o alta alta_main.c alta.o apgSampleCmn.o -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl
#	g++ -g -o alta alta.o apgSampleCmn.o -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl
