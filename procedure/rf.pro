parameter istan

printf 'echo RF %i6 >>master2com' istan >./rf.inp
printf 'echo "Y" >>master2com ' >>./rf.inp

printf 'Sending  commands:'
printf 'echo RF %i6 >>master2com' istan 
printf 'echo "Y" >>master2com ' 
$csh rf.inp

call get

call coord 1
printf 'Sending  commands:'
printf 'echo RF %i6 >>master2com' istan 
printf 'echo "Y" >>master2com ' 
$csh rf.inp
call get

end
