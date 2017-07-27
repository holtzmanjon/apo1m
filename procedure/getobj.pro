parameter uttest

! Get current time
call time uttest 1 1
utnow=uttim
jdnow=jd
iprio=-3

nextprio:
error continue
close objinp
iprio=iprio+1
error goto definp
open objinp ./{root}.inp
goto readobj

definp:
open objinp ./default.inp

readobj:
if iprio<0&uttest>-1
eof goto nextprio
else
eof goto noobj
end_if

error goto noobj

! Read header lines
string line '{objinp}'
string line '{objinp}'
string line '{objinp}'
string line '{objinp}'
nfilt={line.1}
string line '{objinp}'
string line '{objinp}'

iobj=0 iobj2=0 icount=0
nobj=0 nobj2=0 ifile=0
nextobj:
if iobj2>0&iobj2<=nobj2
 if iobj2<nobj2
  string line '{objinp2}'
  string line2 '{objinp2}'
  iobj2=iobj2+2
  icount=icount+1
 else_if iobj2==nobj2
    close objinp2
    iobj2=0 nobj2=0
    goto nextobj
  end_if
else
  string line '{objinp}'
  strcmp '{objinp.-1}' '#' silent
  if strcmpok==1
    goto nextobj
  end_if
  strcmp '{objinp.-1}' '+' silent
  if strcmpok==1
    ifile=ifile+1
    tlow={line.3} thigh={line.4}     
    fprio=0
    error continue
    fprio={line.5}
    if (uttim<tlow)|(uttim>thigh)&(uttest>=0)
      printf 'Group: {objinp.-2} out of localtime range' 
      open objinp2 ./{objinp.-2}
      stat nobj2=count[objinp2]
      close objinp2
      if uttest<0
        do icount=1,(nobj2-2)/2
          string var 'ndo%i5.5' ifile*1000+icount
          {var}=0
          string var 'nsk%i5.5' ifile*1000+icount
          {var}=0
          string var 't%i5.5' ifile*1000+icount
          {var}=-100000
        end_do
      end_if
      nobj=nobj+(nobj2-2)/2
      goto nextobj
    end_if
    iobjinp2=0
    error goto nextobj
    open objinp2 ./{objinp.-2}
    string group {objinp.-2}
    stat nobj2=count[objinp2]
    ! Header lines
    string line '{objinp2}'
    string line '{objinp2}'
    nobj2=nobj2-2
    string line '{objinp2}'
    string line2 '{objinp2}'
    iobj2=iobj2+2
    icount=1
  else
    string line2 '{objinp}'
    icount=icount+1
  end_if
end_if
!printf 'LINE: {line}'
!printf 'LINE: {line2}'
!type iobj2 nobj2
!pause
error goto nextobj
string name '{line.1}'
!nobj=nobj+1
nobj=ifile*1000+icount

printf '%i5.5 : {name} %i %i' nobj ifile icount
doinit=0
if uttest<0
  doinit=1
end_if
! Are variables not initialized (i.e. has this object been added since initvar)?
error doinit=1
string var 'ndo%i5.5' nobj
junk={var}

if doinit==1
  acquire=0
  string var 'ndo%i5.5' nobj
  {var}=0
  string var 'nsk%i5.5' nobj
  {var}=0
  string var 'pri%i5.5' nobj
  {var}=0
  string var 't%i5.5' nobj
  {var}=-100000
end_if
if uttest<0
  goto nextobj
end_if

error goto nextobj
! Commented out?
$'rm' junk.dat
$echo "{name}" | awk '\{print substr($1,1,1)\}' >./junk.dat
open jjunk ./junk.dat
string first {jjunk}
close jjunk
$'rm' junk.dat
!strcmp {name} '#' silent
!printf 'line: {line}'
!printf 'first: {first}'
strcmp A{first} 'A#' silent
type strcmpok
if strcmpok==1
  goto nextobj
end_if

! correct priority?
string var 'pri%i5.5' nobj
printf 'priority %i %i %i'  nobj {var} iprio
if ({var}~=iprio)&(fprio~=iprio)
  goto nextobj
end_if

! Already marked to skip?
string skipvar 'nsk%i5.5' nobj
if {skipvar}==1
  printf 'Object: %i5 ({name}) already marked to skip' nobj
  goto nextobj
end_if

