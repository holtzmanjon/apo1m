#!/usr/bin/tclsh
#
#  Convert scripts to reside in /Applications/ApogeeCamera.app/Contents
#

set all [glob *]
set fout [open /tmp/doit w]
puts $fout "#!/bin/sh"
foreach i $all {
   puts $fout "perl -pi -w -e 's/\\/opt\\/apogee/\\/Applications\\/ApogeeCamera.app\\/Contents/g;' $i"
}
close $fout
exec chmod a+x /tmp/doit
exec /tmp/doit


