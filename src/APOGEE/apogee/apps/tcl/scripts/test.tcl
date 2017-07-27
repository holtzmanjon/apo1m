#
# Define the global environment, everything lives under /opt/apogee
# Change TKAPOGEE to move the code somewhere else
#
set TKAPOGEE $env(TKAPOGEE)
set DEBUG 1
set RAWTEMP 0

#
# Load the procedures
#
source $TKAPOGEE/scripts/general.tcl
source $TKAPOGEE/scripts/display.tcl
source $TKAPOGEE/scripts/temperature.tcl
source $TKAPOGEE/scripts/calibration.tcl
source $TKAPOGEE/scripts/observe.tcl

# Create the status window. This window is used to display informative messages
# during initialization.
#

toplevel .status -width 500 -height 100
wm title .status "Apogee Camera Control"
wm geometry .status +20+30
label .status.msg -bg LightBlue -fg Black -text Initialising -width 50 -font "Helvetica 30 bold"
pack .status.msg


#
# Define the path to the shared libraries.
# These libraries are used to add facilities to the default tcl/tk
# wish shell. 
#
set libs $TKAPOGEE/lib

#
# Load the tcl interface to FITS (Flexible image transport system) disk files
# FITS is the standard disk file format for Astronomical data
#

showstatus "Loading FitsTcl"
load $libs/libfitsTcl.so

# Prepare for Guide Star Catalog access
package ifneeded gsc 3.1       [load $libs/libgsc.so]

# Prepare for Digital sky survey access
package ifneeded dss 3.1       [load $libs/libdss.so]

# Prepare for Oracle (target ephemeris prediction)
package ifneeded oracle 2.1    [load $libs/liboracle.so]

# Prepare for generic astrometry
package ifneeded xtcs 3.1      [load $libs/libxtcs.so]

# Prepare for Graphics widget package
package ifneeded BLT 2.4       [load $libs/libBLT24.so]

# Prepare for Ccd image buffering package
package ifneeded ccd 1.0       [load $libs/libccd.so]

# Load packages provided by dynamically loadable libraries
showstatus "Loading Digital Sky survey access"
package require dss
showstatus "Loading GSC catalog access"
package require gsc
showstatus "Loading Oracle"
package require oracle
showstatus "Loading graphics package"
package require BLT
namespace import blt::graph
showstatus "Loading CCD package"
package require ccd
lappend auto_path $libs/BWidget-1.2.1
package require BWidget

#
#
#  Set defaults for the directories used to store calibration 
#  library frames
#
set CALS(home) $env(HOME)/calibrations
set CALS(zero,dir) $CALS(home)/zero
set CALS(dark,dir) $CALS(home)/dark
set CALS(flat,dir) $CALS(home)/flat
set CALS(skyflat,dir) $CALS(home)/skyflat
set SCOPE(datadir) $env(HOME)

#
#  Define globals for temperature control

set TEMPS ""
set STATUS(tempgraph) 1


set DRIFTDELAY 0

set FRAME 1
set STATUS(readout) 0




#
#  Update status display
#
showstatus "Building user interface"
set SCOPE(driftdelay) 300000
set SCOPE(driftrows) 512
set SCOPE(telescope) " "
set SCOPE(instrument) " "
set SCOPE(equinox) "2000.0"

#
#  Create countdown window widgets
#
set f "Helvetica -30 bold"
toplevel .countdown -bg orange -width 535 -height 115
label .countdown.lf -text "Frame # " -bg orange -font $f
label .countdown.lt -text "Seconds : " -bg orange -font $f
label .countdown.f -text "???" -bg orange -font $f
label .countdown.t -text "???" -bg orange -font $f
place .countdown.lf -x 10 -y 40
place .countdown.f -x 140 -y 40
place .countdown.lt -x 230 -y 40
place .countdown.t -x 380 -y 40
wm withdraw .countdown


