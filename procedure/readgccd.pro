!tryagain:
!error goto tryagain
!open fin ./ccdstatus.fin
!close fin

gexpos=0
string gccdroot '000000'
string gfilter ' '
gincval=0

err goto gend
eof goto gend

open gccd ./guidestatus.doc
string line '{gccd}'
gexpos=@gccd.2
string gccdroot '{gccd.-5}'
string gfilter '{gccd.-10}'
gincval=@gccd.15
gx=@gccd.18
gy=@gccd.19
gpa=@gccd.20
grad=@gccd.21
gmag=@gccd.22
gcy=@gccd.33
close gccd

gend:
!err continue
end

