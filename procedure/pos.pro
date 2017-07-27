parameter auto
!string root '?Enter root file name for images: '
ask 'Starting image number: ' start
ask 'Ending image number: ' end
first=1
do i=start,end
  string file '{root}.%i3.3' i
  printf 'Hit C to continue when image {file} is available'
!  pause
!  rd 1 '{file}.fits'
!add 1 c=32768
  call read i
  if ireadbuf~=1
  copy 1 $ireadbuf
  dispose $ireadbuf
  end_if
  printf '%i3 {?1:obs_az} {?1:obs_alt} {?1:object}' i >>./{root}.lis
!if auto==0
if (auto==0)|(auto>=2)
  tv 1
end_if
if (auto==0)
  r1=0 c1=0 r=0 c=0
  printf 'Hit C on star, E to exit. If no star present, just hit E'
  mark new
  r1=r c1=c
else
  box 1 n=1000 cr=512 cc=512
  box 1 n=500 cr=256 cc=256
  abx 1 1 high=high high_row=r1 high_col=c1
  type high r1 c1
  if (high<5000)
    r1=-1 
  else
    if (auto>=2)
      box 1 n=21 cr=r1 cc=c1
      tvbox box=1 
    end_if
  end_if
end_if
  if r1>0&c1>0
    gettime {?1:ra}
    rah=hh ram=mm ras=ss
    ra0=rah+ram/60.+ras/3600.
    gettime {?1:dec}
    decd=hh decm=mm decs=ss sign=sg
    dec0=sg*abs[decd+decm/60+decs/3600]
    gettime {?1:date-obs}
    precess ra=ra0 dec=dec0 epoch0={1:epoch} epoch=hh+mm/12 
    obsra=raf
    obsdec=decf
    obsrah=ifix[obsra]
    obsram=ifix[(obsra-obsrah)*60.]
    obsras=(obsra-obsrah-obsram/60.)*3600.
    isign=obsdec/abs[obsdec]
    obsdecd=ifix[abs[obsdec]]
    obsdecm=ifix[(abs[obsdec]-obsdecd)*60.]
    obsdecs=(abs[obsdec]-obsdecd-obsdecm/60.)*3600.
    if isign==-1
      string sign '-'
    else
      string sign ' '
    end_if
    rafh=obsrah rafm=obsram rafs=obsras
    decfd=obsdecd decfm=obsdecm decfs=obsdecs

!  PI
!    dra=(c1-560)*.812
!    ddec=(r1-532)*.812

!  Apogee
    dra=-(c1-230)*.794
    ddec=-(265-r1)*.794
    dra=-(c1-227)*.794
    ddec=-(276-r1)*.794

    obsra=raf+dra/cosd[decf]/15./3600.
    obsdec=decf+ddec/3600.
    obsrah=ifix[obsra]
    obsram=ifix[(obsra-obsrah)*60.]
    obsras=(obsra-obsrah-obsram/60.)*3600.
    isign=obsdec/abs[obsdec]
    obsdecd=ifix[abs[obsdec]]
    obsdecm=ifix[(abs[obsdec]-obsdecd)*60.]
    obsdecs=(abs[obsdec]-obsdecd-obsdecm/60.)*3600.
    if isign==-1
      string sign2 '-'
    else
      string sign2 ' '
    end_if

    if first==1
      first=0
      gettime {?1:date-obs}
      printf '%i2.2-%i2.2-%i4\n\n' mm ss hh >./{root}.test
      printf '32 46 49.1 %i4 %i2.2 %i2.2 10.0  718.6 2800.0 0.20 0.55 0.0065\n\n' |
        hh mm ss >>./{root}.test
      printf '%i2.2-%i2.2-%i4\n\n' mm ss hh >./{root}.test2
      printf '32 46 49.1 %i4 %i2.2 %i2.2 10.0  718.6 2800.0 0.20 0.55 0.0065\n\n' |
        hh mm ss >>./{root}.test2
    end_if
    gettime {?1:lst}

    obsaz={1:obs_az}
    printf |
'%i2%i3%f6.2 {sign}%i2.2%i3%f6.2                    %i2%i3%f6.2 {sign2}%i2.2%i3%f6.2%i3%f8.4%f12.4%i3' |
rafh rafm rafs decfd decfm decfs obsrah obsram obsras |
obsdecd obsdecm obsdecs hh mm+ss/60. obsaz i >>./{root}.test
    printf |
'%i2%i3%f6.2 {sign}%i2.2%i3%f6.2                    %i2%i3%f6.2 {sign2}%i2.2%i3%f6.2%i3%f8.4%f12.4%i3' |
rafh rafm rafs decfd decfm decfs rah ram ras |
decd decm decs hh mm+ss/60. obsaz i >>./{root}.test2
    
 
    printf '%i2 %i2 %f4.1  %i3 %i2 %f4.1  %i2 %i2 %i2  %2f8.2 {?1:obs_az} %i3' |
      rah ram ras decd decm decs hh mm ss c1 r1 i
    printf '%i2 %i2 %f4.1  %i3 %i2 %f4.1  %i2 %i2 %i2  %2f8.2 {?1:obs_az} %i3' |
      rah ram ras decd decm decs hh mm ss c1 r1 i >>./{root}.dat
  end_if
  if auto==2
    itv
  end_if
end_do
end
