parameter start end spe thresh string=name

$'rm' {name}.dat
if spe==1
  string root 't'
else
  string root '971126
end_if

do i=1,9
  string var 'sum%i2.2' i
  {var}=0
  string var 'sum2%i2.2' i
  {var}=0
end_do
nobs=0
do i=start,end
  if spe==1
    string file '{root}%i7.7' i
    string file '{file}.spe'
  else
    string file '{root}.%i3.3' i
  end_if
  err goto nextim
  rd 1 '{file}' maxtry=0
  if (i==start|spe==1)
    automark 1 new range=thresh,100000 radius=3
  else
    automark 1 auto
  end_if
  err goto nextim
  save coo=./junk.coo sr=0 sc=0
  open input ./junk.coo
  stat n=count[input]
  do j=1,3
    read input
  end_do
  string outvar ' '
  do j=4,n
    read input
    string var 'x%i2.2' j-3
    {var}=@input.2
    string outvar '{outvar} {var}'
    string var 'y%i2.2' j-3
    {var}=@input.3
    string outvar '{outvar} {var}'
    string var 'sum%i2.2' j-3
    {var}={var}+sqrt[(@input.2)^2+(@input.3)^2]
    string var 'sum2%i2.2' j-3
    {var}={var}+(@input.2)^2+(@input.3)^2
  end_do
  nobs=nobs+1
  nn=min[(n-3),4]
  string nstr '%i1' nn*2
  printf '%i6 %{nstr}f8.2' i {outvar} >>./{name}.dat
  close input
  nextim:
  get coo=./junk.coo sr=0 sc=0 
end_do
do i=1,nn
  string sum 'sum%i2.2' i
  string sum2 'sum2%i2.2' i
  string rms 'rms%i2.2' i
  {rms}=sqrt[({sum2}-{sum}^2/nobs)/nobs]
  printf '%i6 %f8.3' i {rms}
end_do
end


