#!/usr/bin/tclsh


set olddir [lindex $argv 0]
set newdir [lindex $argv 1]

set exclude "resource.h StdAfx.cpp StdAfx.h Camera.cpp Camera2.cpp Camera2.h" 
set files [exec find $newdir/. -print]
foreach i $files {
   set name [file tail $i]
   if { [lsearch $exclude $name] < 0 } {
    set ext [file extension $i]
    if { $ext == ".cpp" || $ext == ".h" } {
      set fd [open /tmp/find w]
      puts $fd "#!/bin/sh\nfind $olddir/. -name \'$name\' -print\n"
      close $fd
      exec chmod 755 /tmp/find
      catch { set comp [exec /tmp/find] } ok
      if { $comp == "" } {
         puts stdout "No older version of $name"
      } else {
        catch { set res [exec diff $i $comp] } ok
        if { $ok != "" } {
           puts stdout "Change in $comp"
        }
      }
    }
   }
}

