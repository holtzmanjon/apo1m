retry:
error goto retry
eof goto retry
!open fin ./ccdstatus.fin
!close fin

if scidet==17
open ccd ./accdstatus.doc
else
open ccd ./ccdstatus.doc
end_if
string line '{ccd}'
exposure=@ccd.2
string ccdroot '{ccd.-5}'
string filter '{ccd.-11}'
telfoc=@ccd.13
incval=@ccd.15
gx=@ccd.18
gy=@ccd.19
gpa=@ccd.20
grad=@ccd.21
gmag=@ccd.22
ccdtemp=@ccd.34
close ccd

end

