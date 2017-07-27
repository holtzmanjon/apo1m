parameter i ibox guider nmin

error goto finderr
if nmin==0
  nmin=1
end_if

! Only look in box if specified
unmask
if ibox>0
  clip $i max=-1000 box=1 maskonly
  masktoim 201 nr=nr[i] nc=nc[i] sr=sr[i] sc=sc[i]
  unmask
  clip 201 max=0.5 maskonly
end_if

! Dont find stars around saturated pixels
clip $i max=maxbad rad=50 maskonly

! zap out bad pixels
zap $i sig=0 size=3

! Find stars using DAOPHOT FIND
daofiles coo=./junk
opt fw=3/scale lr=-1.0 hr=1.0 hs=1.0 ls=0.0 wa=0 ma=50000
if guider==1
  thresh=300 hmin0=30
  ! put this in in case we have all zeros from a bad bias level
  !photons $i mean=0 rn=2
else
  thresh=100 hmin0=30
  thresh=50 hmin0=15
  if scidet==17 
   scale=1.6
   opt fw=3/scale 
  end_if
end_if
find 201 $i thresh=thresh lowbad=-50 nwant=nmin,5 hmin=hmin nfound=nfound
type hmin
if hmin<hmin0
  $'rm' junk2.coo
  $head -3 junk.coo >junk2.coo
  $mv junk2.coo junk.coo
end_if
get coo=./junk sr=sr[i]-1 sc=sc[i]-1

! Associate proper ID numbers??
!automark $i auto id=ii-start+1

!      unmask
!      clip $i max=maxbad rad=50 maskonly
!      abx $i 1 high_row=hr high_col=hc mask
!      box 11 n=3 cr=hr cc=hc
!
!!      daosky $i
!!      daofiles coo=./focus
!!      opt fw=3/scale
!!      find 201 $i thresh=5*skysig lowbad=-50
!!      daofiles file=./focus.coo file2=./focus2.coo
!!      sort index=4 norenum
!!      $'rm' focus.coo
!!      $head -4 focus2.coo >./focus.coo
!!      get coo=./focus sr=0 sc=0
!
!      automark $i range=0,100000 id=ii-start+1 box=11 new
goto findend
finderr:
printf 'Error in findstar.pro!'
pause

findend:
end

