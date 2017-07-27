parameter guider start end noauto

err goto focerr
if end==0
  end=start
end_if
high=0 axmin=1e10
highfoc=focus axfoc=focus

string noplot 'noplot'
!string noplot ' '

! Read in images
if abs[guider]==1
  call readg start end
else
  call read start end
end_if

call ireadbuf start
nnr=nr[ireadbuf]-40
nnc=nc[ireadbuf]-40
nsr=sr[ireadbuf]+20
nsc=sc[ireadbuf]+20
box 1 nr=nnr nc=nnc sr=nsr sc=nsc
!if abs[guider]==1
!  box 1 cc=512 nc=512 cr=512 nr=512 
!  box 1 cc=512 nc=800 cr=512 nr=800 
!end_if
if abs[guider]==1
  idet=guidedet
else
  idet=scidet
end_if
call getdet idet

! Find stars on middle image of sweep
if noauto==2
  ii=int[(start+end)/2]
  call ireadbuf ii
  copy 199 $ireadbuf
  call findstar 199 1 guider
end_if

fwmin=99
jj=1
do ii=start,end
  call ireadbuf ii
  i=ireadbuf
  if idet==15
    photons $i mean=0 rn=2
  end_if
  string var 'f%i2.2' ii-start+1
  if guider<0
    string card guidefoc
  else
    string card focus
  end_if
  {var}={i:{card}}
  if noauto~=2
    tv $i
  end_if
  
  ! If we are not automatically finding stars, mark star
  if (noauto~=2)&(ii==start|noauto==1)
      mark new id=1
      box 11 nr=50 nc=50 cr=r cc=c
  else
    if noauto==2
      automark $i auto id=ii-start+1 radius=51
    else
      abx $i box=11 high_row=hr high_col=hc silent
      type r c hr hc
      mark auto exit id=ii-start+1 dr=hr-r dc=hc-c
      r=r+dr c=c+dc
    end_if
  end_if

  ! Aperture photometry and get image parameters
  aperstar $i star=5 sky=10,15
  fwavg=99
  starplot $i scale=scale phot gauss {noplot} load   !silent

  ! Save results
  if ii==start
    save phot=./focus
  else
    save phot=./tmp
    get phot=./focus
    get phot=./tmp append
    save phot=./focus
    get phot=./tmp
  end_if

  ! subtract sky and add into summary image

  sky $i silent
  sub $i c=sky 
  if noauto~=2
    if ii=start
      create 101 nc=nc[i] nr=nr[i]+(end-start+2)*30
    end_if
    add 101 $i dr=(ii-start)*30
  end_if

  ! figure out high pixel from first star in the list
  save coo=./junk sr=0 sc=0
  daofiles coo=./junk mag=./junk pro=./junk.nst
  photom $i rad=20/scale skyrad=30/scale,50/scale
  shortap
  open incoo ./junk.coo
  open inpro ./junk.nst
  do iline=1,4
    read incoo
    read inpro
  end_do
  cc=@incoo.2 cr=@incoo.3 tot=10^(-0.4*(@inpro.4-25))
  close incoo
  close inpro
  box 12 n=21 cr=cr cc=cc
  abx $i 12 high=h silent
  if (h/tot>high)
    high=h/tot highfoc={i:{card}}
  end_if

  box 12 n=11 cr=cr cc=cc
  axes $i box=12 sky=0
  if (axfw<axmin)
    axmin=axfw axfoc={i:{card}}
  end_if
  xtilt={i:x_tilt} ytilt={i:y_tilt}
  if fwavg<fwmin
    fwmin=fwavg minfoc={i:{card}} minxt={i:x_tilt} minyt={i:y_tilt}
  end_if
  string var 'out%i' ii
  string {var} '%3f8.3' xtilt ytilt fwavg
  iy=mod[jj-1,3]+1
  ix=ifix[(jj-1)/3]+1
  string var 't%i%i' ix iy
  {var}=fwavg
  jj=jj+1
end_do
printf 'min: %3f8.3' fwmin minxt minyt
do ii=start,end
  string var 'out%i' ii
  printf '{{var}}'
end_do
printf '\n'
printf '%3f8.3' t11 t21 t31
printf '%3f8.3' t12 t22 t32
printf '%3f8.3' t13 t23 t33

! Show the summary image
if noauto~=2
  mn 101
  tv 101
  save coo=./focus sr=0 sc=0
  open coo ./focus.coo
  stat n=count[coo]
  nstars=ifix[n/(end-start+1)]
  read coo
  read coo
  read coo
  do i=start,end
    string var 'f%i2.2' i-start+1
    string foctext '%f8.1' {var}
    do istar=1,nstars
      read coo
      x=@coo.2+10
      y=@coo.3+(i-start)*30
      tvplot p=y,x text=foctext
    end_do
  end_do
  close coo
end_if
focerr=0
goto focend
focerr:
printf 'Error determining focus'
focerr=1

focend:
END
