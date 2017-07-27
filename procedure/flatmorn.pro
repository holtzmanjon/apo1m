parameter sfilt

if sfilt==0
  sfilt=nfilt
end_if

call altaz 270 70
call readccd

send 'guidehome'
send 'window full'
!send 'guideloc 1200'
!send 'guideloc 1500'

if nfilt>5
  ntarg=3
else
  ntarg=5
end_if
!ntarg=20
call ordflat
send '+display'
send '-35m'
do ifilt=sfilt,1
 call readstat 1
 if domeopen==1
!  string var 'filt%i2.2' ifilt
  string var 'flat%i2.2' ifilt
  string filt {{var}}
  nflat=incval
  send 'setfilt {filt}'
  if ifilt==sfilt
    string com 'flat %i3 10 1.0 0.9' ntarg
  else
    string com 'flat %i3 10 0 0.9' ntarg
  end_if
  send '{com}'
  call readccd
  printf 'call doflat %i4 %i4 201 -1' nflat incval-1 >>../images/{root}/flat.pro
  printf 'error continue' >>../images/{root}/flat.pro
  printf 'wd 201 {root}morn{filt} full' >>../images/{root}/flat.pro
  printf '%2i4 {root}morn{filt}' nflat incval-1 >>../images/{root}/flat.{filt}
  string comment 'Done flat fields, {filt} filter'
  call writelog 1 incval-1 '{comment}'
  call writesum 0 '{comment}'

 end_if
end_do
send '+35m'

!nflat=incval
!send 'setfilt Z'
!send 'flat 3 5 1 0.9'
!call readccd
!printf 'call doflat %i4 %i4 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}mornz' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt I'
!send 'flat 3 5 0 0.9'
!call readccd
!printf 'call doflat %i4 %i4 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}morni' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt R '
!send 'flat 3 5 0 0.9'
!call readccd
!printf 'call doflat %i4 %i4 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}mornr' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt V '
!send 'flat 3 5 0 0.9'
!call readccd
!printf 'call doflat %i4 %i4 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}mornv' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt B'
!send 'flat 3 10 0 0.9'
!call readccd
!printf 'call doflat %i4 %i4 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}mornb' >>../images/{root}/flat.pro
!
end
