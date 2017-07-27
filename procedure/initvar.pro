printf 'Initializing observing variables....'

! Parameters for observing
daytest=0
photom=0 dtlstan=1 dthstan=2 tlstan=-100 thstan=-100
dtfoc=2 dtempfoc=2 ldome=0 tfoc=-100 foctemp=-100 nextf=-100 dofoc=0
dtflat=1 tflat=-100
lastfill=-100
tfudge=1 ccdfast=0
nfill=0
noapogee=1 dithpos=0

! Initialize variables to set home many times we have observed each object
curobj=0 nobj=0 maxobj=99
call getobj -1
!do i=1,maxobj
!  string var 'ndo%i5.5' i
!  {var}=0
!  string var 'nsk%i5.5' i
!  {var}=0
!  string var 't%i5.5' i
!  {var}=-100
!end_do

end
