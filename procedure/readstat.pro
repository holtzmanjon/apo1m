parameter lfast

if lfast==0
  send 'we'
  if daytest==0
    $sleep 1
  end_if
end_if

retry:
$'rm' statr.doc
$tail -n -2 /home/export/tocc/statr.doc >./statr.doc
err goto retry
eof goto retry
open statr ./statr.doc
string line '{statr}'
!ratarg=@statr.2
ratarg=@statr.5
ratarg=ratarg*180/pi/15.
!dectarg=@statr.3
dectarg=@statr.6
dectarg=dectarg*180/pi
pa=@statr.4*180/pi

epoch=@statr.7
utc=@statr.8
string utc '{line.8}'
lst=@statr.9
az=@statr.10
alt=@statr.11
rot=@statr.12
port=@statr.13
init=@statr.14
home=@statr.15
track=@statr.16
domeinit=@statr.27
domeslav=@statr.28
domeopen=@statr.29
ldomopen=@statr.30
domeaz=@statr.31
tpos=@statr.36
upos=@statr.37
vpos=@statr.38
rawfoc=(tpos+upos+vpos)/3
outtemp=@statr.39
cabtemp=@statr.40
auxtemp=@statr.41
shutdown=@statr.45
gxc=@statr.53
gyc=@statr.54
ccdtemp=@statr.56
filtno=@statr.60

close statr
!$weath.py > weath.out
!open weath ./weath.out
!read weath
!outtemp=@weath.1
!auxtemp=@weath.1
!close weath

printf 'Manually OVERRIDING TEMPERATURE'
outtemp=20

end

