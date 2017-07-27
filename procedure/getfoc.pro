parameter guessfoc inst param saotype coarse foctype

! Note inst==1 means science camera, inst==2 means guider, inst==-2 means adjust guider focus not telescope
!   with inst=1, param=0 means I filter, otherwise leave current filter
!   with inst=2, param=0 sends guider to center
!                param=1 sends guider to home
!                param=other leaves guider wherever it has been set
!   saotype=0 means USNO star
!          =1 means SAO star
!          =2 means SAOFOC star
!          <0 means use current position
!   foctype=0 means use minfoc for final focus
!          =1 means use focus for final focus
!          =2 means use highfoc for final focus
!          =3 means use axfoc for final focus
if inst==0
  inst=1
end_if
if inst==1|inst==3
  string instname 'science'
else
  string instname 'guider'
end_if
if coarse==0
  coarse=2
end_if
ghome=param curfilt=param
error goto doerr
call check 1
if check==0
  goto gfocend
end_if
if domeopen==0&daytest==0
  goto doerr
end_if

! procedure to move to a focus star, take focus run with guider, and determine
!  best focus!

if daytest==1
  maxiter=1
else
  if scidet==17
    maxiter=4
    maxcoar=2
  else
    maxiter=6
    maxcoar=3
  end_if
end_if
if inst==1
  foctime=4
else
  foctime=1
end_if
nfoc=7 dfoc0=20 dfocc=50
nfoc1=7 dfoc1=50
nfoc2=7 dfoc2=15
nfoc3=5 dfoc3=10
nfoc4=7 dfoc4=20
nfoc5=7 dfoc5=15

if guessfoc==0
  call readstat
  guessfoc=focref-(outtemp-tref)*18
end_if
string com 'fo %f12.1' guessfoc
send '{com}'
if daytest==1
  focerr=0
  goto gfocend
end_if

! Move guider stage appropriately and set correct instrument
if abs[inst]==2
  if ghome==0
    send 'guideloc 0'
  else_if ghome==1
    send 'guidehome'
  end_if
end_if
string com 'inst %i1' abs[inst]
send '{com}'

! Find a nearby focus star
if saotype==1
  call sao 0 5 9
else_if saotype==2
  saofoc
else_if saotype>=0
  call usnofoc
end_if

! get current file name and take exposure to move star to good location for guider
if abs[inst]==2
  send 'gccdinit'
  send 'g-disk'
  send 'gexp 0'
  send g+disk
  call readgccd
  sfoc=gincval
  send 'gexp 1'
  call readg sfoc
! Dont find stars around saturated pixels
  unmask
  clip $ireadbuf max=maxbad rad=50 maskonly
  zap $ireadbuf sig=0 size=3
!  if idet==33
!    box 10 n=500 cr=512 cc=512
!    abx $ireadbuf 10 high_row=hr high_col=hc mask
!    string com 'goffset %2f8.1' hc-512 hr-512
!    send '{com}'
!  end_if
else
  send +disk
  call readccd
  sfoc=incval
end_if

! Get temperature for output logging
call readstat

! Get estimated focus adjustment for altitude and rotator angle
call focadj

! Top of iteration loop
curfoc=guessfoc+df
niter=0
focrun:
niter=niter+1
if niter==1
  $date >>../getfoc.log
end_if

if niter>=maxiter+1
  printf 'Didnt converge on focus in %i1 iterations of focus run' maxiter
  printf '  Didnt converge on focus in %i1 iterations of focus run' maxiter |
      >>../getfoc.log
  focerr=1
  focus=guessfoc
  string com 'fo %f12.3' focus+df
  send '{com}'
  string comment 'Error determining {instname} focus'
  call writelog abs[inst] sfoc+nfoc-1 '{comment}'
  call writesum 1 '{comment}'
  goto gfocend
end_if

! do the focrun, with parameters depending on value of coarse
string var 'dfoc%i1' coarse
dfoc={var}
string var 'nfoc%i1' coarse
nfoc={var}
startfoc=curfoc-ifix[nfoc/2]*dfoc
if abs[inst]==1
  send +disk
  call readccd
  sfoc=incval
  if curfilt==0
    if photom==2
    send 'qfilt sdssi'
    else
    !send 'qfilt i'
    send 'qfilt sdssi'
    !send 'qfilt v'
    end_if
    !send 'guidehome'
  end_if
  focrun startfoc dfoc nfoc foctime
else_if abs[inst]==2
  send g+disk
  call readgccd
  sfoc=gincval
  gfocrun startfoc dfoc nfoc foctime
else
  send +disk
  call readccd
  sfoc=incval
  focrun startfoc dfoc nfoc foctime
end_if

string comment 'Start focus run'
call writelog abs[inst] sfoc '{comment}'

! get the focus value
if abs[inst]==1|abs[inst]==3
  idet=scidet guider=0
else
  idet=guidedet guider=1