#
#  Create main window and its menus
#
wm title . "Apogee Camera Control"
frame .mbar -width 520 -height 30 -bg gray
menubutton .mbar.file -text "File" -fg black -bg gray -menu .mbar.file.m
menubutton .mbar.edit -text "Edit" -fg black -bg gray -menu .mbar.edit.m
menubutton .mbar.observe -text "Observe" -fg black -bg gray -menu .mbar.observe.m
menubutton .mbar.calib -text "Calibrate" -fg black -bg gray -menu .mbar.calib.m
menubutton .mbar.temp -text "Temperature" -fg black -bg gray -menu .mbar.temp.m
menubutton .mbar.help -text "Help" -fg black -bg gray -menu .mbar.help.m
menubutton .mbar.tools -text "Tools" -fg black -bg gray -menu .mbar.tools.m
pack .mbar
place .mbar.file -x 0 -y 0
place .mbar.edit -x 40 -y 0
place .mbar.observe -x 80 -y 0
place .mbar.temp -x 150 -y 0
place .mbar.calib -x 240 -y 0
place .mbar.tools -x 300 -y 0
place .mbar.help -x 460 -y 0
menu .mbar.file.m 
menu .mbar.edit.m
menu .mbar.observe.m
menu .mbar.calib.m
menu .mbar.temp.m
menu .mbar.tools.m
menu .mbar.help.m
#.mbar.file.m add command -label "Open" -command fileopen
.mbar.file.m add command -label "Save" -command savestate
#.mbar.file.m add command -label "Save As" -command filesaveas
.mbar.file.m add command -label "Exit" -command shutdown
.mbar.edit.m add command -label "Properties" -command showconfig
.mbar.observe.m add command -label "Single" -command "observe single"
.mbar.observe.m add command -label "Continuous" -command "observe multiple"
.mbar.observe.m add command -label "Snap-region" -command "observe region"
.mbar.observe.m add command -label "Reset full-frame" -command "observe fullframe"
.mbar.observe.m add command -label "Drift-scan" -command "toggle .drift"
### NYI .mbar.calib.m add command -label "Focus" -command "opencalibrate focus"
.mbar.calib.m add command -label "Load Dark calibration" -command "loadcalibrate dark"
.mbar.calib.m add command -label "Load Flat calibration" -command "loadcalibrate flat"
.mbar.calib.m add command -label "Load SkyFlat calibration" -command "loadcalibrate skyflat"
.mbar.calib.m add command -label "Load Zero calibration" -command "loadcalibrate zero"
.mbar.calib.m add command -label "Build Dark library" -command "opencalibrate dark"
.mbar.calib.m add command -label "Build Flat library" -command "opencalibrate flat"
.mbar.calib.m add command -label "Build SkyFlat library" -command "opencalibrate sky"
.mbar.calib.m add command -label "Build Zero library" -command "opencalibrate zero"
.mbar.temp.m add command -label "Cooler on" -command "setpoint on"
.mbar.temp.m add command -label "Cooler off" -command "setpoint off"
.mbar.temp.m add command -label "Cooler to ambient" -command  {set ok [confirmaction "Ramp temperature to ambient"] ; if {$ok} {setpoint amb}}
.mbar.temp.m add command -label "Plot averaged temps" -command {set RAWTEMP 0}
.mbar.temp.m add command -label "Plot raw temps" -command {set RAWTEMP 1}
### NYI .mbar.calib.m add command -label "Calculate WCS" -command "calcwcs"
### NYI .mbar.calib.m add command -label "Collimation" -command "observe collimate"
### NYI .mbar.tools.m add command -label "Auto-locate" -command autoIdentify
.mbar.tools.m add command -label "DSS" -command getDSS
.mbar.tools.m add command -label "GSC" -command getGSC
### NYI .mbar.tools.m add command -label "USNO" -command getUSNO
### NYI .mbar.tools.m add command -label "RNGC" -command getRNGC
.mbar.help.m add command -label "Users Guide" -command {exec netscape file:/opt/apogee/doc/user-guide.html &}


#
#  Initialize telescope/user variables
#
source $TKAPOGEE/scripts/tele_init.tcl
frame .main -bg gray -width 520 -height 330
pack .main -side bottom
set iy 10
foreach item "target ra dec equinox observer telescope instrument site latitude longitude" {
   label .main.l$item -bg gray -fg black -text $item
   place .main.l$item -x 300 -y $iy
   entry .main.v$item -bg white -fg black -relief sunken -width 12 -textvariable SCOPE($item)
   place .main.v$item -x 400 -y $iy
   incr iy 24 
}
button .main.ssite -bg gray -fg black -text "?" -font "Helvetica -10 bold" -command "wm deiconify .psite"
place .main.ssite -x 491 -y 178


