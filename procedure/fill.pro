parameter filltime
if filltime==0
  filltime=4.5
end_if
$power1m on motors
$date >>../fill.log
$power1m on autofill
$sleep 5
!send 'ST'
send 'FILLRESET'
$date
string com 'FI %f8.2' filltime
send '{com}'
send 'FILLRESET'
printf '  %f8.2' filltime >>../fill.log
string comment 'Filled dewar: %f8.2 minutes' filltime 
call writesum 2 '{comment}'
$power1m off autofill
!call tempwait -70 1
!
!$power1m on autofill
!$sleep 5
!send 'FI 4'
!filltime=4
!printf '  %f8.2' filltime >>../fill.log
!string comment 'Filled dewar: %f8.2 minutes' filltime 
!call writesum 2 '{comment}'
$date >>../fill.log
call time
lastfill=localtim
$sleep 120

end
