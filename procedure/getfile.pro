parameter ifile
if ifile<1000
  string file '{root}.%i3.3' ifile
else
  string file '{root}.%i4.4' ifile
end_if
string file '{file}.fits'
string filetype ' '
end
