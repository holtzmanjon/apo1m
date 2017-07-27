parameter string=wtime dtime
init=1
start:
if init==0
  $sleep 30
  ! Dummy placeholder routine for one time execution
  call doit
  printf 'end' >procedure/1m/doit.pro
end_if
init=0
call time
if localtim<{wtime}+dtime
  goto start
end_if

end

