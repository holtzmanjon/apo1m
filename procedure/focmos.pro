parameter imstart imend dest nbox r1 c1
box 1 n=nbox cr=r1 cc=c1
create $dest n=5*nbox sr=1 sc=1
do i=imstart,imend
  dx=ifix[(i-imstart)/5]
  dy=mod[(i-imstart),5]
  call readg i
  window $ireadbuf box=1
  fixhead $ireadbuf origin
  add $dest $ireadbuf dr=dy*nbox dc=dx*nbox
end_do
END
