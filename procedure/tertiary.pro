parameter lport
call readstat
if port==lport
  goto tend
end_if

! 12/08/08 collimation:
!na1pos=27900
!na2pos=16000
$power1m on motors
send '-35m'
send 'clear'
if lport==2
  !from NA1 to NA2
  if (port==lport)
    printf 'Tertiary already at port %i!!' lport
  else
    send 'STUP'
    send 'TMOVE -50000'
    send 'THOMER'
    send 'NA2'
    send 'DF 4770'
    focref=focref+4770
    focus=0 nfocus=0
    send 'NOGCCD'
  end_if
  printf 'echo TPFILE >> master2com ' >./com.inp
  printf 'echo na2.mod >> master2com ' >>./com.inp
  $csh com.inp
  call get
  send 'inst 3'
  send 'apogee'
  send 'zdisable'
  scidet=17
else_if lport==1
  !from NA2 to NA1
  if (port==lport)
    printf 'Tertiary already at port %i!!' lport
  else
    send 'STUP'
    send 'TMOVE -50000'
    send 'THOMER'
    send 'NA1'
    send 'DF -4770'
    focref=focref-4770
    focus=0 nfocus=0
    send 'LEACH'
    send '+GCCD'
  end_if
  printf 'echo TPFILE >> master2com ' >./com.inp
  printf 'echo na1.mod >> master2com ' >>./com.inp
  $csh com.inp
  call get
  send 'inst 1'
  send 'leach'
  send 'zenable'
  scidet=32
  send ccdinit
  send -disk
  send 'settemp -125'
  send 'qck 0'
  send 'qck 0'
  send ccdinit
  send 'qck 0'
end_if
send '+35m'

! Make sure the camera is initialized for date
call initdate
! Reset focus variables
tfoc=-100 nextf=-100 dofoc=0

! initialize telescope
!$echo "INIT" >> master2com
!$echo "I" >> master2com
!$echo "y" >> master2com
!call readstat 1
!if domeinit==0
!  $echo "n" >> master2com
!end_if
!if domeslav==0
!  $echo "n" >> master2com
!end_if
!call get


send 'td 50'



!from NA1 to NA1
!THOMER
!TMOVE -20000
!THOME
!NA1

!from NA2 to NA2
!THOMER
!TMOVE -5000
!THOME
!NA2
tend:
end
