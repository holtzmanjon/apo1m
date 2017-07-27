#!/usr/bin/tclsh

proc wm { args } {}
proc winfo { args } { return 0 }
wm withdraw .


# Implement the service
# This example just writes the info back to the client...
proc doService {sock msg} {
global TLM SCOPE
    puts stdout "echosrv:$msg"
    switch [lindex $msg 0] {
         "snapsleep"     { set res [snapsleep $SCOPE(datadir)/[lindex $msg 1] [lindex $msg 2] 0 [lindex $msg 3]]; puts $sock  "$res" }
         "snapshot"      { set res [snapshot  $SCOPE(datadir)/[lindex $msg 1] [lindex $msg 2] 0 [lindex $msg 3]]; puts $sock  "$res" }
         "driftscan"     { 
                           set res [driftscan $SCOPE(datadir)/[lindex $msg 1] [lindex $msg 2] [lindex $msg 3] [lindex $msg 4]] ; puts $sock $res
                         }
         "setpoint"      { set res [setpoint  [lindex $msg 1] [lindex $msg 2]];  puts $sock "$res" }
	 "getpoint"	 { set res [getpoint] ; puts $sock "$res" }
         "shutdown"      { puts $sock "ok" ; exit }
	 "fan"		 { set res [fanmode [lindex $msg 1]] ; puts $sock "$res" }
	 "datadir"	 { set d [lindex $msg 1] 
                           if { [file isdirectory $d] } {
                              set SCOPE(datadir) $d
                              puts $sock "ok"
                           } else {
			      puts $sock "ERROR: $d is not a directory"
                           }
                         }
         "setinfo"	 { set item [lindex $msg 1]
                           if { [info exists SCOPE($item)] } {
                              set SCOPE($item) "[lrange $msg 2 end]"
                              puts $sock "ok"
			   } else {
			      puts $sock "ERROR: $item is not configurable"
                           }
                         }
         "showinfo"      { set resp ""
                           foreach i [lsort [array names SCOPE]] {
                              set resp "$resp\n $i = $SCOPE($i)\n"
                           }
                           puts $sock "$resp"
                         }
	 "abort"	 { abortexposure ; puts $sock "ok" }
	 "status"	 { set s [getstatus] ; puts $sock "$s" }
         "speed"         { altamode [lindex $msg 1] ;  puts $sock "ok" }
         default         { puts $sock "ERROR: unknown $msg" }
    }
}


proc getstatus { {id 0} } {
global CAMERAS SCOPE STATUS SCOPE ALTA
   set camera $CAMERAS($id)
   if { $ALTA } {
     set c [$camera GetCoolerStatus]
     set cs [lindex "Off RampingToSetPoint AtSetPoint Revision" $c]
     set t [$camera GetTempCcd]
     set f [$camera GetFanMode]
     set fs [lindex "Off Slow Medium Fast" $f]
     set sh [$camera GetShutterState]
     set shs [lindex "Closed Open" $sh]
     set i [$camera GetImagingStatus]
     set is [lindex "DataError PatternError Idle Exposing ImagingActive ImageReady Flushing WaitingOnTrigger" [expr $i+2]] 
     return "$t , $cs , $fs , $shs , $is"
   } else {
     set t [get_temp]
     return $t
   }
}



# Handles the input from the client and  client shutdown
proc  svcHandler {sock} {
  set l [gets $sock]    ;# get the client packet
  if {[eof $sock]} {    ;# client gone or finished
     close $sock        ;# release the servers client channel
  } else {
    doService $sock $l
  }
}

# Accept-Connection handler for Server.
# called When client makes a connection to the server
# Its passed the channel we're to communicate with the client on,
# The address of the client and the port we're using
#
# Setup a handler for (incoming) communication on
# the client channel - send connection Reply and log connection
proc accept {sock addr port} {

  # if {[badConnect $addr]} {
  #     close $sock
  #     return
  # }

  # Setup handler for future communication on client socket
  fileevent $sock readable [list svcHandler $sock]

  # Note we've accepted a connection (show how get peer info fm socket)
  puts "Accept from [fconfigure $sock -peername]"

  # Read client input in lines, disable blocking I/O
  fconfigure $sock -buffering line -blocking 0

  # Send Acceptance string to client
  #  puts $sock "$addr:$port, You are connected to the echo server."
  #  puts $sock "It is now [exec date]"

  # log the connection
  puts "Accepted connection from $addr at [exec date]"
}

 

source /opt/apogee/apps/tcl/scripts/nogui.tcl

# Create a server socket on port $svcPort.
# Call proc accept when a client attempts a connection.
set svcPort 2001
socket -server accept $svcPort
vwait events    ;# handle events till variable events is set


