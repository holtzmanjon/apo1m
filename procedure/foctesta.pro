
error goto setftest
ftest=ftest+1
goto dofoc

setftest:
ftest=1

dofoc:
call readccd
string comment 'Start foctesta'
call writelog 1 incval '{comment}'

string fname 'focus%i1' ftest

send 'setfilt I'

inst=1 param=0 saotype=0 coarse=4

do alt=80,30,-10
  call altaz 180 alt
  call coord 1
  call check
  call getfoc focus inst param saotype coarse
  string out '{fname}%i2.2' alt
  $mv lastfocus.ps {out}a.ps
  send 'exp 2'
end_do
do alt=30,80,10
  call altaz 180 alt
  call coord 1
  call check
  call getfoc focus inst param saotype coarse
  string out '{fname}%i2.2' alt
  $mv lastfocus.ps {out}b.ps
  send 'exp 2'
end_do

!alt=70
!call altaz 180 alt
!call coord 1
!call check
!call getfoc focus inst param saotype coarse
!send 'exp 2'
!do alt=80,30,-10
!  call altaz 180 alt
!  call coord 1
!  saofoc
!  send 'exp 2'
!end_do

call readccd
string comment 'End foctesta'
call writelog 1 incval-1 '{comment}'

end
