parameter linst start del nfoc time 

if linst==1
!  send '-display'
!  send 'fast'

!  send 'window 192 320 192 320'
!  send 'window full'
  if scidet==32
!    send 'window 769 1280 769 1280'
    send 'window 750 1450 650 1350'
   continue
  else_if scidet==17
    !send 'window 192 320 192 320'
    send 'window 156 356 156 356'
  end_if
  $echo "focrun" >> master2com
else_if linst==2
  if guidedet==33
!    send 'gwindow 512 1024 400 1024'
    send 'gwindow 300  800 200  800'
  end_if
  $echo "gfocrun" >> master2com
else_if linst==-2
  string com 'gfo %i5' start-del
  send '{com}'
  do ifoc=1,nfoc
    string com 'gfo %i5' start+(ifoc-1)*del
    send '{com}'
    string com 'gexp %f5.1' time
    send '{com}'
  end_do  
else
  printf 'first argument must be 1 or 2'
  pause
end_if
if linst>0
  string com '%f12.3' start
  $echo "{com}" >> master2com
  string com '%f12.3' del
  $echo "{com}" >> master2com
  string com '%f12.3' nfoc
  $echo "{com}" >> master2com
  string com '%f12.3' time
  $echo "{com}" >> master2com

  call get
end_if

if linst==1
!  send '+display'
!  send 'nofast'
  send 'window full'
else
  send 'gwindow full'
end_if

end

