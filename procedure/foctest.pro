!parameter initfoc

!$sleep 1800

initfoc=focus

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

focaz=150
focaz=220

call altaz focaz 70
!call coord 1
call check

doguider=0
docolors=1
doalt=0
dotilt=0
dostage=0
saotype=0
if doalt==1
 send 'setfilt I'
 do alt=80,30,-5
   call altaz focaz alt
   !call newfoc 1
   call getfoc initfoc 1 1 saotype 5
   $mv lastfocus.ps {fname}i.ps
   if dotilt==1
     call tiltrun -0.03 0.08 0.05
   end_if
   call check
 end_do
end_if

if docolors==1

 send 'setfilt U'
 call getfoc initfoc 1 1 2 5
 $mv lastfocus.ps {fname}u.ps

 call altaz focaz 70
 send 'setfilt B'
 call getfoc initfoc 1 1 0 5
 $mv lastfocus.ps {fname}b.ps

 call altaz focaz 70
 send 'setfilt V'
 call getfoc initfoc 1 1 0 5
 $mv lastfocus.ps {fname}v.ps

 call altaz focaz 70
 send 'setfilt R'
 call getfoc initfoc 1 1 0 5
 $mv lastfocus.ps {fname}r.ps

call altaz focaz 70
send 'setfilt I'
call getfoc initfoc 1 1 0 5
$mv lastfocus.ps {fname}i.ps

 call altaz focaz 70
 send 'setfilt sdssu'
 call getfoc initfoc 1 1 2 5
 $mv lastfocus.ps {fname}sdssg.ps

 call altaz focaz 70
 send 'setfilt sdssg'
 call getfoc initfoc 1 1 0 5
 $mv lastfocus.ps {fname}sdssg.ps

 call altaz focaz 70
 send 'setfilt sdssr'
 call getfoc initfoc 1 1 0 5
 $mv lastfocus.ps {fname}sdssr.ps

 call altaz focaz 70
 send 'setfilt sdssi'
 call getfoc initfoc 1 1 0 5
 $mv lastfocus.ps {fname}sdssi.ps

 call altaz focaz 70
 send 'setfilt sdssz'
 call getfoc initfoc 1 1 0 5
 $mv lastfocus.ps {fname}z.ps

! call altaz focaz 70
! send 'setfilt 658n'
! call getfoc initfoc 1 1 2 5
! $mv lastfocus.ps {fname}658n.ps
!
! call altaz focaz 70
! send 'setfilt 659n'
! call getfoc initfoc 1 1 2 5
! $mv lastfocus.ps {fname}659n.ps
!
! call altaz focaz 70
! send 'setfilt 673n'
! call getfoc initfoc 1 1 2 5
! $mv lastfocus.ps {fname}673n.ps
!
! call altaz focaz 70
 send 'setfilt 665m'
! call getfoc initfoc 1 1 2 5
! $mv lastfocus.ps {fname}645m.ps
!
end_if

call readccd
string comment 'End foctest'
call writelog 1 incval-1 '{comment}'

!Guider focus at nominal position, various PAs
if doguider==1
  call readgccd
!  string comment 'Start PA foctest'
!  call writelog 0 gincval '{comment}'
! guider focus at center 
!  call newfoc 2
!  call getfoc initfoc 2
!  $mv lastfocus.ps {fname}guide.ps
  ! guider focus at home
  do ipa=0,360,45
    string com 'pa %i3' ipa
    send '{com}'
    call getfoc initfoc 2 1 0 4
  end_do
  send 'guidehome'
end_if

if dostage==1
  call readgccd
  string comment 'Start stage foctest'
  call writelog 0 gincval '{comment}'
  send 'pa 60'
  do istage=0,1200,200
    string com 'guideloc %i3' istage
    send '{com}'
    call getfoc initfoc 2 1 0 4
  end_do
end_if


focend:

end
