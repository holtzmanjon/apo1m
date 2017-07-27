parameter string=objfile amlow amhigh ndesired 
error goto doerr
call check
if domeopen==0&daytest==0
  goto doerr
end_if

printf 'echo OS >>master2com' >./com.inp
printf 'echo {objfile} >>master2com ' >>./com.inp
printf 'Sending  commands:'
$cat com.inp
$csh com.inp
call get

! reset position angle to 0 (now done automatically as part of RS)
! send 'pa 0'

! Set filters to do
nsfilt=nfilt
if photom==1
  ifilt1=1 ifilt2=5
else_if photom==2
  ifilt1=6 ifilt2=10
end_if

! move filter to last filter in set so that focus will
!  be appropriate at end (STAN command previously didnt adjust focus)
!string var 'filt%i2.2' nsfilt
!string filt {{var}}
!send 'setfilt {filt}'

open test ../scripts/{objfile}
stat n=count[test]
ndone=0
do i=1,n
  string name '{test}'
  call airmass ../scripts/{objfile} i
  if airmass>amlow&airmass<amhigh
    call rs i
    call writesum 3 'OBSERVE: {name}'
    string com '(NR==%i)' i
    $'rm' id.stn
    $awk -F'\t' '{com} \{print $12\}' ../scripts/{objfile} >./id.stn
    open stn ./id.stn
    read stn
    jstan=@stn.1
    close stn
    do ifilt=ifilt1,ifilt2
      string var 'filt%i2.2' ifilt
      string filt {{var}}
      call readccd
      send 'stan {filt}'
      printf '%i4 %i4' incval jstan >>../images/{root}/{root}.{filt}
!      call readccd
!      send 'stan {filt}'
!      printf '%i4 %i4' incval i >>../images/{root}/{root}.{filt}
    end_do

    printf 'STANDARD %i5 %i5' i jstan
    printf '{name}'
    if jstan<0
      ndone=ndone+2
    else
      ndone=ndone+1
    end_if
  end_if
  if ndone>=ndesired
    goto doneobj
  end_if
end_do

doneobj:
close test
goto doend

doerr:
printf 'Error in dostan'
doend:
END
