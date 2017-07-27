parameter image string=date

string test '1{date}'
err goto readit
if {test}~=1
  string root '{date}'
  setdir im dir=/home/1m/{date}
  idet=17
  call getdet idet
  biasramp=-1
end_if

readit:
call read image
window $ireadbuf box=1
mn $ireadbuf

end

