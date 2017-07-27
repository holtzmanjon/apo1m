! Procedure to combine flat field exposures
!   NOTE:  string root must be predefined
! Parameters:   start - start image number
!               end - end image number
!               buf - output buffer number
!               niter = 0 for median, otherwise combine with piccrs
!                      1 for one iteration, 2 for two ...
parameter start end buf dt
$'rm' /usr/tmp/temp.lis
$'rm' /usr/tmp/flat.lis
unalias rd
unalias wd

string im ' '
do i=start,end
  n=ifix[i/100]
  if (mod[i,100]==0)
    n=n-1
  end_if
  call read i 
  if dt>0
    call fixdt dt
  end_if
  unmask
  string filt 'filt{ireadbuf:filter}'
  if i==start
    string filt0 {filt}
    {filt}=-999
  end_if
  err goto nextim
  nfilt={filt}
  if {filt}~=nfilt
    goto nextim
  end_if
  clip $ireadbuf max=maxbad min=-50 maskonly
  masktoim 101 nr=nr[ireadbuf] nc=nc[ireadbuf] sr=sr[ireadbuf] sc=sc[ireadbuf]
  abx 101 total=tot
  if (tot/(nr[ireadbuf]*nc[ireadbuf])>0.75)
    daosky $ireadbuf box=10 3sig
    div $ireadbuf c=sky
    string im '{im} %i3' ireadbuf
  end_if
  nextim:
end_do

printf 'median $buf {im} nomean'
median $buf {im} nomean
abx $buf 10 mean=m silent
div $buf c=m

if dt<0
  goto flatend
end_if

do i=start,end
  n=ifix[i/100]
  if (mod[i,100]==0)
    n=n-1
  end_if
  ireadbuf=i-n*100
  string filt 'filt{ireadbuf:filter}'
  err goto nextim2
  nfilt={filt}
  if {filt}~=nfilt
    goto nextim
  end_if
  div $ireadbuf $buf
  tv $ireadbuf z=0.8 l=0.4
  pause
  printf 'Hit C to continue...'
  nextim2:
end_do

flatend:
{filt0}=-998
dispose {im}

end
