parameter string=catalog jd 
error goto objerr
call check
if domeopen==0
  goto objerr
end_if

gtime=1
nexp=1

error goto objerr

! Create the USNO guide star file
open cat /home/tcomm/scripts/{catalog}
if jd==1
  goto readit
end_if
do i=1,jd-1
  read cat
end_do
readit:
string objstr '{cat.1}'
string rastr '{cat.-2}'
string decstr '{cat.-3}'
string epoch  '{cat.-4}'
close cat
string command 'square {rastr} {decstr} %f12.6 {epoch}' 48.8/60.
${command} >usno/{objstr}.usno

! Move to desired star
move:
send 'inst 1'

! File is a USNO output file
call readusno usno/{objstr}.usno
!ty gx gy pa
!pause
string command 'NE %f8.1' epoch
send '{command}'
string command 'CO %2f12.6' rah decd
printf 'echo {command} >>master2com' >./obj.inp
printf 'echo "Y" >>master2com ' >>./obj.inp
printf 'echo {command} >>master2com' 
printf 'echo "Y" >>master2com ' 
$csh obj.inp
call get
if gbright<10
    gtime=1
    gupdate=5
else_if gbright<11
    gtime=3
    gupdate=5
else
    gtime=5
    gupdate=3
end_if

! Set desired PA
string command 'pa %f8.2' pa
send '{command}'

! Update coordinates on bright star near the target
call coord 1

! Move back to target
$csh obj.inp
read com2master

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
!if jd>0
!  box 10 n=500 cr=256 cc=256
if gbright<90
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
!if jd>0
!  gx=hc gy=hr
!else
  string com 'goffset %2f8.1' hc-gx hr-gy
  send '{com}'
!end_if

call readgccd
$sleep 5
string command 'gexp %f8.1' gtime
send '{command}'
call writelog 0 gincval 'Final guide acquisition: {catalog}'
call readg gincval
zap $ireadbuf sig=0 size=3
box 10 n=100 cr=gy cc=gx
abx $ireadbuf 10 high_row=hr high_col=hc

string com 'newguide %2f8.1 %i4 %i8 %i4' hc-1 hr-1 gsize gtime*1000 gupdate
send '{com}'

send 'OBJECT {objstr}'
goto ending

objerr:
printf 'Error in object script'

ending:
end
