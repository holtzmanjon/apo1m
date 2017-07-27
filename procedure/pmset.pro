parameter string=pmfile

printf 'indat ../images/{root}/{root}{pmfile}.dat' >./tpoint.inp
printf 'inmod /home/export/tocc/051130.mod' >>./tpoint.inp
printf 'fit n' >>./tpoint.inp
printf 'fix npae ca nrx nry' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'mask r g 60' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'fit' >>./tpoint.inp
printf 'outmod {pmfile}.mod' >>./tpoint.inp
printf 'end' >>./tpoint.inp
$tpoint < tpoint.inp
$'rm' /home/export/tocc/{root}t.mod
$sed 's/=/ /' {pmfile}.mod >/home/export/tocc/{root}t.mod

open mod  ./{pmfile}.mod
read mod
read mod
nmod=@mod.2
rms=@mod.3
close mod

if nmod>3&rms<40
  printf 'echo rmc >>master2com' >./com.inp
  a={root}
  printf 'echo e:\\tocc\\%i6.6t.mod >>master2com' a >>./com.inp
  $csh com.inp
  call get
end_if

end
