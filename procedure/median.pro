parameter i1 i2 norm dosbias string=sbfile nointer

$'rm' median.inp
do i=i1,i2
  call read i 
  if norm==1
    daosky $ireadbuf box=10
    mean=sky
  else
    mean=1
  end_if
  printf '{file}%2f12.3' mean b >>./median.inp
end_do

if dosbias==1
  string sbiasmed 'bias={sbfile}'
  rd 109 {sbfile}
else
  string sbiasmed ' '
end_if
bigmedian 101 list=./median.inp {sbiasmed}

if nointer==0
do i=i1,i2
  call read i
  if dosbias==1
    sub $ireadbuf 109
  end_if
  if norm==1
    daosky $ireadbuf box=10
    div $ireadbuf c=sky
  end_if
  div $ireadbuf 101
  tv $ireadbuf z=0.9 l=0.2
end_do
end_if
end

