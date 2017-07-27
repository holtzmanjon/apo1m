parameter focus nfocus sleep

$power1m off mirfan

if sleep<0
  goto loop
end_if

! Parameters for observing
call initvar
! Read parameters from observing file
error goto definp
open objinp ./{root}.inp
goto readobj
definp:
open objinp ./default.inp
readobj:
string line '{objinp}'
string line '{objinp}'
dtfoc=@objinp.1 dtempfoc=@objinp.2
photom=@objinp.3 dtlstan=@objinp.4 dthstan=@objinp.5
ldome=@objinp.6
daytest=@objinp.7
string line '{objinp}'
string line '{objinp}'
nfilt=@objinp.1
do ifilt=1,nfilt
  string var 'objinp.-%i2.2' ifilt+1
  string f 'filt%i2.2' ifilt
  string {f} {{var}}
end_do
close objinp

error goto alldone

call startup

newdate:
dispose all
printf 'READY TO START NEW OBSERVING NIGHT'
!$kill_rvideo
!$start_rvideo
call testobs
printf 'READY TO START NEW OBSERVING NIGHT, using approximate schedule above'
printf 'If observing schedule is not as desired, adjust input files now'
printf '\n Enter C to continue to telescope and dome reinitialization'
no35m=0 
pause
call time
send initccd
call readccd
string root '{ccdroot}'

reask:
printf 'Do you want to open louvers and open dome ~1 hr before sunset?'
string doinit '?Enter Y/y for yes, otherwise no: '
error goto reask
strcmp {doinit} Y silent
a=strcmpok
strcmp {doinit} y silent
if strcmpok==1|a==1
  preopen=1
else
  preopen=0
end_if

!$power1m on apogee
$power1m on rackfan
$power1m on motors
$sleep 10
!$power1m on apogee
$power1m on rackfan
$power1m on motors
!call ccdtemp
printf 'The telescope will now be reinitialized. You must watch in the'
printf '  remote video to confirm proper behavior. If something looks '
printf '  wrong, kill the power!!'
redo:
printf 'Enter C to continue with initialization'
printf 'Enter S to skip telescope and dome initialization'
string doinit '?Enter C or S: '
error goto redo
strcmp {doinit} S silent
a=strcmpok
strcmp {doinit} s silent
if strcmpok==1|a==1
  goto setup
end_if
strcmp {doinit} C silent
a=strcmpok
strcmp {doinit} c silent
if strcmpok==0&a==0
  goto redo
end_if
call send pstatus

!$kill_rvideo
!$start_rvideo
$echo "INIT" >> master2com
$echo "I" >> master2com
$echo "y" >> master2com
call readstat 1
if domeinit==0
  $echo "n" >> master2com
end_if
if domeslav==0
  $echo "n" >> master2com
end_if
call get
printf 'The dome will now be reinitialized.'
printf 'Enter C to continue with initialization'
send 'DI'
send 'XDOME 75'
send 'DI'
send '-35m'
send 'clear'

setup:
send 'filtinit'
send 'guidehome'

reask2:
printf 'Do you want to fill the dewar?'
string dofill '?Enter Y/y for yes, otherwise no: '
error goto reask2
strcmp {dofill} Y silent
a=strcmpok
strcmp {dofill} y silent
if strcmpok==1|a==1
  call time
  sleep=(sunset-0.35-localtim)*3600
  if sleep>0
    $power1m off motors
    string com 'sleep %i' sleep
    printf 'sleeping: {com}'
    ${com}
  end_if
  call fill
end_if
sleep=0
! check dome and open when 3.5m is open
call time
if daytest==0&localtim<(sunset-0.75)&localtim>ntwimorn
  printf 'The motors generate a significant amount of heat that'
  printf '  is difficult to vent during the day. Motor power will now'
  printf '  be turned off, and script will be suspended until 45 minutes,'
  printf '  before sunset after which it will automatically continue.'
  $sleep 120
  $power1m off motors
  sleep=(sunset-0.75-localtim)*3600
end_if
if sleep>0
  printf 'Sleeping for %i seconds' sleep
  string com 'sleep %i' sleep
  ${com}
  $power1m on motors
!  $power1m on apogee
!  call ccdtemp
end_if

