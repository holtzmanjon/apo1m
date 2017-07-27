parameter ltype string=lcomment

$date +%H:%M:%S >./time.log
open intime ./time.log
string time {intime}
close intime

string sumfile /home/tcomm/public_html/{root}sum.html
if ltype==-1
  printf '{lcomment}<p>' >{sumfile}
  printf '<a href={root}.html> Science camera image log</A><br>' >>{sumfile}
  printf '<a href={root}g.html> Guider camera image log</A><br><p>' >>{sumfile}
else_if ltype==0
  printf '<font color=black> {time}'  >>{sumfile}
  printf '<font color=green> {lcomment}<br>' >>{sumfile}
else_if ltype==1
  printf '<font color=black> {time} : '  >>{sumfile}
  printf '<font color=red> {lcomment}<br>' >>{sumfile}
else_if ltype==2
  printf '<font color=black> {time} : '  >>{sumfile}
  printf '<font color=yellow> {lcomment}<br>' >>{sumfile}
else_if ltype==3
  printf '<font color=black> {time} : '  >>{sumfile}
  printf '{lcomment}<br>' >>{sumfile}
end_if

end
