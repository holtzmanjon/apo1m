ii=67
create 1 n=500 sr=-200 sc=-200
do i=-1,1,1
 do j=-1,1,1
type ii
!  tv $ii
  string var 'r%i1' ii-66
  r={var}
  string var 'c%i1' ii-66
  c={var}
!  mark
  box 1 n=60 sr=r-30 sc=c-30
  copy 99 $ii box=1
  fixhead 99 origin
  add 1 99 dc=j*100 dr=i*100
  ii=ii+1
 end_do
end_do
end
