parameter nowait fast

call time
call readstat 1
check=1
if fast==0
  $power1m off autofill
end_if
if filtno<0
  send 'filtinit'
  $sleep 2
  call readstat 1
  if filtno<0
    send 'filtinit'
  end_if
end_if

! Dummy placeholder routine for one time execution
call doit
printf 'end' >procedure/1m/doit.pro

!if dome is not opened, open it if possible
if domeopen==0
  curobj=0 check=0
  send 'guideoff'
  if nowait==0
    send 'xdome 75'
    call open 
  end_if
!else_if fast==0
!  $power1m off mirfan
end_if

!if dome is not initialized, init it
if domeopen==1&domeinit==0&daytest==0
  send 'di'
end_if

!if dome is not slaved, slave it
if domeopen==1&domeslav==0
  send 'dm'
end_if

! have we tracked to a soft limit?
if track==0
  printf 'Telescope tracking appears to be off! Limit hit?'
  curobj=0 check=0
  toaz=min[az,400] toaz=max[toaz,0]
  toalt=min[alt,85] toalt=max[toalt,20]
  call altaz toaz toalt
  send TR
end_if

! are we at a fill or stow position?
if home>0
  printf 'Telescope is at a home/stow/fill position'
  curobj=0 check=0
end_if

! is it morning?
if (localtim<12&localtim>ntwimorn+0.25&daytest==0)
  check=0
end_if

! turn off domefan after midnight
if (fast==0&localtim<12)
  $power1m off domefan
end_if
end
