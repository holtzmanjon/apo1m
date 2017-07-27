parameter i1 i2
do i=i1,i2
  call read i
  mn $ireadbuf
  tv $ireadbuf
  mark new
  save coo=./junk sr=0 sc=0
  open input ./junk.coo
  read input
  read input
  read input
  read input
  c1=@input.2 r1=@input.3
  read input
  c2=@input.2 r2=@input.3
  close input
  angle=arctand[(r2-r1)/(c2-c1)]
  if i==i1
    drot=0.
  else
    drot=angle-angleold
    if (drot<-70) 
      drot=drot+180
    else_if (drot>70)
      drot=drot-180
    end_if
  end_if
  angleold=angle
  printf '%i3 %f8.3 %f8.3 {ireadbuf:obs_rot}' i angle drot
  printf '%i3 %f8.3 %f8.3 {ireadbuf:obs_rot}' i angle drot >>./drot.dat
 
end_do 

end
