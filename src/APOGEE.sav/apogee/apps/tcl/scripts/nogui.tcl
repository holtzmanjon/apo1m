#
# Define the global environment, everything lives under /opt/apogee
# Change TKAPOGEE to move the code somewhere else
#
set TKAPOGEE $env(TKAPOGEE)
set DEBUG 0
set RAWTEMP 0
set APOGEEGUI 0
set CONFIG(temperature.Target) -20.0
wm withdraw .

#
# Load the procedures
#
source $TKAPOGEE/apps/tcl/scripts/general.tcl
source $TKAPOGEE/apps/tcl/scripts/display.tcl
source $TKAPOGEE/apps/tcl/scripts/temperature.tcl
source $TKAPOGEE/apps/tcl/scripts/calibration.tcl
source $TKAPOGEE/apps/tcl/scripts/observe.tcl

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

if { $tcl_platform(os) != "Darwin" } {
  load $libs/libfitsTcl.so


# Prepare for Guide Star Catalog access
#package ifneeded gsc 3.1       [load $libs/libgsc.so]

# Prepare for Digital sky survey access
#package ifneeded dss 3.1       [load $libs/libdss.so]

# Prepare for Oracle (target ephemeris prediction)
#package ifneeded oracle 2.1    [load $libs/liboracle.so]

# Prepare for generic astrometry
#package ifneeded xtcs 3.1      [load $libs/libxtcs.so]

# Prepare for Graphics widget package
###package ifneeded BLT 2.4       [load $libs/libBLT24.so]

# Prepare for Ccd image buffering package
package ifneeded ccd 1.0       [load $libs/libccd.so]

# Load packages provided by dynamically loadable libraries
#showstatus "Loading Digital Sky survey access"
#package require dss
#showstatus "Loading GSC catalog access"
#package require gsc
#showstatus "Loading Oracle"
#package require oracle
#showstatus "Loading graphics package"
###package require BLT
###namespace import blt::graph
showstatus "Loading CCD package"
package require ccd
###lappend auto_path $libs/BWidget-1.2.1
###package require BWidget
} else {
package require BWidget
proc getlocaltime { } {exec date}
}


proc getlocaltime { } {exec date}

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
set SCOPE(driftdelay) 300000
set SCOPE(driftrows) 512
set SCOPE(telescope) " "
set SCOPE(instrument) " "
set SCOPE(equinox) "2000.0"


#
#  Initialize telescope/user variables
#
source $TKAPOGEE/apps/tcl/scripts/tele_init.tcl


#
#  Create main observation management widgets
#
#
if { $tcl_platform(os) != "Darwin" } {
   set bwkey label
   set bwfont labelfont
} else {
   set bwkey text
   set bwfont font
}
set SCOPE(autodisplay) 0
set SCOPE(autobias) 0
set SCOPE(autocalibrate) 0
set SCOPE(overwrite) 0
set STATUS(abort) 0
set STATUS(pause) 0
set STATUS(readout) 0

#
#  Define a default sub-region
#  
set ACQREGION(xs) 200
set ACQREGION(xe) 64
set ACQREGION(ys) 200
set ACQREGION(ye) 64

set CALS(dark,exp) 10
set CALS(flat,exp) 0.5
set CALS(skyflat,exp) 0.5
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
#
#  Call the camera setup code, and the telescope setup code
#
showstatus "Initializing camera"
source  $TKAPOGEE/apps/tcl/scripts/camera_init.tcl
source  $TKAPOGEE/apps/tcl/scripts/tele_init.tcl
set STATUS(busy) 0

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
   set SCOPE(camera) [$CAMERA GetSensor]
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
set SCOPE(StartCol) 1
set SCOPE(StartRow) 1 
set SCOPE(NumCols)  $CONFIG(geometry.Imgcols) 
set SCOPE(NumRows)  $CONFIG(geometry.Imgrows) 
set SCOPE(darktime) 0.0
set SCOPE(autodisplay) 0
set SCOPE(overwrite) 1
set DEBUG 0