end_if
call getdet idet
call focus guider sfoc sfoc+nfoc-1 2

! If focus determination failed, try again (clouds??)
if focerr==1
  printf 'Error determining focus' >>../getfoc.log
  printf 'Error determining focus' 
  if abs[inst]==1|abs[inst]==3
    string com 'newext %i' sfoc
  else
    string com 'gnewext %i' sfoc
  end_if
  send '{com}'
  goto focrun
end_if

call readstat 1
if inst==1|inst==3
call readccd
else
call readgccd
end_if
printf '  %i3 %f12.1  %f8.1 %i3 %i4 %i4 %4f12.3 %2f8.2 %2f8.2' |
   inst startfoc dfoc nfoc sfoc sfoc+nfoc-1 focus highfoc axfoc minfoc |
   fwhm fwmin*scale outtemp auxtemp >>../getfoc.log
$'rm' date.txt

! If best focus is at one end of focus run, redo focus run centered on best
h=highfoc s=startfoc
if high>0.12
  endtest=highfoc
else
  endtest=axfoc
end_if
if foctype==0
  endtest=minfoc
else_if foctype==1
  endtest=focus
else_if foctype==2
  endtest=highfoc
else_if foctype==3
  endtest=axfoc
end_if
if endtest<=s+1|endtest>=(s+(nfoc-1)*dfoc-1)
  curfoc=axfoc
  printf 'Redo focrun centered at %f12.3' curfoc
  printf 'Redo focrun centered at %f12.3 (minfoc: %f12.3)' curfoc minfoc >>../getfoc.log
  if abs[inst]==1|abs[inst]==3
    string com 'newext %i' sfoc
  else
    string com 'gnewext %i4' sfoc
  end_if
  goto focrun
end_if

! If were at coarse<maxcoarse, redo at next level of fineness
if coarse<maxcoar
  coarse=coarse+1
  curfoc=minfoc
  printf 'Do finer focrun centered at %f12.3' curfoc
  printf '  Do finer focrun centered at %f12.3' curfoc >>../getfoc.log
  if abs[inst]==1|abs[inst]==3
    string com 'newext %i' sfoc
  else
    string com 'gnewext %i4' sfoc
  end_if
  send '{com}'
  goto focrun
else
  if foctype==0
    newfoc=minfoc
  else_if foctype==1
    newfoc=focus
  else_if foctype==2
    newfoc=highfoc
  else_if foctype==3
    newfoc=axfoc
  end_if

  if inst>0
    string com 'fo %f12.3' newfoc
    send '{com}'
    printf 'Focus (PI/GUIDEHOME) has been set to %f12.3'  newfoc
    printf '  Set focus to %f12.3'  newfoc >>../getfoc.log
  else
    string com 'fo %f12.3' guessfoc+df
    send '{com}'
    string com 'guidefoc %f12.3' guessfoc+df-newfoc
    send '{com}'
    printf 'Guider focus (PI/GUIDEHOME) has been changed by %f12.3'  guessfoc+df-newfoc
    printf '  Changed guider focus to %f12.3'  guessfoc+df-newfoc >>../getfoc.log
  end_if
  string comment |
'Good focus run, niter: %i3 focus:%f12.3, fwhm fit/min:%f8.2/%f8.2, out:%f8.2 aux:%f8.2' |
niter newfoc fwhm fwmin*scale outtemp auxtemp
  call writelog abs[inst] sfoc+nfoc-1 '{comment}'

  call getdate
  call ireadbuf sfoc
  string filt '{ireadbuf:filtname}'
  printf ' {date} %i2 %f10.1 %f6.1 %i2 %i4 %i4 %4f11.3 %2f7.2 %i1 %2f7.2 %3f8.1 {filt}' |
     inst startfoc dfoc nfoc sfoc sfoc+nfoc-1 focus highfoc axfoc minfoc |
     fwhm fwmin*scale niter outtemp auxtemp az alt rot |
     >>../getfoc.sum

  string comment |
'Good {instname} focus run, niter: %i3 focus:%f12.3, fwhm fit/min:%f8.2/%f8.2, out:%f8.2 aux:%f8.2 filter: {filt} alt: %f8.1 ' |
niter newfoc fwhm fwmin*scale outtemp auxtemp alt
  call writesum 0 '{comment}'

  if inst>0
    focus=newfoc-df
  else
    focus=guessfoc
  end_if
!  if inst==1
!    string com 'exp %i' foctime
!    send '{com}'
!    string comment 'Test focus exposures...'
!    call writelog abs[inst] sfoc+nfoc '{comment}'
!  end_if
end_if

goto gfocend
doerr:
focerr=1
printf 'Error in getfoc'

gfocend:

! Move guider back out
if inst==2&param<2
  send guidehome
end_if

if inst==3
send 'inst 3'
else
send 'inst 1'
end_if
$date >>../getfoc.log

dispose all

end
