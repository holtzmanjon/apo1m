set XLATE(SHORT) 0
set XLATE(LONG)  1
set CAMERAS(0) [CCameraIO]
set CAMERA $CAMERAS(0)


#
#  This set of definitions is at the heart of the API interface. It defines an
#  assocation between items in the C++ API, and the generic elements of the
#  user interface. These elements are defined in the tcl_config.dat file.
#  Entries in this file automatically generate GUI widgets, and the 
#  lines below link these widgets with C++ callbacks in the CameraIO object.
#
set CCAPI(geometry.BIC) BIC
set CCAPI(geometry.BIR) BIR
set CCAPI(system.Base) BaseAddress
set CCAPI(geometry.BinX) BinX
set CCAPI(geometry.BinY) BinY
set CCAPI(ccd.Color) Color 
set CCAPI(geometry.Columns) Columns
set CCAPI(temperature.Mode) CoolerMode
set CCAPI(temperature.Target) CoolerSetPoint
set CCAPI(temperature.Status) CoolerStatus
set CCAPI(system.Data_Bits) DataBits 
set CCAPI(filter.Position) FilterPosition
set CCAPI(filter.StepPos) FilterStepPos
set CCAPI(ccd.Gain) Gain
set CCAPI(system.Guider_Relays) GuiderRelays 
set CCAPI(geometry.HFlush) HFlush 
set CCAPI(system.HighPriority) HighPriority
set CCAPI(geometry.Imgcols) ImgColumns
set CCAPI(geometry.Imgrows) ImgRows
set CCAPI(system.Interface) Interface
set CCAPI(system.Cable) LongCable
set CCAPI(system.MaxBinX) MaxBinX 
set CCAPI(system.MaxBinY) MaxBinY 
set CCAPI(ccd.Mode) Mode
set CCAPI(ccd.Noise) Noise
set CCAPI(geometry.NumCols) NumX
set CCAPI(geometry.NumRows) NumY
set CCAPI(ccd.PixelXSize) PixelXSize
set CCAPI(ccd.PixelYSize) PixelYSize
set CCAPI(system.RegisterOffset) RegisterOffset
set CCAPI(geometry.Rows) Rows
set CCAPI(ccd.Sensor) Sensor
set CCAPI(system.Sensor) SensorType
set CCAPI(system.Shutter_Speed) Shutter
set CCAPI(geometry.SkipC) SkipC
set CCAPI(geometry.SkipR) SkipR
set CCAPI(geometry.StartCol) StartX
set CCAPI(geometry.StartRow) StartY
set CCAPI(system.CameraStatus) Status
set CCAPI(geometry.DriftScan) TDI 
set CCAPI(temperature.Cal) TempCalibration 
set CCAPI(temperature.Control) TempControl
set CCAPI(temperature.Scale) TempScale 
set CCAPI(temperature.Value) Temperature
set CCAPI(system.Test2) Test2Bits
set CCAPI(system.Test) TestBits
set CCAPI(system.Timeout) Timeout
set CCAPI(system.Trigger) UseTrigger 
set CCAPI(geometry.VFlush) VFlush

#
#  Create reverse loopup into the CCAPI
#

foreach i [array names CCAPI] {
   set CCAPI($CCAPI($i)) $i
}


set DEBUG 1



#
#  Setup the camera, using values read from the .ini file
#
#    These  2 value are no longer set here, they are specified
#    when the kernel module is loaded
#
#	apogeeSetVal system.Base BaseAddress
#	apogeeSetVal system.Reg_offset RegisterOffset
#

apogeeSetVal geometry.Rows Rows
apogeeSetVal geometry.Columns Columns
apogeeSetVal system.PP_Repeat PPRepeat

$CAMERA InitDriver 0
$CAMERA Reset 
$CAMERA write_LongCable $XLATE($CONFIG(system.Cable))
$CAMERA write_UseTrigger 0
$CAMERA write_ForceShutterOpen 0

