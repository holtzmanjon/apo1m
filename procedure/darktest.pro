! 300s darks and/or dome "flats" at a variety of temperatures

dodark=1
doflat=0
$power1m off lights
do temp=-50,-30,5
  call readstat
  if outtemp<(temp+55)
    string com 'settemp %i5' temp
    send '{com}'
    $sleep 300
    if dodark==1
      string out 'dark300_%i2.2' abs[temp]
      call biasdark 300 5 {out}
    end_if
    if doflat==1
      $power1m on lights
      send 'mexp 1 5'
      $power1m off lights
    end_if
  end_if
end_do

end

