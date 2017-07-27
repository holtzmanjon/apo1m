! Procedure to set up appropriate detector parameters
! Sets:  jdet - detector code, if 0 then on exit idet 
!               is set to chosen code
!        nrow, ncol, srow, scol, over
!        rn, gain
!        sbias=1 for superbias, else sbias=0
!        box 2 = overscan
!        box 10 = normalization region
!        box 11 = full frame without overscan
!        box 12 = full frame with overscan
!        nbadi=no. of bad boxes to interpolate (boxes 13:13+nbadi-1)
!        nbad=no. of bad boxes set to blank (boxes 13+nbadi:13+nbadi+nbad-1)
parameter jdet
!idet=jdet
if jdet==0 
  printf 'Enter appropriate detector: '
  printf '      1:   IFPS / NCCD 800x800 windowed to 650x650'
  printf '      2:   DCCD / Loral 800x800'
  printf '      3:   DCCD / Loral binned 2x2'
  printf '      4:   SPICA CCD / Loral binned 2x2'
  printf '      5:   IRIM '
  printf '      6:   COB '
  printf '      7:   OSIRIS '
  printf '      8:   DIS blue '
  printf '      9:   DIS red '
  printf '     38,40,42:   new DIS blue '
  printf '     39,41,43:   new DIS red '
  printf '     10:   1m Princeton 1024x1024 '
  printf '     11:   SPICAM 2048x2048, binned to 1024x1024 '
  printf '     12:   Spectrasource 512x512'
  printf '     13:   CTIO 2048x2048, quad amps, 1.5m f/13.5'
  printf '     14:   CTIO 2048x2048, quad amps, 1.5m f/8'
  printf '     15:   Spectrasource 192x165'
  printf '     16:   APO echelle 2048x2048'
  printf '     17:   Apogee 512x512'
  printf '     18:   APO PT 2048x2048'
  printf '     19:   DIS slitviewer: Apogee 512x512'
  printf '     20:   NOAO/CTIO MOSAIC 8x4096x2048, quad amps, 4m'
  printf '  21-28:   NOAO/CTIO MOSAIC 4096x2048, single chip'
  printf '     29:   NOAO/CTIO MOSAIC 8x4096x2048, binned 4x4, and crude mosaic'
  printf '     31:   UH 2.2m, 1024x2048'
  printf '     32:   Leach 2048x2048'
  printf '     33:   FLI E2V 1024x1024'
  printf '101-104:   WFPC2 PC1/WF2/WF3/WF4'
  ask    '   Detector:  ' jdet
  idet=jdet
end_if

nchan=1
string chan1 ' '
if abs[jdet]==1
  ! IFPS / NCCD 800x800 windowed to 650x650
  nrow=650
  ncol=650
  if jdet>0
    srow=141
    scol=151
  end_if
  over=32
  rn=13
  gain=1.54
  scale=0.5
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=200 nc=200 cr=400 cc=400 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+over nc=ncol+over sr=srow sc=scol    ! full sized image
  create 110 box=12 
  box 13 sr=628 nr=srow+nrow+over-628-1 sc=539 nc=1  ! part of col 539 is bad
  unmask
  clip 110 box=13 min=1 maskonly
  save mask=/usr/tmp/interp
  unmask
  if jdet>0
    box 13 sr=srow nr=nrow+over sc=799 nc=2
    clip 110 box=13 min=1 maskonly
  end_if
  save mask=/usr/tmp/blank
  maxbad=20000
  lowbad=-50
else_if jdet==2
  ! DCCD Loral 800x800
  nrow=800
  ncol=800
  srow=1
  scol=1
  over=32
  rn=9
  gain=2.3
  scale=0.35
  sbias=1
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=200 nc=200 cr=400 cc=400 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=17500
  lowbad=-50
else_if jdet==3
  ! DCCD Loral 800x800 binned 2x2 to 400x400
  nrow=400
  ncol=400
  srow=1
  scol=1
  over=32
  rn=9
  gain=2.3
  scale=0.7
  sbias=1
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=200 nc=200 cr=200 cc=200 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=17500
  lowbad=-50
