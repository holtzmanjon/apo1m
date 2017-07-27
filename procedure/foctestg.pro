
error goto setftest
ftest=ftest+1
goto dofoc

setftest:
ftest=1

dofoc:
call readccd
string comment 'Start foctestg'
call writelog 1 incval '{comment}'

string fname 'focus%i1' ftest


call altaz 180 70
send 'inst 1'
call coord 1
call check

send 'inst 2'
inst=2 param=1 saotype=2 coarse=4 foctype=2
do ipa=0,360,60
  string com 'pa %i3' ipa
  send '{com}'
  call getfoc focus inst param saotype coarse foctype
  string out '{fname}%i3.3' pa
  $mv lastfocus.ps {out}.ps
end_do

call readccd
string comment 'End foctestg'
call writelog 1 incval-1 '{comment}'

end
