parameter inst noslew havefilt nogrid

!inst=1
citer=1
!goto doerr
error goto doerr
call check
if domeopen==0&daytest==0
  goto doerr
end_if

$date >>../coord.log
call readstat 1
if alt>80
  printf 'Reset coordinates: altitude too high (%f8.2)' alt
  printf 'Reset coordinates: altitude too high (%f8.2)' alt >>../coord.log
  goto cend
end_if

if inst==1
 send 'inst 1'
 if havefilt==0
   send 'qfilt sdssg'
 end_if
 idet=scidet
 if scidet==17
   off=500
 else_if scidet==32
   off=1800
   send 'window full'
 end_if
else_if inst==2
 send 'inst 2'
 idet=guidedet
 off=500
else_if inst==3
 send 'inst 3'
 idet=scidet
 off=500
end_if
call getdet idet
if noslew==0
  call sao 0 4 6.5
end_if

if nogrid>0
  npos=1
else
  npos=9
end_If
dx1=0 dy1=0
dx2=0 dy2=off
dx3=0 dy3=-off
dx4=-off dy4=0
dx5=off dy5=0
dx6=off dy6=off
dx7=off dy7=-off
dx8=-off dy8=off
dx9=-off dy9=-off

cagain:

if inst==1|inst==3
  send +disk
  call readccd
  sfoc=incval
  iwrite=1
else
  send 'g-disk'
  send 'gexp 0'
  send g+disk
  call readgccd
  sfoc=gincval
  iwrite=0
end_if

dx=0 dy=0 dxtot=0 dytot=0
do ipos=1,npos
  string dx 'dx%i1' ipos
  string dy 'dy%i1' ipos
  dx={dx}-dxtot dy={dy}-dytot
  if (dx~=0|dy~=0)
    string com 'offset %f8.1 %f8.1' dx dy
    send '{com}'
    send 'sleep 3'
    $sleep 5
  end_if
  dxtot=dxtot+dx dytot=dytot+dy

  if inst==1|inst==3
    string com 'newext %i' sfoc
    send '{com}'
    send 'exp 0.5'
    call read sfoc
  else
    string com 'gnewext %i' sfoc
    send '{com}'
    send 'gexp 0.5'
    call readg sfoc
  end_if

  call ireadbuf sfoc

  zap $ireadbuf sig=0 size=3
  if inst==1|inst==3
    if idet==17
      nmax=500 cr=256 cc=256
      box 10 n=500 cr=256 cc=256
    else_if idet==32
      nmax=2000 cr=1024 cc=1024
      box 10 n=2000 cr=1024 cc=1024
    else
      nmax=1000 cr=512 cc=512
      box 10 n=1000 cr=512 cc=512
    end_if
    thresh=2000
  thresh=3000
  else
    if idet==12
      nmax=500 cr=256 cc=256
      box 10 n=500 cr=256 cc=256
    else_if idet==33
      nmax=1000 cr=512 cc=512
      box 10 n=1000 cr=512 cc=512
    else
      nmax=160 cr=165/2 cc=192/2
      box 10 n=160 cr=165/2 cc=192/2
    end_if
    thresh=500
  end_if

  ibox=3
  newbox:
  if ibox==1
    box 10 n=nmax/4 cr=cr cc=cc
  else_if ibox==2
    box 10 n=nmax/2 cr=cr cc=cc
  else_if ibox==3
    box 10 n=nmax cr=cr cc=cc
  end_if
  abx $ireadbuf 10 high_row=hr high_col=hc high=high
  printf 'Found high pixel with value %f8.1 at (%i4,%i4)' high hc hr
  if high<thresh&ibox<3
    ibox=ibox+1
    goto newbox
  end_if
  
  printf '  inst: %i3  high pixel: %f8.1 at (%i4,%i4)' inst high hc hr |
         >>../coord.log
  
  crow=nr[ireadbuf]/2 ccol=nc[ireadbuf]/2
  if high>thresh
    xoff=hc-ccol yoff=hr-crow
    if inst==1|inst==3
      string com 'offset %2f8.1' xoff yoff
    else
      if inst<0
        drot=yoff/30
        string com 'ur %f8.1' drot
      else
        string com 'goffset %2f8.1' xoff yoff
      end_if
    end_if
    send '{com}'
    $sleep 5
  
    if inst<0
      printf '  Updated rotator' >>../coord.log
      printf '  Coordinates have been update. To undo, you need to reinitialize'
      
      string comment 'Reset rotator, UR: %2f8.1' drot yoff
      call writelog 0 sfoc '{comment}'
      call writesum 0 '{comment}'
    else
      $echo "uc" >> master2com
      $echo "1" >>master2com
      call get
      printf '  Updated coordinates' >>../coord.log
      printf '  Coordinates have been update. To undo, you need to reinitialize'
    
      string comment 'Reset coordinates, offset: %i4 %i4' xoff yoff
      call writelog iwrite sfoc '{comment}'
      call writesum 0 '{comment}'
    end_if
    goto cend
!  else_if citer==1
!    printf 'Failure to find bright star'
!    string comment 'FAILED: Reset coordinates'
!    call writelog iwrite sfoc '{comment}'
!    call writesum 1 '{comment}<font color=red>'
!    citer=2
!  !  send 'DI'
!  !  $sleep 50
!  !  goto cagain
  else_if ipos==npos
    printf 'Failure to find bright star'
    string comment 'FAILED: Reset coordinates'
    call writelog iwrite sfoc '{comment}'
  end_if
end_do

doerr:
printf 'Error in coord'

cend:
if inst==3 
  send 'inst 3'
else
  send 'inst 1'
end_if
send '+disk'
send 'g+disk'

end
