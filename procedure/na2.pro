call tertiary 2

!send 'apo status'
!send 'apo status'
!if val<0 
!  printf 'Error connecting with apogee'
!  pause
!end_if


end
