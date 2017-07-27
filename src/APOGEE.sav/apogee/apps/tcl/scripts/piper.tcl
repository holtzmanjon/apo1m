#!/usr/bin/wish


#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#  Procedure set : piper.tcl
#
#  The procedures in this set are used to provide interactive real-time
#  monitoring of an image-in-progress. The image is created using the 
#  BufferDriftScan method and is intended for use in sky tracking mode.
#
#  After each user-defined interval , a single image line is read out and
#  the rest of the image shifted down one row. The image data is piped
#  through a FIFO and can optionally be monitored by a seperate task.
#  This procedure set provides the functionality of the monitor.
#
#  Usage : 
#
#		piper.tcl   nx  width  roi1  roi2 .....   roin
#
#  where
#
#	nx = line size in pixels
#
#---------------------------------------------------------------------------
#---------------------------------------------------------------------------

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : openraw
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure opens a raw image data source. This can be a disk
#  file or a pipe linked to a running copy of the Apogee camera controller.
#
#  Arguments  :
#
#		pname 	-	Name of pipe/raw image file
#
proc openraw { {pname /tmp/apgpipe} } {
#
#  Globals    :
#
#               CAMERAS -       Camera id's
#               SCOPE   -       Telescope parameters, gui setup
#               DEBUG   -       Set to 1 for verbose logging
#		FPIPE	- 	File handle for piped or raw image data
global CAMERAS SCOPE DEBUG
global FPIPE
   set FPIPE [open $pname r]
   fconfigure $FPIPE -eofchar {} -translation binary
   return 0
}


#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : readrawline
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure reads raw image line data from either a pipe or file
#  linked to a running copy of the Apogee camera controller.
#
#  Arguments  :
#
proc readrawline { nx } {
global FPIPE
   set bytes [expr $nx*2]
   catch { 
      set rawdata [read $FPIPE $bytes]
   }
    binary scan $rawdata s* xlate
   return $xlate
}


#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : plotline
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure creates/updates a plot of the latest image line
#
#  Arguments  :
#
proc plotline { nx {ymn 1.0} {ymx 10000.0} } {
global LDATA BKGND
  if { [winfo exists .g] == 0 } {
      toplevel .g -width 600 -height 200
      wm title .g "Line scan"
      graph .g.d  -width 600 -height 200
      .g.d element create flux
      .g.d element create bgnd
      set xaxis ""
      set i 1
      while { $i < $nx } {lappend xaxis $i ; incr i 1}
      .g.d element configure flux -xdata $xaxis -symbol none -color green
      .g.d element configure bgnd -xdata $xaxis -symbol none -color black
      pack .g.d
  }
  .g.d yaxis configure -min $ymn -max $ymx
  .g.d element configure flux -ydata $LDATA
  .g.d element configure bgnd -ydata $BKGND
}

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : plotfluxes
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure creates/updates an arbitrary number of graphica plots.
#  Each plot shows the latest npoints samples of a summed area of interest.
#
#  Arguments  :
#
proc plotfluxes { {npoints 500} } {
global SAMPLES FLUXES FLUXP
  foreach o [array names SAMPLES] {
     if { [winfo exists .s$o] == 0 } {
        toplevel .s$o -width 600 -height 200
        wm title .s$o "Object $o at x=$SAMPLES($o)"
        graph .s$o.d  -width 600 -height 200
        .s$o.d element create flux
        set xaxis ""
        set FLUXP($o) ""
        set i 1
        while { $i < $npoints } {lappend xaxis $i ; lappend FLUXP($o) 0.0 ; incr i 1}
        .s$o.d element configure flux -xdata $xaxis -ydata $FLUXP($o) -symbol none -color blue
        pack .s$o.d
     }
     set FLUXP($o) [lrange $FLUXP($o) 1 end]
     lappend FLUXP($o) $FLUXES($o)
     .s$o.d element configure flux -ydata $FLUXP($o)
  }
}


#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : liveplot
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure updates the line plots each time new image data is 
#  available.
#
#  Arguments  :
#
proc liveplot { nx {ymn 1.0} {ymx 10000.0} } {
global LDATA FPIPE BGITER
  openraw
  set n 0
  set LDATA "init"
  while { $LDATA != "" } {
       set LDATA [readrawline $nx]
       if { [llength $LDATA] == $nx } {
          calcbackground $BGITER
          plotline $nx $ymn $ymx
          calcfluxes
          incr n 1
          puts stdout "line $n"
          update
       }
  }
  close $FPIPE
}

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : calcfluxes
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure calculates summed flux estimates for an arbitrary number
#  of regions of interest on an image line
#
#  Arguments  :
#
proc calcfluxes { } {
global SAMPLES FLUXES LDATA BKGND SSIZE
   foreach o [array names SAMPLES] {
      set x $SAMPLES($o)
      set idx [expr $x - $SSZIE/2]
      set sum 0
      while { $idx < [expr $x + $SSIZE/2] } {
         set sum [expr $sum + [lindex $LDATA $idx] ]
### - [lindex $BKGND $idx]]
         incr idx 1
      }
      set FLUXES($o) $sum
   }
   plotfluxes
}


#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : calcbackground
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure estimates the background flux using a variable size
#  median filter
#
#  Arguments  :
#
proc calcbackground { {ssamp 51}  } {
global LDATA BKGND MEDIAN DVEC
    set BKGND ""
    set lwork $LDATA
    set i 1
    set btmp [lindex $lwork 0]
    set last [lindex $lwork 0]
    while { $i < [llength $lwork] } {
      if { [lindex $lwork $i] < [lindex $lwork [expr $i-1]] && [lindex $lwork $i] < [lindex $lwork [expr $i+1]] } {
          lappend btmp [lindex $lwork $i]
          set last [lindex $lwork $i]
      } else {
          lappend btmp $last
      }
      incr i 1
    }
    tovector $btmp
    set i 0
    set imax [expr [llength $LDATA] -1]
    while { $i < $imax } {
      set j [expr min($i+$ssamp,$imax)]
      MEDIAN expr mean(DVEC($i:$j))
      lappend BKGND $MEDIAN(0)
      incr j 1
      incr i 1
    }
    lappend BKGND $MEDIAN(0)
}

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : tovector
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (rfactory@theriver.com)
#  Version    : 0.7
#  Date       : Nov-20-2002
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure converts an arbitrary list of numerical items
#  into a vector and calculates the mean and median values
#
#  Arguments  :
#
proc tovector { alist } {
global DVEC MEAN MEDIAN
  DVEC set [eval list $alist]
  MEAN   expr mean(DVEC)
  MEDIAN expr median(DVEC)
}




proc trackdriftscan { name { shutter 1} {nrow 512} {delay 100000} {npipe 1} {id 0} } {
global CAMERAS
  set camera $CAMERAS($id)
  $camera Flush
  $camera configure -m_TDI 1
  $camera Expose $shutter 0
  $camera BufferDriftScan drift $delay $row 1 $npipe
  saveandshow drift $name
}

wm withdraw .
set BGITER 5
package require BLT
namespace import blt::*
vector create DVEC
vector create MEAN
vector create MEDIAN

set SSIZE [lindex $argv 2]
set isamp 1
foreach region [lrange $argv 3 end] {
    set SAMPLES($isamp) $region
    incr  isamp 1
}


liveplot [lindex $argv 1]


 

