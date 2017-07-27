parameter niter
do j=1,niter
 do i=2,7
  az=mod[i,6]*60
  string com 'xdome %i5' az
  printf '{com}'
  send '{com}'
  $sleep 600
 end_do
end_do
send 'di'
end
