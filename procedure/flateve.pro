parameter sfilt
sfilt=1
if sfilt==0
  sfilt=1
end_if
call check
call altaz 90 70
call readccd

send 'guidehome'
send 'window full'
!send 'guideloc 1200'
!send 'guideloc 1500'
!send 'guideloc 900'

call ordflat

if nfilt>5
  ntarg=3
else
  ntarg=5
end_if
send '+display'
do ifilt=sfilt,nfilt
  string var 'filt%i2.2' ifilt
  string var 'flat%i2.2' ifilt
  string filt {{var}}
  nflat=incval
  send 'setfilt {filt}'
  if ifilt==sfilt
    string com 'flat %i3 10 1.0 1.1' ntarg
  else
    string com 'flat %i3 10 0 1.1' ntarg
  end_if
  send '{com}'
  call readccd
  printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
  printf 'error continue' >>../images/{root}/flat.pro
  printf 'wd 201 {root}eve{filt} full' >>../images/{root}/flat.pro
  printf '%2i4 {root}eve{filt}' nflat incval-1 >>../images/{root}/flat.{filt}
  string comment 'Done flat fields, {filt} filter'
  call writelog 1 incval-1 '{comment}'
  call writesum 0 '{comment}'


end_do

!nflat=incval
!send 'setfilt 656n'
!send 'flat 3 10 1.0 1.1'
!call readccd
!printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}eveb' >>../images/{root}/flat.pro

!nflat=incval
!send 'setfilt B'
!send 'flat 3 10 1.0 1.1'
!call readccd
!printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}eveb' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt C '
!send 'flat 3 5 0 1.1'
!call readccd
!printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}evec' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt V '
!send 'flat 3 5 0 1.1'
!call readccd
!printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}evev' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt R '
!send 'flat 3 5 0 1.1'
!call readccd
!printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}ever' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt I'
!send 'flat 3 5 0 1.1'
!call readccd
!printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}evei' >>../images/{root}/flat.pro
!
!nflat=incval
!send 'setfilt Z'
!send 'flat 3 5 0 1.1'
!call readccd
!printf 'call doflat %i3 %i3 201 -1' nflat incval-1 >>../images/{root}/flat.pro
!printf 'wd 201 {root}evez' >>../images/{root}/flat.pro
!
end
