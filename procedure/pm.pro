parameter string=pmfile recenter pmfast guider hamax close

printf 'DO YOU WANT TO DO A CAZ FIRST?'
bell r

!$'rm' ../images/{root}/{root}{pmfile}.out
!$'rm' ../images/{root}/{root}pmtest.out

itime=2

if guider==1
  jdet=guidedet
  string exp 'gexp'
  string readccd 'readgccd'
  string read 'readg'
  string incval 'gincval'
  string newext 'gnewext'
  send 'inst 0'
else
  jdet=scidet
  string exp 'exp'
  string readccd 'readccd'
  string read 'read'
  string incval 'incval'
  string newext 'newext'
  send 'inst 1'
end_if
printf '#' >./pm.out
if recenter<=0
  npos=1
else
  npos=recenter
  idet=jdet
  call getdet idet
end_if
if hamax==0
  nrep=1
else
  nrep=1000
end_if

!Rotator center here
if jdet==32
  rcen=1024 ccen=1100 linst=1
  thresh=10000 vmin=4 vmax=7
else_if jdet==33
  rcen=512 ccen=512 linst=2
  rcen=250 ccen=475 linst=2
  thresh=10000 vmin=4 vmax=7
else_if jdet==17
  rcen=255 ccen=465 linst=3
  rcen=255 ccen=255 linst=3
  thresh=10000 vmin=5 vmax=7
else
  printf 'Unknown jdet'
  pause
end_if

if daytest==0
!  call caz
end_if

call {readccd}
string comment 'Start pointing model: {pmfile}'
call writelog 1 {incval} '{comment}'

send 'setfilt b'
send 'pa 0'

!send 'ysep'
!if daytest==1
!  n=10
!end_if
!send 'inst 0'
if jdet==17
  off=450
else_if jdet==32
  off=1800
else_if jdet==33
  off=1000
end_if
dx1=0 dy1=0
dx2=0 dy2=off
dx3=0 dy3=-off
dx4=-off dy4=0
dx5=off dy5=0
dx6=off dy6=off
dx7=off dy7=-off
dx8=-off dy8=off
dx9=-off dy9=-off

dx10=-(2*off) dy10=-(2*off)
dx11=-(2*off) dy11=-off
dx12=-(2*off) dy12=0
dx13=-(2*off) dy13=off
dx14=-(2*off) dy14=(2*off)
dx15=-off dy15=(2*off)
dx16=0 dy16=(2*off)
dx17=off dy17=(2*off)
dx18=(2*off) dy18=(2*off)
dx19=(2*off) dy19=off
dx20=(2*off) dy20=0
dx21=(2*off) dy21=-off
dx22=(2*off) dy22=-(2*off)
dx23=off dy23=-(2*off)
dx24=0 dy24=-(2*off)
dx25=-off dy25=-(2*off)

do irep=1,nrep

open pminput ./{pmfile}.dat
stat nstars=count[pminput]
type nstars

ipm=1
do i=1,nstars
 read pminput
 nrepeat=1
 az=@pminput.1 alt=@pminput.2
 az=az+ran[-1,1]
 if az<0
   az=az+360
 else_if az>360
   az=az-360
 end_if
 error continue
! nrepeat=@pminput.3
 if (irep>1)
   string var 'pra%i3.3' i
   ratarg={var}
   string var 'pdec%i3.3' i
   dectarg={var}
   call hipa 4 ratarg dectarg vmin vmax
 else
   hipa az alt vmin vmax
   call readstat
   string var 'pra%i3.3' i
   {var}=ratarg
   string var 'pdec%i3.3' i
   {var}=dectarg
 end_if

 send 'sleep 8'
 call readstat 1
 !printf '%2f12.6 %f8.1 {utc} %3f12.6' ratarg dectarg epoch lst az alt |
 !            >>../images/{root}/{root}pmtest.out

