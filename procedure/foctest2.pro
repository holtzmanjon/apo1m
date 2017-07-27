parameter initfoc

error goto setftest
ftest=ftest+1
goto dofoc

setftest:
ftest=1

dofoc:
call readccd
string comment 'Start foctest'
call writelog 1 incval '{comment}'

string fname 'focus%i1' ftest

call altaz 220 70
call check

! guider focus at center
!call getfoc initfoc 0

! PI focus
send 'setfilt U'
call getfoc initfoc 3
$mv lastfocus.ps {fname}u.ps

send 'setfilt C'
call getfoc initfoc 3
$mv lastfocus.ps {fname}c.ps

send 'setfilt B'
call getfoc initfoc 2
$mv lastfocus.ps {fname}b.ps

send 'setfilt V'
call getfoc initfoc 2
$mv lastfocus.ps {fname}v.ps

send 'setfilt R'
call getfoc initfoc 2
$mv lastfocus.ps {fname}r.ps

send 'setfilt I'
call getfoc initfoc 2
$mv lastfocus.ps {fname}i.ps

send 'setfilt Z'
call getfoc initfoc 2
$mv lastfocus.ps {fname}z.ps

call readccd
string comment 'End foctest'
call writelog 1 incval-1 '{comment}'

!Guider focus at nominal position, various PAs
call readgccd
string comment 'Start foctest'
call writelog 0 gincval '{comment}'
call getfoc initfoc 0 1
$mv lastfocus.ps {fname}guide.ps

send 'pa 60'
call getfoc focus 0 2
send 'pa 120'
call getfoc focus 0 2
send 'pa 180'
call getfoc focus 0 2
send 'pa 240'
call getfoc focus 0 2
send 'pa 300'
call getfoc focus 0 2
send 'pa 0'
call getfoc focus 0 2

send 'guidefoc 50'
call getfoc focus 0 2
send 'guidehome'

focend:
end