#
#  Create main observation management widgets
#
#
SpinBox .main.exposure -width 7 -label "Exposure (in seconds) : " -font fixed -labelfont "fixed"  -range "0.0 1048.75 1" -textvariable SCOPE(exposure)
place .main.exposure -x 20 -y 20
SpinBox .main.numexp -width 12 -label "Number of frames : " -font fixed  -labelfont "fixed"  -range "1 1000 1" -textvariable SCOPE(numframes)
place .main.numexp -x 20 -y 50
set opts "Object Focus Acquire Flat SkyFlat Dark Zero"
ComboBox .main.exptype -width 15 -label "Exposure type : " -font fixed -labelfont "fixed"  -values "$opts" -textvariable SCOPE(exptype)
place .main.exptype -x 20 -y 80
set SCOPE(exptype) Object
button .main.seldir -width 24 -text "Configure data directory" -command "choosedir data data"
place .main.seldir -x 20 -y 110
label .main.lname -bg gray -fg black -text "File name :"
place .main.lname -x 20 -y 140
entry .main.imagename -width 16 -bg white -fg black -textvariable SCOPE(imagename)
place .main.imagename -x 90 -y 140
.main.imagename insert 0 test
entry .main.seqnum -width 6 -bg white -fg black -textvariable SCOPE(seqnum)
place .main.seqnum -x 220 -y 140
set SCOPE(seqnum) 1
button .main.observe -width 5 -height 2 -text "Observe" -bg gray -command startsequence
button .main.abort -width 5 -height 2 -text "Abort" -relief sunken -bg gray -command abortsequence
button .main.pause -width 5 -height 2 -text "Pause" -relief sunken -bg gray -command pausesequence
button .main.resume -width 5 -height 2 -text "Resume" -relief sunken -bg gray -command resumesequence

place .main.observe -x 20  -y 170
place .main.abort   -x 83 -y 170
place .main.pause   -x 148 -y 170
place .main.resume  -x 211 -y 170
checkbutton .main.autodisplay -bg gray  -text "Automatic display" -variable SCOPE(autodisplay)
place .main.autodisplay -x 20 -y 217
checkbutton .main.overwrite -bg gray  -text "Overwrite files" -variable SCOPE(overwrite)
place .main.overwrite -x 170 -y 217
checkbutton .main.autocalib -bg gray  -text "Automatic calibration (uses library frames)" -variable SCOPE(autocalibrate)
place .main.autocalib -x 20 -y 271
checkbutton .main.comptimer -bg gray  -text "Computer control of exposure time" -variable SCOPE(comptimer)
place .main.comptimer -x 20 -y 298
set SCOPE(lobias) 0
set SCOPE(hibias) 0
set SCOPE(comptimer) 0
entry .main.lobias -bg white -fg black -width 6 -textvariable SCOPE(lobias)
entry .main.hibias -bg white -fg black -width 6 -textvariable SCOPE(hibias)
place .main.lobias -x 220 -y 244
place .main.hibias -x 280 -y 244

checkbutton .main.autobias -bg gray  -text "Automatic bias subtraction" -variable SCOPE(autobias)
place .main.autobias -x 20 -y 244
.main.abort configure -relief sunken -fg LightGray
.main.pause configure -relief sunken -fg LightGray
.main.resume configure -relief sunken -fg LightGray
set SCOPE(autodisplay) 1
set SCOPE(autobias) 0
set SCOPE(autocalibrate) 0
set SCOPE(overwrite) 0
set STATUS(abort) 0
set STATUS(pause) 0
set STATUS(readout) 0
toplevel .psite -bg gray -width 225 -height 695
listbox .psite.l -width 30 -height 40
scrollbar .psite.s -orient vertical 
pack .psite.l -side left
pack .psite.s -side right -expand yes -fill y 
.psite.l configure -yscrollcommand ".psite.s set"  
.psite.s configure  -command ".psite.l yview" 
wm withdraw .psite
set fin [open $TKAPOGEE/config/sites.dat r]
set i 0
while { $i < 23 } {gets $fin rec ; incr i 1}
set i 0
while { [gets $fin rec] > -1 } {
   set s [split $rec ";"]
   .psite.l insert end [lindex $s 0]
   set SITES($i) [string trim "[lindex $s 1] | [lindex $s 2]" ]
   incr i 1
}
close $fin
bind  .psite.l <Double-1> {pastelocation}
wm title .psite "double-click to select"



#
#  Load all the help texts
#
showstatus "Loading online help"

