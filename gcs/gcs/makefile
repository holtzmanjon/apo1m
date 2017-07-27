CFLAGS=-c -mm
CPPFLAGS=-c -mm 
#CPPFLAGS=-c -mm -DBIGCCD 
#CPPFLAGS=-c -mm -Dno_filter -Dno_guider -Dno_pc38 -DBIGCCD
LFLAGS=-mm
gcs: gcs.obj io.obj fits.obj systimer.obj evtimer.obj error.obj median.obj \
      gettime.obj pcx.obj filter.obj guider.obj ccd.obj scf.obj
	tlib speclib -+io.obj -+fits.obj -+systimer.obj -+evtimer.obj 
	tlib speclib -+error.obj -+median.obj -+gettime.obj -+pcx.obj 
	tlib speclib -+filter.obj -+guider.obj -+ccd.obj -+scf.obj
	bcc $(LFLAGS) -egcs gcs.obj speclib.lib mbclynx.lib
gcsbig: gcs.obj io.obj fits.obj systimer.obj evtimer.obj error.obj median.obj \
      gettime.obj pcx.obj filter.obj guider.obj ccd.obj scf.obj
	tlib speclib -+io.obj -+fits.obj -+systimer.obj -+evtimer.obj 
	tlib speclib -+error.obj -+median.obj -+gettime.obj -+pcx.obj 
	tlib speclib -+filter.obj -+guider.obj -+ccd.obj -+scf.obj
	bcc $(LFLAGS) -egcs gcs.obj speclib.lib mbck0216.lib
spec: spec.obj io.obj fits.obj systimer.obj evtimer.obj error.obj median.obj \
      gettime.obj pcx.obj filter.obj guider.obj
	tlib speclib -+io.obj -+fits.obj -+systimer.obj 
	tlib speclib -+evtimer.obj -+error.obj -+median.obj
	tlib speclib -+gettime.obj -+pcx.obj -+filter.obj -+guider.obj
	bcc $(LFLAGS) -espec spec.obj speclib.lib mbclynx.lib
oldspec: spec.obj io.obj fits.obj systimer.obj evtimer.obj error.obj \
      median.obj gettime.obj
	bcc $(LFLAGS) -eoldspec spec.obj io.obj fits.obj error.obj \
              evtimer.obj systimer.obj median.obj gettime.obj mbclynx.lib
test: test.obj
	bcc $(LFLAGS) -etest test.obj fits.obj mbck0216.lib
a: a.obj
	bcc $(LFLAGS) -ea a.obj
new: new.obj io.obj pcx.obj systimer.obj evtimer.obj
	bcc $(LFLAGS) -enew new.obj io.obj pcx.obj systimer.obj evtimer.obj