! if mod[az,360]>50&mod[az,360]<70
!   send 'DI'
!   $sleep 30
! end_if
 do irepeat=1,nrepeat
  if daytest==0

    call check
    if check==0
      goto pmend
    end_if
    ! Is it time to stop?
    call time
    if (localtim<12&localtim>ntwimorn+0.35&daytest==0)
      goto pmend
    end_if

    dx=0 dy=0 dxtot=0 dytot=0
    if irepeat==1
      nnn=npos
    else
      nnn=1
    end_if 
    do ipos=1,nnn
     string dx 'dx%i1' ipos
     string dy 'dy%i1' ipos
     dx={dx}-dxtot dy={dy}-dytot
     if (dx~=0|dy~=0)
       string com 'guideinst %i3 %f8.1 %f8.1' linst dx dy
       send '{com}'
       send 'sleep 10'
     end_if
     dxtot=dxtot+dx dytot=dytot+dy

     call {readccd}
     string com '{exp} %f8.1' itime
     send '{com}'

     if abs[recenter]>0
      call {read} {incval}
      if ipos==1
        string ratarg '{ireadbuf:ra}'
        string dectarg '{ireadbuf:dec}'
        string aztarg '{ireadbuf:az}'
        string alttarg '{ireadbuf:alt}'
      end_if
      if jdet==17
        box 1 n=500 cr=256 cc=256
      else_if jdet==32
        box 1 n=2000 cr=1024 cc=1024
      else_if jdet==33
        box 1 n=1020 cr=512 cc=512
      end_if
      abx $ireadbuf 1 high=high high_row=r1 high_col=c1
      if high<thresh&recenter<0
        pause
        call {read} {incval}+1
        abx $ireadbuf 1 high=high high_row=r1 high_col=c1
      end_if
      dispose all
      if high>thresh
        string com 'guideinst %i3 %f8.1 %f8.1' linst c1-ccen r1-rcen
        send '{com}'
        dxtot=dxtot+c1-ccen
        dytot=dytot+r1-rcen
        send 'sleep 8'

        if pmfast==0
          string com '{newext} %i4' {incval}
          send '{com}'
          string com '{exp} %f8.1' itime
          send '{com}'
          call {read} {incval}
          fits $ireadbuf char=ratarg '{ratarg}'
          fits $ireadbuf char=dectarg '{dectarg}'
          fits $ireadbuf char=aztarg '{aztarg}'
          fits $ireadbuf char=alttarg '{alttarg}'
          string out 'pm%i3.3' ipm
          wd $ireadbuf {out}
          ipm=ipm+1
          printf '%i3 {ratarg} {dectarg} {ireadbuf:ra} {ireadbuf:dec} ' ipos  |
             >>./pm.out

          ! Final recentering move
          if jdet==17
            box 1 n=200 cr=rcen cc=ccen
          else_if jdet==32
            box 1 n=400 cr=rcen cc=ccen
          else_if jdet==33
            box 1 n=400 cr=rcen cc=ccen
          end_if
          abx $ireadbuf 1 high=high high_row=r1 high_col=c1
          string com 'guideinst %i3 %f8.1 %f8.1' linst c1-ccen r1-rcen
          send '{com}'
          send 'sleep 8'
          dxtot=dxtot+c1-ccen
          dytot=dytot+r1-rcen
        end_if

        ! Direct output to pointing model file with current position and time
	call readstat 1
	printf '%2f12.6 %f8.1 {utc} %3f12.6 %f8.1' ratarg dectarg epoch lst az alt high |
             >>../images/{root}/{root}pm.out
	printf '%2f12.6 %f8.1 {utc} %3f12.6 %f8.1' ratarg dectarg epoch lst az alt high |
             >>../images/{root}/{root}{pmfile}.out
        if recenter<0
          {dx}=dxtot {dy}=dytot
        end_if
        goto nextstar
      else
        string com '{newext} %i4' {incval}
        send '{com}'
      end_if
     end_if
    end_do

  end_if
  nextstar:
  if nrep>1
    call readstat
    ha=lst-ratarg
    if (ha>12)
      ha=ha-24
    end_if
    if ha>hamax
      close pminput
      goto pmdone
    end_if 
!  else
!    send 'gexp 0'
  end_if
  if nrepeat>1&irepeat~=nrepeat
    send 'sleep 10'
  end_if
 end_do

end_do
close pminput

end_do

pmdone:

if daytest==0
!  call caz
end_if

call {readccd}
string comment 'End pointing model'
call writelog 1 {incval}-1 '{comment}'

makepm:

$'rm' ../images/{root}/{root}pm.dat
yy=2000+ifix[{root}/10000]
mm=ifix[({root}-(yy-2000)*10000)/100]
dd={root}-(yy-2000)*10000-(mm*100)
string ppp '%i %i %i' yy mm dd
$posrecen {ppp} < ../images/{root}/{root}pm.out > ../images/{root}/{root}pm.dat
$posrecen {ppp} < ../images/{root}/{root}{pmfile}.out > ../images/{root}/{root}{pmfile}.dat

send 'inst 1'
ver n
pmend:

if close==1
  send 'st'
  call close
  call shutdown
end_if
end

