

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : flattobuffer
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a flat-field exposure and saves it in an
#  in-memory buffer name FLAT-n. It is intended to be used as part
#  of a calibration library creation sequence.
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               id	-	Camera id (for multi-camera use) (optional, default is 0)
 
proc flattobuffer { n {id 0} } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               CAMERAS	-	Camera id's
#               DEBUG	-	Set to 1 for verbose logging
#               CALS	-	Calibration run parmaeters
#               SCOPE	-	Telescope parameters, gui setup
global STATUS CAMERAS DEBUG CALS SCOPE ALTA
    set camera $CAMERAS($id)
    set STATUS(busy) 1
    setutc
    if { $ALTA } {GetImagingStatus}
    $camera StartExposure $CALS(flat,exp) 1  
    if { $ALTA } {$camera GetImagingStatus}
    set SCOPE(exposure) $CALS(flat,exp)
    set SCOPE(shutter) 1
    set SCOPE(exptype) Flat
    if { $DEBUG } {debuglog "exposing (flattobuffer)"}
    set timeout [waitforimage [expr int($CALS(flat,exp))] $id]
    if { $timeout } {
       puts stdout "TIMEOUT/ABORT"
    } else {
      if { $DEBUG } {debuglog "Reading out..."}
      $camera GetImage
      set STATUS(readout) 0
      store_calib tempobs FLAT $n 
    }
    set STATUS(busy) 0
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : darktobuffer
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a dark exposure and saves it in an
#  in-memory buffer name DARK-n. It is intended to be used as part
#  of a calibration library creation sequence.
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               id	-	Camera id (for multi-camera use) (optional, default is 0)
 
proc darktobuffer { n {id 0} } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               CAMERAS	-	Camera id's
#               DEBUG	-	Set to 1 for verbose logging
#               CALS	-	Calibration run parmaeters
#               SCOPE	-	Telescope parameters, gui setup
global STATUS CAMERAS DEBUG CALS SCOPE ALTA
    set camera $CAMERAS($id)
    set STATUS(busy) 1
    setutc
    if { $ALTA } {$camera GetImagingStatus}
    $camera StartExposure $CALS(dark,exp) 0  
    if { $ALTA } {$camera GetImagingStatus}
    set SCOPE(exposure) $CALS(dark,exp)
    set SCOPE(shutter) 0
    set SCOPE(exptype) Dark
    if { $DEBUG } {debuglog "exposing (darktobuffer)"}
    set timeout [waitforimage [expr int($CALS(dark,exp))] $id]
    if { $timeout } {
       puts stdout "TIMEOUT/ABORT"
    } else {
      if { $DEBUG } {debuglog "Reading out..."}
      update
      $camera GetImage
      set STATUS(readout) 0
      store_calib tempobs DARK $n 
    }
    set STATUS(busy) 0
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : zerotobuffer
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a zero length exposure and saves it in an
#  in-memory buffer name ZERO-n. It is intended to be used as part
#  of a calibration library creation sequence.
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               id	-	Camera id (for multi-camera use) (optional, default is 0)
 
proc zerotobuffer { n {id 0} } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               CAMERAS	-	Camera id's
#               DEBUG	-	Set to 1 for verbose logging
#               CALS	-	Calibration run parmaeters
#               SCOPE	-	Telescope parameters, gui setup
global STATUS CAMERAS DEBUG CALS SCOPE ALTA
    set camera $CAMERAS($id)
    set STATUS(busy) 1
    setutc
    if { $ALTA } {$camera GetImagingStatus}
    $camera StartExposure $CALS(zero,exp) 0  
    if { $ALTA } {$camera GetImagingStatus}
    set SCOPE(exposure) 0.0
    set SCOPE(shutter) 0
    set SCOPE(exptype) Zero
    if { $DEBUG } {debuglog "exposing (zerotobuffer)"}
    set timeout [waitforimage 1 $id]
    if { $timeout } {
       puts stdout "TIMEOUT/ABORT"
    } else {
      if { $DEBUG } {debuglog "Reading out..."}
      update
      $camera GetImage
      set STATUS(readout) 0
      store_calib tempobs ZERO $n 
    }
    set STATUS(busy) 0
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : fskytobuffer
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a skyflat-field exposure and saves it in an
#  in-memory buffer name FSKY-n. It is intended to be used as part
#  of a calibration library creation sequence.
#  Arguments  :
#
#               n	-	Number of frame(s)
#               id	-	Camera id (for multi-camera use) (optional, default is 0)
 
