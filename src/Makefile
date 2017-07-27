FC=gfortran
CC=gcc

MACHINE=linux64
CFLAGS=-DHAVETOCC -DHAVECCD -UMSBF -Dlinux -DSOCKET
CURSES=ncurses
LDFLAGS=-L/usr/X11R6/lib

VISTALIB=/usr/local/src/xvista/lib
BINDIR=/usr/local/bin

all: command port sockport status accd fccd lccd power comment
command: io.o error.o send.o vector.o nrutil.o focus.o getresp.o \
	powerutils.o filter.o socket.o coord.o
	$(CC) -o ../bin/command $(CFLAGS) -UHAVEPI -UCURSES \
	command.c readstatus.c writeccdstatus.c io.o error.o powerutils.o \
	filter.o send.o vector.o nrutil.o focus.o getresp.o socket.o coord.o \
	-lslalib -lc -lm
filt: filt.o filter.o
	$(CC) -o ../bin/filt filt.o filter.o
sockport: socket.o send.o
	$(CC) -o ../bin/sockport $(CFLAGS) -UNEWCOM sockport.c \
	socket.o send.o -lm -lc
port: getresp.o socket.o
	$(CC) -o ../bin/port $(CFLAGS) -UNEWCOM port.c \
	getresp.o socket.o -lm -lc
status: send.o io.o vector.o nrutil.o focus.o getresp.o filter.o socket.o
	$(F77) $(LDFLAGS) -o ../bin/status $(CFLAGS) -DAPOGEE \
	status.c readstatus.c send.o io.o vector.o nrutil.o focus.o filter.o \
	getresp.o socket.o -lslalib -lmongo -lreadline -lhistory -l$(CURSES) \
        -lX11 $(F77LIBS) -lm -lc 
rstatus: io.o vector.o nrutil.o focus.o getresp.o filter.o
	$(F77) $(LDFLAGS) -o ../bin/rstatus $(CFLAGS) -DAPOGEE -DRSTATUS \
	status.c readstatus.c io.o vector.o nrutil.o focus.o filter.o \
	getresp.o -lslalib -lmongo -l$(CURSES) \
        -lX11 $(F77LIBS) -lm -lc 
lccd: io.o send.o focus.o error.o \
	vector.o nrutil.o fits.o getresp.o filter.o \
	zap.o quick.o socket.o powerutils.o 
	$(F77) $(LDFLAGS) -o ../bin/lccd $(CFLAGS) \
	-DE2V2K -DLEACH -DNEWCOM \
	ccd.c  readstatus.c error.o \
	writeccdstatus.c io.o send.o focus.o vector.o nrutil.o \
	fits.o getresp.o writehtml.c filter.o zap.o quick.o \
	leach/leach.o socket.o powerutils.o $(VISTALIB)/xtv.$(MACHINE).a \
	leach/libastropciAPI.a -lslalib -lX11  -lc -lm -lncurses
accd: io.o send.o focus.o error.o \
	vector.o nrutil.o fits.o getresp.o filter.o \
	zap.o quick.o socket.o powerutils.o 
	#$(F77) $(LDFLAGS) -o ../bin/accd $(CFLAGS) \
	#-DAPOGEE -DNEWCOM \
	#ccd.c  readstatus.c error.o \
	#writeccdstatus.c io.o send.o focus.o vector.o nrutil.o \
	#fits.o getresp.o writehtml.c filter.o zap.o quick.o \
	#APOGEE/alta.o socket.o powerutils.o $(VISTALIB)/xtv.$(MACHINE).a \
	#-lslalib -lX11  -lc -lm -lncurses
	$(F77) -o ../bin/accd $(CFLAGS) -DAPOGEE -DKAF1603 -DNEWCOM ccd.c readstatus.c error.o writeccdstatus.c io.o send.o focus.o vector.o nrutil.o fits.o getresp.o writehtml.c filter.o zap.o quick.o APOGEE/alta.o APOGEE/apgSampleCmn.o socket.o powerutils.o $(VISTALIB)/xtv.$(MACHINE).a -lslalib -L/opt/apogee/lib -lccd -lcfitsio -lapogee -lwcs -lusb-1.0 -lcurl -ltcl -lX11 -lc -lm -lncurses
