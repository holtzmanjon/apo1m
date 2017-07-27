do alt=75,35,-10
  call altaz 180 alt
  call coord 1
  call getfoc focus 1 0 0 5
  send 'window 750 1450 650 1350'
  call tiltrun 0.06 0.02 0 0.02
  printf 'call tilt %i' sfoc >>./tlook.pro
  send 'window full'
end_do
end

