
proc benchmark { n } {
global CAMERAS
   set c $CAMERAS(0)
   set i 0
   set begin [clock clicks]
   while { $i < $n } {
     $c Expose 0.01 0
     puts stdout "[format %6.6d $i]"
     flush stdout
     $c BufferImage READOUT
     incr i 1
   }
   set end [clock clicks]
   return [expr ($end - $begin)/$n/1000.]
}
 
#Benchmark results for U1 (768x512)
#
#  Raw frames (slow readout)	530 msec = 1.45Mb/sec
#  Raw frames (fast readout)	170 msec = 4.52Mb/sec
#  256x256 region (slow)	366 msec
#  256x256 region (fast)	164 msec
#  128x128 region (slow)	325 msec 
#  128x128 region (fast)	165 msec
#   64x64  region (slow)	307 msec
#   64x64  region (fast)	173 msec 
#
#Benchmark results for E1 (768x512)
#
#  Raw frames 			2734 msec = 0.28Mb/sec
#  256x256 region 		1020 msec
#  128x128 region 		749  msec
#   64x64  region 		670  msec
