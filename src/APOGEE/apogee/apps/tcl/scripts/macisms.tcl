.main.observe configure -font "System 10" -highlightbackground gray -width 7
.main.abort   configure -font "System 10" -highlightbackground gray -width 7
.main.pause   configure -font "System 10" -highlightbackground gray -width 7
.main.resume  configure -font "System 10" -highlightbackground gray -width 7
.main.seldir  configure -highlightbackground gray
.main.ssite   configure -highlightbackground gray
.main.numexp  configure -width 7
.main.exptype configure -width 7
.main configure -width 620
.mbar configure -width 620
place .main.numexp -x 100
place .main.exptype -x 100
place .main.exposure -x 100
label .main.tnumexp -text "Num. frames" -bg gray
label .main.texptype -text "Exp. type" -bg gray
label .main.texposure -text "Exposure" -bg gray
.main.imagename configure -font "System 12" -relief sunken -borderwidth 2
.main.seqnum configure -font "System 12" -relief sunken -borderwidth 2
.main.lobias configure -font "System 12" -relief sunken -borderwidth 2
.main.hibias configure -font "System 12" -relief sunken -borderwidth 2
.mbar.temp  configure -text "Temp."
.mbar.calib  configure -text "Calib."
place .main.imagename -x 100 -y 143
place .main.seqnum -x 234 -y 143
place .main.tnumexp -x 2 -y 50
place .main.texptype -x 2 -y 80
place .main.texposure -x 2 -y 20
place .mbar.file -x 0 -y 0
place .mbar.edit -x 80 -y 0
place .mbar.observe -x 160 -y 0
place .mbar.temp -x 260 -y 0
place .mbar.tools -x 1000
place .mbar.calib -x 355 -y 0
place .mbar.help -x 530 -y 0
place .mbar.leds -x 443 -y 0
place .main.lobias -x 220 -y 246
place .main.hibias -x 280 -y 246
wm geometry .drift 450x360
set iy 10
foreach item "target ra dec equinox observer telescope instrument site latitude longitude" {
   place .main.l$item -x 350 -y $iy
   place .main.v$item -x 450 -y $iy
   incr iy 24
}
place .main.ssite -x 541 -y 176
.mbar.help.m delete 1 end
.mbar.help.m add command -label "Users Guide (OS X)" -command {exec open $TKAPOGEE/doc/user-guide.html}
set SCOPE(numframes) 1
exec mkdir -p calibrations
exec mkdir -p calibrations/dark
exec mkdir -p calibrations/flat
exec mkdir -p calibrations/skyflat
exec mkdir -p calibrations/zero



