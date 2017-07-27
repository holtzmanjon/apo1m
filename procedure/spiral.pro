parameter offset guider itime dopause
nbox=1
if offset==0 
  offset=2100
end_if
if itime==0
  itime=5
end_if
if guider==1 
  string prefix 'g'
  string offset 'guideinst 2'
else
  string prefix ''
  string offset 'offset'
end_if
string expcom '{prefix}exp %i' itime

sign=1
totx=0
toty=0
again:
do i=1,nbox
 dx=sign*offset dy=0
 string com '{offset} %2i6' dx dy
 totx=totx+sign*offset
 send '{com}'
 printf 'total move: %2i6' totx toty
 $sleep 3
 call readccd
 send {expcom}
 call read incval
 abx $ireadbuf high=high 
 if high>40000 then
   pause
 end_if
 printf 'total move: %2i6' totx toty
end_do
do i=1,nbox
 dy=sign*offset dx=0
 toty=toty+sign*offset
 string com '{offset} %2i6' dx dy
 send '{com}'
 printf 'total move: %2i6' totx toty
 $sleep 6
 call readccd
 send {expcom}
 call read incval
 abx $ireadbuf high=high 
 if high>40000 then
   pause
 end_if
 printf 'total move: %2i6' totx toty
end_do
sign=sign*-1
nbox=nbox+1
goto again
end