alta:
	(cd APOGEE; make alta)
apogee.o: apogee.c
	$(CC) -O2 -g -Wall -fno-strength-reduce -c -DLINUX -D_APG_PPI apogee.c
libapogee:
	(cd apogee ; make)
libfli:
	(cd libfli-1.104 ; make)
slitview: io.o send.o focus.o error.o \
	vector.o nrutil.o fits.o getresp.o filter.o \
	zap.o quick.o apogee2.o socket.o update.o
	$(F77) $(LDFLAGS) -o ../bin/slitview $(CFLAGS) \
	-DAPOGEE -DNEWCOM -DSLITVIEW -DOFFSET -DCURSES \
	ccd.c  readstatus.c error.o \
	writeccdstatus.c io.o send.o focus.o vector.o nrutil.o \
	fits.o getresp.o writehtml.c filter.o zap.o quick.o update.o \
	apogee2.o socket.o $(VISTALIB)/xtv.$(MACHINE).a \
	apogee/libapogee.a -lslalib -lX11  -lc -lm -lncurses
apogee2.o: apogee.c
	$(CC) -o apogee2.o -O2 -g -Wall -fno-strength-reduce -c -DLINUX -D_APG_PPI -DOFFSET apogee.c
gccd: io.o send.o focus.o error.o readusno.o \
	vector.o nrutil.o fits.o getresp.o filter.o \
	zap.o quick.o socket.o
	$(F77) $(LDFLAGS) -o ../bin/gccd $(CFLAGS) \
	-DGUIDER -DSPEC -DLYNXX -DLOCALCOM -DNEWCOM -DHAVEREMOTE \
	ccd.c  readstatus.c error.o readusno.o \
	writeccdstatus.c io.o send.o focus.o vector.o nrutil.o \
	fits.o getresp.o writehtml.c filter.o zap.o quick.o \
	socket.o $(VISTALIB)/xtv.$(MACHINE).a \
	-lslalib -lX11  -lc -lm
fccd: io.o send.o focus.o error.o readusno.o \
	vector.o nrutil.o fits.o getresp.o filter.o \
	zap.o quick.o socket.o 
	$(F77) $(LDFLAGS) -o ../bin/fccd $(CFLAGS) \
	-DFLI132 -DGUIDER -DE2V1K -DFLI -DNEWCOM \
	ccd.c readstatus.c error.o FLI/fli.o readusno.o \
	writeccdstatus.c io.o send.o focus.o vector.o nrutil.o \
	fits.o getresp.o writehtml.c filter.o zap.o quick.o \
	socket.o $(VISTALIB)/xtv.$(MACHINE).a \
	FLI/libfli-1.104/libfli.a -lslalib -lX11  -lc -lm
#	libfli-1.32/libfli.a -lslalib -lX11  -lc -lm
ccd: ccd.o io.o send.o focus.o vector.o error.o \
	nrutil.o fits.o getresp.o filter.o \
	zap.o quick.o socket.o
	$(F77) $(LDFLAGS) -o ../bin/ccd $(CFLAGS) \
	-USPEC -DHAVEPI -DLOCALCOM -DHAVEREMOTE \
	ccd.c  readstatus.c error.o \
	writeccdstatus.c io.o send.o  focus.o vector.o nrutil.o \
	fits.o getresp.o writehtml.c filter.o zap.o quick.o \
	socket.o $(VISTALIB)/xtv.$(MACHINE).a \
	-lslalib -lX11 -lc -lm
power: power.o powerutils.o
	$(CC) $(LDFLAGS) -o ../bin/power power.c powerutils.c -lX11
xlamps: power.o powerutils.o
	$(CC) -DLAMPS $(LDFLAGS) -o ../bin/xlamps power.c powerutils.c -lX11
comment: comment.o util.o 
	$(CC) -o ../bin/comment $(CFLAGS) -DAPOGEE \
	comment.o util.o writehtml.c
