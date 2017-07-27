ask start
ask end

do i=start,end
  get coo=./junk sr=0 sc=0
  automark $i auto
  string rvar 'dr%i3.3' i
  {rvar}=dr
  string cvar 'dc%i3.3' i
  {cvar}=dc
  sky $i 
  sub $i c=sky
  psffit $i load
  add $i c=sky
  string var 'sx%i3.3' i
  {var}=p4
  string var 'sy%i3.3' i
  {var}=p5
end_do

do i=start,end
  string rvar 'dr%i3.3' i
  string cvar 'dc%i3.3' i
  string xvar 'sx%i3.3' i
  string yvar 'sy%i3.3' i
  printf 'Image: %i3 {i:ut} dr: %f6.2 dc: %f6.2  FWHM(x):%f8.2 FWHM(y):%f8.2' |
        i {rvar} {cvar} {xvar}*2.354*0.81 {yvar}*2.354*0.81
end_do

end
  

