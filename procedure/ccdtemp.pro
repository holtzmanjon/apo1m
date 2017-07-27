parameter force
! Check for proper CCD set point
call readstat
temp=outtemp
if temp>25
  atemp=-25
  ftemp=-25
else_if temp>20
  atemp=-30
  ftemp=-25
else_if temp>15
  atemp=-35
  ftemp=-25
else_if temp>10
  atemp=-40
  ftemp=-30
else_if temp>5
  atemp=-45
  ftemp=-35
else
  atemp=-50
  ftemp=-40
end_if
string com 'gsettemp %i' ftemp
send '{com}'
if scidet==17
  string com 'settemp %i' atemp
  send '{com}'
else_if scidet==32
  call checkfill force
end_if
end
