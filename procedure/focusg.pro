parameter start end
if end==0
  end=start
end_if
create 1 n=1024
ask 'Start focus: ' sfoc
ask 'Delta focus: ' dfoc
foc=sfoc
do ii=start,end
  call readg ii
  i=ireadbuf
  string var 'f%i2.2' ii-start+1
  {var}={i:focus}
  tv $i
!  if ii==start
    mark new id=1 focus=foc,dfoc
    box 11 nr=50 nc=50 cr=r cc=c
!  else
!    abx $i box=11 high_row=hr high_col=hc
!    mark auto exit id=ii-start+1 dr=hr-r dc=hc-c focus=foc,dfoc
!    r=r+dr c=c+dc
!  end_if
  foc=foc+dfoc
  aperstar $i star=5 sky=10,15
!  psffit $i radius=int[1.5/scale]
  starplot $i phot gauss 
  if ii==start
    save phot=./focus
  else
    save phot=./tmp
    get phot=./focus
    get phot=./tmp append
    save phot=./focus
    get phot=./tmp
  end_if
  sky $i
  sub $i c=sky 
  add 1 $i dr=(ii-start)*30
end_do
get phot=./focus
plotfocus scale=scale
mn 1
tv 1
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
END