else_if jdet==4
  ! SPICA CCD / Loral binned 2x2'
  nrow=400
  ncol=430
  srow=1
  scol=1
  over=20
  rn=9
  gain=2.3
  sbias=1
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=200 nc=200 cr=200 cc=225 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=17500
  lowbad=-50
else_if jdet==5
  ! IRIM
  nrow=256
  ncol=256
  srow=1
  scol=1
  over=0
  rn=32
  gain=10
  sbias=0
  nbias=0
  biasramp=0
  box 10 n=25 cr=30 cc=60
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
else_if jdet==6
  ! COB
  nrow=256
  ncol=256
  srow=1
  scol=1
  over=0
  rn=35
  gain=6.6
  sbias=0
  nbias=0
  biasramp=0
  box 10 n=25 cr=30 cc=60
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
else_if jdet==7
  ! OSIRIS
  nrow=256
  ncol=256
  srow=1
  scol=1
  over=0
  rn=32
  gain=6
  sbias=0
  nbias=0
  biasramp=0
  box 10 nr=16 nc=41 sr=236 sc=191
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  box 13 nr=10 nc=14 sr=112 sc=46
  box 14 nr=11 nc=11 sr=119 sc=58
  box 15 nr=16 nc=10 sr=121 sc=67
  unmask
  save mask=/usr/tmp/interp
  create 110 box=12
  clip 110 min=1e10 maskonly box=13
  clip 110 min=1e10 maskonly box=14
  clip 110 min=1e10 maskonly box=15
  save mask=/usr/tmp/blank
else_if abs[jdet]==8
  nchan=2
  string chan1 'b'
  string chan2 'r'
  ! DIS blue
  newdis=0
  nrow=512
  ncol=512
  srow=1
  scol=52
  over=23
  rn=16
  gain=0.99
  scale=1.086
  sbias=1
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=255 cc=325 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+over nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=48000
  lowbad=-50
! Need to transpose to wavelength along rows?
  strans=1
! Need to flip to get low wavelength at low column
  sflip=1
!  slit length and center for low wavelength at low row
!  nslit=340
!  cslit=135+170
else_if abs[jdet]==9
  nchan=2
  string chan1 'b'
  string chan2 'r'
  ! DIS red
  newdis=0
  nrow=800
  ncol=800
  srow=1
  scol=51
  over=19
  rn=16
  gain=1.98
  gain=1.44
  gain=1.49
  scale=0.605
  sbias=1
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=400 cc=500 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+over nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=48000
  lowbad=-50
! Need to transpose to wavelength along rows?
  strans=1
! Need to flip to get low wavelength at low column
  sflip=0
!  slit length and center for low wavelength at low row
!  nslit=640
!  cslit=105+320
else_if abs[jdet]==38|abs[jdet==40]|abs[jdet]==42
  nchan=2
  string chan1 'b'
  string chan2 'r'
  ! new DIS blue detector
  newdis=1
  nrow=1024
  ncol=2048
  srow=1
  scol=1
  over=50
  rn=3.9
  gain=1.79
  if abs[jdet==38]
    scale=0.6
  else
    scale=0.419
  end_if
  sbias=1
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=512 cc=1024 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+over nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=48000
  lowbad=-50
! Need to transpose to wavelength along rows?
  strans=0
! Need to flip to get low wavelength at low column
  sflip=0
!  rotation from low wavelength at low col to low wavelength at low row
!  slit length and center for low wavelength at low row
!  nslit=340
!  cslit=135+170
else_if abs[jdet]==39|abs[jdet]==41|abs[jdet]==43
  nchan=2
  string chan1 'b'
  string chan2 'r'
  ! new DIS red detector
  newdis=1
  nrow=1024
  ncol=2048
  srow=1
  scol=1
  over=50
  rn=4.9
  gain=1.74
  gain=1.89
  if abs[jdet]==39|abs[jdet]==41
    scale=0.605
  else
    scale=0.419/1.057
  end_if
  sbias=1
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=400 cc=500 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+over nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=48000
  lowbad=-50
! Need to transpose to wavelength along rows?
  strans=0
! Need to flip to get low wavelength at low column
  sflip=1
