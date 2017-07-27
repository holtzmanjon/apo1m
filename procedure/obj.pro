open input ./obj037.dat
stat n=count[input]
do i=1,n
  read input
  string root '%i6.6' @input.1
  n=@input.2
  setdir im dir=/1m/{root}
  call read n
  tv $ireadbuf
  pause
end_do
 
END
