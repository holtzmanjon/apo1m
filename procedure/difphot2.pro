parameter string=obj string=filt
setup apo
open input ./{obj}.{filt}
stat nepoch=count[input]
ntot=0
do j=1,nepoch
 string line {input}
 i1=@input.1 
 i2=i1
 ntot=ntot+(i2-i1+1)
end_do
close input

open input ./{obj}.{filt}
nframes=0
do j=1,nepoch
 string line {input}
 i1=@input.1 i2=i1
 string root '{input.-2}'
 setdir im dir=/home/1m/{root}
 
 do i=i1,i2
  nframes=nframes+1
  call read i
  string file '{root}_%i3.3' i
  
  err goto doit
  open phot ./{file}.nst
  close phot
  goto compile

  doit:
  mn $ireadbuf
  tv $ireadbuf
  !if j==1&i==i1
   mark new
  !else
  ! mark auto
  !end_if
  save coo=./{file} sr=0 sc=0
  daofiles coo=./{file} mag=./{file} pro=./{file}
  photom $ireadbuf rad=5 skyrad=15,20 gain=4 rn=10
  shortap

  compile: 
  hjd $ireadbuf
  hjd={ireadbuf:hjd}
  hjd=hjd-ifix[hjd/1000]*1000
  open phot ./{file}.nst
  stat m=count[phot]
  read phot
  read phot
  read phot
  do k=1,m-3
    read phot
    string var 'm%i2.2' k
    {var}=@phot.4
    string var 'e%i2.2' k
    {var}=@phot.5
    {var}={var}^2
  end_do
  close phot
  npairs=m-4
  do k=1,m-5
    npairs=npairs+k
  end_do
  if nframes==1
    create 201 nc=npairs*2+1 nr=ntot sr=1 sc=1
  end_if
  ipair=0
  do k=1,m-4
    do l=k+1,m-3
      ipair=ipair+1
      string var1 'm%i2.2' k
      string var2 'm%i2.2' l
      diff={var1}-{var2}
      string var1 'e%i2.2' k
      string var2 'e%i2.2' l
      differr=sqrt[{var1}+{var2}]

      a=setval[201,nframes,1,hjd]
      a=setval[201,nframes,1+(ipair-1)*2+1,diff]
      a=setval[201,nframes,1+(ipair-1)*2+2,differr]
    end_do
  end_do

 end_do
end_do
close input

i=0
do k=1,m-4
 do l=k+1,m-3
  i=i+1
  string ycol '201.%i2.2' (i-1)*2+2
  string ecol '201.%i2.2' (i-1)*2+3
  if i>1
    string noerase 'noerase'
  else
    string noerase
  end_if
  mul 201 c=100
  box 1 nc=1 sc=(i-1)*2+2 nr=nr[201] sr=1
  daosky 201 box=1 3sig
  sky=sky/100
  div 201 c=100
  string object '%i2-%i2' k l
  plot xc=$201.1 yc=${ycol} ec=${ecol} points=43 wind=1,npairs,i {noerase} |
     min=sky-0.1 max=sky+0.1 tlabel=object
 end_do
end_do

print 201 >./t{obj}.dat
printf '    DATE/TIME   m1-m2     err    m1-m3    err    m2-m3    err' |
    >./{obj}.dat
$awk 'NR>9 \{print substr($0,10)\}' t{obj}.dat >> {obj}.dat

printf 'Photometry output is in file {obj}.dat'
end
