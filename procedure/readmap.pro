parameter iseq
open input ./map.dat
stat n=count[input]
!create 201 n=1024 sr=-512 sc=-512
create 201 n=2048 sr=-1024 sc=-1024
do i=1,n
  read input
  pa=@input.1+0.01
  loc=@input.2
  ack=@input.3
  sfoc=ack+1

  rad=sqrt[loc^2+210^2]
  theta=arctand[loc/210]

  call readg sfoc+iseq-1
  rot={ireadbuf:obs_rot}

  box 1 n=200 cr=400 cc=400

  window $ireadbuf box=1
  fixhead $ireadbuf origin
  rotate $ireadbuf pa=pa
  fixhead $ireadbuf origin
  copy 199 $ireadbuf
  zap 199 sig=0 size=3
  abx 199 high_row=hr high_col=hc
hr=0 hc=0
  !hr=50 hc=50
  shift $ireadbuf dr=-hr dc=-hc

  dx=(rad/300)*200*cosd[theta+rot]
  dy=(rad/300)*200*sind[theta+rot]
  add 201 $ireadbuf dc=dx dr=dy
end_do

end