!  slit length and center for low wavelength at low row
!  nslit=640
!  cslit=105+320
else_if abs[jdet]==10
  ! Princeton 1024x1024 CCD (NMSU 1m)
  nrow=1024
  ncol=1024
  srow=1
  scol=1
  over=26
  rn=9
  gain=1.3
  scale=0.81
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=512 cc=512 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+over nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=25000
  lowbad=-50
else_if abs[jdet]==11
  ! SPICAM 2048x2048 binned 2x2 to 1024x1024
  nrow=1024
  ncol=1024
  srow=1
  scol=1
  !over=120
  over=50
  rn=5.7
  gain=3.37
  scale=0.28
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=512 cc=512 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  create 110 nr=2*nrow+100 nc=ncol+over sr=srow sc=scol
! Make these long to accomodate drift scans
  box 13 nr=2048 nc=11 sr=srow sc=scol
  unmask
  clip 110 box=13 min=1 maskonly
  box 13 nr=2048 nc=2 sr=srow sc=1010
  save mask=/usr/tmp/blank
  maxbad=50000
  lowbad=-50
else_if abs[jdet]==12
  ! Spectrasource 512x512
  nrow=512
  ncol=512
  srow=1
  scol=1
  over=10
  rn=10
  gain=3
  scale=0.68
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=512 cc=512 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=25000
  lowbad=-50
else_if abs[jdet]==15
  ! Spectrasource 192x165
  nrow=165
  ncol=192
  srow=1
  scol=1
  over=10
  rn=10
  gain=3
  scale=0.5
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=512 cc=512 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=4090
  lowbad=-50
else_if abs[jdet]==16
  ! Echelle
  nrow=2048
  ncol=2048
  srow=1
  scol=1
  over=10
  rn=7
  gain=3.8
  scale=0.79
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=300 nc=300 cr=256 cc=256 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol-1 sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=25000
  lowbad=-50
else_if abs[jdet]==17
  ! Apogee 512x512
  nrow=512
  ncol=512
  srow=1
  scol=1
  over=10
  rn=10
  gain=4
  scale=0.7988/1.000397405
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=300 nc=300 cr=256 cc=256 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol-1 sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=55000
  lowbad=-50
else_if abs[jdet]==18
  ! SDSS PT
  nrow=2048
  ncol=2048
  srow=1
  scol=1
  over=10
  rn=7
  gain=3.8
  scale=1.165
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=300 nc=300 cr=256 cc=256 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol-1 sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=30000
  lowbad=-50
else_if abs[jdet]==19
  ! DIS slitviewer
  nrow=512
  ncol=512
  srow=1
  scol=1
  over=0
  rn=10
  gain=3
  scale=0.30
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  !box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=512 cc=512 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=25000
  lowbad=-50
else_if abs[jdet]==13|abs[jdet]==14
  ! CTIO ARcon Tek 2kx2k, quad readout
  nrow=2048
  ncol=2068
  srow=1
  scol=1
  over=128
  gain=3
  rn=4
  scale=.24
  if abs[jdet]==14
   scale=0.48
  end_if
  sbias=0
  nbias=4
  biasramp=0
  ibias=0
  do iqr=0,1
    do iqc=0,1
      ibias=ibias+1
      string biassr 'biassr%i1' ibias
      string biassc 'biassc%i1' ibias
      string biasnr 'biasnr%i1' ibias
      string biasnc 'biasnc%i1' ibias
      string imsr 'imsr%i1' ibias
      string imsc 'imsc%i1' ibias
      string imnr 'imnr%i1' ibias
      string imnc 'imnc%i1' ibias
      {imsr}=srow+iqr*nrow/2
      {imnr}=nrow/2
      {imnc}=ncol/2
      {imsc}=scol+iqc*ncol/2
      {biasnr}=nrow/2
      {biasnc}=over/4
      {biassr}=srow+iqr*nrow/2
      if iqc==0
        {biassc}={imsc}+ncol/2+over/8
      else
        {imsc}={imsc}+over
        {biassc}={imsc}-3*over/8
      end_if
    end_do
  end_do
  box 10 nr=256 nc=256 cr=1024 cc=1024
  box 11 nr=2048 nc=2048 sr=1 sc=11 ! part of image to change, w/o overscan
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=60000
  lowbad=-50
