parameter start end
copy 99 $start
sub 99 99
dr0=0
dc0=0
tv $start
printf 'Mark two stars to start '
mark new
save coo=./all.coo sr=0 sc=0
do i=start,end
  tv $i
  mark auto radius=11
  save coo=./junk sr=0 sc=0
  open input ./junk.coo
  daofiles file=./all.coo file2=./junk.coo file3=./all2.coo
  append
  $mv all2.coo all.coo
  read input
  read input
  read input
  read input
  c1=@input.2
  r1=@input.3
  read input
  c2=@input.2
  r2=@input.3
  dr0=dr0+dr
  dc0=dc0+dc
  printf '%i5 %3f8.2 {i:obs_rot}' i arctand[(r2-r1)/(c2-c1)] dr0 dc0
  printf '%i5 %3f8.2 {i:obs_rot}' i arctand[(r2-r1)/(c2-c1)] dr0 dc0 >>./g.dat
  add 99 $i dr=-int[dr0] dc=-int[dc0]
end_do
END
