parameter ired igreen iblue

printf 'Images should already be loaded beforing continuing...'
printf 'Enter C to continue...'
pause

call ireadbuf ired
red=ireadbuf
call ireadbuf igreen
green=ireadbuf
call ireadbuf iblue
blue=ireadbuf

string outfile '?Enter output root file name: '

ask 'Enter 1 to align images, 0 to use existing alignment: ' align
if align==1
  tv $red
  printf 'Mark a star with the C key in image display, E to exit'
  mark new
  r1=r c1=c
  tv $green
  printf 'Mark same star with the C key in image display, E to exit'
  mark new
  r2=r c2=c
  tv $blue
  printf 'Mark same star with the C key in image display, E to exit'
  mark new
  r3=r c3=c
  shift $green dr=int[r1-r2] dc=int[c1-c2]
  shift $blue dr=int[r1-r3] dc=int[c1-c3]
end_if

ask 'Enter 1 for auto-sky subtraction, 2 for manual, 0 for no subtraction: ' |
   subsky
if subsky>0
  unmask
  clip $red maskonly min=0
  daosky $red mask
  if subsky==2
    tv $red
    ask 'Red sky: ' sky
  else_if subsky==3
    box 1 int
    daosky $red box=1
  end_if
  rsky=sky

  unmask
  clip $green maskonly min=0
  daosky $green mask
  if subsky==2
    tv $green
    ask 'Green sky: ' sky
  else_if subsky==3
    box 1 int
    daosky $green box=1
  end_if
  gsky=sky

  unmask
  clip $blue maskonly min=0
  daosky $blue mask
  if subsky==2
    tv $blue
    ask 'Blue sky: ' sky
  else_if subsky==3
    box 1 int
    daosky $blue box=1
  end_if
  bsky=sky
else
  rsky=0
  bsky=0
  gsky=0
end_if

rs=sr[red]
rs=max[rs,sr[green]]
rs=max[rs,sr[blue]]
cs=sc[red]
cs=max[cs,sc[green]]
cs=max[cs,sc[blue]]
re=sr[red]+nr[red]-1
re=min[re,sr[green]+nr[green]-1]
re=min[re,sr[blue]+nr[blue]-1]
ce=sc[red]+nc[red]-1
ce=min[ce,sc[green]+nc[green]-1]
ce=min[ce,sc[blue]+nc[blue]-1]
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

abx $red high=rmax
rmax=rmax-rsky
abx $green high=gmax
gmax=gmax-gsky
abx $blue high=bmax
bmax=bmax-bsky
allmax=max[rmax,gmax]
allmax=max[allmax,bmax]
allmax=min[allmax,5000]

create 101 box=1
fits 101 int=naxis 3
fits 101 int=naxis3 3
fits 101 float=datamin 0.
fits 101 float=datamax 256.

printf 'Enter scale factors for red green and blue.'
printf 'Bigger numbers make that color brighter. '
printf 'To make the whole image fainter, use small numbers for all three.'
ask 'Red scale: ' rscale
ask 'Green scale: ' gscale
ask 'Blue scale: ' bscale

wd 101 ./head headonly zero=0 scale=1

copy 101 $red box=1
sub 101 c=rsky
mul 101 c=256/allmax*rscale
clip 101 min=-32766 max=32766
wd 101 ./red nohead notail zero=0 scale=1

copy 101 $green box=1
sub 101 c=gsky
mul 101 c=256/allmax*gscale
clip 101 min=-32766 max=32766
wd 101 ./green nohead notail zero=0 scale=1

copy 101 $blue box=1
sub 101 c=bsky
mul 101 c=256/allmax*bscale
clip 101 min=-32766 max=32766
wd 101 ./blue nohead notail zero=0 scale=1

$cat head.fits red.fits green.fits blue.fits > color.fits
$fitstopnm color.fits > color.pnm

$convert -flip color.pnm {outfile}.jpg

printf '{outfile}.jpg %3i5 %3f8.3' red green blue rscale gscale bscale |
  >>./color.log
printf '{outfile}.jpg %3i5 %3f8.3' red green blue rscale gscale bscale |
  >>./{outfile}.log

printf 'Image in JPEG format can be found in file {outfile}.jpg'
printf 'Scaling parameters logged in {outfile}.log and color.log'
printf 'Use xv command to view these files from another window..'

end
