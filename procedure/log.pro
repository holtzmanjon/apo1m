do idate=1,5

if idate==1
  string root '001103'
  n=168
else_if idate==2
  string root '001110'
  n=467
else_if idate==3
  string root '001114'
  n=385
else_if idate==4
  string root '001118'
  n=270
else_if idate==5
  string root '001120'
  n=243
else_if idate==6
  string root '001121'
  n=
end_if
 
setdir im dir=/1m/{root}
string filt01 '656n'
string filt02 'i'
string filt04 'r'
string filt06 'b'
string filt07 'c'
string filt08 'v'
string filt09 '501n'
string filt10 'z'
 
do i=1,n
  string file '{root}.%i3.3' i
  rd 1 {file}.fits headonly
  printf '{1:object} {1:filter}'
  printf '{1:object} {1:filter}' >>./{root}.dat
  filter={1:filter}
  string fname 'filt%i2.2' filter
  printf '%i3	{root}	{{fname}}	{1:exposure}	{1:pa}' i |
      >>./{1:object}.{{fname}}
  printf '%i3	{root}	{{fname}}	{1:exposure}	{1:pa}' i |
      >>./{1:object}
end_do
end_do
END
