parameter t fast silent
silent=1
th=ifix[t] tm=ifix[(t-th)*60] ts=ifix[(t-th-tm/60)*60]
if t==0
  printf 'A\nT =\nQ\n' >./lst.inp
else
  printf 'A\n g T 1080 t %i %i %i =\nQ\n' th tm ts >./lst.inp
end_if
if fast==0
end_if
$lst >./lst.out
$localtime >./localtime.out
$uttime >./uttime.out
open skycalc ./lst.out
read skycalc
lst=@skycalc.1+@skycalc.2/60.
close skycalc

open skycalc ./localtime.out
read skycalc
localtim=@skycalc.1+@skycalc.2/60.+@skycalc.3/3600.
close skycalc

open skycalc ./uttime.out
read skycalc
uttim=@skycalc.1+@skycalc.2/60.+@skycalc.3/3600. 
utyear=@skycalc.4 utday=@skycalc.6
string month {skycalc.-5}
strcmp {month} Dec silent
if strcmpok==1
  utmonth=12
else
  jan=1 feb=2 mar=3 apr=4 may=5 jun=6 jul=7 aug=8 sep=9 oct=10 nov=11 
  utmonth={month}
end_if
read skycalc
jd=@skycalc.1
close skycalc
if silent==0
  printf '  Localtime:  %f8.3   LST:  %f8.3   UT: %f8.3' localtim lst uttim
  printf '  UT date:  %i6 %i3 %i3   ' utyear utmonth utday
end_if

if fast==0
  printf 'A\na\nQ\n' >./sunset.inp

  $sunset >./sunset.out
  $sunrise >./sunrise.out
  $ntwilight >./ntwilight.out

  open skycalc ./sunset.out
  read skycalc
  sunset=@skycalc.1+@skycalc.2/60.
  close skycalc

  open skycalc ./sunrise.out
  read skycalc
  sunrise=@skycalc.1+@skycalc.2/60.
  close skycalc

  open skycalc ./ntwilight.out
  read skycalc
  ntwieve=@skycalc.1+@skycalc.2/60.
  ntwimorn=@skycalc.3+@skycalc.4/60.
  close skycalc

  if silent==0
    printf '  Sunset:%f8.3  Eve ntwilight:%f8.3  Morn ntwilight:%f8.3 Sunrise:%f8.3'  sunset ntwieve ntwimorn sunrise
  end_If
end_if


end

