parameter force
call readccd
call time
dtfill=localtim-lastfill
if dtfill<0
  dtfill=dtfill+24
end_if
if localtim<12&dtfill>6
 if ccdtemp>100
  call initcced
 else_if ccdtemp>-113|force==1
  $date >>../fill.log
  $power1m on motors
  $sleep 3
  send 'FI 5'
  filltime=5
  printf '  %i  %f8.2' filltime ccdtemp >>../fill.log
  string comment 'Filled dewar: %i minutes  temp: %f8.2' filltime ccdtemp
  call writesum 2 '{comment}'
  send 'FILLRESET'
  nfill=nfill+1
  $power1m off autofill
  $sleep 120
  lastfill=localtim
  call tempwait -90 1
  curobj=0
 end_if
end_if
end
