! set things up command-wise
call startup
send -display
send g-display

send 'settemp -50'
! get some darks
call biasdark 0 50 sbiasmorn
call biasdark 1 5 dark001
call biasdark 5 5 dark005
call biasdark 30 5 dark030
call biasdark 60 5 dark060
call biasdark 300 5 dark300
! 300s darks at a variety of temperatures
call readstat
if outtemp<10
  send 'settemp -45'
  $sleep 300
  call biasdark 300 5 dark300_45
end_if
call readstat
if outtemp<15
  send 'settemp -40'
  $sleep 300
  call biasdark 300 5 dark300_40
end_if
call readstat
if outtemp<20
  send 'settemp -35'
  $sleep 300
  call biasdark 300 5 dark300_35
end_if
call readstat
if outtemp<25
  send 'settemp -30'
  $sleep 300
  call biasdark 300 5 dark300_30
end_if

end