else_if abs[jdet]==20
  ! NOAO MOSAIC, quad readout
  nrow=4096
  ncol=2072
  srow=1
  scol=1
  over=64
  gain=2
  rn=8
  scale=.27
  sbias=1
  biasramp=50
  nbias=1
  ibias=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=256 nc=256 cr=1024 cc=1024
  box 11 nr=4096 nc=2048 sr=1 sc=25 ! part of image to change, w/o overscan
  box 12 nr=4096 nc=2048 sr=1 sc=65 ! part of image to change, w/o overscan
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=30000
  lowbad=-50
else_if abs[jdet]==29
  ! NOAO MOSAIC, bias subracted, binned, and crude mosaiced quad readout
  nrow=2058
  ncol=2078
  srow=1
  scol=1
  over=0
  gain=2*16
  rn=8/4
  scale=0.27*4
  sbias=1
  nbias=1
  ibias=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  !box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=256 nc=256 cr=850 cc=850
  box 11 nr=nrow nc=ncol sr=1 sc=1 ! part of image to change, w/o overscan
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=30000
  lowbad=0.1
else_if abs[jdet]>20&abs[jdet]<30
  ! CTIO MOSAIC, single chip reduced and windowed
  nrow=4096
  ncol=2048
  srow=1
  scol=1
  over=0
  gain=2
  rn=8
  scale=.27
  sbias=1
  biasramp=50
  nbias=1
  ibias=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  !box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=256 nc=256 cr=1024 cc=1024
  if jdet<25
    box 11 nr=4096 nc=2048 sr=1 sc=25 ! part of image to change, w/o overscan
  else
    box 11 nr=4096 nc=2048 sr=1 sc=65 ! part of image to change, w/o overscan
  end_if
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  unmask
  create 110 box=12 
  if jdet==21
    box 13 sr=1991 nr=srow+nrow-1991-1 sc=1021-25 nc=2  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=3384 nr=srow+nrow-3384-1 sc=1471-25 nc=2  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=3919 nr=srow+nrow-3919-1 sc=753-25 nc=2  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=3801 nr=srow+nrow-3801-1 sc=152-25 nc=2  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=3534 nr=srow+nrow-3534-1 sc=304-25 nc=1  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=2172 nr=srow+nrow-2172-1 sc=341-25 nc=2  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=1990 nr=srow+nrow-1990-1 sc=1021-25 nc=3  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=3370 nr=srow+nrow-3370-1 sc=1056-25 nc=3  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=1206 nr=srow+nrow-1206-1 sc=1589-25 nc=3  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
    box 13 sr=1844 nr=srow+nrow-1844-1 sc=1736-25 nc=3  ! part of col 539 is bad
    clip 110 box=13 min=1 maskonly
  else_if jdet==22
  else_if jdet==23
  else_if jdet==24
  else_if jdet==25
  else_if jdet==26
  else_if jdet==27
  else_if jdet==28
  end_if
  save mask=/usr/tmp/blank
  save mask=/usr/tmp/blank
  maxbad=30000
  lowbad=-50
else_if abs[jdet]==31
  ! UH2.2 1024x2048
  nrow=2048
  ncol=1024
  srow=1
  scol=1
  !over=120
  over=30
  rn=5
  gain=0.5
  scale=0.275
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=100 nc=100 cr=1024 cc=512 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  create 110 nr=2*nrow+100 nc=ncol+over sr=srow sc=scol
! Make these long to accomodate drift scans
  box 13 nr=2048 nc=11 sr=srow sc=scol
  unmask
  clip 110 box=13 min=1 maskonly
  box 13 nr=2048 nc=2 sr=srow sc=1010
  save mask=/usr/tmp/blank
  maxbad=28000
  lowbad=-50
else_if abs[jdet]==32
  ! Leach at 1m
  nrow=2048
  ncol=2048
  srow=1
  scol=1
  over=100
  rn=7
  gain=1
  scale=0.467
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=500 nc=500 cr=1024 cc=1024 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol-1 sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=65000
  lowbad=-50
