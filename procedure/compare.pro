parameter start end delta
if delta==0
  delta=10
end_if
do i=start+1,end
  copy $i+delta $i
  div $i+delta $start
  call norm i+delta
  tv $i+delta z=.95 l=.1
end_do
end
