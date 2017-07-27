parameter b1 b2
!  Given a picture with two stars at two different rotator angles
!   r0,c0 = center of rotation
!   r1,c1 position of star 1 at rotator position1
!   r2,c2 position of star 1 at rotator position2
!   r3,c3 position of star 2 at rotator position1
!   r4,c4 position of star 2 at rotator position2

buf=b1
tv $buf
printf 'Mark two stars, E to exit'
mark new
save coo=./junk sr=0 sc=0
open input ./junk.coo
read input
read input
read input
read input
r1=@input.3 c1=@input.2
read input
r3=@input.3 c3=@input.2
close input

buf=b2
tv $buf
printf 'Mark same two stars, E to exit'
mark new
save coo=./junk sr=0 sc=0
open input ./junk.coo
read input
read input
read input
read input
r2=@input.3 c2=@input.2
read input
r4=@input.3 c4=@input.2
close input


c0=2*(r1-r2)*((r3^2+c3^2)-(r4^2+c4^2))-2*(r3-r4)*((r1^2+c1^2)-(r2^2+c2^2))
c0=c0/4/((r1-r2)*(c3-c4)-(r3-r4)*(c1-c2))
r0=(r1^2+c1^2)-(r2^2+c2^2)-2*c0*(c1-c2)
r0=r0/2/(r1-r2)

type r0 c0

type sqrt[(r1-r0)^2+(c1-c0)^2]
type sqrt[(r2-r0)^2+(c2-c0)^2]
type sqrt[(r3-r0)^2+(c3-c0)^2]
type sqrt[(r4-r0)^2+(c4-c0)^2]

end