shortlog: shortlog.o
	$(CC) -o ../bin/shortlog $(CFLAGS) -DAPOGEE \
	shortlog.o writehtml.c
writehtml: writehtml.o
	$(CC) -o ../bin/writehtml $(CFLAGS) -DMAIN -DAPOGEE writehtml.c
install-bin:
	cp ../bin/command $(BINDIR)/command
	cp ../bin/accd $(BINDIR)/accd
	cp ../bin/fccd $(BINDIR)/fccd
	cp ../bin/lccd $(BINDIR)/lccd
	cp ../bin/status $(BINDIR)/status
install:
	cp ../bin/command $(BINDIR)/command
	#-cp ../bin/ccd $(BINDIR)/ccd
	#cp ../bin/gccd $(BINDIR)/gccd
	-rm $(BINDIR)/accd
	cp ../bin/accd $(BINDIR)/accd
	chmod 4755 $(BINDIR)/accd
	cp ../bin/fccd $(BINDIR)/fccd
	chmod 4755 $(BINDIR)/fccd
	cp ../bin/lccd $(BINDIR)/lccd
	chmod 4755 $(BINDIR)/lccd
	cp ../bin/status $(BINDIR)/status
	cp ../bin/port $(BINDIR)/port
	cp ../bin/sockport $(BINDIR)/sockport
	cp ../bin/power $(BINDIR)/power
	cp ../bin/tcomm ../bin/dotcomm ../bin/killtcomm ../bin/killrvideo $(BINDIR)/
	cp ../bin/pysend $(BINDIR)/
	cp ../bin/transfer ../bin/dotransfer $(BINDIR)/
	cp ../bin/power1m ../bin/ms ../bin/ms1 ../bin/ms2 ../bin/ms3 $(BINDIR)/
	cp ../bin/dosftp $(BINDIR)/
	cp ../bin/alive $(BINDIR)/
	cp ../bin/restart_ccd $(BINDIR)/
	cp ../bin/dorestart_ccd $(BINDIR)/
	cp ../bin/shutter35m ../bin/shut35a ../bin/readtemp $(BINDIR)/
	cp ../bin/CLEAR_LOUVER $(BINDIR)/
	cp ../bin/comment /home/httpd/cgi-bin/comment.pl

clean:
	-rm -f *.o *.$(MACHINE) comment
	(cd apogee ; make clean)

remote:
	scp xfer ganymede.nmsu.edu:/usr/local/bin
	scp doxfer ganymede.nmsu.edu:/usr/local/bin
	scp gxfer ganymede.nmsu.edu:/usr/local/bin
	scp dogxfer ganymede.nmsu.edu:/usr/local/bin
	scp tcomm ganymede.nmsu.edu:/usr/local/bin
	scp copyscript ganymede.nmsu.edu:/usr/local/bin
	scp alive ganymede.nmsu.edu:/usr/local/bin
getsao: getsao.o
	cc -o ../bin/getsao getsao.o -lm -lc
testfli: fli.c
	cc -DMAIN -o ../bin/testfli fli.c libfli-1.104/libfli.a -lm
testapogee: apogee.c
	cc -DMAIN -O2 -g -Wall -fno-strength-reduce -DLINUX -D_APG_PPI -o ../bin/testapogee apogee.c  apogee/libapogee.a
leach3: leach3.c
	c++ -IARC_API/3.5/CArcDevice/src -IARC_API/3.5/CArcDeinterlace/src -IARC_API/3.5/CArcFitsFile/src -o ../bin/leach3 leach3.c -LARC_API/3.5/Release/x64 -lCArcDevice -lCArcDeinterlace
testleach: leach.o
#	cc -o ../bin/testleach leach.o leach/libastropciAPI.a -lm
	cc -o ../bin/testleach leach.o ARC_API/3.0/Release/x64/libCameraAPI.so
client: client.o socket.o
	cc -o ../bin/client client.o socket.o
server: server.o socket.o
	cc -o ../bin/server server.o socket.o
