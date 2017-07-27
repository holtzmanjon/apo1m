! Are we already open?
call readstat 1

$power1m on motors
send GUIDEOFF
!send CM
if ldomopen==1|domeopen==1
  send '-35m'
  send 'clear'
  call altaz az 25
end_if
if ldomopen==1
  send CLD
  $sleep 20
end_if

$power1m off domefan
if domeopen==1
  send XCD
end_if
!send '-35m'
!send 'clear'
!send STUP
send ST
send '+35m'
send 'close'
closeend:

send CL
$power1m on mirfan
send 'XDOME 25'

call  writesum 2 'Closed 1m dome'

dend:
end
