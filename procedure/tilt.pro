parameter start auto oned
! 12/08/08  collimation xtilt,ytilt = (0.07,0.50)
!    visually needed (0.7,0.1)!!  but lots of astigmatism
if start==0
  ask 'Start image: 'start
end_if

if scidet==17
  aaa=511
  size=128
else_if scidet==32
  aaa=2047
  size=256
  size=64
end_if
if oned==0
 create 199 n=3*size sr=0-3*size/2 sc=0-3*size/2
else
 create 199 nr=7*size nc=size sr=0-11*size/2 sc=0-1*size/2
end_if
im=start
iix=0
if oned==1
  ix1=0 ix2=0 iy1=-5 iy2=1
else
  ix1=-1 ix2=1 iy1=-1 iy2=1
end_if
do ix=ix1,ix2
  iix=iix+1
  iiy=0
  do iy=Iy1,iy2
    iiy=iiy+1
    call read im
    if auto==0
      tv $ireadbuf
      printf 'Mark center of image with J key, then E to exit'
      mark new box=size/2
    else
      box 1 n=512 cr=1024 cc=1024
      abx $ireadbuf 1 high_row=r high_col=c
    end_if
    box 1 n=size sr=min[aaa-size,max[1,r-size/2]] |
                 sc=min[aaa-size,max[1,c-size/2]]
    window $ireadbuf box=1
    shift $ireadbuf dc=0-c dr=0-r
    shift $ireadbuf dc=ix*nr[ireadbuf] dr=iy*nc[ireadbuf]
    add 199 $ireadbuf
    string var 'xt%i1%i1' iix iiy
    {var}={ireadbuf:x_tilt}
    string var 'yt%i1%i1' iix iiy
    {var}={ireadbuf:y_tilt}
    im=im+1
  end_do
end_do
mn 199
tv 199
iix=0
do ix=ix1,ix2
  iix=iix+1
  iiy=0
  do iy=iy1,iy2
    iiy=iiy+1
    string var 'xt%i1%i1' iix iiy
    xtilt={var}
    string var 'yt%i1%i1' iix iiy
    ytilt={var}
    x=ix*size-3/8*size/2
    y=iy*size-7/8*size/2
    string foctext '(%f4.2,%f4.2) ' xtilt ytilt
    tvplot p=y,x text=foctext
  end_do
end_do
end
