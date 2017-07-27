
send 'td 70'
string com 'window full'
send '{com}'
gpa=0.
string command 'CO %2f12.6 %f10.2' rah decd gpa
printf 'echo {command} >>master2com' >./obj.inp
printf 'echo "Y" >>master2com ' >>./obj.inp
printf 'echo {command} >>master2com'
printf 'echo "Y" >>master2com '
$csh obj.inp
call get

call coord 3
$csh obj.inp
call get

x0=150 y0=256  ! hole #3

send 'guideoff'
send 'window full'
send 'newcent 256 256'

iter=1
reiter:
send 'qck 1'
send 'scale 6500 6800'
send 'samescale'
send 'settemp -30'
call readccd
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

call findstar ireadbuf
open coo ./junk.coo
stat nlines=count[coo]
read coo
read coo
read coo
dmin=1e10
do i=1,nlines-3
  read coo
  x=@coo.2 y=@coo.3
  dist=(x-hc)^2+(y-hr)^2
  if dist<dmin&dist>100
   if (x+dx)>(xs+20)&(y+dy)>(ys+20)
    if (x+dx)<(xe-20)&(y+dy)<(ye-20)
      xg=x yg=y dmin=dist
    end_if
   end_if
  end_if
end_do
type xtarg ytarg
type xg yg

!tv $ireadbuf z=6400 l=400
!printf 'Mark target star'
!mark new
!x=c y=r
!printf 'Mark guide star'
!mark new
!xg=c yg=r

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
xs=max[1,x0-dist]
ys=max[1,y0-dist]
xe=min[512,x0+dist]
ye=min[512,y0+dist]
string com 'window %4i4' xs xe ys ye
send '{com}'

time=5
update=1
size=21
!string com 'exposure %f8.3' time
! need to modify guide command to take exposure
! set XTF 0.25?
! set TD 50?
string com 'newcent %2f8.3' x0 y0
send '{com}'
string com 'guide %2f10.3 %i %f8.1 %i4' xguide yguide size time update
printf '{com}'
send '{com}'

string com 'apo dither A'
send '{com}'
$sleep 10
string com 'apo expose 47'
send '{com}'
string com 'apo dither B'
send '{com}'
$sleep 10
string com 'apo expose 47'
send '{com}'
string com 'apo expose 47'
send '{com}'
string com 'apo dither A'
send '{com}'
$sleep 10
string com 'apo expose 47'
send '{com}'
send 'xdome 270'
$power1m on lights
send 'apo expose 3'
$power1m off lights
send 'ds'
end

