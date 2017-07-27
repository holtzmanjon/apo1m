parameter new

printf 'Current time info:'
call time 
tdiff=uttim-localtim
if tdiff<0
  tdiff=tdiff+24
end_if
testtime=ntwieve+tdiff
endtime=ntwimorn+tdiff
if testtime>24
  testtime=testtime-24
end_if
call time testtime 0 1
string testfile 'testobs.%i4.4%i2.2%i2.2' utyear utmonth utday
if new==1
  goto maketest
end_if
error goto maketest
open testfile testobs/{testfile}
close testfile
$cat testobs/{testfile}
goto endtest

maketest:
printf 'Making estimated schedule, this make take several minutes....'
printf '============================' >testobs/{testfile}
printf 'Estimated observing schedule: %i %i %i' utyear utmonth utday >>testobs/{testfile}
printf '   UT   OBJECT' >>testobs/{testfile}
call initvar
newtime:
printf '=============TESTTIME==========='
call time testtime 1
call getobj testtime
printf '%f9.3 {name} %3i5' testtime testim dofoc acquire >>testobs/{testfile}
testtime=testtime+testim/3600
if testtime<endtime
  goto newtime
end_if
printf '============================' >>testobs/{testfile}
endtest:
end
