parameter s1 s2


i=0
do k=1,m-4
 do l=k+1,m-3
  i=i+1
  if s1>0&k~=s1
    goto next
  end_if
  if s2>0&l~=s2
    goto next
  end_if
  xcol=1
  ycol=(i-1)*3+4
  ecol=(i-1)*3+5
  string ycol '202.%i3.3' ycol
  string ecol '202.%i3.3' ecol
  if i>1
    string noerase 'noerase'
  else
    string noerase
  end_if
  mul 202 c=100
  box 1 nc=1 sc=ycol nr=nr[202] sr=1
  daosky 202 box=1 3sig
  sky=sky/100
  div 202 c=100
  string object '%i2-%i2' k l
  if s1==0
   plot xc=$202.1 yc=${ycol} ec=${ecol} points=43 wind=1,npairs,i {noerase} |
     min=sky-0.1 max=sky+0.1 tlabel=object
  else
   plot xc=$202.1 yc=${ycol} ec=${ecol} points=43 tlabel=object
     ! min=sky-0.1 max=sky+0.1
  end_if
  next:
 end_do
end_do

if s1>0
  again:
  printf 'Hit 1 key on desired point'
  pause
  dist=1e10
  do irow=1,nr[202]
    dd=(getval[202,irow,xcol]-x1)^2+100*(getval[202,irow,ycol]-y1)^2
    if dd<dist
      dist=dd
      ipoint=irow
    end_if
  end_do
  printf 'DATE: %i6.6  obs: %i3.3  diff: %f12.3' |
     getval[202,ipoint,2] getval[202,ipoint,3] getval[202,ipoint,ycol]
  goto again
end_if

end
