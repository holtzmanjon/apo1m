parameter start
 
im=start
do i=1,100000000
  string file 't%i7.7' im
  err goto wait
  rd 1 /home/export/spec/{file}.spe
  tv 1 noerase
  goto next
 
  wait:
  $sleep 1
  goto retry
 
  next:
  im=im+1
 
  retry:
end_do
 
end
