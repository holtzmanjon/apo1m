parameter istan

printf 'echo RS %i6 >>master2com' istan >./rs.inp
printf 'echo "Y" >>master2com ' >>./rs.inp

printf 'Sending  commands:'
printf 'echo RS %i6 >>master2com' istan 
printf 'echo "Y" >>master2com ' 
$csh rs.inp

call get

call coord 1 0 0 1
printf 'Sending  commands:'
printf 'echo RS %i6 >>master2com' istan 
printf 'echo "Y" >>master2com ' 
$csh rs.inp
call get

end
