parameter gdra gddec

! Get instrument block
$tail -n -7 /home/export/tocc/sysscf.new | head -5 >./guider.dat
open inguide ./guider.dat
read inguide
sx=@inguide.3
read inguide
sy=@inguide.3
read inguide
cx=@inguide.3
read inguide
cy=@inguide.3
read inguide
theta=@inguide.3
close inguide

! Compute position angle of target
if gdra==0
  gdra=0.1
end_if
!gpa=arctand[-1*gddec/gdra]
gpa=arctan2[-1*gddec,gdra]*180/3.14159
type gpa
if gdra>0
  gpa=gpa+180
end_if
! Compute position angle of guider
if cx==0
  cx=0.1
end_if
!pa0=arctand[cy/cx]
pa0=arctan2[cy,cx]*180/3.14159

! Compute desired position angle
gpa=gpa-pa0

!printf 'gdra: %f8.2 gddec: %f8.2 gpa: %f8.1 bright: %f6.1' |
!     gdra gddec gpa gbright

! Compute rotator coordinates
xprime=-gdra*cosd[gpa]+gddec*sind[gpa]
yprime=gdra*sind[gpa]+gddec*cosd[gpa]

!printf 'xprime %f8.2   yprime: %f8.2' xprime yprime

! Compute guider coordinates
x=(xprime-cx)*cos[theta]+(yprime-cy)*sin[theta]
x=x/sx+192/2
y=(xprime-cx)*sin[theta]-(yprime-cy)*cos[theta]
y=y/sy+165/2

end
