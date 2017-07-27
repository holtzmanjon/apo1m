parameter string=obj string=filt auto

err goto nodark
rd 207 /home/1m/cal/dark_050 maxtry=0
darkbuf=207
goto getflat
nodark:
darkbuf=0

getflat:
!err goto noflat
!rd 209 /home/1m/cal/flat{filt} maxtry=0
!flatbuf=209
!goto getsky
!noflat:
!flatbuf=0

getsky:
err goto nosky
rd 208 /home/1m/cal/sky5{filt} maxtry=0
!rd 208 /home/1m/cal/sky{filt} maxtry=0
skybuf=208
goto doit
nosky:
skybuf=0

doit:
setup apo
open input ./{obj}.{filt}
stat nepoch=count[input]
ntot=0
do j=1,nepoch
 read input
 i1=@input.1 i2=@input.2
 ntot=ntot+(i2-i1+1)
end_do
close input

open input ./{obj}.{filt}
nframes=0
nep=0
do j=1,nepoch
 string line {input}
 i1=@input.1 i2=@input.2
 if i1<0 
   goto nextline
 end_if
 string root '{input.-3}'
 setdir im dir=/home/backupb/1m/{root}

 call getflat 209 {root}
 flatbuf=209
 
 do i=i1,i2

  iframe=i
  idate={root}
  string file '{root}_%i3.3' i

  ! If .bad file exists, skip frame
  err goto isnst
  open coo ./{file}.bad
  close coo
  goto nextfram

  ! If .nst file exists then photometry is done
  isnst:
  err goto iscoo
  open coo ./{file}.nst
  ! make the shift file
  open ref ./master.coo
  do k=1,5
    read coo
    if k==2
      airmass=@coo.7
    end_if
    read ref
  end_do
  dr=@coo.3-@ref.3
  dc=@coo.2-@ref.2
  printf '%2f12.3' dr dc >./{file}.shift
  close ref
  close coo
  goto logit

  iscoo:
  call reduce i 0 flatbuf 0 1 0 skybuf darkbuf
  string file '{root}_%i3.3' i
  daosky $ireadbuf box=10
  if sky>10000
    $touch {file}.bad
    goto nextfram
  end_if
