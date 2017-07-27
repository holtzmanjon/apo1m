call getdet 10
do idate=1,3
  if idate==1
    string root '000624'
    n=75
  else_if idate==2
    string root '000625'
    n=49
  else_if idate==3
    string root '000626'
    n=39
  end_if
  setdir im dir=/1m/{root}
  do i=1,n
    string file '{root}.%i3.3' i
    rd 1 {file}.fits headonly
    err goto next
    a={1:stannum}
    string out 'obj%i3.3' a
    printf '{root} %i3' i >>./{out}
 
    next:
  end_do
end_do
END
