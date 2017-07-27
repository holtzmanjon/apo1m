#!/usr/bin/wish


#
#  To rebuild the SelectFilters
#
#  g++ -g -O2 apgSampleCmn.o   SelectFilter.cpp   -o SelectFilter -I/opt/apogee/include/libapogee-3.0/ -I. -L/opt/apogee/lib -lapogee -lusb-1.0
#
#
proc savefilterconfig { } {
global FWHEEL FILTERWHEEL FILTERS WHEELID env
   set fout [open $env(HOME)/.apfw.ini w]
   puts $fout "set FILTERWHEEL $FILTERWHEEL"
   puts $fout "set WHEELID $WHEELID"
   set i 0
   while { $i < $FWHEEL($FILTERWHEEL) } {
      incr i 1
      puts $fout "set FILTERS($i,name) $FILTERS($i,name)"
      puts $fout "set FILTERS($i,focus) $FILTERS($i,focus)"
   }
   close $fout
   catch {.filters.save configure -bg gray}
}

proc watchfilters { args } {
  .filters.save configure -bg yellow
}

proc selectfilter { n } {
global FILTERWHEEL FWHEEL MAXFILTERS SETFILTER WHEELID
  set t $FWHEEL($FILTERWHEEL,t)
  set i 1
  while { $i <= $MAXFILTERS } {
     .filters.f$i configure -relief raised -bg gray -activebackground gray
     incr i 1
  }
  .filters.f$n configure -bg yellow -activebackground yellow
  update
  catch {set result [exec $SETFILTER $WHEELID $n]} ok
  set msg [split $ok \n]
  if { [string range [lindex $msg 3] 0 9] == "USB error:" } {
    set it [ tk_dialog .d "Apogee Filter Wheel" "$ok" {} -1 OK]
  } else {
    .filters.f$n configure -bg green -relief sunken -activebackground green
    set FHWEEL(position) $n
    if { [info exists /opt/apogee/bin/adjustTeleFocus] } {
       set delta  [expr $FILTERS($FWHEEL(position),focus) - $FWHEEL(focusoffset)]
       catch {exec /opt/apogee/bin/adjustTeleFocus $delta}
       set FWHEEL(focusoffset) $FILTERS($FWHEEL(position),focus)
    }
  }
}


set FWHEEL(FW50-7S)  7
set FWHEEL(FW50-9R)  9
set FWHEEL(AFW30-7R) 7
set FWHEEL(AFW50-5R)  5
set FWHEEL(AFW25-4R)  4
set FWHEEL(AFW50-10S) 10
set FWHEEL(CFW25_6R) 6
set FWHEEL(CFW31_8R) 8
set FWHEEL(AFW31-17R) 17
set FILTERWHEEL unknown
set FWHEEL(FW50-9R,t)  1
set FWHEEL(FW50-7S,t)  2
set FWHEEL(AFW25-4R,t) 3
set FWHEEL(AFW30-7R,t) 4
set FWHEEL(AFW50-5R,t) 5
set FWHEEL(AFW50-10S,t) 6
set FWHEEL(CFW25_6R,t) 7
set FWHEEL(CFW31_8R,t) 8
set FWHEEL(AFW31-17R,t) 9

foreach i "1 2 3 4 5 6 7 8 9" {
  set FILTERS($i,name) unknown
  set FILTERS($i,focus) 0
}
 
if { [file exists $env(HOME)/.apfw.ini] } {
   source $env(HOME)/.apfw.ini
} else {
   set it [tk_dialog .d "Apogee Filter Wheel" "Select type of filter wheel" {} -1 FW50-9R FW50-7S AFW25-4R AFW30-7R AFW50-5R AFW50-10S CFW25_6R CFW31_8R AFW31-17R]
   set FILTERWHEEL [lindex "FW50-9R FW50-7S AFW25-4R AFW30-7R AFW50-5R AFW50-10S CFW25_6R CFW31_8R AFW31-17R" $it]
   set WHEELID [expr $it+1]
   puts stdout "Selected filter wheel type is $FILTERWHEEL"
   savefilterconfig
}


set SETFILTER /opt/apogee/bin/SelectFilter
catch {set result [exec $SETFILTER $WHEELID 99]} ok
set MAXFILTERS [lindex [lindex [split $ok \n] 3] 6]
set FWHEEL(position) [lindex [lindex [split $ok \n] 4] 5]
puts stdout "Selected filter wheel type is $FILTERWHEEL"
savefilterconfig

destroy .filters
wm withdraw .
toplevel .filters -bg gray
wm title .filters "Apogee Filter Wheel control"
label .filters.lpos   -text "Position" -bg gray -fg black
label .filters.lname  -text "Filter Name" -bg gray -fg black
label .filters.lfocus -text "Focus offset" -bg gray -fg black
place .filters.lpos -x 30 -y 20
place .filters.lname -x 150 -y 20
place .filters.lfocus -x 290 -y 20

wm geometry .filters 390x[expr $MAXFILTERS*35 + 110]
set i 0
set iy 50
while { $i < $MAXFILTERS } {
   incr i 1
   button .filters.f$i -text "$i" -relief raised -bg gray -fg black -command "selectfilter $i" -width 8
   entry  .filters.name$i -textvariable FILTERS($i,name) -bg LightBlue -fg black -width 20
   entry  .filters.focus$i -textvariable FILTERS($i,focus) -bg LightBlue -fg black -width 8
   place  .filters.f$i -x 20 -y $iy
   incr iy 3
   place  .filters.name$i -x 110 -y $iy
   place  .filters.focus$i -x 300 -y $iy
   incr iy 30
}

button .filters.save -text "Save configuration" -fg black -bg green -width 33 -command "savefilterconfig"
place .filters.save -x 110 -y $iy
button .filters.exit -text "Exit" -fg black -bg gray -width 33 -command "exit"
place .filters.exit -x 110 -y [expr $iy+30]
set family [exec grep Family $env(HOME)/.apccd.ini]

if { $FWHEEL(position) == "" || $FWHEEL(position) == 0 } {set FWHEEL(position) 1}
.filters.f$FWHEEL(position) configure -bg green -relief sunken -activebackground green
set FWHEEL(focusoffset) $FILTERS($FWHEEL(position),focus)
trace variable FILTERS w watchfilters