call initdate
! Parameters for observing
call initvar
! Read parameters from observing file
error goto definp2
open objinp ./{root}.inp
goto readobj2
definp2:
open objinp ./default.inp
readobj2:
string line '{objinp}'
string line '{objinp}'
dtfoc=@objinp.1 dtempfoc=@objinp.2
photom=@objinp.3 dtlstan=@objinp.4 dthstan=@objinp.5
ldome=@objinp.6
daytest=@objinp.7
string line '{objinp}'
string line '{objinp}'
nfilt=@objinp.1
do ifilt=1,nfilt
  string var 'objinp.-%i2.2' ifilt+1
  string f 'filt%i2.2' ifilt
  string {f} {{var}}
end_do
close objinp
call initproc

error goto alldone
! Comments for the log
!printf 'NMSU 1m auto-observing' >/home/tmp/{root}.program.comment
!printf 'NMSU 1m auto-observer' >/home/tmp/{root}.observer.comment
string oroot '%i6.6' {root}-1 
!printf '<a href=http://galileo.apo.nmsu.edu/sky/irsc/data/mpegarchive/IRSCmpg-20{oroot}.mpg> MPEG archive </a> (<a href=http://galileo.apo.nmsu.edu/sky/irsc/data/Latest.mpg> latest </a>)' |
!  >/home/tmp/{root}.weather.comment
call writesum -1 '<H3> {root} Summary Observation Log</h3>'
call initccd

if preopen==1
  send '-35m'
  send 'clear'
  call altaz 90 70
  printf 'Opening louvers ...'
  $echo "OL" >>master2com
  $echo "Y" >>master2com
  call get
  send 'xod'
!  send 'old'
  $power1m on domefan
end_if

printf 'Checking for 3.5m dome open to wait to open....'
call check
! Is it time to stop?
call time
if (localtim<12&localtim>ntwimorn+0.35&daytest==0)
  goto twimorn
end_if

! Initial focus guess
focus0=focus
if focus0==0
  printf 'INITIAL FOCUS GUESS'
  call readstat
  focus=focref-(outtemp-tref)*18
end_if

send +display
send g+display
send -xfer
send g-xfer

string com 'fo %f12.3' focus
send '{com}'

! get some twilight flats pointing east if its not dark yet
call time
if (localtim>12&localtim<ntwieve-0.5&daytest==0)
  call twiwait ntwieve -0.85
  printf 'EVENING FLATS'
  if focus0==0
    printf 'INITIAL FOCUS GUESS'
    call readstat
    focus=focref-(outtemp-tref)*18
  end_if
  call flateve 
end_if

! wait for nautical twilight-24 minutes
if (localtim>12&daytest==0)
  call twiwait ntwieve -0.40
end_if

! initial pointing correction (first star problem)
send 'settime'
printf 'INITIAL POINTING CORRECTION'
call check
call altaz 220 60
saot
call coord 1 0 0 1

! Open guider shutter
!send 'guidefac 0.7'
send 'noshutter'
do i=1,20
  send 'open'
end_do

! Set tracking dtime
send 'td 50'
send 'guidefac 0.75'

if focus0==0
  printf 'INITIAL FOCUS GUESS'
  call readstat
  focus=focref-(outtemp-tref)*18
!  call time
!  if (localtim>12&localtim<(ntwieve+1))
!    focus=focus+150
!  end_if
end_if

!call spin 3
if no35m==0
  send '+35m'
end_if
$(cd ../alert; make start)
send '+alert'

! Initial focus
call checkfoc

! Here starts the main observing loop for the whole night!!
loop:

printf 'TOP OF LOOP'

call check
! call ccdtemp after check in case weve been closed for a while
call ccdtemp

! Is it time to stop?
call time
if (localtim<12&localtim>ntwimorn)
  send '-35m'
end_if
if (localtim<12&localtim>ntwimorn+0.35&daytest==0)
  goto twimorn
end_if

! Put focus in getobj
if 0==1
! Do we need to focus? 
if dofoc>=0|dofoc<-1
 call time
 call readstat
! call ccdtemp
 if dofoc>0|(uttim>nextf)|((uttim-tfoc)>dtfoc)|(abs[(auxtemp-foctemp)]>dtempfoc)
  printf 'FOCUS: %6f10.2' uttim tfoc dtfoc auxtemp foctemp dtempfoc
  call getdate
  printf '{date} FOCUS: %6f10.2' uttim tfoc dtfoc auxtemp foctemp dtempfoc >>./focus.log
