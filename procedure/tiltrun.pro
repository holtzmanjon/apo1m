parameter x0 y0 dx dy

if dy==0
  dy=dx
end_if
if dx==0&dy~=0
  ix1=0 ix2=0
  iy1=-5 iy2=1
else
  ix1=-1 ix2=1
  iy1=-1 iy2=1
end_if

send 'g+disk'

if x0==0
  ask x0
end_if
if y0==0
  ask y0
end_if
if del==0
  ask del
end_if

guider=0 init=1

if init==1
  ! need to reset xoff and yoff to 0 after reacquiring star
  ! dont do this if no reacquistion has been done and you want to continue
  call tiltsetup x0 y0
end_if

etime=4

if guider==1
  call readgccd
  sfoc=gincval
else
  call readccd
  sfoc=incval
end_if

! 3x3 tilt run
do ix=ix1,ix2
  do iy=iy1,iy2
    xtilt=x0+ix*dx
    ytilt=y0+iy*dy
    string com 'xtilt %f8.3' xtilt
    send '{com}'
    string com 'ytilt %f8.3' ytilt
    send '{com}'

    azoffset=ix*dx*dazperxt
    aloffset=ix*dx*dalperxt
    azoffset=azoffset+iy*dy*dazperyt
    aloffset=aloffset+iy*dy*dalperyt
    azoffset=azoffset-azoff
    aloffset=aloffset-altoff
    string com 'daa %2f12.3' azoffset aloffset
    send '{com}'
    $sleep 5

    azoff=azoff+azoffset
    altoff=altoff+aloffset

    if guider==1
      string com 'gexp %f8.1' etime
    else
      string com 'exp %f8.1' etime
    end_if
    send '{com}'

  end_do
end_do

call tiltset x0 y0
string com 'Inspect results using: call tilt %i3' sfoc
printf '{com}'
string comment 'Start tilt run'
call writelog 1 sfoc '{comment}'


end
