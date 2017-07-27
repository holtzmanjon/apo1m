#!/opt/apogee/bin/wish8.3

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
global env tcl_platform
  if { $tcl_platform(os) == "Darwin" } {
    set i [exec /sbin/ifconfig en0]
  } else {
    set i [exec /sbin/ifconfig eth0]
  }
  set n [lsearch $i inet]
  if { $tcl_platform(os) == "Darwin" } {
    set subnet [join [lrange [split [lindex $i [expr $n+1]] .] 0 2] .]
  } else {
    set subnet [join [lrange [split [lindex [split [lindex $i [expr $n+2]] :] 1] .] 0 2] .]
  }
  set try 1
  set found 0
  while { $try < 254 } {
     .m configure -text "Search for ALTA-E - addr = $subnet.$try"
     update
     set p "no"
     catch {set p [exec /opt/apogee/bin/http_ping -count 1 http://$subnet.$try]}
     if { $p != "no" } {
        set isit [catch {exec /opt/apogee/bin/wget -S -T 1 http://$subnet.$try} hdrs]
        set n [lsearch $hdrs Server:]
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
 
wm geometry . 400x100+400+300
label .m -text "Searching for ALTA-E - addr = "
place .m -x 30 -y 30
altaeDiscover 
exec sleep 3
exit



