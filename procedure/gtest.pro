send 'guideloc 0'
send 'guidefac 0.'
send 'inst 2'
call altaz 220 70
send 'setfilt i'
saofoc

gsize=15 gtime=1 gupdate=5

ipa=0
!do ipa=0,360,60

 string com 'gwrite %i8' iloop*1000+(ipa/60)*1000
 send '{com}'

 string command 'pa %i3' ipa
 send '{command}'

 call readgccd
 $sleep 5
 string command 'gexp %f8.1' gtime
 send '{command}'
! call writelog 0 gincval 'Final guide acquisition: {catalog}'
 call readg gincval
 zap $ireadbuf sig=0 size=3
 box 10 n=500 cr=256 cc=256
 abx $ireadbuf 10 high_row=hr high_col=hc
 
 string com 'newguide %2f8.1 %i4 %i8 %i4' hc-1 hr-1 gsize gtime*1000 gupdate
 send '{com}'
 $sleep 30

 send 'exp 300'
!end_do
send 'inst 1'
send 'gnowrite'
send 'guidehome'
send 'guidefac 1.'

end
