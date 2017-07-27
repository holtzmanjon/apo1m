!ask i
!  call read i
!  call parang ireadbuf
!  printf '{ireadbuf:obs_az} {ireadbuf:obs_alt} %f8.4' parang2
 
  hjd $ireadbuf
  ha={ireadbuf:ha}*180/pi
  gettime {ireadbuf:ra}
  zd={ireadbuf:z}*180/pi
  gettime {ireadbuf:dec}
  decd=hh+mm/60+ss/3600
 
!  ask ha
!  ask zd
!  ask decd
 
  call parang ha zd decd latitude
  parang=-parang
 
!  cr=sr[ireadbuf]+nr[ireadbuf]/2
!  cc=sc[ireadbuf]+nc[ireadbuf]/2
!  tv $ireadbuf
!!  tvplot p=512,512 p=512-50*cosd[parang],512-50*sind[parang]
!  tvplot c=cr,cc pa=parang compass=50
!  tvplot p=cr-70*cosd[parang],cc-70*sind[parang] text='+alt'
!  tvplot p=cr-70*cosd[parang-90],cc-70*sind[parang-90] text='+az'
  parang0=parang
 
!  pause
  clear vec
  cr=sr[ireadbuf]+nr[ireadbuf]/4
  cc=sc[ireadbuf]+nc[ireadbuf]/4
  parang={ireadbuf:pa}
  tvplot c=cr,cc pa=parang compass=50
  tvplot p=cr-70*cosd[parang],cc-70*sind[parang] text='+N'
  tvplot p=cr-70*cosd[parang-90],cc-70*sind[parang-90] text='+E'
 
!  pause
!  clear vec
  cr=sr[ireadbuf]+nr[ireadbuf]*3/4
  cc=sc[ireadbuf]+nc[ireadbuf]/4
  parang=parang0-{ireadbuf:pa}
  tvplot c=cr,cc pa=parang compass=50
  tvplot p=cr-70*cosd[parang+180],cc-70*sind[parang+180] text='+alt'
  tvplot p=cr-70*cosd[parang-90],cc-70*sind[parang-90] text='+az'
  printf 'HA: %f8.3' {ireadbuf:ha}*180/3.14159/15
  printf 'PA: {ireadbuf:pa}'
  printf 'OBS_AZ: {ireadbuf:obs_az}'
  printf 'OBS_ALT: {ireadbuf:obs_alt}'
  printf 'OBS_ROT: {ireadbuf:obs_rot}'
  itv
END
