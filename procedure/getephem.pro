parameter string=name epoch

open ephem ./{name}.ephem
stat n=count[ephem]
read ephem
string epochs '%f6.1' @ephem.1
close ephem

printf '{name}' >./getephem.inp
printf '%f8.3' epoch >>./getephem.inp
call time 0 1 1
$julian >>./getephem.inp
$'rm' getephem.out
$getephem < getephem.inp >./getephem.out

open aaa ./getephem.out
read aaa
rah=@aaa.1
read aaa
decd=@aaa.1
close aaa
irah=ifix[rah]
iram=ifix[(rah-irah)*60]
iras=(rah-irah-iram/60)*3600
if iras<10
  string ras '%i2.2:%i2.2:0%f3.1' irah iram iras
else
  string ras '%i2.2:%i2.2:%f4.1' irah iram iras
end_if

sign=abs[decd]/decd
idecd=ifix[abs[decd]]
idecm=ifix[(abs[decd]-idecd)*60]
idecs=(abs[decd]-idecd-idecm/60)*3600
if sign>0
  string sign '+'
else
  string sign '-'
end_if
string decs '{sign}%i2.2:%i2.2:%i2.2' idecd idecm idecs

printf '{ras} {decs} {epochs}'
end

!call time
!$julian >./julian.out
!open skycalc ./julian.out
!read skycalc
!julian=@skycalc.1
!close skycalc
!type julian
!if epoch>0
!  julian=ifix[julian]+epoch/24
!end_if
!type julian
!pause
!
!open ephem ./{name}.ephem
!stat n=count[ephem]
!read ephem
!string epochs '%f6.1' @ephem.1
!jdmin=1e10
!do i=1,n-1
!  read ephem
!  jd=@ephem.1
!  if (abs[julian-jd]<jdmin) 
!    jdmin=abs[julian-jd]
!    rad=@ephem.2
!    decd=@ephem.3
!    dra=@ephem.4
!    ddec=@ephem.5
!
!    dt=(julian-jd)*24
!
!    dra=dra/cosd[decd]
!    rah=(rad+dra*dt/3600)/15
!    decd=decd+ddec*dt/3600
!
!    irah=ifix[rah]
!    iram=ifix[(rah-irah)*60]
!    iras=(rah-irah-iram/60)*3600
!    if iras<10
!      string ras '%i2.2:%i2.2:0%f3.1' irah iram iras
!    else
!      string ras '%i2.2:%i2.2:%f4.1' irah iram iras
!    end_if
!
!    sign=abs[decd]/decd
!    decd=abs[decd]
!    idecd=ifix[decd]
!    idecm=ifix[(decd-idecd)*60]
!    idecs=(decd-idecd-idecm/60)*3600
!    if sign>0
!      string sign '+'
!    else
!      string sign '-'
!    end_if
!    string decs '{sign}%i2.2:%i2.2:%i2.2' idecd idecm idecs
!
!    type dt julian jd
!    printf '{ras} {decs}'
!  end_if
!end_do
!end
!
