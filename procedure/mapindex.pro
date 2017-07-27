start=360
nfoc=15
start=73
nfoc=7
do pa=0,360,60
!  do loc=0,900,300
  do loc=300,1200,900
    acq=start
    foc1=start+1
    foc2=start+nfoc
    printf '%i5 %i5 %i3 %i3-%i3' pa loc acq foc1 foc2 >>./map.dat
    start=start+nfoc+1
  end_do
end_do
end