!tv $ireadbuf z=sky-5*skysig l=20*skysig

  ! If .coo file exists, then do photometry
  err goto isshift
  open coo ./{file}.coo
  close coo
  get coo=./{file} sr=0 sc=0
  goto photit

  isshift:
  if auto==0
    err goto newmark
  else
    err goto automark
  end_if
  open coo ./{file}.shift
  read coo
  dr=@coo.1 dc=@coo.2
  close coo
  get coo=./master sr=0 sc=0
  automark $ireadbuf auto dr=dr dc=dc radius=5
  save coo=./{file} sr=0 sc=0 high=maxbad
  goto photit

  automark:
  err goto newmark
  open coo ./master.coo
  do k=1,5
    read coo
  end_do
  r0=@coo.3 c0=@coo.2
  close coo
  box 1 n=50 cr=r0 cc=c0
  abx $ireadbuf 1 high_row=r high_col=c
  dr=r-r0  dc=c-c0
  printf '%2f8.3' dr dc >./{file}.shift
  goto isshift

  newmark:
  daosky $ireadbuf box=10
  tv $ireadbuf z=sky-5*skysig l=20*skysig
  if j==1&i==i1
   mark new
   save coo=./{file} sr=0 sc=0 high=maxbad
  else
   get coo=./last sr=0 sc=0
   printf 'If the displayed positions are OK, hit E in display to use them'
   printf 'If the positions are OK, but offset, hit the 1 key on the star'
   printf '  corresponding to the green box, then E to exit'
   printf 'To start with a new set of stars, hit Q in the display window,'
   printf '  then E to exit'
   r1=0 c1=0
   mark auto
   if r1>0
     open incoo ./last.coo
     read incoo
     read incoo
     read incoo
     read incoo
     r0=@incoo.3 c0=@incoo.2
     close incoo
     get coo=./last sr=0 sc=0
     clear vec
     mark auto dr=r1-r0 dc=c1-c0
     printf 'If the displayed positions are OK, hit E in display to use them'
     printf 'To start with a new set of stars, hit Q in the display window,'
     printf '  then E to exit'
   end_if
   err goto markit
   save coo=./{file} sr=0 sc=0 high=maxbad
   goto photit

   markit:
   mark new
   $touch {file}.bad
   err goto nextfram
   save coo=./{file} sr=0 sc=0 high=maxbad
   $'rm' {file}.bad
  end_if

  photit:

  $touch {file}.bad
  err goto nextfram
  save coo=./last sr=0 sc=0 high=maxbad
  $'rm' {file}.bad
  daofiles coo=./{file} mag=./{file} pro=./{file} psf=./{file} grp=./{file}
  photom $ireadbuf rad=-12,4,2 skyrad=10,24 gain=gain rn=rn skyerr=0.0015

  !PSF fitting results, v. large fitting radius
  opt ip=1 ig=1 ps=25 fi=10 is=1
  psf $ireadbuf stars=2,3,4
  daolib $ireadbuf n=4
  $'rm' {file}.psf
  group crit=10
  opt ip=0
  multistar $ireadbuf exp=1 col=1 gain=gain rn=rn skyrad=10,24 |
     fi=12 is=1 keep
  daofiles file=./{file}.nst file2=./tmp.nst
  sort index=1 norenum
  $mv tmp.nst {file}psf.nst

  ! Save aperture results in .nst file
  daofiles pro=./{file}
  shortap

  hjd $ireadbuf
  hjd={ireadbuf:hjd}
  hjd=hjd-ifix[hjd/10000]*10000
  airmass={ireadbuf:airmass}

  starplot $ireadbuf load noplot scale=scale phot gauss silent
  fwhm=fwavg
  printf '%f14.6 %f10.3' hjd fwhm >./{file}.dat

  logit: 
  nframes=nframes+1
  open fwhm ./{file}.dat
  read fwhm
  fwhm=0
  error continue
  hjd=@fwhm.1 fwhm=@fwhm.2
  close fwhm
  open phot ./{file}.nst
  open psfphot ./{file}psf.nst
  stat m=count[phot]
  do k=1,3
    read phot
    if k==2
      ap=@phot.10
    end_if
    read psfphot
  end_do
  do k=1,m-3
    read phot
    string var 'm%i3.3' k
    {var}=@phot.4
    string var 'e%i3.3' k
    {var}=@phot.5
    {var}={var}^2
    string var 's%i3.3' k
    {var}=@phot.6
  end_do
  do k=1,m-3
    read psfphot
    string var 'psfm%i3.3' k
    {var}=@psfphot.4
    string var 'psfe%i3.3' k
    {var}=@psfphot.5
    {var}={var}^2
    string var 'psfs%i3.3' k
    {var}=@psfphot.6

    string apmag 'm%i3.3' k
    string mag 'nm%i3.3' k
    string sky 's%i3.3' k
    {mag}=10.^(-0.4*({apmag}-25))+3.14159*ap^2*({sky}-{var})
    {mag}=-2.5*log10[{mag}]
  end_do
  close phot
  close psfphot
!  npairs=m-4
!  do k=1,m-5
!    npairs=npairs+k
!  end_do
  if nframes==1
!    create 202 nc=npairs*3+3 nr=ntot sr=1 sc=1
    create 203 nc=(m-3)*3+3 nr=ntot sr=1 sc=1
    create 204 nc=(m-3)*3+3 nr=ntot sr=1 sc=1
    create 205 nc=(m-3)*3+4 nr=ntot sr=1 sc=1
  end_if
  do ibuf=203,205
    a=setval[ibuf,nframes,1,hjd]
    a=setval[ibuf,nframes,2,idate]
    a=setval[ibuf,nframes,3,iframe]
  end_do
  a=setval[205,nframes,(m-3)*3+4,airmass]
  
  do k=1,m-3
    string var1 'm%i3.3' k
    string var2 'e%i3.3' k
    a=setval[205,nframes,3+(k-1)*3+1,{var1}]
    a=setval[205,nframes,3+(k-1)*3+2,sqrt[{var2}]]
    a=setval[205,nframes,3+(k-1)*3+3,fwhm]
    string var1 'psfm%i3.3' k
    string var2 'psfe%i3.3' k
    a=setval[203,nframes,3+(k-1)*3+1,{var1}]
    a=setval[203,nframes,3+(k-1)*3+2,sqrt[{var2}]]
    a=setval[203,nframes,3+(k-1)*3+3,fwhm]
    string var1 'nm%i3.3' k
    string var2 'e%i3.3' k
    a=setval[204,nframes,3+(k-1)*3+1,{var1}]
    a=setval[204,nframes,3+(k-1)*3+2,sqrt[{var2}]]
    a=setval[204,nframes,3+(k-1)*3+3,fwhm]
  end_do
