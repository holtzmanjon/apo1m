parameter string=list movie
if movie>0
  nloop=movie
else
  nloop=1
end_if
do iloop=1,nloop
  open input ./{list}
  stat n=count[input]
  do i=1,n
    read input
    im1=@input.1 im2=@input.2
    do im=im1,im2
      if iloop==1
        call read im
      end_if
!      autotv ireadbuf
      tv $ireadbuf noerase
      if movie==0
      tvstar scale=scale
      end_if
    end_do
  end_do
  close input
end_do
END
