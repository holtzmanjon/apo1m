parameter string=dummy objnum t silent

string com 'NR==%i4' objnum

if t==0
  $awk -F'\t' '{com} \{printf("A\n r %s d %s \n T =\nQ \n" ,$2, $3)}' {dummy} | sed 's/+/ /g' | sed 's/:/ /g' >./airmass.inp
else
  string t 'g t %f8.2 0 0 ' t
  $awk -F'\t' '{com} \{printf("A\n r %s d %s \n {t} =\nQ \n" ,$2, $3)}' {dummy} | sed 's/+/ /g' | sed 's/:/ /g' >./airmass.inp
end_if
airmass=-1
$airmass >./airmass.out
$moon >>./airmass.out
err goto airend
dmoon=0
open skycalc ./airmass.out
eof goto airend
read skycalc
airmass=@skycalc.1
read skycalc
phase=@skycalc.1
read skycalc
dmoon=@skycalc.1
close skycalc

if silent==0
printf 'Airmass: %f7.2  Moon phase:  %f8.3    Dist moon: %f8.3' |
   airmass phase dmoon
end_if
airend:
end

