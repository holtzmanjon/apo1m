! Procedure to read in one or more images into buffer(s) which correspond 
!   (mod 100) to input frame number(s), and do bias subtraction
! Buffer used for last frame will be in variable ireadbuf
! If display==1, display image(s) when done
! If rend is not specified, only one image (rstart) will be read
! iext is used for MOSAIC images (idet=20). If iext=-10, then crude binned
!    frame will be read (extension 's'). If iext is between -1 and -8, then 
!    the single specified (abs[iext]) chip will be read from the full
!    MOSAIC extension file. If iext is between 1 and 8, then the single chip
!    will be read from an individual processed files (extension _%i1)

parameter rstart rend display headonly iext
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
  call getfile iread iext
  ! string file '{root}.%i3.3' iread
  ! string filetype ' '
  if idet==20&iext==0
    iread1=1
    iread2=8
  else_if idet==20
    iread1=abs[iext]
    iread2=abs[iext]
  else
    iread1=1
    iread2=1
  end_if
  do iiread=iread1,iread2

    if idet==20&iext==0
      ireadbuf=iiread
      rd $ireadbuf '{file}' {filetype}  {headonly} nim=iiread
      unfit $ireadbuf card=extname
      unfit $ireadbuf card=extver
      if iiread>4
        flip $ireadbuf cols
      end_if
    else
      ireadbuf=iread
      n=ifix[ireadbuf/100]
      if (mod[ireadbuf,100]==0)
        n=n-1
      end_if
      ireadbuf=ireadbuf-n*100
      if idet==20&iext<0
        rd $ireadbuf '{file}' {filetype}  {headonly} nim=abs[iext]
        unfit $ireadbuf card=extname
        unfit $ireadbuf card=extver
      else
        rd $ireadbuf '{file}' {filetype}  {headonly}
      end_if
      if iiread>4
        flip $ireadbuf cols
      end_if
    end_if
!  gettime {ireadbuf:date-obs}
!  if (hh>90)
!    string date '%i2.2/%i2.2/%i2.2' ss mm hh
!    fits $ireadbuf char=date-obs {date}
!  end_if
!tv $ireadbuf
  do ibias=1,nbias
    string biassr 'biassr%i1' ibias
    string biassc 'biassc%i1' ibias
    string biasnr 'biasnr%i1' ibias
    string biasnc 'biasnc%i1' ibias
    string imsr 'imsr%i1' ibias
    string imsc 'imsc%i1' ibias
    string imnr 'imnr%i1' ibias
    string imnc 'imnc%i1' ibias
    biassr=max[{biassr},sr[ireadbuf]]
    biassc=max[{biassc},sc[ireadbuf]]
    biaser=min[{biassr}+{biasnr}-1,er[ireadbuf]]
    biasec=min[{biassc}+{biasnc}-1,ec[ireadbuf]]
    biasnr=biaser-biassr+1
    biasnc=biasec-biassc+1
    imsr=max[{imsr},sr[ireadbuf]]
    imsc=max[{imsc},sc[ireadbuf]]
    imer=min[{imsr}+{imnr}-1,er[ireadbuf]]
    imec=min[{imsc}+{imnc}-1,ec[ireadbuf]]
    imnr=imer-imsr+1
    imnc=imec-imsc+1
    if biasnr>0&biasnc>0 
      box 2 nr=biasnr nc=biasnc sr=biassr sc=biassc
      box 1 nr=imnr nc=imnc sr=imsr sc=imsc
      ! abx $ireadbuf 2 mean=b silent
      if biasramp==0
        daosky $ireadbuf box=2 3sig
        b=sky
        string comment |
          'Subtracting bias value of : %f10.3 from [%i4,%i4:%i4,%i4]' |
          b {imsc} {imsc}+{imnc}-1 imsr imsr+imnr-1
        printf '\n {comment}'
        fits $ireadbuf char=comment  '{comment}'
        error continue
        sub $ireadbuf c=b box=1
      else
        mash 101 $ireadbuf sp=biassc,biassc+biasnc-1 col norm
        if biasramp>0
          zap 101 size=biasramp sig=0
        end_if
        stretch 102 101 size={imnc} start={imsc} horiz
        string comment |
          'Subtracting bias ramp from [%i4,%i4:%i4,%i4]' |
          {imsc} {imsc}+{imnc}-1 imsr imsr+imnr-1
        printf '\n {comment}'
        fits $ireadbuf char=comment  '{comment}'
        sub $ireadbuf 102
      end_if
    end_if
!tvbox box=1
!tvbox box=2
!pause
    ! For quad readout paste image back together without central overscan
    if nbias==4
      copy $ibias+100 $ireadbuf box=1
      if ibias==2|ibias==4
        shift $ibias+100 dc=-over
      end_if
    end_if
  end_do

  if nbias==4
!  Now paste images together, but preserve original header
    box 1 nr=nrow nc=ncol sr=srow sc=scol
    window $ireadbuf box=1
    sub $ireadbuf $ireadbuf
    do ibias=1,4
      add $ireadbuf $100+ibias
    end_do
  end_if
  if idet==20
      if iiread>4
        flip $ireadbuf cols
      end_if
  end_if

  if idet==17
    unmask
    clip $ireadbuf min=-1000 maskonly
    masktoim 101
    sub 101 c=1
    mul 101 c=-65536
    add $ireadbuf 101
  end_if

  if display>0&headonly==0
    tv $ireadbuf
  end_if
  end_do
  printf '----------------------------------------------------------------'
end_do
END
