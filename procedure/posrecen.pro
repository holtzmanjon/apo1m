parameter nobs
$'rm' pm.out pm.app pm.test
do i=1,nobs
  string file 'pm%i3.3' i
  error goto next
  rd 1 raw/{file} headonly maxtry=0

    gettime {1:ratarg}
    rah=hh ram=mm ras=ss
    ra0=rah+ram/60.+ras/3600.
    gettime {1:dectarg}
    decd=abs[hh] decm=abs[mm] decs=abs[ss] sign=sg
    dec0=sg*(decd+decm/60+decs/3600)
    gettime {1:lst}
    lst=hh+mm/60.+ss/3600.
    ratarg=ra0 dectarg=dec0

    gettime {1:date-obs}
    epoch=hh+mm/12
    epoch0={1:epoch}
    precess ra=ra0 dec=dec0 epoch0=epoch0 epoch=hh+mm/12
    rap=raf decp=decf

    obsra=raf
    obsdec=decf
    obsrah=ifix[obsra]
    obsram=ifix[(obsra-obsrah)*60.]
    obsras=(obsra-obsrah-obsram/60.)*3600.
    isign=obsdec/abs[obsdec]
    obsdecd=ifix[abs[obsdec]]
    obsdecm=ifix[(abs[obsdec]-obsdecd)*60.]
    obsdecs=(abs[obsdec]-obsdecd-obsdecm/60.)*3600.
    if isign==-1
      string sign '-'
    else
      string sign ' '
    end_if
    rafh=obsrah rafm=obsram rafs=obsras
    decfd=obsdecd decfm=obsdecm decfs=obsdecs

    gettime {1:ra}
    rah=hh ram=mm ras=ss
    ra0=rah+ram/60.+ras/3600.
    gettime {1:dec}
    decd=abs[hh] decm=abs[mm] decs=abs[ss] sign=sg
    dec0=sg*(decd+decm/60+decs/3600)
    gettime {1:date-obs}
    epoch0={1:epoch}
    precess ra=ra0 dec=dec0 epoch0=epoch0 epoch=hh+mm/12
    obsra=raf
    obsdec=decf
    obsrah=ifix[obsra]
    obsram=ifix[(obsra-obsrah)*60.]
    obsras=(obsra-obsrah-obsram/60.)*3600.
    isign=obsdec/abs[obsdec]
    obsdecd=ifix[abs[obsdec]]
    obsdecm=ifix[(abs[obsdec]-obsdecd)*60.]
    obsdecs=(abs[obsdec]-obsdecd-obsdecm/60.)*3600.
    if isign==-1
      string sign2 '-'
    else
      string sign2 ' '
    end_if
    robsap=raf decobsap=decf

printf '%2f12.6 {1:epoch} {1:utc} %f12.6 {1:obs_az} {1:obs_alt} %4f12.6' |
   ratarg dectarg lst rap decp robsap decobsap
printf '%2f12.6 {1:epoch} {1:utc} %f12.6 {1:obs_az} {1:obs_alt} %4f12.6' |
   ratarg dectarg lst rap decp robsap decobsap >>./pm.out
 
    gettime {1:lst}
    obsaz={1:obs_az}
 
 
    printf |
'%i2%i3%f6.2 {sign}%i2.2%i3%f6.2                    %i2%i3%f6.2 {sign2}%i2.2%i3%f6.2%i3%f8.4%f12.4%i3' |
rafh rafm rafs decfd decfm decfs obsrah obsram obsras |
obsdecd obsdecm obsdecs hh mm+ss/60. obsaz i >>./pm.test
 
  next:
end_do
END
