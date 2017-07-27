ask start
ask end
ask outbuf
box 1 n=200 cr=512 cc=512
string im ' '
do i=start,end
  string file '{root}.%i3.3' i
  rd $i ./{file}
  abx $i 2 mean=b
  sub $i c=b
  abx $i 1 mean=m
  div $i c=m
  string im '{im} %i2' i
end_do
median $outbuf {im} nomean
do i=start,end
  div $i $outbuf
end_do
end
