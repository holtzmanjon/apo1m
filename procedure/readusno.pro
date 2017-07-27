parameter string=usnocat lnskip guide0

open cat {usnocat}

stat n=count[cat]
read cat
rah=@cat.3
decd=@cat.4
epoch=2000

send 'guideoff'
if guide0==0
  string com 'guideloc %i' gloc0
  send '{com}'
else
  string com 'guideloc %i' guide0
  send '{com}'
end_if
$sleep 5

again:
string com 'readusno {usnocat} %i' lnskip
send '{com}'
call readgccd
printf 'Builtin READUSNO returns: '
printf 'pa: %f8.1 bright: %f6.1  pos: (%i3,%i3)' gpa gmag gx gy
if gmag>90&gcy<1200
  guide0=gcy+50
  string com 'guideloc %i' guide0
  send '{com}'
  $sleep 5
  lnskip=0
  goto again
end_if
goto endusno

read cat
read cat
read cat
read cat
gbright=99
printf '{usnocat}'
do i=1,n-5
  read cat
  grah=@cat.1 gram=@cat.2 gras=@cat.3
  gdecd=@cat.4 gdecm=@cat.5 gdecs=@cat.6
  gmag=@cat.7 ! gdist=@cat.12
  string sdec '{cat.-4}:{cat.-5}:{cat.-6}'

  if gmag<gbright&gmag>9
    grah=grah+gram/60+gras/3600
    gettime {sdec}
    gdecd=sg*(abs[hh]+mm/60+ss/3600)
!    if gdecd>0
!      gdecd=gdecd+gdecm/60+gdecs/3600
!    else_if gdecd<0
!      gdecd=gdecd-gdecm/60-gdecs/3600
!    else
!      printf 'Error: readusno procedure not yet set for decd=0'
!      pause
!    end_if
    gddec=(gdecd-decd)*3600
    gdra=(grah-rah)*15*3600*cosd[decd]
    call guidepos gdra gddec
!    if x>100&x<460&y>100&y<460

    if x>30&x<160&y>30&y<140
      gbright=gmag gx=x gy=y dra=gdra ddec=gddec pa=gpa
!      printf 'dra: %f8.2 ddec: %f8.2 pa: %f8.1 bright: %f6.1 gdist: %f8.1' |
!         gdra gddec gpa gmag gdist
    end_if
  end_if
end_do

gmag=gbright
if gmag<99
  printf 'CHOOSE: '
 printf 'dra: %f8.2 ddec: %f8.2 pa: %f8.1 bright: %f6.1  pos: (%i3,%i3)' |
       dra ddec pa gmag gx gy
else
 gx=0 gy=0
 printf 'No guide star found'
end_if
endusno:

close cat
end
