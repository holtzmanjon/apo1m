proc initcamera { camtype camaddr camfirm cammodel } {
global CAMERA CAMERAS APGSELECTED
   if { [string range $camtype 0 5] == "Ascent" } { set CAMERA [Ascent] }
   if { [string range $camtype 0 4] == "AltaU" }  { set CAMERA [Alta] }
   if { [string range $camtype 0 4] == "AltaF" }  { set CAMERA [AltaF] }
   if { [string range $camtype 0 4] == "Aspen" }  { set CAMERA [Aspen] }
   $CAMERA OpenConnection usb $camaddr $camfirm $cammodel
   $CAMERA Init
   set CAMERAS(0) $CAMERA
   set APGSELECTED "$camtype @ $camaddr"
}

if { $CONFIG(system.Interface) == "USB" } {
   if { [info exists env(APOGEE_CAMERA)] } {
      set item $env(APOGEE_CAMERA)
      set detail [split $item "=,"]
      set camaddr [lindex $detail 1]
      set camfirm [format %d [lindex $detail 9] 1]
      set cammodel  [format %d [lindex $detail 7] 1]
      set camtype [lindex $detail 11]
      set camcount 1
      puts stdout "APOGEE_CAMERA environment used to select camera"
   } else {
    toplevel .afinder -width 600 -height 100
    wm title .afinder "Select Camera"
    wm geometry .afinder +200+400
    catch {set finder [exec /opt/apogee/bin/listApogee]} result
    set result [split $result "\<\>"]
    set camcount 0
    set fwcount 0
    foreach item $result {
      if { [string range $item 0 6] == "address" } {
         puts stdout "Found Apogee device $item"
         set detail [split $item "=,"]
         if { [lindex $detail 7] == "filterWheel" } {
            incr fwcount 1
            exec wish -f /opt/apogee/apps/tcl/scripts/filters.tcl &
         } else {
           set camaddr [lindex $detail 1]
           set camfirm [format %d [lindex $detail 9] 1]
           set cammodel  [format %d [lindex $detail 7] 1]
           set camtype [lindex $detail 11]
           if { [lindex $detail 5] == "camera" } {
             button .afinder.c$camcount -width 20 -text $camtype -command "initcamera $camtype $camaddr $camfirm $cammodel"
             pack .afinder.c$camcount -side left
             incr camcount 1
           }
         }
      }
    }
   }
   if { $camcount == 1 } {
     initcamera $camtype $camaddr $camfirm $cammodel
   } else {
     vwait APGSELECTED
   }
}
     

destroy .afinder
puts stdout $APGSELECTED



