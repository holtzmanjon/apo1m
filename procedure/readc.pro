! Procedure to read in one or more images into buffer(s) which correspond 
!   (mod 100) to input frame number(s), and do bias subtraction
! Buffer used for last frame will be in variable ireadbuf
! If display==1, display image(s) when done
! If rend is not specified, only one image (rstart) will be read

parameter rstart rend display headonly
if rend==0
  rend=rstart
end_if

if headonly==1
  string headonly 'headonly'
else
  string headonly ' '
end_if

! Do we have root name?
err call getroot
string tmp {root}
! Do we know the detector type?
err call getdet
rrr=idet

! Loop over all requested images
do iread=rstart,rend
  !call getfile iread
  string file '{root}c.%i3.3' iread
  string filetype ' '
  ireadbuf=iread
  n=ifix[ireadbuf/100]
  if (mod[ireadbuf,100]==0)
    n=n-1
  end_if
  ireadbuf=ireadbuf-n*100
  rd $ireadbuf '{file}.fits' {filetype}  {headonly}

  binx={ireadbuf:cdelt1}
  biny={ireadbuf:cdelt2}
  if binx==2&biny==2
    replicate $ireadbuf bin=2
  end_if
  if display>0&headonly==0
    tv $ireadbuf
  end_if
  printf '----------------------------------------------------------------'
end_do
END