source  $TKAPOGEE/scripts/tcl_aphelp.tcl
source  $TKAPOGEE/scripts/tcl_cathelp.tcl
source  $TKAPOGEE/scripts/tcl_telehelp.tcl
source  $TKAPOGEE/scripts/tcl_calhelp.tcl

#
#  Define a default sub-region
#  
set ACQREGION(xs) 200
set ACQREGION(xe) 64
set ACQREGION(ys) 200
set ACQREGION(ye) 64



#
#  Create the properties panels
#
showstatus "Loading camera properties"
toplevel .p 
wm title .p  "Properties (Alt-p to open/close window)"
wm geometry .p +20+90
wm withdraw .p
NoteBook .p.props -width 520 -height 300
.p.props insert 1 Telescope -text Telescope
.p.props insert 2 System -text System
.p.props insert 3 Geometry -text Geometry
.p.props insert 4 Temperature -text Temperature
.p.props insert 5 CCD -text CCD
.p.props insert 6 Filter -text Filter
.p.props insert 7 Catalogs -text Catalogs
pack .p.props
set spc "                "
set xsize 220
set ysize 30


#
#  Add all the actual properties to the panels. These are defined
#  in terms of type, range, default, and so forth.
#
showstatus "Configuring user interface"

set fin [open $TKAPOGEE/config/tcl_config.dat r]
while { [gets $fin rec] > -1 } {
   catch {
   if { [lsearch "Telescope System Geometry Temperature CCD Filter Catalogs" $rec] > -1 } {
      set idx [expr [lsearch "Telescope System Geometry Temperature CCD Filter Catalogs" $rec] +1]
      set f [.p.props getframe $rec]
      set subsys [string tolower $rec]
      set ix 0
      set iy 0
      showstatus "Configuring user interface ($subsys)"
   } else {
      set name   [string tolower [lindex $rec 0]]
      set lname "$name[string range $spc 0 [expr 16-[string length $name]]]"
      set cname $subsys.[lindex $rec 0]
      set range   [lindex $rec 1]
      set default [lindex $rec 2]
      if { $range == "READONLY" } {
         label $f.$name -text "$lname" -fg black -bg gray -relief sunken -width 27
      }
      if { $range == "URL" } {
         button $f.$name -text "$lname" -fg black -bg gray -command "choosedir catalogs $name" -width 27
      }
      if { $range == "ONOFF" } {
         ComboBox $f.$name -label "$lname" -helptext "$APHELP($name)" -labelfont "fixed" -bg gray -values "Off On" -width 10 -textvariable CONFIG($cname)
         $f.$name setvalue first
      }
      if { [llength [split $range :]] > 1 } {
         set vmin [lindex [split $range :] 0]
         set vmax [lindex [split $range :] 1]
         SpinBox $f.$name -label "$lname" -helptext "$APHELP($name)" -labelfont "fixed"  -range "$vmin $vmax 1" -width 10 -textvariable CONFIG($cname)
         set CONFIG($cname) $default
      }
      if { [llength [split $range "|"]] > 1 } {
         set opts [split $range "|"]
         ComboBox $f.$name -label "$lname" -helptext "$APHELP($name)" -labelfont "fixed"  -values "$opts" -width 10 -textvariable CONFIG($cname)
         set CONFIG($cname) $default
      }
      place $f.$name -x [expr 10+$xsize*$ix] -y [expr 10+$ysize*$iy]
      incr ix 1
      if { $ix > 1 } {
         set ix 0
         incr iy 1
      }
    }
   }
}
close $fin

#
#  Create the calibrations library control GUI
#

