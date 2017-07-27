parameter nspin
call readstat 1
if domeslav==1
  send 'DM'
end_if
do i=1,nspin
  send 'DI'
  send 'XDOME 85'
end_do
send 'DM'

end
