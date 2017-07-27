im=5
im=73
im=184
nbox=50
create 901 nr=7*4*nbox nc=7*nbox
do k=1,7
  create $800+k n=1024 sr=-512 sc=-512
end_do
box 9 nr=nbox nc=nbox cr=165/2 cc=192/2
jj=0
!do i=0,360,60
do i=300,345,45
! do j=0,900,300
! do j=300,1200,900
 do j=300,1200,300
  jj=jj+1
  im=im+1
  do k=1,7
    call readg im
    copy 900 $ireadbuf
    window 900 box=9
    fixhead 900 origin
    add 901 900 dr=(jj-1)*nbox dc=(k-1)*nbox

    align 900 dsp=0.52/0.455
    fixhead 900 origin
    rot={ireadbuf:obs_rot}
    rotate 900 pa=-rot
    fixhead 900 origin
    loc=j
    x=loc*cosd[rot]/4
    y=loc*sind[rot]/4
    add $800+k 900 dr=y-nbox/2 dc=x-nbox/2

    im=im+1
  end_do
 end_do
end_do


copy 902 901
align 902 dsp=0.52/0.455

tv 902 100.
ii=1
do i=0,360,60
  tvplot p=ii,sc[902] p=ii,ec[902]
  ii=ii+4*nbox
end_do
 
end
