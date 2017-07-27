parameter iseq
open input ./map.dat
stat n=count[input]
focus=-12095 nfoc=15
do i=1,n
  read input
  pa=@input.1+0.01
  loc=@input.2
  ack=@input.3
  sfoc=ack+1

  rad=sqrt[loc^2+100^2]
  theta=arctand[loc/100]

  call focus 1 sfoc sfoc+nfoc-1 2
  rot={ireadbuf:obs_rot}

  printf '%2i5 %f8.1 %f8.2 %f8.1 %f8.2' theta+rot loc focus fwhm minfoc fwmin 
  printf '%2i5 %f8.1 %f8.2 %f8.1 %f8.2' theta+rot loc focus fwhm minfoc fwmin |
     >>./mapfocus.dat

pause

end_do

end
