parameter alra aldec alpa

alert=0
if alra>0 
  alert=1
  goto havera
end_if

error goto noalert
open inalert /home/tcomm/alert.dat
stat n=count[inalert]

nalert:
read inalert
alert=alert+1
alra=@inalert.1 
alra=alra/15.
aldec=@inalert.2 
almjd=@inalert.3+@inalert.4/(3600*24)
alpa=0
close inalert

havera:
$date
string command 'CO %2f12.6 %f10.2' alra aldec alpa
printf 'echo {command} >>master2com' >./obj.inp
printf 'echo "Y" >>master2com ' >>./obj.inp
!printf 'echo {command} >>master2com'
!printf 'echo "Y" >>master2com '
$csh obj.inp
call get
status={return.2}
type status

send 'setfilt i'
$date
do i=1,1
  printf 'exp 60'
!  $sleep 60
end_do

noalert:
end