proc fskytobuffer { n {id 0} } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               CAMERAS	-	Camera id's
#               DEBUG	-	Set to 1 for verbose logging
#               CALS	-	Calibration run parmaeters
#               SCOPE	-	Telescope parameters, gui setup
global STATUS CAMERAS DEBUG CALS SCOPE ALTA
    set camera $CAMERAS($id)
    set STATUS(busy) 1
    setutc
    if { $ALTA } {$camera GetImagingStatus}
    $camera StartExposure $CALS(skyflat,exp) 1
    if { $ALTA } {$camera GetImagingStatus}
    set SCOPE(exposure) $CALS(skyflat,exp)
    set SCOPE(shutter) 1
    set SCOPE(exptype) Skyflat
    if { $DEBUG } {debuglog "exposing (fskytobuffer)"}
    set timeout [waitforimage 1 $id]
    if { $timeout } {
       puts stdout "TIMEOUT/ABORT"
    } else {   
      if { $DEBUG } {debuglog "Reading out..."}
      $camera GetImage
      set STATUS(readout) 0
      store_calib tempobs FSKY $n 
    }
    set STATUS(busy) 0
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : loadcalibrations
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This routine loads the best (nearest temperature match) calibration frames
#  if available.
#  The frames are buffered in memory, and can be used  by the write_calibrated
#  c-code call.
#
#  Arguments  :
#
 
