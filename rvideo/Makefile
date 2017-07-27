VISTALIB=/usr/local/src/xvista/lib
default: rvideo rvideo2
rvideo: rvideo.o xgets.o
	cc -o rvideo rvideo.c xgets.o $(VISTALIB)/xtv.linux.a \
	-L/usr/X11R6/lib -lX11 -lreadline -lhistory -lcurses
rvideo2: rvideo.c xgets.o
	cc -o rvideo2 -DQUICKCAM4000PRO rvideo.c xgets.o $(VISTALIB)/xtv.linux.a \
	-L/usr/X11R6/lib -lX11 -lreadline -lhistory -lcurses
install:
	cp rvideo /usr/local/bin
	chmod 4755 /usr/local/bin/rvideo
	cp rvideo2 /usr/local/bin
	chmod 4755 /usr/local/bin/rvideo2
clean:
	-rm *.o rvideo
