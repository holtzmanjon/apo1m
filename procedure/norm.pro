parameter rstart rend
if rend==0
  rend=rstart
end_if
do iread=rstart,rend
  ireadbuf=iread
  n=ifix[ireadbuf/100]
  if (mod[ireadbuf,100]==0)
    n=n-1
  end_if
  ireadbuf=ireadbuf-n*100
  ! abx $ireadbuf 10 mean=b silent
typ ireadbuf
  daosky $ireadbuf box=10 
  b=sky
  printf '\n Normalizing by mean value of : %f10.3' b
  div $ireadbuf c=b
  printf '----------------------------------------------------------------'
end_do
END