! Have we done all the requested visits yet?
nvisits={line.19}
string var 'ndo%i5.5' nobj
if nvisits>0&{var}>=nvisits
  {skipvar}=1
  printf 'Object: %i5 ({name}) already done requested visits' nobj
  goto nextobj
end_if

! Has it been long enough since the last visit?
!string var 'ndo%i5.5' nobj
!if {var}>0
  dtvisit={line.20}
  string var 't%i5.5' nobj
  jdlast=-100000
  error goto checkjd
  open jdlast obj/{name}.jd
  read jdlast
  jdlast=@jdlast.1
  close jdlast
  checkjd:
  if (jdlast==-100000)   !  abs[jd-jdlast]<0.9
    dt=uttim-{var}
  else
    dt=(jd-jdlast)*24
  end_if
!  if (dtvisit<10&uttim-{var}<dtvisit)|(dt<dtvisit&dtvisit>0)
  if (dtvisit<=10&uttim-{var}<dtvisit)|(dt<dtvisit&dtvisit>10)
    printf 'Object: %i5.5 ({name}) has been observed too recently to repeat (%f8.1)' |
       nobj dt
    goto nextobj
  end_if
!end_if

! Next block only needs to be checked once per night
if {skipvar}==-1
  goto utcheck
end_if

! Is date acceptable?
year={line.7}
!if (utyear<year|utyear>year)
!  printf 'Object: %i5 ({name}) out of UT year range' nobj
!  {skipvar}=1
!  goto nextobj
!end_if
mlow={line.8} mhigh={line.10}
if (utmonth<mlow|utmonth>mhigh)
  printf 'Object: %i5 ({name}) out of UT month range' nobj
  {skipvar}=1
  goto nextobj
end_if
dlow={line.9} dhigh={line.11}
if utmonth==mlow&utday<dlow
  printf 'Object: %i5 ({name}) out of UT day range' nobj
  {skipvar}=1
  goto nextobj
end_if
if utmonth==mhigh&utday>dhigh
  printf 'Object: %i5 ({name}) out of UT day range' nobj
  {skipvar}=1
  goto nextobj
end_if
{skipvar}=-1

utcheck:
! Is UT time acceptable?
tlow={line.13}
thigh={line.14}
if (uttim<tlow)|(uttim>thigh)
  printf 'Object: %i5 ({name}) out of localtime range' nobj
  goto nextobj
end_if

! Is airmass acceptable?
printf '{line.1}	{line.2}	{line.3}	{line.4}' >./tmp.apo
call airmass ./tmp.apo 1 uttest 1
xlow={line.5}
xhigh={line.6}
if (airmass<xlow)|(airmass>xhigh)
  printf 'Object: %i5 ({name}) out of airmass range (%f5.2) UT: %f8.2' nobj airmass uttest
  goto nextobj
end_if

! Is moon acceptable?
maxphase={line.17}
mindist={line.18}
if (phase>maxphase)&(dmoon<mindist)
  {skipvar}=1
  printf 'Object: %i5 ({name}) too near to moon (phase: %f5.2  dmoon: %f5.2) ' nobj phase dmoon
  goto nextobj
end_if

! Is hour angle acceptable for first observation?
string rra {line.2}
getcoord {rra}
rah=hh+mm/60+ss/3600
string ddec {line.3}
getcoord {ddec}
decd=sg*(abs[hh]+mm/60+ss/3600)
epoch0={line.4}
string var 'ndo%i5.5' nobj
if {var}==0
  if epoch0>0
    precess ra=rah dec=decd epoch0=epoch0 epoch=2005.
    hha=lst-raf
  else
    hha=lst-rah
  end_if
  if hha>12
    hha=hha-24
  end_if
  if hha<-12
    hha=hha+24
  end_if
  halow={line.15}
  hahigh={line.16}
type halow hahigh hha
  if ((hha<halow)|(hha>hahigh))      !&nobj~=curobj
    if hha>hahigh
      {skipvar}=1
    end_if
    printf 'Object: %i5 ({name}) past starting hour angle range (%f4.1) ' nobj hha
    goto nextobj
  end_if
  utstart={line.12}
  if uttim>utstart
    {skipvar}=1
    printf 'Object: %i5 ({name}) will start too late ' nobj
    goto nextobj
  end_if
end_if
close objinp

