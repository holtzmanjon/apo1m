do i=0,360,60
  string com 'pa %i' i
  send '{com}'
  $sleep 5
  send 'gexp 1'
pause
end_do
end
