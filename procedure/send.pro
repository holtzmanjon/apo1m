parameter string=command gstart get

if get==0
  printf 'sending command: {command}'
  $ echo "{command}" >> master2com
end_if

if gstart>0
  gstart=gstart+1
  gmon:
  string file 't%i7.7' gstart
  err goto wait
  open junk /home/export/spec/{file}.spe
  close junk
  string file 't%i7.7' gstart-1
  rd 199 /home/export/spec/{file}.spe maxtry=0
  daosky 199
  tv 199 z=sky-3*skysig l=20*skysig noerase ncolor=16

  $echo "DONE: -1" >> com2master
  read com2master
  ret=@com2master.2
  if ret==0
    goto done
  end_if

  goto next

  wait:
  $sleep 1
  goto gmon

  next:
  gstart=gstart+1
  goto gmon
end_if
 
done: 
status=1
retry:
! keep reading from pipe until the first word is STATUS
! return value stored as val
string return '{com2master}'
printf '{return}'
val={return.2}
string cval '{return.2}'
string first {return}
printf '{first}' >./response.dat
$sed 's/://' response.dat > response2.dat
open input ./response2.dat
string first {input}
close input
err goto retry
a={first}
if {first}~=status
  goto retry
end_if
!call tocom
if val==-999
  curobj=0 objerr=1
  string ret '1'
  call writesum 1 'GRB ALERT COMPLETED'
else
  string ret ' '
end_if
return {ret}
end
