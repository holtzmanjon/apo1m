open coo ./focus.coo
stat n=count[coo]
nstars=ifix[n/(end-start+1)]
read coo
read coo
read coo
do i=start,end
  string var 'f%i2.2' i-start+1
  string foctext '%f8.1' {var}
  do istar=1,nstars
    read coo
    x=@coo.2+10
    y=@coo.3+(i-start)*30
    tvplot p=y,x text=foctext
  end_do
end_do
close coo
end