! We have an acceptable object!
! if this is a multivisit object, increase priority so we will finish it!
if nvisits==-1|nvisits<-100
  string var 'pri%i5.5' nobj
  {var}=-1
end_if
if nvisits<-100
  do iv=1,abs[nvisits]-100
    string var 'pri%i5.5' nobj+iv
    {var}=-2
    string var 'ndo%i5.5' nobj+iv
    {var}=0
    string var 'nsk%i5.5' nobj+iv
    {var}=0
  end_do
end_if

printf 'OBSERVE: {name}'
! Get focus and exposure information
error goto nextobj
dofoc={line.21}
delfoc={line.22}
guide={line.23}

gloc={line.24}
nskip={line.25}
nwind=0 xoffset=0 yoffset=0
error continue
nwind={line.26}
error continue
xoffset={line.27}
error continue
yoffset={line.28}
gpa=1060
gpa=970
gpa=0
!error continue
!gpa={line.26}

npar=0 nexp=0
string var 'line2.%i2.2' npar+1
error goto nextobj
nset={{var}}
do ifilt=1,nfilt
  error goto nextobj
  string var 'line2.%i2.2' npar+1+(ifilt-1)*2+1
  string t 'nexp%i2.2' ifilt
  {t}={{var}}
  nexp=nexp+{t}
  string var 'line2.%i2.2' npar+1+(ifilt-1)*2+2
  string t 'exp%i2.2' ifilt
  {t}={{var}}
end_do

! Get a guide star file
epoch={line.4}
if epoch<24
  call getephem {name} epoch
  goto newusno
else
  string ras {line.2}
  string decs {line.3}
  string epochs '{line.4}'
end_if
epoch={epochs}
if uttest<0
  send 'NE {epochs}'
end_if
error goto newusno
open usno usno/{name}.usno
close usno
goto newobj

newusno:
$square {ras} {decs} 0.813333 {epochs} | head -4 >usno/{name}.usno
$square {ras} {decs} 0.813333 {epochs} | tail -n +6 | sort -n --field=6 >>usno/{name}.usno

newobj:
if nobj==curobj
  acquire=0
else_if dofoc<0
  acquire=dofoc
else_if dofoc>0
  acquire=dofoc
else
  acquire=2
end_if

if uttest>0
  objerr=0
  if acquire==0
    testim=60
  else_if abs[acquire]==1
    testim=720
  else
    testim=300
  end_if
  do ifilt=1,nfilt
    string var 'exp%i2.2' ifilt
    string nvar 'nexp%i2.2' ifilt
    if (abs[{nvar}]>0)
      if dofoc>99
        testim=testim+abs[nset]*(5+(abs[{var}]*10))
      else
        testim=testim+abs[nset]*(5+abs[{nvar}]*(abs[{var}]+20))
      end_if
    end_if
  end_do
else
  ! Make the observation!!
  call writesum 3 'OBSERVE: {name}'

! Do we have a custom procedure to run?
  if dofoc>99
    if dofoc>100
     dopause=1
    else
     dopause=0
    end_if
    dofoc=0
    call checkfoc
    call apogee {name} 0 0 0 dopause
  else_if nset==0
    string var 'line2.%i2.2' npar+1+2*nfilt+1
    string proc '{{var}}'
    call {proc}
    objerr=0
  else
    string var 'ndo%i5.5' nobj
    guidemon=nobj*1000000+{var}*10000
    guidemon=0
    if dofoc==0
      call checkfoc
    end_if
    call newobject {name} guide acquire gloc nskip guidemon gpa nwind
  end_if
end_if

! Get current time
call time uttest 1 1
utnow=uttim
jdnow=jd
string var 't%i5.5' nobj
{var}=utnow
string var 'jd%i5.5' nobj
{var}=jdnow

if objerr==0
  curobj=nobj
  string var 'ndo%i5.5' nobj
  {var}={var}+1
  if uttest==0
    printf '%f12.6' jdnow-0.0001 >obj/{name}.jd
  end_if
else
  curobj=0
  string var 'nsk%i5.5' nobj
  {var}=1
end_if

goto doneobj

noobj:
nobj=-1
printf 'No acceptable object found'
string name 'NONE'
testim=1800
if uttest==0
  send 'STUP'
  $sleep 120
end_if

doneobj:
end
