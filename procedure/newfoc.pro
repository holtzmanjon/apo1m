parameter inst nofind

$date >>../getfoc.log
if inst==1
  call getdet scidet
  call readccd
  sfoc=incval
  string prefix ' '
else
  call getdet guidedet
  call readgccd
  sfoc=gincval
  string prefix 'g'
end_if
string com 'inst %i' inst
send '{com}'

if nofind==0
  call usnofoc
end_if
if inst==1
  send 'exp 3'
else
  send 'gexp 3'
end_if
call read{prefix} sfoc

zap $ireadbuf sig=0 size=3
xs=10 xe=ncol-10 ys=10 ye=nrow-10
box 10 nr=ye-ys nc=xe-xs sr=ys sc=xs
abx $ireadbuf 10 high_row=hr high_col=hc high=high
if inst==1
  string com 'offset %2i6' hc-ncol/2 hr-nrow/2
else
  string com 'goffset %2i6' hc-ncol/2 hr-nrow/2
end_if
send '{com}'

send '{prefix}-disk'
if inst==1
  string com 'newfoc %2i4 41 3 5 20' ncol/2 nrow/2
else
  string com 'gnewfoc %2i4 41 3 5 20' ncol/2 nrow/2
end_if
printf '{com}'
send  '{com}'

if inst==1
  string com 'newext %i' sfoc
else
  string com 'gnewext %i' sfoc
end_if
send  '{com}'
if inst==1
  send 'exp 3'
else
  send 'gexp 3'
end_if

call read{prefix} sfoc
xs=ncol/2-20 xe=ncol/2+20 ys=nrow/2-20 ye=nrow/2+20
box 10 nr=ye-ys nc=xe-xs sr=ys sc=xs
abx $ireadbuf 10 high_row=hr high_col=hc high=high
starplot $ireadbuf cen=hr,hc gauss noplot load   !silent

if inst==1
  call readccd
  focus=telfoc
  string comment ' newfoc: telescope focus %f7.1, fw: %f7.2' focus fwhm
else
  call readgcs
  string comment ' newfoc: Guider focus %f10.1, fw: %f7.2' guidypos fwhm
end_if

call writelog abs[inst] sfoc '{comment}'
call writesum 0 '{comment}'
printf '{comment}' >>../getfoc.log
$date >>../getfoc.log
end