proc loadcalibrations { } {
 
#
#  Globals    :
#  
#               LASTTEMP	-	Last temperature used for calibration
#               AVGTEMPS	-	Average temps for plotting
#               DEBUG	-	Set to 1 for verbose logging
#               SCOPE	-	Telescope parameters, gui setup
#               CALS	-	Calibration run parmaeters
global LASTTEMP AVGTEMPS DEBUG SCOPE CALS
  if { $SCOPE(autocalibrate) } {
    if { [expr abs($LASTTEMP-$AVGTEMPS)] > 1.0 } {
       set tnow [expr int($AVGTEMPS)]
       if { [file exists $CALS(zero,dir)/temp$tnow.fits] } {
          if { $DEBUG } {debuglog "loading calibration library data from  $CALS(zero,dir)/temp$tnow.fits"}
          read_image CALIBRATION-ZERO  $CALS(zero,dir)/temp$tnow.fits
       }
       if { [file exists $CALS(dark,dir)/temp$tnow.fits] } {
          if { $DEBUG } {debuglog "loading calibration library data from  $CALS(dark,dir)/temp$tnow.fits"}
          read_image CALIBRATION-DARK  $CALS(dark,dir)/temp$tnow.fits
       }
       if { [file exists $CALS(flat,dir)/temp$tnow.fits] } {
          if { $DEBUG } {debuglog "loading calibration library data from  $CALS(flat,dir)/temp$tnow.fits"}
          read_image CALIBRATION-FLAT  $CALS(flat,dir)/temp$tnow.fits
       }
       if { [file exists $CALS(skyflat,dir)/temp$tnow.fits] } {
          if { $DEBUG } {debuglog "loading calibration library data from  $CALS(skyflat,dir)/temp$tnow.fits"}
          read_image CALIBRATION-FSKY  $CALS(skyflat,dir)/temp$tnow.fits
       }
       set LASTTEMP $AVGTEMPS
    }
  }
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : darklibrary
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure generates a library of dark frames. It repeatedly 
#  steps thru a set of temperatures, and call dakrframes at each
#  point to take the exposures.
#
#  Arguments  :
#
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
#               n	-	Number of frame(s)
 
proc darklibrary { t high n } {
 
#
#  Globals    :
#  
   if { $t < $high } {
      wm title .countdown "countdown - Acquiring darks at $t deg C"
      setpoint set $t
      waitfortemp $t "darkframes $n $t $high"
      incr t 1
   }
}






#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : flatlibrary
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure generates a library of flat-field frames. It repeatedly 
#  steps thru a set of temperatures, and call flatframes at each
#  point to take the exposures.
#
#  Arguments  :
#
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
#               n	-	Number of frame(s)
 
proc flatlibrary { t high n } {
 
#
#  Globals    :
#  
   if { $t < $high } {
      wm title .countdown "countdown - Acquiring flat-fields at $t deg C"
      setpoint set $t
      waitfortemp $t "flatframes $n $t $high"
   }
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : skyflatlibrary
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure generates a library of sky flat frames. It repeatedly 
#  steps thru a set of temperatures, and call skyflatframes at each
#  point to take the exposures.
#
#  Arguments  :
#
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
#               n	-	Number of frame(s)
 
proc skyflatlibrary { t high n } {
 
#
#  Globals    :
#  
   if { $t < $high } {
      wm title .countdown "countdown - Acquiring skyflat-fields at $t deg C"
      setpoint set $t
      waitfortemp $t "skyflatframes $n $t $high"
   }
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : zerolibrary
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure generates a library of zero frames. It repeatedly 
#  steps thru a set of temperatures, and call zeroframes at each
#  point to take the exposures.
#
#  Arguments  :
#
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
#               n	-	Number of frame(s)
 
proc zerolibrary { t high n } {
 
#
#  Globals    :
#  
   if { $t < $high } {
      wm title .countdown "countdown - Acquiring zeroes at $t deg C"
      setpoint set $t
      waitfortemp $t "zeroframes $n $t $high"
   }
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : flatframes
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a set of n flat-field frames and stores them in 
#  memory buffers.
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
 
proc flatframes { n  t high } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               CALS	-	Calibration run parmaeters
global STATUS CALS
   set i 0
   while { $i < $n } {
      set FRAME $i
      countdown [expr $CALS(flat,exp)]
      puts stdout "flat frame $i"
      flattobuffer $i
      incr i 1
   }
   if { [file exists  $CALS(flat,dir)/temp$t.fits] } {
      exec rm -f $CALS(flat,dir)/temp$t.fits
   }
   if { [file exists $CALS(zero,dir)/temp$t.fits] } {
      read_image CALIBRATION-ZERO  $CALS(zero,dir)/temp$t.fits
   }
   if { [file exists $CALS(dark,dir)/temp$t.fits] } {
      read_image CALIBRATION-DARK  $CALS(dark,dir)/temp$t.fits
   }
   write_fimage $CALS(flat,dir)/temp$t.fits $CALS(flat,exp) 0
   incr t 1
   flatlibrary $t $high $n 
   countdown off
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : skyflatframes
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a set of n skyflat-field frames and stores them in 
#  memory buffers.
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
 
proc skyflatframes { n t high } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               CALS	-	Calibration run parmaeters
global STATUS CALS
   set i 0
   while { $i < $n } {
      set FRAME $i
      countdown [expr $CALS(skyflat,exp)]
      puts stdout "flat frame $i"
      fskytobuffer $i
      incr i 1
   }
   if { [file exists  $CALS(skyflat,dir)/temp$t.fits] } {
      exec rm -f $CALS(skyflat,dir)/temp$t.fits
   }
   if { [file exists $CALS(zero,dir)/temp$t.fits] } {
      read_image CALIBRATION-ZERO  $CALS(zero,dir)/temp$t.fits
   }
   if { [file exists $CALS(dark,dir)/temp$t.fits] } {
      read_image CALIBRATION-DARK  $CALS(dark,dir)/temp$t.fits
   }
   write_simage $CALS(skyflat,dir)/temp$t.fits $CALS(skyflat,exp) 0
   incr t 1
   skyflatlibrary $t $high $n
   countdown off
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : darkframes
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a set of n dark frames and stores them in 
#  memory buffers.
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
 
proc darkframes { n t high } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               FRAME	-	Frame number in a sequence
#               CALS	-	Calibration run parmaeters
global STATUS FRAME CALS
   set i 1
   while { $i <= $n } {
      set FRAME $i
      countdown [expr $CALS(dark,exp)]
      puts stdout "dark frame $i"
      darktobuffer $i 
      incr i 1
   }
   if { [file exists  $CALS(dark,dir)/temp$t.fits] } {
      exec rm -f $CALS(dark,dir)/temp$t.fits
   }
   if { [file exists $CALS(zero,dir)/temp$t.fits] } {
      read_image CALIBRATION-ZERO  $CALS(zero,dir)/temp$t.fits
   }
   write_dimage $CALS(dark,dir)/temp$t.fits $CALS(dark,exp) 0
   incr t 1
   darklibrary $t $high $n 
   countdown off
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : zeroframes
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure takes a set of n zero frames and stores them in 
#  memory buffers.
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               t	-	Temperature (degrees c)
#               high	-	Maximum temperature for a calibration run
 
proc zeroframes { n t high } {
 
#
#  Globals    :
#  
#               STATUS	-	Exposure status
#               FRAME	-	Frame number in a sequence
#               CALS	-	Calibration run parmaeters
global STATUS FRAME CALS
   set i 1
   while { $i <= $n } {
      set FRAME $i
      countdown 1
      puts stdout "zero frame $i"
      zerotobuffer $i
      incr i 1
   }
   if { [file exists  $CALS(zero,dir)/temp$t.fits] } {
      exec rm -f $CALS(zero,dir)/temp$t.fits
   }
   write_zimage $CALS(zero,dir)/temp$t.fits $CALS(zero,exp) 1
   incr t 1
   zerolibrary $t $high $n 
   countdown off
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : caltest
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This is a test routine to exercise the calibration routines
#
#  Arguments  :
#
#               n	-	Number of frame(s)
#               exp	-	Exposure time in seconds
#               t	-	Temperature (degrees c) (optional, default is -30)
 
proc caltest { n exp {t -30} } {
 
#
#  Globals    :
#  
   catch {exec rm zero$n.fits dark$n.fits}
   setpoint set $t
   zeroframes $n $t $t
   darkframes $n $t $t
}




#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : opencalibrate
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This routine is a stub to support the calibrate menu entries
#
#  Arguments  :
#
#               type	-	Calibration type (flat,dark,sky,zero)
 
proc opencalibrate { type} {
 
#
#  Globals    :
#  
   wm deiconify .cal
   .cal.$type.sel invoke
}





#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : createcalibrations
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure calls the appropriate calibration library routine
#
#  Arguments  :
#
#               type	-	Calibration type (flat,dark,sky,zero)
 
proc createcalibrations { type } {
 
#
#  Globals    :
#  
#               CALS	-	Calibration run parmaeters
#               CCDID	-	Camera id
global CALS CCDID
   set dt [expr $CALS($type,tmax)-$CALS($type,tmin)+1]
   set t [format %.2f [expr $dt * $CALS($type,navg) * (9 + $CALS($type,exp))]]
   set it [ tk_dialog .d "$type calibrations" "Click OK to run a $type calibration library\nsequence of $CALS($type,navg) frames. This will take approx. $t seconds" {} -1 "Cancel to select new mode" OK]      
   if { $it } {
      switch $type {
          zero { zerolibrary $CALS(zero,tmin) $CALS(zero,tmax) $CALS(zero,navg) }
          dark { darklibrary $CALS(dark,tmin) $CALS(dark,tmax) $CALS(dark,navg) }
          flat { flatlibrary $CALS(flat,tmin) $CALS(flat,tmax) $CALS(flat,navg) }
          skyflat { skyflatlibrary $CALS(skyflat,tmin) $CALS(skyflat,tmax) $CALS(skyflat,navg)}
      }
   }
}

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : loadcalibrate
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-04-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure loads the a calibration frame chosen by the user
#
#  Arguments  :
#
#               type	-	Calibration type (flat,dark,skyflat,zero)

proc loadcalibrate { type } {

#
#  Globals    :
#  
#               CALS	-	Calibration run parmaeters
global CALS SCOPE
   set d $CALS($type,dir)
   set cal [tk_getOpenFile -initialdir $d -filetypes {{{Calibrations} {.fits}}}]   
   switch $type {
      dark    { read_image CALIBRATION_DARK $cal }
      zero    { read_image CALIBRATION_ZERO $cal }
      skyflat { read_image CALIBRATION_FSKY $cal }
      flat    { read_image CALIBRATION_FLAT $cal }
   }
   set SCOPE(autocalibrate) 0
}