!  ipair=0
!  do k=1,m-4
!    do l=k+1,m-3
!      ipair=ipair+1
!      string var1 'm%i3.3' k
!      string var2 'm%i3.3' l
!      diff={var1}-{var2}
!      string var1 'e%i3.3' k
!      string var2 'e%i3.3' l
!      differr=sqrt[{var1}+{var2}]
!
!      a=setval[202,nframes,3+(ipair-1)*3+1,diff]
!      a=setval[202,nframes,3+(ipair-1)*3+2,differr]
!      a=setval[202,nframes,3+(ipair-1)*3+3,fwhm]
!
!      string var 'n%i1%i1' k l
!      string var2 'd%i1%i1' k l
!      if i==i1
!        {var}=0
!        {var2}=0
!      else_if abs[diff]<10
!        {var}={var}+diff/differr^2
!        {var2}={var2}+1/differr^2
!      end_if
!    end_do
!  end_do

 nextfram:
 end_do
! if d12>0
!  nep=nep+1
!  if nep==1
!    create 201 nc=npairs*3+1 nr=nepoch sr=1 sc=1
!  end_if
!  ipair=0
!  do k=1,m-4
!   do l=k+1,m-3
!     ipair=ipair+1
!     string var 'n%i1%i1' k l
!     string var2 'd%i1%i1' k l
!     a=setval[201,nep,1,hjd]
!     a=setval[201,nep,1+(ipair-1)*3+1,{var}/{var2}]
!     a=setval[201,nep,1+(ipair-1)*3+2,sqrt[1./{var2}]]
!     a=setval[201,nep,1+(ipair-1)*3+3,fwhm]
!   end_do
!  end_do
! end_if
 nextline:
end_do
close input

box 1 nc=nc[205] sc=sc[205] nr=nframes sr=1
window 205 box=1
box 1 nc=nc[203] sc=sc[203] nr=nframes sr=1
window 203 box=1
box 1 nc=nc[204] sc=sc[204] nr=nframes sr=1
window 204 box=1
!box 1 nc=nc[202] sc=sc[202] nr=nframes sr=1
!window 202 box=1
!if nep>0
!  box 1 nc=nc[201] sc=sc[201] nr=nep sr=1
!  window 201 box=1
!end_if

wd 205 ./{obj}.{filt}.fits full
print 205 ncout=50 nsig=7 >./t{obj}.dat
$'rm' {obj}.{filt}.dat
$awk 'NR>9 \{print substr($0,11)\}' t{obj}.dat > {obj}.{filt}.dat

wd 203 ./{obj}psf.{filt}.fits full
print 203 ncout=50 nsig=7 >./t{obj}.dat
$'rm' {obj}psf.{filt}.dat
$awk 'NR>9 \{print substr($0,11)\}' t{obj}.dat > {obj}psf.{filt}.dat

wd 204 ./{obj}psfsky.{filt}.fits full
print 204 ncout=50 nsig=7 >./t{obj}.dat
$'rm' {obj}psfsky.{filt}.dat
$awk 'NR>9 \{print substr($0,11)\}' t{obj}.dat > {obj}psfsky.{filt}.dat

!wd 202 ./{obj}diff.{filt}.fits full
!print 202 ncout=50 nsig=7 >./t{obj}.dat
!$'rm' {obj}.{filt}diff.dat
!$awk 'NR>9 \{print substr($0,11)\}' t{obj}.dat > {obj}diff.{filt}.dat

!if nep>0
!wd 201 ./{obj}ave.{filt}.fits full
!print 201 ncout=50 nsig=7 >./t{obj}.dat
!$'rm' {obj}ave.{filt}.dat
!$awk 'NR>9 \{print substr($0,11)\}' t{obj}.dat > {obj}ave.{filt}.dat
!end_if

!pause
!i=0
!do k=1,m-4
! do l=k+1,m-3
!  i=i+1
!  string ycol '202.%i3.3' 3+(i-1)*3+1
!  string ecol '202.%i3.3' 3+(i-1)*3+2
!  if i>1
!    string noerase 'noerase'
!  else
!    string noerase
!  end_if
!  mul 202 c=100
!  box 1 nc=1 sc=3+(i-1)*3+1 nr=nr[202] sr=1
!  daosky 202 box=1 3sig
!  sky=sky/100
!  div 202 c=100
!  string object '%i2-%i2' k l
!  plot xc=$202.1 yc=${ycol} ec=${ecol} points=43 wind=1,npairs,i {noerase} |
!     min=sky-0.1 max=sky+0.1 tlabel=object
! end_do
!end_do

printf 'Photometry output is in file {obj}.{filt}.dat'
cd /home/1m

end
