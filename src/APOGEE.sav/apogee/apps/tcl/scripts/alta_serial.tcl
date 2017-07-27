#---------------------------------------------------------------------------
#---------------------------------------------------------------------------
#
#  Procedure  : altaserial
#
#---------------------------------------------------------------------------
#  Author     : Dave Mills (djm@randomfactory.com)
#  Version    : 1.4
#  Date       : April-30-2005
#  Copyright  : The Random Factory, Tucson AZ
#  License    : GNU GPL
#  Changes    :
#
#  This procedure communicates with the serial ports
#  on an ALTA-E camera. 
#
#  Arguments  :
#		port - A or B
#		op   - operation open/close/send
#		cmd  - text to transmit for send commands
#
#  Globals    : Expects ALTAIP to contain the IP address of the camera
#		Sets ALTAA or ALTAB to the socket handle
#

proc oldaltaserial { port op {cmd ""} } {
global ALTAA ALTAB ALTAIP
   switch $op { 
	open { if { $port == "A" } {
                 set ALTAA [socket $ALTAIP 2572]
                 fconfigure $ALTAA -buffering line
               } else {
                 set ALTAB [socket $ALTAIP 2573]
                 fconfigure $ALTAA -buffering line
               }
             }
	close { if { $port == "A" } {
                 catch {close $ALTAA}
               } else {
                 catch {close $ALTAB}
               }
             }
	send { if { $port == "A" } {
                  puts $ALTAA "$cmd"
               } else {
                  puts $ALTAB "$cmd"
               }
             }
   }
}

proc altaserial { port op {cmd ""} } {
global ALTAA ALTAB ALTAIP env
   switch $op { 
	open { if { $ALTAIP != "" } {
                 set c1 [split [lindex $ALTAIP 0] .]
                 set ipdec [expr 256*256*256*[lindex $c1 0] + 256*256*[lindex $c1 1] + 256*[lindex $c1 2] + [lindex $c1 3]]
		 if { $port == "A" } {
                   $ALTAA InitPort $ipdec 2572 0
                 } else {
                   $ALTAB InitPort $ipdec 2573 1
                 }
               } else {
                 if { [info exists env(ALTA_NUM)] } {
                    set camid $env(ALTA_NUM)
                 } else {
                    set camid 1
                 }
		 if { $port == "A" } {
                   $ALTAA InitPort $camid "/dev/usb/alta$camid"
                 } else {
                   $ALTAB InitPort $camid "/dev/usb/alta$camid"
                 }
               }
              }
	close { if { $port == "A" } {
                 catch {$ALTAA ClosePort}
               } else {
                 catch {$ALTAB ClosePort}
               }
             }
	send { if { $port == "A" } {
                  set s [$ALTAA cget -m_SerialSocket]
                  $ALTAA Write $s $cmd [string length $cmd]
               } else {
                  set s [$ALTAB cget -m_SerialSocket]
                  $ALTAB Write $s $cmd [string length $cmd]
               }
             }
	read { if { $port == "A" } {
                  set s [$ALTAA ReadBuffer]
               } else {
                  set s [$ALTAB ReadBuffer]
               }
               return $s
             }
   }
}

#
#  Load the correct shared library for this model
#
if { $ALTAIP == "" } {
   load $TKAPOGEE/lib/serial_USB.so
} else {
   load $TKAPOGEE/lib/serial_NET.so
}


#
#  Create serial port objects for both ports
#

set ALTAA [CApnSerial]
set ALTAB [CApnSerial]
