parameter junk
ask 'Number of images to stack (Enter 0 to read file list): ' nstack
if nstack==0
  string file '?Enter file name with file list: '
  open input ./{file}
  stat nim=count[input]
else
  string root '?Enter observation date (.e.g, 001114): '
  nim=nstack
end_if
do i=1,nim
  if nstack==0
    string line {input}
    im=@input.1
    string root '{input.-2}'
   ! string root '{input.-3}'
  else
    string prompt 'Enter number of image %i3: ' i
    ask '{prompt}' im
  end_if
  load im {root}
  call ireadbuf im
  string file '{root}_%i3.3' im
  im=ireadbuf
  err goto doit
  open dat ./{file}.stack
  read dat
  r=@dat.1
  c=@dat.2
  close dat
  goto shift

  doit:
  tv $im
  printf 'Mark a star with the C key in image display, E to exit'
  mark new
  printf '%2f8.3' r c >./{file}.stack

  shift:
  box 1 n=150 cr=r cc=c
  axes $im box=1
  if i==1
    r0=r c0=c
    copy 201 $im
    addbuf=201
    rs=sr[im]
    cs=sc[im]
    re=sr[im]+nr[im]-1
    ce=sc[im]+nc[im]-1
  else
    add $addbuf $im dr=r0-r dc=c0-c
    shift $im dr=r0-r dc=c0-c
    rs=max[rs,sr[im]]
    cs=max[cs,sc[im]]
    re=min[re,sr[im]+nr[im]-1]
    ce=min[ce,sc[im]+nc[im]-1]
  end_if
  copy $110+i $im

end_do

printf 'Summed image is in buffer: %i3 ' addbuf

if mod[re-rs+1,2]==0
  rn=re-rs+1
else
  rn=re-rs
end_if
if mod[ce-cs+1,2]==0
  cn=ce-cs+1
else
  cn=ce-cs
end_if
box 1 nr=rn nc=cn sr=rs sc=cs
!create 210 box=1
copy 210 111
fixhead 210 origin
rs=sr[210]
cs=sc[210]
ce=sc[210]+nc[210]-1

ndim=8
create 202 nc=ndim*nc[210] nr=(ifix[nim/ndim]+1)*nr[210] sr=sr[210] sc=sc[210]

do i=1,nim
!  window $110+i box=1
  fixhead $110+i origin
  dc=mod[i-1,ndim]*nc[210]
  dr=ifix[(i-1)/ndim]*nr[210]
  add 202 $110+i dr=dr dc=dc
!  hjd $110+i
!  hjd={110+i:hjd}-2440000
!  string hjd '%f12.3' hjd
!  tv $110+i box=1 noerase
!  r=rs+10
!  c=(cs+ce)/2.
!  tvplot text=hjd c=r,c
end_do

tv 202
do i=1,nim
  dc=mod[i-1,ndim]*nc[210]
  dr=ifix[(i-1)/ndim]*nr[210]
  hjd $110+i
  hjd={110+i:hjd}-2440000
  date=1100+ifix[hjd-11876+28]
  obsnum={110+i:obsnum}
  string hjd '%i6.6 %i3.3 %f9.3' date obsnum hjd
  !string hjd '%f12.3' hjd
  r=rs+10
  !c=(cs+ce)/2.
  c=cs
  tvplot text=hjd c=r+dr,c+dc
end_do

printf 'Summed image is in buffer: %i3 ' addbuf

end 
