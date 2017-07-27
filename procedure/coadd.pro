parameter i1 i2 dest

dr0=0 dc0=0
copy $dest $i1
sub $dest $dest
tv $i1
mark new
do i=i1,i2
  tv $i
  mark auto 
  dr0=dr0-dr
  dc0=dc0-dc
  add $dest $i dr=dr0 dc=dc0
end_do

END

