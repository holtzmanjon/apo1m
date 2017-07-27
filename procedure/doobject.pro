parameter string=objfile xlow xhigh rastart ndesired bexp vexp rexp iexp zexp nexp nousno
ndone=0
err goto doerr
call check
if domeopen==0
  goto doerr
end_if

printf 'echo OF >>master2com' >./com.inp
printf 'echo {objfile} >>master2com ' >>./com.inp
printf 'Sending  commands:'
$cat com.inp
$csh com.inp
call get

open test ../scripts/{objfile}
stat n=count[test]
if ndesired<0
  i1=abs[ndesired]
  i2=abs[ndesired]
  npass=1
  ndesired=1
else
  if rastart<0
    ! next two lines are for old way of just going through file in order
    i1=1 i2=n
    npass=1
  else
  ! This assumes coordinate file is sorted in RA!!!
  ! Give first priority to objects starting at rastart
    npass=2
    do i=1,n
      printf '{test.2}' >./ra.inp
      $sed 's/:/ /g' ./ra.inp > ./ra.out
      open rafile ./ra.out
      read rafile
      rah=@rafile.1+@rafile.2/60+@rafile.3/3600
      close rafile
      if rah<rastart
        iend=i
      end_if
    end_do
    !limits for second pass
    i1=iend+1 i2=n
  end_if

end_if
close test

do ipass=1,npass
 open test ../scripts/{objfile}
 if ipass==2
   i1=1
   i2=iend
 end_if
 do i=1,n
  string name {test}
  if i>=i1&i<=i2
   call airmass ../scripts/{objfile} i
   if airmass>xlow&airmass<xhigh
     if bexp==-1
       bexp=@test.7 
     end_if
     if vexp==-1
       vexp=@test.8 
     end_if
     if rexp==-1
       rexp=@test.9 
     end_if
     if iexp==-1
       iexp=@test.10 
     end_if
     if zexp==-1
       zexp=@test.11 
     end_if
     if nexp==-1
       nexp=@test.12
     end_if
     if nousno==1
       call object {objfile} i 0 1 bexp vexp rexp iexp zexp nexp 0
     else
       call object {name} -1 -1 1 bexp vexp rexp iexp zexp nexp 0
     end_if
     ndone=ndone+1
   end_if
   if ndone==ndesired
     goto doneobj
   end_if
   call time
   if localtim<12&localtim>ntwimorn
     goto doneobj
   end_if
  end_if
 end_do
 close test
end_do

goto doneobj
doerr:
printf 'Error in doobject'

doneobj:
END
