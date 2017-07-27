call readstat 1

send 'zdisable'
do i=1,5
  call altaz az 30
  call altaz az 80
end_do
send 'zenable'

end
