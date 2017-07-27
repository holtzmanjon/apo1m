box 1 n=512 cr=512 cc=512
string root '?Enter root file name for images: '
ask 'Starting image number: ' start
ask 'Ending image number: ' end
do i=start,end
  string file '{root}.%i3.3' i
  printf 'Hit C to continue when image {file} is available'
!  pause
  rd 1 ./{file}
  printf '%i3 {?1:obs_az} {?1:obs_alt} {?1:object}' i >>./{root}.lis
!  tv 1
  r1=0 c1=0
!  printf 'Hit 1 on star, E to exit. If no star present, just hit E'
!  mark new
  abx 1 1 high_row=r1 high_col=c1
  if r1>0&c1>0
    gettime {?1:ra}
    rah=hh ram=mm ras=ss
    gettime {?1:dec}
    decd=hh decm=mm decs=ss
    gettime {?1:lst}
 
    printf '%i2 %i2 %f4.1  %i3 %i2 %f4.1  %i2 %i2 %i2  %2f8.2 {?1:obs_az} %i3' |
      rah ram ras decd decm decs hh mm ss c1 r1 i
    printf '%i2 %i2 %f4.1  %i3 %i2 %f4.1  %i2 %i2 %i2  %2f8.2 {?1:obs_az} %i3' |
      rah ram ras decd decm decs hh mm ss c1 r1 i >>./{root}.dat
  end_if
end_do
end