toplevel .cal -bg gray -width 500 -height -250
wm  title .cal "Calibrations (Alt-c to open/close window)"
wm withdraw .cal
foreach t "zero dark flat skyflat" {
   frame .cal.$t -bg gray
   label .cal.$t.l -bg gray -text $t -width 8 
   checkbutton .cal.$t.auto -text automatic -variable CALS($t,auto)
   button .cal.$t.sel -text "Select library" -bg gray -command "choosedir calibrations $t"
   SpinBox .cal.$t.tmin -label "   min temp" -helptext "$APHELP(caltmin)" -labelfont "fixed"  -range "-40 40 1" -width 4 -textvariable CALS($t,tmin)
   SpinBox .cal.$t.tmax -label "   max temp" -helptext "$APHELP(caltmax)" -labelfont "fixed"  -range "-40 40 1" -width 4 -textvariable CALS($t,tmax)
   SpinBox .cal.$t.navg -label "   num. frames" -helptext "$APHELP(calnavg)" -labelfont "fixed"  -range "1 100 1" -width 4 -textvariable CALS($t,navg)
   SpinBox .cal.$t.exp -label "   exposure" -helptext "$APHELP(calexp)" -labelfont "fixed"  -range "0.02 65535. .01" -width 4 -textvariable CALS($t,exp)
   set CALS($t,tmin) -30
   set CALS($t,tmax) 10
   set CALS($t,navg) 10
   pack .cal.$t.l -side left -fill both -expand yes
   pack .cal.$t.auto -side left -fill both -expand yes
   pack .cal.$t.sel -side left -fill both -expand yes
   pack .cal.$t.tmin -side left -fill both -expand yes
   pack .cal.$t.tmax -side left -fill both -expand yes
   pack .cal.$t.navg -side left -fill both -expand yes
   pack .cal.$t.exp -side left -fill both -expand yes
   pack .cal.$t -side top -fill both -expand yes
}
set CALS(dark,exp) 10
set CALS(flat,exp) 0.5
set CALS(skyflat,exp) 0.5
foreach t "zero dark flat skyflat" {
   button .cal.run$t -text "Create $t calibrations library" -bg gray -command "createcalibrations $t"
   pack .cal.run$t -side top -fill both -expand yes
}
button .cal.close -text "close" -bg yellow -command "wm withdraw .cal"
pack .cal.close -side top -fill both -expand yes

#
#  Create the drift-scan control GUI
#

showstatus "Loading driftscan properties"
toplevel .drift 
wm title .drift "Drift-scan (Alt-d to open/close window)"
wm geometry .drift 365x327+20+247 
wm withdraw .drift
wm title .drift "Drift-scan control"
label .drift.msg -text "Drift-scan mode is still experimental!"
place .drift.msg -x 10 -y 10
label .drift.mmeas -text "Measured per-line drift rate at current DEC is "
place .drift.mmeas -x 10 -y 40
entry .drift.vmeas -bg white -fg black -width 8 -textvariable SCOPE(driftsamp)
place .drift.vmeas -x 290 -y 40
button .drift.calib -text "Calculate per-line rate for DEC=+00:00:00" -command driftcalib
place .drift.calib -x 10 -y 70
label .drift.mbase -text "Per-line drift rate for DEC=+00:00:00 is "
place .drift.mbase -x 10 -y 100
entry .drift.vbase -bg white -fg black -width 8 -textvariable SCOPE(driftdelay)
place .drift.vbase -x 290 -y 100
label .drift.mrows -text "Number of rows for drift image "
place .drift.mrows -x 10 -y 130
entry .drift.vrows -bg white -fg black -width 8 -textvariable SCOPE(driftrows)
place .drift.vrows -x 290 -y 130
button .drift.calc -text "Calculate duration and per-line rate in microsecs" -command driftcalc
place .drift.calc -x 10 -y 160
label .drift.mdur -text "Total image duration (hh:mm:ss) =  "
place .drift.mdur -x 10 -y 200
label .drift.vdur -textvariable SCOPE(driftexp)
place .drift.vdur -x 290 -y 200
label .drift.mmsec -text "Microsecsond delay per row =  "
place .drift.mmsec -x 10 -y 230
label .drift.vmsec -textvariable SCOPE(driftdcalc)
place .drift.vmsec -x 290 -y 230
button .drift.go -text "Start drift scan exposure" -width 46 -height 3 -bg gray -command "driftscan 0 0"
place .drift.go -x 10 -y 260

#
#  Set up the default structures for temperaure control/plotting
#
set LASTTEMP 0.0
set TIMES "0"
set SETPOINTS "0.0"
set AVGTEMPS "0.0"
set i -60
set xdata ""
set ydata ""
set ysetp ""
while { $i < 0 } {
  lappend xdata $i
  lappend ydata $AVGTEMPS
  lappend ysetp $SETPOINTS
  incr i 1
}


#
#  Create the temperature history graphic
#

set f [.p.props getframe Temperature]
set TEMPWIDGET $f.plot
graph $f.plot -title "Temperature" -width 500 -height 220
$f.plot element create Temp
$f.plot element create SetPoint
$f.plot element configure Temp -symbol none -xdata $xdata -ydata $ydata
$f.plot element configure SetPoint -color red -symbol none -xdata $xdata -ydata $ysetp
place $f.plot -x 0 -y 60
wm geometry .cal +20+445
wm withdraw .cal
update

