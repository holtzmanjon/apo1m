call altaz 220 70
call coord 1

send 'inst 2'
!send 'guideloc 0'
call getfoc focus 0 2

saoa 220 70

send 'g+display'
ver y
send 'guidefoc 500'
send 'gexp 5'
send 'guidefoc -500'
send 'gexp 5'

n=2
sign=-1
do i=1,n
  send 'df 100'
  send 'guidefoc 100'
  send 'gexp 5'
  string com 'guidefoc %i5' sign*1000
  send '{com}'
  send 'gexp 5'
  sign=-1*sign
end_if
string com 'df %i4' -100*n
send {com}

send 'g-display'
ver n

call altaz 220 70
send 'guidehome'
send 'inst 1'
call getfoc focus 1

end