apogeeSetBool system.High_priority HighPriority
apogeeSetVal system.Data_Bits DataBits
apogeeSetBool system.Sensor SensorType 

$CAMERA write_Mode $CONFIG(system.Mode)
$CAMERA write_TestBits $CONFIG(system.Test)
if { [info exists CONFIG(system.Test2)] } {
   $CAMERA write_Test2Bits $CONFIG(system.Test2)
}

switch $CONFIG(system.Shutter_Speed) {
      normal {
              $CAMERA configure -m_FastShutter 0
              $CAMERA configure -m_MaxExposure 1048.75
              $CAMERA configure -m_MinExposure 0.01
             }
      fast   {
              $CAMERA configure -m_FastShutter 1
              $CAMERA configure -m_MaxExposure 1048.75
              $CAMERA configure -m_MinExposure 0.001
             }
      dual   {
              $CAMERA configure -m_FastShutter 1
              $CAMERA configure -m_MaxExposure 1048.75
              $CAMERA configure -m_MinExposure 0.001
             }
}
set CONFIG(tmp1) [expr $CONFIG(system.Shutter_Bits) & 0x0f]
apogeeSetVal tmp1 FastShutterBits_Mode
set CONFIG(tmp1) [expr ($CONFIG(system.Shutter_Bits) & 0xf0) >> 4]
apogeeSetVal tmp1 FastShutterBits_Test
apogeeSetVal system.MaxBinX MaxBinX
apogeeSetVal system.MaxBinY MaxBinY
apogeeSetBool system.Guider_Relays GuiderRelays
apogeeSetVal system.Timeout Timeout
apogeeSetVal geometry.BIC BIC
apogeeSetVal geometry.BIR BIR
apogeeSetVal geometry.SkipC SkipC
apogeeSetVal geometry.SkipR SkipR
apogeeSetVal geometry.Imgcols ImgColumns
apogeeSetVal geometry.Imgrows ImgRows
apogeeSetVal geometry.HFlush HFlush
apogeeSetVal geometry.VFlush VFlush
apogeeSetVal geometry.Imgcols NumX
apogeeSetVal geometry.Imgrows NumY
apogeeSetBool temp.Control TempControl 
apogeeSetVal temp.Cal TempCalibration 
apogeeSetVal temp.Scale TempScale 
$CAMERA write_CoolerSetPoint $CONFIG(temp.Target)
###$CAMERA write_CoolerMode  0
$CAMERA write_CoolerMode  1   
apogeeSetBool ccd.Color Color 
apogeeSetVal ccd.Noise Noise
apogeeSetVal ccd.Gain Gain
apogeeSetVal ccd.PixelXSize PixelXSize
apogeeSetVal ccd.PixelYSize PixelYSize
$CAMERA Flush 



#
#
#  Set default values for all items included in headers.
#  The GUI will populate these with the correct values later.
#

set CAMSTATUS(CoolerMode) 0
set CAMSTATUS(Temperature) 0.0
set CAMSTATUS(BinX) 1
set CAMSTATUS(BinY) 1
set CAMSTATUS(Gain) 1.0
set SCOPE(site) unknown
set SCOPE(camera) unknown
set SCOPE(detector) unknown
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


#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : test
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
 
proc test { exp name {light 0} } {
 
#
#  Globals    :
#  
#               CAMERA	-	id of current camera
#               DEBUG	-	Set to 1 for verbose logging
global CAMERA DEBUG
  if { $DEBUG } {debuglog "T = [$CAMERA read_Temperature]"}
  if { $DEBUG } {debuglog "Starting exposure"}
  $CAMERA Expose $exp $light
  if { $DEBUG } {debuglog "Waiting..." }
  exec sleep $exp
  if { $DEBUG } {debuglog "Reading out..."}
  $CAMERA BufferImage READOUT
  if { $DEBUG } {debuglog "Saving to FITS $name"}
  write_image READOUT $name.fits
}