else_if abs[jdet]==33
  ! FLI E2V1K
  nrow=1024
  ncol=1064
  srow=1
  scol=1
  over=8
  rn=7
  gain=1
  scale=0.437
  sbias=0
  nbias=1
  biasramp=0
  biassr1=srow+25 biasnr1=nrow-50 biassc1=scol+ncol+over/4 biasnc1=over/2
  imsr1=srow imnr1=nrow imsc1=scol imnc1=ncol
  box 2 nc=biasnc1 nr=biasnr1 sr=biassr1 sc=biassc1
  box 10 nr=500 nc=500 cr=1024 cc=1024 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol-1 sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow+100 nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  save mask=/usr/tmp/blank
  maxbad=65000
  lowbad=-50
else_if abs[jdet]==101
  ! WFPC2 PC1
  nrow=800
  ncol=800
  srow=1
  scol=1
  over=0
  rn=6
  gain=14
  scale=0.044
  sbias=0
  nbias=0
  biasramp=0
  box 10 nr=100 nc=100 cr=400 cc=400 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  unmask
  create 110 box=12 
  box 13 sr=1 sc=1 nr=nrow nc=50  ! pyramid
  clip 110 box=13 min=1 maskonly
  box 13 sr=1 sc=1 nc=ncol nr=60  ! pyramid
  clip 110 box=13 min=1 maskonly
  save mask=/usr/tmp/blank
  maxbad=4096
  lowbad=-50
else_if abs[jdet]==102
  ! WFPC2 WF2
  nrow=800
  ncol=800
  srow=1
  scol=1
  over=0
  rn=6
  gain=14
  scale=0.1
  sbias=0
  nbias=0
  biasramp=0
  box 10 nr=100 nc=100 cr=400 cc=400 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  unmask
  create 110 box=12 
  box 13 sr=1 sc=1 nr=nrow nc=50  ! pyramid
  clip 110 box=13 min=1 maskonly
  box 13 sr=1 sc=1 nc=ncol nr=30  ! pyramid
  clip 110 box=13 min=1 maskonly
  save mask=/usr/tmp/blank
  maxbad=4096
  lowbad=-50
else_if abs[jdet]==103
  ! WFPC2 WF3
  nrow=800
  ncol=800
  srow=1
  scol=1
  over=0
  rn=6
  gain=14
  scale=0.1
  sbias=0
  nbias=0
  biasramp=0
  box 10 nr=100 nc=100 cr=400 cc=400 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  unmask
  create 110 box=12 
  box 13 sr=1 sc=1 nr=nrow nc=30  ! pyramid
  clip 110 box=13 min=1 maskonly
  box 13 sr=1 sc=1 nc=ncol nr=50  ! pyramid
  clip 110 box=13 min=1 maskonly
  save mask=/usr/tmp/blank
  maxbad=4096
  lowbad=-50
else_if abs[jdet]==104
  ! WFPC2 WF4
  nrow=800
  ncol=800
  srow=1
  scol=1
  over=0
  rn=6
  gain=14
  scale=0.1
  sbias=0
  nbias=0
  biasramp=0
  box 10 nr=100 nc=100 cr=400 cc=400 ! for normalizing, cant include bad cols.
  box 11 nr=nrow nc=ncol sr=srow sc=scol ! part of image to change, w/o overscan
  box 12 nr=nrow nc=ncol+over sr=srow sc=scol    ! full sized image
  unmask
  save mask=/usr/tmp/interp
  unmask
  create 110 box=12 
  box 13 sr=1 sc=1 nr=nrow nc=45  ! pyramid
  clip 110 box=13 min=1 maskonly
  box 13 sr=1 sc=1 nc=ncol nr=45  ! pyramid
  clip 110 box=13 min=1 maskonly
  save mask=/usr/tmp/blank
  maxbad=4096
  lowbad=-50
end_if
box 13 sr=srow sc=scol nr=nrow nc=ncol
opt ma=maxbad
unmask
$chmod 666 /usr/tmp/blank.msk
$chmod 666 /usr/tmp/interp.msk
end