! send 'pa 0'
  fwhm=10
  oldfoc=focus
  if nfocus==0
    coarse=1
  else
    coarse=0
  end_if
  ! If first time, do full focus run
  ! If not first time, set current object to 0 to force reacquisition
  !   of next object with attendant refocus
  if nfocus<=1000
    call altaz 150 70
    call coord 1 0 0 1
    call getfoc focus 1 0 0 coarse
    focerr1=focerr
!    call getfoc focus -2 2 0 4
  else
    focerr1=0
    focerr=0
    curobj=0
  end_if
  if (focerr==0&focerr1==0)|(daytest==1)
    call time
    nfocus=nfocus+1 
    if nfocus==1
      nextf=uttim+0.5
    else_if nfocus<4
      nextf=uttim+1
    else
      slope=(focus-oldfoc)/(uttim-tfoc)
      if (abs[slope]>0)
        nextf=uttim+max[0.5,50/abs[slope]]
      else
        nextf=100
      end_if
    end_if
    tfoc=uttim
    foctemp=auxtemp
  end_if
  printf 'FOCUS nextf: %f10.2' nextf
  curobj=0
 end_if
end_if
end_if

! Do we need to stop?
call time
if (localtim<12&localtim>ntwimorn+0.35&daytest==0)
  goto twimorn
end_if

! Do we need to do standards?
if photom>0
  if photom==1
    string stanfile 'stan.apo'
  else
    string stanfile 'stansdss.apo'
  end_if
!  send 'XDOME 60'
!  send 'DI'
  call time
  if (uttim-tlstan>dtlstan)
    printf 'LOW AIRMASS STANDARD'
    call dostan {stanfile} 1 1.3 4 
    call time
    tlstan=uttim
    curobj=0
  end_if
  call time
  if (uttim-thstan>dthstan)
    printf 'HIGH AIRMASS STANDARD'
    call dostan {stanfile} 1.55 1.9 4 
    call time
    thstan=uttim
    curobj=0
  end_if
end_if

! Do we need to stop?
call time
if (localtim<12&localtim>ntwimorn+0.35&daytest==0)
  goto twimorn
end_if

! Get the next acceptable object and do it
printf 'GETOBJ'
call getobj

goto loop

send '-alert'
! were finished with objects, if its more than 3/4 hour past nautical, just
!   close up, otherwise do flats
call time
if localtim<12&localtim>ntwimorn+0.75
  goto alldone:
end_if

twimorn:
send '-alert'
printf 'twimorn'
call twiwait ntwimorn 0.4
call time
if localtim<12&localtim<ntwimorn+0.85
  call readstat 1
  if domeopen==1
!    call flatmorn 5
    call flatmorn 
  end_if
end_if

alldone:
send '-alert'
call close

! get some darks
!call biasdark 300 5 dark300
call biasdark 0 10 sbiasmorn
!call biasdark 1 5 dark001
!call biasdark 5 5 dark005
!call biasdark 30 5 dark030
!call biasdark 60 5 dark060
! 300s darks at a variety of temperatures
!call darktest

! Shut everything down
call shutdown

! Remove duplicate log entries and remake HTML log page
$cp ../images/{root}/{root}.log ../images/{root}/{root}.log.sav
!$shortlog {root}
printf '</body></html>' >>/home/tcomm/public_html/{root}sum.html

! Compress files
$zzz ../images/{root}
$cp ../.htaccess ../images/{root}/
error goto noprops
$mv ../images/{root}/{root}.props ../images/{root}/{root}.props.all
$sort ../images/{root}/{root}.props.all | uniq > ../images/{root}/{root}.props
open props ../images/{root}/{root}.props
stat n=count[props]
if n==0
  close props
  goto noprops
end_if
string users ' '
do i=1,n
  string prop {props}
  $awk '($1=="{prop}") \{printf("Require user %s\n",$2)\}' /home/tcomm/props/users >>../images/{root}/.htaccess
end_do
close props

noprops:
! Start auto-sftp job
$dosftp images/{root} now

! try to make biases/darks/flats
!call ../images/{root}/biasdark
!call ../images/{root}/flat

focus=0 nfocus=0 sleep=0
call testobs
goto newdate

close com2master

end
