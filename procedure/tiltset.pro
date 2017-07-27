parameter xtilt ytilt 

if xtilt==0
  ask 'Desired xtilt: ' xtilt
end_if
if ytilt==0
  ask 'Desired ytilt: ' ytilt
end_if
string com 'xtilt %f8.3' xtilt
send '{com}'
string com 'ytilt %f8.3' ytilt
send '{com}'

! Determine offsets from original positions
azoffset=(xtilt-x0)*dazperxt
aloffset=(xtilt-x0)*dalperxt
azoffset=azoffset+(ytilt-y0)*dazperyt
aloffset=aloffset+(ytilt-y0)*dalperyt

! Determine offsets from last position
azoffset=azoffset-azoff
aloffset=aloffset-altoff
string com 'daa %2f12.3' azoffset aloffset
send '{com}'
$sleep 5

! Keep track of current offsets
azoff=azoff+azoffset
altoff=altoff+aloffset

end
