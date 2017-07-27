set TKAPOGEE /opt/apogee
load $TKAPOGEE/lib/libfitsTcl.so
load $TKAPOGEE/lib/libccd.so
load /opt/apogee/lib/apogee_USB.so

###set CAMERAS(0) [CCameraIO]
set CAMERAS(0) [CApnCamera]
set CAMERA $CAMERAS(0)
$CAMERA InitDriver 0 0 0
set c $CAMERA

set CREAD ""
catch {$c help} api
foreach i $api { 
    if { [string range $i 0 4] == "read_" } {
       lappend CREAD $i
   }
}
set CREAD [lsort $CREAD]

set CVARS ""
set api [info commands]
foreach i $api { 
    if { [string range $i 0 12] == "CApnCamera_m_" } {
        if { [lindex [split $i _] 3] == "get" } {
           lappend CVARS  [lindex [split $i _] 2] 
       }
   }
}
set CVARS [lsort $CVARS]


proc altastatus { {id 0} } {
global CAMERAS CREAD CVARS
  set c $CAMERAS($id)
  $c sensorInfo
  foreach i $CREAD {
      catch {
        puts stdout "[string range $i 5 end] =	 	[$c $i]"
      }
  }
  foreach i $CVARS {
      catch {
        puts stdout "$i =	 	[CApnCamera_m_[set i]_get $c]"
      }
  }
}

set ALTAMODELS "U1 U2 U3 U4 U5 U6 U7 U8"
set CAMSTATUS(CoolerMode) 1
set CAMSTATUS(Temperature) [$CAMERA read_TempCCD]
set CAMSTATUS(BinX) 1
set CAMSTATUS(BinY) 1
set CAMSTATUS(Gain) 1.0
set SCOPE(site) unknown
set SCOPE(camera) "Apogee ALTA - [lindex $ALTAMODELS [$CAMERA cget -m_CameraId]]
"
set SCOPE(detector) [$CAMERA cget -m_Sensor]
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




