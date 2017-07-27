parameter i1 i2
do j=i1,i2
  call read j
  n=ifix[j/100]
  if (mod[j,100]==0)
    n=n-1
  end_if
  i=j-n*100
  filt={i:filter}
  itime={i:exposure}
  err goto nextim
  objnum=0
  objnum={i:objnum}
  if filt==2
    string filter 'b'
  else_if filt==3
    string filter 'v'
  else_if filt==4
    string filter 'r'
  else_if filt==5
    string filter 'i'
  end_if
  if (objnum>0)
    printf '%i %i' j objnum >>./{root}.{filter}
  end_if
  nextim:
  printf '%i {i:ut} {i:ra} {i:dec} %f8.2 %i3 %i3' j itime filt objnum |
       >>./{root}.log
  dispose $i
end_do
end
