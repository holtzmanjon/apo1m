parameter doff guider dopause itime ngrid2
printf 'use spiral.pro!?'
pause

if ngrid2==0
  ngrid2=1
end_if
if itime==0
  itime=5
end_if
if doff==0
  doff=400
end_if
if guider==0
  string prefix ' '
  string offset 'offset '
else
  string prefix 'g'
  string offset 'guideinst 2'
end_if
x0=0 y0=0
do i=-ngrid2,ngrid2
  y=i*doff
  do j=-ngrid2,ngrid2
!    call check 1
!    if check==0
!      goto serr
!    end_if
    x=j*doff
    dx=x-x0
    dy=y-y0
    limit:
    if (dx>5000)
      string com '{offset} 5000 0' 
      send '{com}'
      dx=dx-5000
      goto limit
    end_if
    if (dy>5000)
      string com '{offset} 0 5000' 
      send '{com}'
      dy=dy-5000
      goto limit
    end_if
    if (dx<-5000)
      string com '{offset} -5000 0' 
      send '{com}'
      dx=dx+5000
      goto limit
    end_if
    if (dy<-5000)
      string com '{offset} 0 -5000' 
      send '{com}'
      dy=dy+5000
      goto limit
    end_if
 
    string com '{offset} %i5 %i5' dx dy
    send '{com}'
    if dopause==0
      send 'sleep 3'
      string com '{prefix}exp %i' itime
      send '{com}'
pause
    else_if dopause<0
      pause
    else
      string com 'sleep %i' dopause
      send '{com}'
      string com '{prefix}exp %i' itime
      send '{com}'
    end_if
    x0=x
    y0=y
  end_do
end_do
serr:
x=0 y=0
dx=x-x0
dy=y-y0
string com '{offset} %i5 %i5' dx dy
send '{com}'
end

