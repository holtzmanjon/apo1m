parameter buf x y
box 1 n=50 cr=y cc=x
!create 999 box=1 const=1
!copy 901 999
!add 901 901 dr=1
!add 901 const=sr[999]-1
!copy 902 999
!add 902 902 dc=1
!add 902 const=sc[999]-1
!pause
!mul 901 $buf
!mul 902 $buf
!abx $buf 1 total=tot
!abx 901 total=ytot
!abx 902 total=xtot
!xcen=xtot/tot
!ycen=ytot/tot

axes $buf box=1
abx $buf 1 high=high
automark $buf box=1 range=high-5,high+10 
save coo=./junk
$head -3 junk.coo >./hfd.coo
printf '     1 %2f8.2' axc axr >>./hfd.coo

daofiles coo=./hfd mag=./hfd pro=./hfd
photom $buf rad=-12,2,2 skyrad=50,60
shortap

end
