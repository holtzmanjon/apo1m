! Are we already open?
call readstat 1
if daytest==1
  domeopen=1
  send '-35m'
  send 'clear'
end_if
if domeopen==1
  goto openend
end_if

call writesum 2 '1m dome is closed'

! Check to make sure 3.5m is open
iter=1
start:
call time
if localtim<12&localtim>(sunrise-0.5)
  goto openend
end_if
$date
if iter>1
  ! Turn off motor power while we are waiting
  if iter==2&preopen==0
    $power1m off motors
  end_if
  $sleep 30
  call time
  ! If it is after midnight, see if we need to fill dewar while waiting
  call ccdtemp
  ! Dummy placeholder routine for one time execution
  call doit
  printf 'end' >procedure/1m/doit.pro
end_if
iter=min[999,iter+1]
if no35m==1
  goto open
end_if

err goto start
open shut35 /home/export/tocc/35mopen.dat
close shut35
printf '3.5m is open!'

! Check to make sure 3.5m is not closed
err goto open
! use following line to prevent reopening at all
!goto start
open shut35 /home/export/tocc/35mclose.dat
close shut35
printf '3.5m is closed!'
goto start

open:

! Make sure that the sun is not up, just in case
call time
if localtim<12&localtim>ntwimorn+0.5
  goto openend
end_if
if no35m==1
  early=sunset+0.1
else
  early=sunset-0.5
end_if
if localtim>12&localtim<early
  goto start
end_if

! clear shutdown flags in case there is a delay in TOCC reading 3.5m open
! temporarily turn off 3.5m slaving for a dely
send 'clear'
send '-35m'
$power1m on motors
$power1m on rackfan

printf 'Opening louvers ...'
$echo "OL" >>master2com
$echo "Y" >>master2com
call get

$sleep 5
$power1m on domefan
$power1m on motors
$power1m on rackfan

!printf 'Closing mirror covers (just in case)...'
!$echo "CM" >>master2com
!call get

call readstat 1
printf 'Sending telescsope to low position...'
call altaz 350 35
!send ST

printf 'Moving dome to 160 and turn on lights...'
if domeinit==0
  send 'DI'
end_if
$echo "OD" >>master2com
$echo "Y" >>master2com
printf 'Turning off lights ...'
$power1m off lights
printf 'Opening dome...'
!$echo "Y" >>master2com
call get

if ldome==1
  printf 'Opening lower dome slit'
  send 'OLD'
end_if

!call altaz 350 75
!send ST

!printf 'Opening mirror covers...'
!$echo "OM" >>master2com
!call get

call  writesum 2 'Opened 1m dome'
! wait a couple of minutes before slaving to 3.5m to make sure we 
!   get dome open broadcasts
if no35m==0
  send 'stup'
  $sleep 120
end_if

openend:
if no35m==0
  send '+35m'
end_if
call readstat 1
$power1m off mirfan
end

