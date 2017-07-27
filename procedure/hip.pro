parameter itype p1 p2 p3 p4

! itype = 0 --> HIP T
! itype = 1 --> HIP A
! itype = 2 --> SAFOC T
! itype = 3 --> SAFOC A
! itype = 4 --> HIP O

if itype==1|itype==3|itype==4
  az=p1 alt=p2 m1=p3 m2=p4
  if alt==0
    printf 'Error: alt must be >0'
    pause
  end_if
else_if itype==0|itype==2
  m1=p1 m2=p2
else
  printf 'Error: first argument must be 0, 1, 2, or 3'
  pause
end_if

if m1==0
  m2=20
end_if
if m2==0
  m2=20
end_if

printf 'Sending  commands:'
if itype==2|itype==3
  printf 'echo SAFOC >>master2com' >./com.inp
  printf 'echo SAFOC >>master2com' 
else
  printf 'echo HIP >>master2com' >./com.inp
  printf 'echo HIP >>master2com' 
end_if
if itype==1|itype==3
  printf 'echo A >>master2com' >>./com.inp
  printf 'echo "%f8.1\r" >> master2com ' az >>./com.inp
  printf 'echo "%f8.1\r" >> master2com ' alt >>./com.inp
  printf 'echo A >>master2com' 
  printf 'echo "%f8.1\r" >> master2com ' az 
  printf 'echo "%f8.1\r" >> master2com ' alt 
else_if itype==4
  printf 'echo O >>master2com' >>./com.inp
  printf 'echo "%f8.1\r" >> master2com ' az >>./com.inp
  printf 'echo "%f8.1\r" >> master2com ' alt >>./com.inp
  printf 'echo O >>master2com' 
  printf 'echo "%f8.1\r" >> master2com ' az 
  printf 'echo "%f8.1\r" >> master2com ' alt 
else
  printf 'echo T >>master2com' >>./com.inp
  printf 'echo T >>master2com' 
end_if
printf 'echo "%2f8.1\r" >> master2com ' m1 m2 >>./com.inp
printf 'echo "Y" >>master2com ' >>./com.inp
printf 'echo "%2f8.1\r" >> master2com ' m1 m2 
printf 'echo "Y" >>master2com ' 

$csh com.inp

call get
end
