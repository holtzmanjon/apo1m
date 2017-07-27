parameter string=obj string=filt auto

err goto nodark
rd 207 /home/1m/cal/dark_050 maxtry=0
darkbuf=207
goto getflat
nodark:
darkbuf=0

getflat:
err goto noflat
rd 209 /home/1m/cal/flat{filt} maxtry=0
flatbuf=209
goto getsky
noflat:
flatbuf=0

getsky:
err goto nosky
rd 208 /home/1m/cal/sky5{filt} maxtry=0
!rd 208 /home/1m/cal/sky{filt} maxtry=0
skybuf=208
goto doit
nosky:
skybuf=0

doit:
setup apo
open input /home/1m/{obj}/{obj}.{filt}
stat nepoch=count[input]
ntot=0
do j=1,nepoch
 read input
 i1=@input.1 i2=@input.2
 ntot=ntot+(i2-i1+1)
end_do
close input

open input /home/1m/{obj}/{obj}.{filt}
nframes=0
nep=0
do j=1,nepoch
 string line {input}
 i1=@input.1 i2=@input.2
 if i1<0 
   goto nextline
 end_if
 string root '{input.-3}'
 setdir im dir=/home/backupb/1m/{root}
 idet=17
 call getdet idet
 
 do i=i1,i2

  iframe=i
  idate={root}
  string file '{root}_%i3.3' i

  call reduce i 0 flatbuf 0 1 0 skybuf darkbuf
  string out '{root}.%i3.3' i
  wd $ireadbuf ./{out}.fits full

 nextfram:
 end_do
end_do
close input

end
