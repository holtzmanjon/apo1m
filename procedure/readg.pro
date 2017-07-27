parameter rstart rend display
if rend==0
  rend=rstart
end_if
err call getroot
string tmp {root}
err call getdet
idet=guidedet
call getdet idet
do iread=rstart,rend
  if iread<1000
    string file '{root}g.%i3.3' iread
  else
    string file '{root}g.%i4.4' iread
  end_if
  ireadbuf=iread
  n=ifix[ireadbuf/100]
  if (mod[ireadbuf,100]==0)
    n=n-1
  end_if
  ireadbuf=ireadbuf-n*100
  rd $ireadbuf '{file}.fits'
  error goto nobias
  abx $ireadbuf 2 mean=b silent
  printf '\n Subtracting bias value of : %f10.3' b
  sub $ireadbuf c=b
  if idet==33
    copy 999 $ireadbuf
    smooth 999 fw=100 runmean
    sub $ireadbuf 999
  end_if
  nobias:
  if display>0
    tv $ireadbuf
  end_if
  printf '----------------------------------------------------------------'
end_do
END
