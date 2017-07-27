send 'settemp -130'
call readstat 
string com 'gsettemp %i' outtemp
send '{com}'

!$power1m off apogee
! sleep a few minutes just in case dome is still closing?
$sleep 180
$power1m off motors
$power1m off domefan
$power1m on mirfan
$power1m off autofill
!$echo "QU" >> master2com
!$echo "y" >> master2com
!$echo "y" >> master2com
!call get


end
