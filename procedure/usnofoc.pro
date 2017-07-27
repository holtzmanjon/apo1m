parameter ratarg dectarg

if ratarg==0
  call readstat 1
end_if
string com 'square %f12.3 %f12.3 0.813333 %f7.1' ratarg dectarg epoch
${com} >usno/focus.usno

open infoc usno/focus.usno
stat n=count[infoc]
read infoc
read infoc
read infoc
read infoc
read infoc
nextfoc:

read infoc
m=@infoc.7
if ((m<10.45)|(m>11.55))
  goto nextfoc
end_if

lrah=@infoc.1+@infoc.2/60+@infoc.3/3600
!ldecd=abs[@infoc.4]+@infoc.5/60+@infoc.6/3600
!ldecd=ldecd*(@infoc.4/abs[@infoc.4])
string ldecd '{infoc.-4}'
string decm '{infoc.-5}'
string decs '{infoc.-6}'
string dec {ldecd}:{decm}:{decs}
getcoord {dec}
ldecd=sg*(abs[hh]+mm/60+ss/3600)
epoch=2000

string com 'square %f12.6 %f12.6 0.006 %f7.1' lrah ldecd epoch
${com} >usno/neigh.usno
open neigh usno/neigh.usno
stat neigh=count[neigh]
neigh=neigh-6
type neigh
if neigh>0
  goto nextfoc
end_if

close infoc

string command 'NE %f8.1' epoch
send '{command}'
string command 'CO %3f12.6' lrah ldecd pa
printf 'echo {command} >>master2com' >./usnofoc.inp
printf 'echo "Y" >>master2com ' >>./usnofoc.inp
printf 'echo {command} >>master2com'
printf 'echo "Y" >>master2com '
$csh usnofoc.inp
call get

!string com 'square %f12.3 %f12.3 0.813333 %f7.1' lrah ldecd epoch
!${com} >usno/focus.usno

end

