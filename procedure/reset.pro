parameter date idet
notv=0 noplot=0
if date==0
  call getroot
  call getdet
else_if idet==17
  string root '%i6.6' date
  setdir im dir=../images/{root}
  idet=17
  call getdet idet
else
  string root '%i6.6' date
  call getmonth {root}
  setdir im dir=/home/apo/{month}/{root}
  if idet==0|idet==8|idet==38|idet==40|idet==42
    if date<020315
      idet=8
    else_if date<030115
      idet=38
    else_if date<030515
      idet=40
    else
      idet=42
      printf 'Need to confirm correct setup for this date!'
    end_if
  end_if
  call getdet idet
end_if

end
