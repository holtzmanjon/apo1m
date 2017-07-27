parameter guider i1 i2 ibox cfrac

if cfrac==0
  cfrac=2
end_if
  
if i2==0
  i2=i1
end_if

do icom=i1,i2

  ! Read in image and copy to work buffer
  if guider==1
    call getdet 12
    call readg icom
  else
    call getdet 10
    call getdet 17
    call read icom
  end_if
  copy 101 $ireadbuf

  ! If box specified, only use that area - if no box, bin 2x2 
  if ibox==0
   !  window 101  box=1
    bin 101 bin=2 norm
  else
    window 101 box=ibox
  end_if

  ! Get sky sigma, and quantize to frac*skysig
  if cfrac>0
    daosky 101 
    div 101 c=cfrac*skysig
    nint 101
    mul 101 c=cfrac*skysig
    fits 101 float=skycompr cfrac*skysig
  end_if

  ! Write it out, and gzip
  string out '{root}c.%i3.3' icom
  wd 101 ./{out}.fits
  $gzip ./{out}.fits
end_do
end
