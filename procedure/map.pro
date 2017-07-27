parameter foc dfoc nfoc acktime foctime

wayout=0
foc=focus
dfoc=20
nfoc=15
!nfoc=7
acktime=4
foctime=1

if foc==0
  ask 'Middle focus value: ' foc
end_if
if dfoc==0
  ask 'Focus step size: ' dfoc
end_if
if nfoc==0
  ask 'Number of focus positions: ' nfoc
end_if
if acktime==0
  ask 'Exptime for acquisition image: ' acktime
end_if
if foctime==0
  ask 'Exptime for focus run: ' foctime
end_if

send 'inst 2'
send g+disk

startfoc=foc-ifix[nfoc/2]*dfoc

do pa=0,360,60
  call altaz 210 70
  string com 'pa %i5' pa
  send '{com}'

  if nfoc>1
    string com 'fo %i8' foc
    send '{com}'
  end_if
  send 'guidehome'
!  call coord 1
!  send 'guideloc 600'
  send 'inst 2'

  do rad=1200,1200,300
    if nfoc>1
      string com 'fo %i8' foc
      send '{com}'
    end_if

    saofoc
    string com 'guideloc %i5' rad
    send '{com}'
    $sleep 10
    send 'gqck 0'
    call readgccd
    sfoc=gincval
    string com 'gexp %i4' acktime
    send '{com}'

    call readg sfoc
    call ireadbuf sfoc
    zap $ireadbuf sig=0 size=3
    c0=sc[ireadbuf]+nc[ireadbuf]/2
    r0=sr[ireadbuf]+nr[ireadbuf]/2
    box 10 n=0.9*nr[ireadbuf] cr=r0 cc=c0
    abx $ireadbuf 10 high_row=hr high_col=hc high=high
    printf 'Found high pixel with value %f8.1 at (%i4,%i4)' high hc hr
    string comment 'Map: acquisition image pa: %d  rad: %d' pa rad
    call writelog 0 sfoc '{comment}'

    c0=sc[ireadbuf]+nc[ireadbuf]/2
    c0=768
    r0=sr[ireadbuf]+nr[ireadbuf]/2

    string com 'goffset %2f8.1' hc-c0 hr-r0
    send '{com}'

    if nfoc>1
      string com 'fo %i8' startfoc
      send '{com}'
      $sleep 5
      gfocrun startfoc dfoc nfoc foctime
    else
      $sleep 5
      string com 'gexp %i4' foctime
      send '{com}'
    end_if

    call readstat 
    printf '%i5 %i5 %f6.1 %i3 %i3-%i3' pa rad rot sfoc sfoc+1 sfoc+nfoc >>./map.dat

    if wayout==1
      call focus 1 sfoc+1 sfoc+nfoc 2
      string com 'fo %i8' highfoc-600
      send '{com}'
      send 'gexp 10'
      string com 'fo %i8' highfoc+600
      send '{com}'
      send 'gexp 10'
    end_if

    string com 'goffset %2f8.1' c0-hc r0-hr
    send '{com}'

  end_do
end_do

send 'guidehome'
send 'inst 1'
end
