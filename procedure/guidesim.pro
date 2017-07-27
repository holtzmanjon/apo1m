$'rm' guide.sum
do k=5,10
  fudge=k/10
do j=1,5

x0=64 y0=64 dt=10 drift=0.01 counts=10000
xc=0 yc=0 rn=30
create 1 n=128 sr=1 sc=1
create 11 n=128 sr=1 sc=1
$'rm' guide.dat
do i=1,100
  x=x0+(i-1)*drift*dt+xc+ran[0,1]
  y=y0+(i-1)*drift*dt+yc+ran[0,1]

  sub 1 1
  photons 1 at=y,x counts=counts,counts gauss fw=2 mean=-1 rn=rn |
     kernel=-(j*10)+i
  add 11 1
  box 1 nr=5 nc=5 cr=y cc=x
  automark 1 range=3*rn,10000 new box=1
  save coo=./junk sr=0 sc=0
  open coo ./junk.coo
  read coo
  read coo
  read coo
  read coo
  c=@coo.2 r=@coo.3
  xc=xc+fudge*(x0-c) yc=yc+fudge*(y0-r)
  printf '%6f8.3' x y c r xc yc 
  printf '%6f8.3' x y c r xc yc >>./guide.dat
!  tv 1 l=5*rn
end_do
  automark 11 range=3*rn,100000000 new box=1
  starplot 11 phot gauss load
  printf '%2f8.3' fudge fwhm
  printf '%2f8.3' fudge fwhm >>./guide.sum

end_do
end_do
end
