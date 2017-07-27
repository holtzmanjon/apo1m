CFLAGS=-c -mh -DSOCKET
CPPFLAGS=-c -mh -DSOCKET
LFLAGS=-mh
SOCKLIBS = hsocket.lib hnetlib.lib hpc.lib hconfig.lib
tocc: toccauto.obj altaz_c.obj altaz_n.obj command.obj \
 equat_f.obj error.obj evtimer.obj globals.obj guiding.obj io.obj key_util.obj \
 keypad.obj ocs.obj pcx.obj serial.obj shutdown.obj sounds.obj \
 systimer.obj tcs.obj tcs_math.obj tech80.obj \
 tracking.obj util.obj uts10.obj weather.obj window.obj inst.obj scf.obj \
 gettime.obj adc12.obj cp4016.obj guider.obj filter.obj tertiary.obj socket.obj
	tlib tocc -+altaz_c.obj -+altaz_n.obj -+command.obj -+io.obj
	tlib tocc -+equat_f.obj -+error.obj -+evtimer.obj -+globals.obj -+ guiding.obj -+key_util.obj 
	tlib tocc -+keypad.obj -+ocs.obj -+pcx.obj -+serial.obj -+shutdown.obj 
	tlib tocc -+sounds.obj -+systimer.obj -+tcs.obj -+tcs_math.obj 
	tlib tocc -+tracking.obj -+window.obj -+uts10.obj -+util.obj  -+adc12.obj
	tlib tocc -+weather.obj -+tech80.obj -+inst.obj -+scf.obj -+gettime.obj -+cp4016.obj
	tlib tocc -+guider.obj -+filter.obj  -+tertiary.obj -+socket.obj
	bcc $(LFLAGS) -etocc toccauto.obj tocc.lib ..\slalib\slalib_c.lib \
	  $(SOCKLIBS)
socket: socket.obj
	bcc $(LFLAGS) -esocket socket.obj $(SOCKLIBS)
comm: comm.obj
	bcc -ecomm comm.obj
telpos: telpos.obj
	bcc -etelpos telpos.obj
w: w.obj ioshort.obj
	bcc -ew w.obj serial.obj ioshort.obj gettime.obj
adc10b: adc10b.obj adc10drv.obj
	bcc $(LFLAGS) -eadc10b adc10b.obj adc10drv.obj
write:	write.obj	
	bcc $(LFLAGS) -ewrite write.obj
