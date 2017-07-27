parameter iseq
open input ./map.dat
stat n=count[input]
do i=1,n
  read input
  pa=@input.1+0.01
  loc=@input.2
  ack=@input.3
  sfoc=ack+1

  rad=sqrt[loc^2+100^2]
  theta=arctand[loc/100]

  call focus 1 sfoc sfoc+10
  rot={ireadbuf:obs_rot}

  printf '%2i5 %f8.1 %f8.2' theta+rot loc focus fwhm >>./mapfocus.dat
end_do

end