#
#
#  Create the interrupt masking interface
#



#
#
#  Call the camera setup code, and the telescope setup code
#
showstatus "Initializing camera"
source  $TKAPOGEE/scripts/camera_init.tcl
source  $TKAPOGEE/scripts/tele_init.tcl
set STATUS(busy) 0


if { 0 } {


#
#  Synchronize the widgets with the relevant camera CONFIG values
#

foreach i [array names CONFIG] {
   foreach s "CCD System" {
     if { [string tolower $s] == [lindex [split $i .] 0] } {
       set id [string tolower [lindex [split $i .] 1]]
       set w ".p.props.f$s.$id"
       if { [winfo exists $w] } {
          if { [winfo class $w] == "Label" } {
            puts stdout "$i $s"
            $w configure -text "$id = $CONFIG($i)" -fg black
          }
#          if { [winfo class $w] == "Frame" } {
#            puts stdout "$i $s"
#            set CONFIG($id) $CONFIG($i)
#          }
       }
     }
   }  
}




if { $ALTA == 0 } {
.mbar.observe.m add command -label "Set interrupt mask" -command "toggle .irqs"
toplevel .irqs -width 250 -height 350
set irqs [split [exec cat /proc/interrupts] \n]
label .irqs.m -text "Select IRQS to mask during readout"
place .irqs.m -x 5 -y 5
wm withdraw .irqs
set iy 30
foreach i [lrange $irqs 1 end] { 
     set inum [string trim [lindex $i 0] :]
     set iname [lrange $i 3 end]
     if { [string trim $iname] != "" } {
       set IRQS($inum) 0
       checkbutton .irqs.i$inum -text "IRQ $inum : $iname" -variable IRQS($inum)    
       place .irqs.i$inum -x 10 -y $iy
       incr iy 25
     }
}
button .irqs.apply -width 30 -text "Update mask" -command "setirqmask"
place .irqs.apply -x 5 -y $iy
incr iy 30
button .irqs.close -width 30 -text "Close" -command "wm withdraw .irqs"
place .irqs.close -x 5 -y $iy

proc setirqmask { } {
global IRQS SCOPE CAMERA
   set mask 0
   foreach i [array names IRQS] {
      if { $IRQS($i) } { 
         set mask [expr $mask | 1<<$i]
      }
   }
   set SCOPE(irqmask) $mask
   $CAMERA configure -m_IRQMask $SCOPE(irqmask)
}
bind .irqs <Alt-i> {toggle .irqs}

}


#
# Add some ALTA specific controls
#
if { $ALTA } {
  .mbar.temp.m add command -label "Fan off"    -command {fanmode off}
  .mbar.temp.m add command -label "Fan Slow"   -command {fanmode slow}
  .mbar.temp.m add command -label "Fan Medium" -command {fanmode medium}
  .mbar.temp.m add command -label "Fan Fast"   -command {fanmode fast}
  menubutton .mbar.leds -text "Leds" -fg black -bg gray -menu .mbar.leds.m
  place .mbar.leds -x 340 -y 0
  menu .mbar.leds.m
  .mbar.leds.m add command -label "disable"   -command {ledmode disbable -1 -1}
  .mbar.leds.m add command -label "nonexpose" -command {ledmode nonexpose -1 -1}
  .mbar.leds.m add command -label "enable"    -command {ledmode enable -1 -1}
  .mbar.leds.m add command -label "1=exp"     -command {ledmode -1 0 -1}
  .mbar.leds.m add command -label "1=act"     -command {ledmode -1 1 -1}
  .mbar.leds.m add command -label "1=flush"   -command {ledmode -1 2 -1}
  .mbar.leds.m add command -label "1=trigw"   -command {ledmode -1 3 -1}
  .mbar.leds.m add command -label "1=trigd"   -command {ledmode -1 4 -1}
  .mbar.leds.m add command -label "1=extshut" -command {ledmode -1 5 -1}
  .mbar.leds.m add command -label "1=extread" -command {ledmode -1 6 -1}
  .mbar.leds.m add command -label "1=attemp"  -command {ledmode -1 7 -1}
  .mbar.leds.m add command -label "2=exp"     -command {ledmode -1 -1 0}
  .mbar.leds.m add command -label "2=act"     -command {ledmode -1 -1 1}
  .mbar.leds.m add command -label "2=flush"   -command {ledmode -1 -1 2}
  .mbar.leds.m add command -label "2=trigw"   -command {ledmode -1 -1 3}
  .mbar.leds.m add command -label "2=trigd"   -command {ledmode -1 -1 4}
  .mbar.leds.m add command -label "2=extshut" -command {ledmode -1 -1 5}
  .mbar.leds.m add command -label "2=extread" -command {ledmode -1 -1 6}
  .mbar.leds.m add command -label "2=attemp"  -command {ledmode -1 -1 7}
  .mbar.observe.m add command -label "Slow readout" -command "altamode slow"
  .mbar.observe.m add command -label "Fast readout" -command "altamode fast"
}

set CCDID 0
set RAWTEMP 0
set REMAINING 0


#
#  Set defaults for observation parameters
#

set OBSPARS(Object) "1.0 1 1"
set OBSPARS(Focus)  "0.1 1 1"
set OBSPARS(Acquire) "1.0 1 1"
set OBSPARS(Flat)    "1.0 1 1"
set OBSPARS(Dark)    "100.0 1 0"
set OBSPARS(Zero)    "0.01 1 0"
set OBSPARS(Skyflat) "0.1 1 1"

set LASTBIN(x) 1
set LASTBIN(y) 1
setutc
set d  [split $SCOPE(obsdate) "-"]
set SCOPE(equinox) [format %7.2f [expr [lindex $d 0]+[lindex $d 1]./12.]]

#
#  Do the actual setup of the GUI, to sync it with the camera status
#

showstatus "Loading camera API"
if { $ALTA } {
   inspectapi CApnCamera
} else {
   inspectapi CCameraIO
}
refreshcamdata
trace variable CONFIG w watchconfig
trace variable SCOPE w watchscope

set CONFIG(temperature.Target) [expr $CONFIG(temperature.Target) +1]
set CONFIG(temperature.Target) [expr $CONFIG(temperature.Target) -1]

#
#  Reset to the last used configuration if available
#

if { [file exists $env(HOME)/.apgui.tcl] } {
   source $env(HOME)/.apgui.tcl
}

#
#  Fix the date
#

set SCOPE(obsdate) [join "[lrange $now 1 2] [lindex $now 4]" -]  
set SCOPE(StartCol) $CONFIG(geometry.StartCol)
set SCOPE(StartRow) $CONFIG(geometry.StartRow) 
set SCOPE(NumCols)  $CONFIG(geometry.NumCols) 
set SCOPE(NumRows)  $CONFIG(geometry.NumRows) 
set SCOPE(darktime) 0.0
#
#  Start monitoring the temperature
#
monitortemp
wm withdraw .status
wm geometry . +20+30

#
#  Link ALT key sequences to pop-up the various windows
#

bind . <Alt-p> {toggle .p}
bind . <Alt-c> {toggle .cal}
bind . <Alt-d> {toggle .drift}
bind . <Alt-i> {toggle .irqs}
bind .p <Alt-p> {toggle .p}
bind .p <Alt-c> {toggle .cal}
bind .p <Alt-d> {toggle .drift}
bind .p <Alt-i> {toggle .irqs}
bind .cal <Alt-p> {toggle .p}
bind .cal <Alt-c> {toggle .cal}
bind .cal <Alt-d> {toggle .drift}
bind .cal <Alt-i> {toggle .irqs}
bind .drift <Alt-p> {toggle .p}
bind .drift <Alt-c> {toggle .cal}
bind .drift <Alt-d> {toggle .drift}
bind .drift <Alt-i> {toggle .irqs}


focus .

#
#  Stop the user from destroying the windows by accident
#

wm protocol .countdown WM_DELETE_WINDOW {wm withdraw .countdown}
wm protocol .psite  WM_DELETE_WINDOW {wm withdraw .psite}
wm protocol .status WM_DELETE_WINDOW {wm withdraw .status}
wm protocol .drift  WM_DELETE_WINDOW {wm withdraw .drift}
wm protocol .cal    WM_DELETE_WINDOW {wm withdraw .cal}
wm protocol .p      WM_DELETE_WINDOW {wm withdraw .p}
wm protocol .       WM_DELETE_WINDOW {wm withdraw .status}

#ap7p  set_biascols 1 7, set bic 4
#kx260 set_biascols 1 5, set bic 2



}



