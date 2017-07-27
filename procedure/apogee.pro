parameter string=catalog noapogee noguide noacq dopause

objerr=0
noapogee=1
!dopause=1

! focus offset?
apofoc=50

! guider
!send 'guidefac 0.75'
send 'guidefac 0.9'
gtime=3
gupdate=1
gsize=21

! telescope
send 'td 70'

if dopause==1
  error goto nofind
else
  error goto apoerr
end_if

$power1m off domefan
$power1m off louvers

string com 'window full'
send '{com}'

! goto star
gpa=0.
if noacq<2 
 string command 'CO %2f12.6 %f10.2' rah decd gpa
 printf 'echo {command} >>master2com' >./obj.inp
 printf 'echo "Y" >>master2com ' >>./obj.inp
 printf 'echo {command} >>master2com'
 printf 'echo "Y" >>master2com '
 $csh obj.inp
 call get
 
 if noacq==0
   call coord 3
 end_if
 $csh obj.inp
 call get
 send 'OBJECT {catalog}' 
else
 exp01=20
end_if

x0=150 y0=256  ! hole #3 4?
x0=152 y0=256
x0=146.5 y0=251.5
send 'guideoff'
send 'window full'
send 'newcent 256 256'

! adjust focus for better throughput???
string com 'fo %f8.2' focus+apofoc
send '{com}'

! set exposure time based on number of reads expected
if exp01>10
  itime=2
else_if dopause==0
  itime=0.5
else
  itime=0.1
end_if
if noacq>1
  itime=0.5
end_if
string expcom 'exp %f8.1' itime

! first iteration from center of chip to near hole
! second iteration into hole
iter=1
reiter:
call readccd
send '{expcom}'
send 'scale 6400 7200'
send 'samescale'
send 'settemp -30'
call read incval
xs=sc[ireadbuf] xe=sc[ireadbuf]+nc[ireadbuf]-1
ys=sr[ireadbuf] ye=sr[ireadbuf]+nr[ireadbuf]-1
! first iteration look near center, otherwise look near where we moved star
if iter==1
  box 10 n=128 cr=256 cc=256
else
  box 10 n=128 cr=y0+20 cc=x0
end_if
abx $ireadbuf 10 high_row=hr high_col=hc high=high
box 1 n=31 cr=hr cc=hc 
a=setval[ireadbuf,hr,hc,high+1]
automark $ireadbuf box=1  range=high,high+10 new
save coo=./junk sr=0 sc=0
open coo ./junk.coo
do i=1,4
  read coo
end_do
xtarg=@coo.2 ytarg=@coo.3

if iter==1
  ! first iteration move close to hole, to make sure that any distortion
  ! doesnt mess up the relative position of target and guide star
  string com 'offset %2f10.3' xtarg-x0 ytarg-(y0+20)
  send '{com}'
  $sleep 5
  iter=2
  got reiter
end_if
dx=x0-xtarg dy=y0-ytarg

box  1 n=500 cr=256 cc=256
call findstar ireadbuf 1 0 2
open coo ./junk.coo
stat nlines=count[coo]
read coo
read coo
read coo
dmin=1e10
xg=0 yg=0
do i=1,nlines-3
 read coo
 x=@coo.2 y=@coo.3
 dist=(x-hc)^2+(y-hr)^2
 if dist<dmin&dist>50
  if (x+dx)>(xs+20)&(y+dy)>(ys+20)
   if (x+dx)<(xe-20)&(y+dy)<(ye-20)
     xg=x yg=y dmin=dist
   end_if
  end_if
 end_if
end_do
nofind:
type xtarg ytarg
type xg yg
!pause
!tv $ireadbuf z=6400 l=400
!printf 'Mark target star'
!mark new
!x=c y=r
!printf 'Mark guide star'
!mark new
!xg=c yg=r
if dopause==1
  ask xtarg
  ask ytarg
  ask xg
  ask yg
end_if



string com 'offset %2f10.3' xtarg-x0 ytarg-y0
send '{com}'
$sleep 5
xguide=xg+x0-xtarg 
yguide=yg+y0-ytarg
update=1
xs=min[xguide,x0]-40
ys=min[yguide,y0]-40
xe=max[xguide,x0]+40
ye=max[yguide,y0]+40
dist=sqrt[dmin]+40
if exp01>10
  xs=max[1,x0-dist]
  ys=max[1,y0-dist]
  xe=min[512,x0+dist]
  ye=min[512,y0+dist]
end_if
string com 'window %4i4' xs xe ys ye
send '{com}'



!string com 'exposure %f8.3' time
! need to modify guide command to take exposure
! set XTF 0.25?
! set TD 50?
string com 'newcent %2f8.3' x0 y0
send '{com}'
call readccd
incval0=incval
if dopause==1
  pause
end_if
string com 'guide %2f10.3 %i %f8.1 %i4' xguide yguide gsize gtime gupdate
printf '{com}'
send '{com}'
$sleep 20
if dopause==1
  pause
end_if
if noacq>1
  pause
end_if

if noapogee==0
  send 'apo shutter open'
end_if
ntot=0
err goto setdith
a=dithpos
goto havedith
setdith:
dithpos=0
havedith:
do iset=1,nset
 do i=1,nfilt
   string var 'exp%i2.2' i
   string nvar 'nexp%i2.2' i
   nexp={nvar}
   nreads={var}
   if nexp==1
    if noapogee==0 
     if dithpos~=1
       send 'apo dither A'
     end_if
     dithpos=1
     $sleep 10
     string com 'apo expose %i' nreads
     send '{com}'
    else
     string com 'sleep %i' nreads*10
     ${com}
    end_if
     ntot=ntot+1
   else_if nexp==2
    if noapogee==0
     send 'apo dither B'
     dithpos=2
     $sleep 10
     string com 'apo expose %i' nreads
     send '{com}'
    else
     string com 'sleep %i' nreads*10
     ${com}
    end_if
     ntot=ntot+1
   end_if
   if ntot==1
    string apostart '{cval}'
   end_if
 end_do
end_do
string apostop '{cval}'

call readccd
printf '{apostart} {apostop} {root} %2i8  %4f10.2' incval0 incval xguide yguide xg yg >>../images/{root}/{catalog}.dat
if {apostart}<0|{apostop}<0 
  printf '# {catalog}' >>../images/{root}/{group}.dat
  objerr=1
  goto apoend
else
  printf '{catalog}' >>../images/{root}/{group}.dat
end_if

send 'guideoff'
send 'offset 100 100'
call time
if noapogee==0
  if (uttim-tflat>dtflat)
    printf 'FLAT FIELD'
    send 'xdome 270'
    $power1m on lights
    send 'apo expose 3'
    printf '{cval} {cval} {root}' >>../images/{root}/apogeeflat.dat
    $power1m off lights
    ! send 'apo shutter close'
    send 'ds'
    call time
    tflat=uttim
    curobj=0 
  end_if 
end_if

goto apoend

apoerr:
printf 'Error in apogee.pro!'
printf '{catalog} ERROR IN APOGEE.PRO' >>../images/{root}/{group}.dat
objerr=1


apoend:
string com 'window full'
send '{com}'
send 'newcent 256 256'
end

