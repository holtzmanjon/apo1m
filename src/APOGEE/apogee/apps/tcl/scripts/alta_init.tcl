#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : altaeDiscover
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure searches for Apogee ALTA-E cameras on
#  the local subnet.
#
#  Arguments  :
#

proc altaeDiscover { {nmax 1} } {
global env
  set i [exec /sbin/ifconfig eth0]
  set n [lsearch $i inet]
  set subnet [join [lrange [split [lindex [split [lindex $i [expr $n+2]] :] 1] .] 0 2] .]
  set try 1
  set found 0
  while { $try < 254 } {
     .m configure -text "Search for ALTA-E - addr = $try"
     update
     set p "no"
     catch {set p [exec /opt/apogee/bin/http_ping -count 1 http://$subnet.$try]}
     if { $p != "no" } {
puts stdout "got ping"
        set isit [catch {exec /opt/apogee/bin/wget -S -T 1 http://$subnet.$try} hdrs]
        set n [lsearch $hdrs Server:]
puts stdout "got hdrs"
        if { $n > -1 } {
           set server [split [lindex $hdrs [expr $n +1]] -]
           if { [lindex $server 0] == "Allegro" } {
              set addr $try
              lappend altas $addr
              incr found 1
              if { $found == $nmax } {
                 .m configure -text "ALTA-E located at $subnet.$addr"
                 update
                 set try 255
                 set fout [open $env(HOME)/.apccd.ini w]
                 puts $fout "\[system\]"
                 puts $fout "Interface = NET"
                 foreach c $altas {
                    puts $fout "IP = $subnet.$c"
                 }
                 close $fout
              }
           }
        }
     }
     incr try 1
  }
}





proc checkaltae {  } {
global ALTAIP env
    set ALTAIP ""
    set all [split [exec grep IP $env(HOME)/.apccd.ini] \n]
    foreach check $all {
        set test [lindex $check 2]
        set isit [catch {exec /opt/apogee/bin/wget -S -T 1 http://$test} hdrs]
	set n [lsearch $hdrs Server:]
	if { $n > -1 } {
   	   set server [split [lindex $hdrs [expr $n +1]] -]
	   if { [lindex $server 0] == "Allegro" } {
              lappend ALTAIP $test
           }
        }
    }
}

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : apogeeGetVal
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure is a simple wrapper to easily set Apogee camera parameters
#  using the C++ wrapper interface.
#
#  Arguments  :
#
#               configPar       -       Name of a .ini configuration parameter#
#               objPar    -       Name of C++ instance variable
#               id      -       Camera id (for multi-camera use) (optional, default is 0)
  
proc apogeeGetVal { configPar objPar {id 0} } {
  
 
#
#  Globals    :
#  
#               CAMERAS	-	Camera id's
#               DEBUG	-	Set to 1 for verbose logging
#               CONFIG	-	GUI configuration
global CAMERAS DEBUG CONFIG
  set camera $CAMERAS($id)
  set CONFIG($configPar)   [$camera Get[set objPar]]
}



proc speedtest { n } {
global CAMERAS
   set c $CAMERAS(0)
   set i 0
   set begin [clock clicks]
   while { $i < $n } {
      $c Write 53 $n
      incr i 1
   }
   set end [clock clicks]
   return [expr $end - $begin]
}


#
#  This set of definitions is at the heart of the API interface. It defines an
#  assocation between items in the C++ API, and the generic elements of the
#  user interface. These elements are defined in the tcl_config.dat file.
#  Entries in this file automatically generate GUI widgets, and the 
#  lines below link these widgets with C++ callbacks in the CameraIO object.
#
set CCAPI(geometry.BIC) PreRoiSkipColumns
set CCAPI(geometry.BIR) UnderScanRows
#set CCAPI(system.Base) 
set CCAPI(geometry.BinX) RoiBinningH
set CCAPI(geometry.BinY) RoiBinningV
set CCAPI(ccd.Color) Color 
set CCAPI(geometry.Columns) TotalColumns
set CCAPI(temperature.Mode) CoolerEnable
set CCAPI(temperature.Target) CoolerSetPoint
set CCAPI(temperature.Status) CoolerStatus
set CCAPI(system.Data_Bits) DataBits 
#set CCAPI(filter.Position) 
#set CCAPI(filter.StepPos) FilterStepPos
set CCAPI(ccd.Gain) ReportedGainSixteenBit
#set CCAPI(system.Guider_Relays) GuiderRelays 
set CCAPI(geometry.HFlush) HFlushDisable
#set CCAPI(system.HighPriority) HighPriority
set CCAPI(geometry.Imgcols) ImagingColumns
set CCAPI(geometry.Imgrows) ImagingRows
set CCAPI(system.Interface) CameraInterface
#set CCAPI(system.Cable) LongCable
#set CCAPI(system.MaxBinX) MaxBinX 
#set CCAPI(system.MaxBinY) MaxBinY 
###set CCAPI(ccd.Mode) CameraMode
#set CCAPI(ccd.Noise) Noise
set CCAPI(ccd.PixelXSize) PixelSizeX
set CCAPI(ccd.PixelYSize) PixelSizeY
#set CCAPI(system.RegisterOffset) RegisterOffset
set CCAPI(geometry.Rows) TotalRows
set CCAPI(ccd.Sensor) Sensor
set CCAPI(system.Sensor) SensorTypeCCD
set CCAPI(system.Shutter_Speed) ShutterAmpControl
set CCAPI(geometry.SkipC) PreRoiSkipColumns
set CCAPI(geometry.SkipR) UnderScanRows
set CCAPI(system.CameraStatus) pvtStatusReg
set CCAPI(geometry.DriftScan) TDIRate 
#set CCAPI(temperature.Cal) TempCalibration 
set CCAPI(temperature.Control) CoolerDrive
#set CCAPI(temperature.Scale) TempScale 
set CCAPI(temperature.Value) TempCCD
#set CCAPI(system.Test2) Test2Bits
#set CCAPI(system.Test) TestBits
#set CCAPI(system.Timeout) Timeout
#set CCAPI(system.Trigger) UseTrigger 
set CCAPI(geometry.VFlush) VFlushBinning
#
# old version
set CCAPI(geometry.StartCol) RoiStartX
set CCAPI(geometry.StartRow) RoiStartY
set CCAPI(geometry.NumCols) RoiPixelsH
set CCAPI(geometry.NumRows) RoiPixelsV

# after V1.9 update
set CCAPI(geometry.StartCol) pvtRoiStartX
set CCAPI(geometry.StartRow) pvtRoiStartY
set CCAPI(geometry.NumCols)  pvtRoiPixelsH
set CCAPI(geometry.NumRows)  pvtRoiPixelsV

#
#  Create reverse loopup into the CCAPI
#

foreach i [array names CCAPI] {
   set CCAPI($CCAPI($i)) $i
}


set DEBUG 1





set XLATE(SHORT) 0
set XLATE(LONG)  1
set ALTAIP ""

#
#  Setup the camera, using values read from the .ini file
#
#

if { $CONFIG(system.Interface) == "USB" } {
   source $TKAPOGEE/apps/tcl/scripts/alta_finder.tcl
} else {
   checkaltae
   if { $ALTAIP != "" } {
      catch {exec /opt/apogee/bin/wget http://$ALTAIP/SESSION?Close}
      set c1 [split [lindex $ALTAIP 0] .]
      set ipdec [expr 256*256*256*[lindex $c1 0] + 256*256*[lindex $c1 1] + 256*[lindex $c1 2] + [lindex $c1 3]]
      $CAMERA InitDriver $ipdec 80 0
   } else {
     set it [tk_dialog .d "No camera" "Unable to locate IP address for ALTA-E" {} -1 "OK"]
     exit
   }
}


#$CAMERA ResetSystem
#$CAMERA write_ForceShutterOpen 0


apogeeGetVal geometry.Rows MaxImgRows
apogeeGetVal geometry.Columns MaxImgCols
#apogeeGetVal system.MaxBinX MaxBinX
#apogeeGetVal system.MaxBinY MaxBinY
#apogeeGetVal system.Guider_Relays GuiderRelays
#apogeeGetVal system.Timeout Timeout
#apogeeGetVal geometry.BIC PreRoiSkipColumns
#####apogeeGetVal geometry.BIR UnderScanRows
apogeeGetVal geometry.SkipC NumOverscanCols
#apogeeGetVal geometry.SkipR OverscanRows
apogeeGetVal geometry.Imgcols RoiNumCols
apogeeGetVal geometry.Imgrows RoiNumRows
#apogeeGetVal geometry.HFlush HFlushDisable
apogeeGetVal geometry.VFlush FlushBinningRows
apogeeGetVal geometry.NumcCols RoiNumCols
apogeeGetVal geometry.NumRows RoiNumRows
###apogeeGetVal temp.Control CoolerDrive
#apogeeGetVal temp.Cal TempCalibration 
#apogeeGetVal temp.Scale TempScale 
$CAMERA SetCoolerSetPoint $CONFIG(temperature.Target)
$CAMERA SetCooler 1   
#apogeeGetVal ccd.Color Color 
#apogeeGetVal ccd.Noise Noise
#apogeeGetVal ccd.Gain ReportedGainSixteenBit
apogeeGetVal ccd.PixelXSize PixelWidth
apogeeGetVal ccd.PixelYSize PixelHeight




#
#
#  Set default values for all items included in headers.
#  The GUI will populate these with the correct values later.
#
set CAMSTATUS(CoolerMode) 1
set CAMSTATUS(Temperature) [$CAMERA GetTempCcd]
set CAMSTATUS(BinX) 1
set CAMSTATUS(BinY) 1
set CAMSTATUS(Gain) 1.0
set SCOPE(site) unknown
set SCOPE(camera) "Apogee [$CAMERA GetModel]"
set SCOPE(detector) [$CAMERA GetSensor]
set SCOPE(observer) unknown
set SCOPE(target) unknown
set SCOPE(telescope) unknown
set SCOPE(instrument) unknown
set SCOPE(latitude) 0:00:00
set SCOPE(longitude) 0:00:00
set SCOPE(exposure) 0.0
set SCOPE(darktime) 0.0
set now [split [exec  date -u +%Y-%m-%d,%T] ,]
set SCOPE(obsdate) [lindex $now 0]
set SCOPE(obstime) [lindex $now 1]
set SCOPE(exptype) Object
set SCOPE(equinox) 2000.0
set SCOPE(ra) 00:00:00
set SCOPE(dec) +00:00:00
set SCOPE(secz) 0.0
set SCOPE(filterpos) 0
set SCOPE(filtername) none
set SCOPE(shutter) 1
setpoint set -20.0

#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : altatest
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 0.9
#  Date       : Aug-4-2003
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This is a minimal test procedure to test exposure, in-memory buffer, and save-to-disk
#
#  Arguments  :
#
#               exp	-	Exposure time in seconds
#               name	-	Image file name
#               light	-	Shutter open(1) or closed(0) (optional, default is 0)
 
proc altatest { exp name {light 0} } {
 
#
#  Globals    :
#  
#               CAMERA	-	id of current camera
#               DEBUG	-	Set to 1 for verbose logging
global CAMERA DEBUG
  if { $DEBUG } {debuglog "T = [$CAMERA read_TempCCD]"}
  if { $DEBUG } {debuglog "Starting exposure"}
  $CAMERA StartExposure $exp $light
  if { $DEBUG } {debuglog "Waiting..." }
  exec sleep $exp
  waitforalta
  if { $DEBUG } {debuglog "Reading out..."}
  $CAMERA GetImage
  if { $DEBUG } {debuglog "Saving to FITS $name"}
  write_image tempobs $name.fits
}






