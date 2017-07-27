parameter string=catalog jd pa gtime bexp vexp rexp iexp zexp nexpcode guidemon
error goto objerr
call check
if domeopen==0
  goto objerr
end_if

if jd==0
  ask 'Catalog star number: ' jd
end_if
!if pa==0&jd>0
!  ask 'Position angle: ' pa
!end_if
if gtime==0
!  ask 'Guider exposure time: ' gtime
  gtime=1
end_if
!if bexp==0&vexp==0&rexp==0&iexp==0&zexp==0
!  ask 'B exposure time: ' bexp
!  ask 'V exposure time: ' vexp
!  ask 'R exposure time: ' rexp
!  ask 'I exposure time: ' iexp
!  ask 'Z exposure time: ' zexp
!end_if
if nexpcode==0
  nexpcode=1
end_if

! Check to see if specified user catalogs exists, and if so, open in in tcomm
err goto nocat
if jd<0
  open cat usno/{catalog}.usno
else
  open cat /home/tcomm/scripts/{catalog}
end_if
close cat
if jd>0
  $echo "of" >> master2com
  string command '{catalog}'
  send '{command}'
end_if
goto move

nocat:
printf 'ERROR: catalog not found: /home/tcomm/scripts/{catalog}'
goto ending

error goto objerr

! Move to desired star
move:
send 'inst 1'
if jd>0
  string command 'rf %i4' jd
  printf 'echo {command} >>master2com' >./obj.inp
  printf 'echo "\r" >>master2com ' >>./obj.inp
  printf 'echo {command} >>master2com' 
  printf 'echo "\r" >>master2com ' 
  $csh obj.inp 
  call get
else
  ! File is a USNO output file
  call readusno usno/{catalog}.usno
  string command 'NE %f8.1' epoch
  send '{command}'
  string command 'CO %2f12.6' rah decd
  printf 'echo {command} >>master2com' >./obj.inp
  printf 'echo "\r" >>master2com ' >>./obj.inp
  printf 'echo {command} >>master2com' 
  printf 'echo "\r" >>master2com ' 
  $csh obj.inp
  call get
  if gbright<12
    gtime=1
    gupdate=5
  else_if gbright<13
    gtime=3
    gupdate=5
  else
    gtime=5
    gupdate=3
  end_if
end_if

! Set desired PA
string command 'pa %f8.2' pa
send '{command}'

! Update coordinates on bright star near the target
call coord 1

! Move back to target
$csh obj.inp
call get

! Start guiding
acktime=gtime*3
gsize=15

send 'guidehome'
! take one guider image to flush chip in case shutter has been open
send g-disk
send 'gexp 0'
! now take acquisition guider image and see if we can see a star
send g+disk
call readgccd
string command 'gexp %f8.1' acktime
send '{command}'
call writelog 0 gincval 'Initial guide star acquisition: {catalog}'
call readg gincval
zap $ireadbuf sig=0 size=3
if jd>0
  box 10 n=500 cr=256 cc=256
else_if gbright<90
  xs=max[10,gx-75]
  xe=min[505,gx+75]
  ys=max[10,gy-75]
  ye=min[505,gy+75]
  box 10 nr=ye-ys nc=xe-xs cr=gy cc=gx
else
  printf 'No guide star found. Not guiding'
  goto expos
end_if
abx $ireadbuf 10 high_row=hr high_col=hc high=high
if high<500
  printf 'No guide star found. Not guiding'
  goto expos
end_if
if jd>0
  gx=max[40,hc]
  gx=min[470,hc]
  gy=max[40,hr]
  gy=min[470,hr]
end_if
string com 'goffset %2f8.1' hc-gx hr-gy
send '{com}'

call readgccd
$sleep 5
string command 'gexp %f8.1' gtime
send '{command}'
call writelog 0 gincval 'Final guide acquisition: {catalog}'
call readg gincval
zap $ireadbuf sig=0 size=3
box 10 n=75 cr=gy cc=gx
abx $ireadbuf 10 high_row=hr high_col=hc

string com 'newguide %2f8.1 %i4 %i8 %i4' hc-1 hr-1 gsize gtime*1000 gupdate
send '{com}'
$sleep 30

call readgccd
string command 'gexp %f8.1' gtime
send '{command}'
string comment 'Guide star before sequence: {catalog}, %2f8.2' hc hr
call writelog 0 gincval 'Guide star before sequence: {catalog}'

if guidemon>0
  string com 'gwrite %i8' guidemon
  send '{com}'
else
  send 'gnowrite'
end_if

expos:
nexp=mod[nexpcode,100]
nset=int[(nexpcode-nexp)/100]
if nset==0
  nset=1
end_if
if jd<0
  send 'OBJECT {catalog}'
end_if
call readccd
printf '%i4 %i4 {root}' incval incval+nexp-1 >>../images/{root}/{catalog}.dat
do iset=1,nset
 ! Take specified exposures
 if bexp>0
  send 'setfilt B'
  if bexp<0.1
    bexp=0.03
  end_if
  call readccd
  printf '%i4 %i4 {root}' incval incval+nexp-1 >>../images/{root}/{catalog}.b
  string command 'mexp %f8.2 %i2' bexp nexp
  send '{command}' guidemon
 end_if

 if vexp>0
  send 'setfilt V'
  call readccd
  printf '%i4 %i4 {root}' incval incval+nexp-1 >>../images/{root}/{catalog}.v
  string command 'mexp %f8.2 %i2' vexp nexp
  send '{command}' guidemon
 end_if

 if rexp>0
  send 'setfilt R'
  call readccd
  printf '%i4 %i4 {root}' incval incval+nexp-1 >>../images/{root}/{catalog}.r
  string command 'mexp %f8.2 %i2' rexp nexp
  send '{command}' guidemon
 end_if

 if iexp>0
  send 'setfilt I'
  call readccd
  printf '%i4 %i4 {root}' incval incval+nexp-1 >>../images/{root}/{catalog}.i
  string command 'mexp %f8.2 %i2' iexp nexp
  send '{command}' guidemon
 end_if

 if zexp>0
  if zexp>60
  send 'setfilt z'
  else
  send 'setfilt 656n'
  end_if
  call readccd
  if zexp>60
  printf '%i4 %i4 {root}' incval incval+nexp-1 >>../images/{root}/{catalog}.z
  else
  printf '%i4 %i4 {root}' incval incval+nexp-1 >>../images/{root}/{catalog}.656n
  end_if
  string command 'mexp %f8.2 %i2' zexp nexp
  send '{command}' guidemon
 end_if

 if iexp<0
  send 'setfilt I'
  call guidtest
 end_if
end_do

send 'guideoff'
call readgccd
$sleep 5
string command 'gexp %f8.1' gtime
send '{command}'
call writelog 0 gincval 'Guide star after complete sequence: {catalog}'
goto ending

objerr:
printf 'Error in object script'

ending:
end
