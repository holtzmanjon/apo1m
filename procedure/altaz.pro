parameter az alt

if az>360
  toaz=az-360
else_if az<0
  toaz=az+360
else
  toaz=az
end_if
printf 'echo ALTAZ >>master2com' >./com.inp
printf 'echo "%f8.1\r" >> master2com ' toaz >>./com.inp
printf 'echo "%f8.1\r" >> master2com ' alt >>./com.inp

printf 'Sending  commands:'
$cat com.inp
$csh com.inp

call get
end
