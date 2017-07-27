parameter string=root airmax 

inter=-2

if airmax==0
  airmax=10
end_if


setdir im dir=/1m/{root}
idet=17
call getdet idet
setdir da dir=/1m/{root}
setdir ph dir=/1m/{root}

printf '<html> <body>' >/1m/{root}/{root}.html
do ifilt=1,5
  if ifilt==1
    string filt 'u'
    string scol '1,1,2'
  else_if ifilt==2
    string filt 'b'
    string scol '2,2,3'
  else_if ifilt==3
    string filt 'v'
    string scol '3,2,3'
  else_if ifilt==4
    string filt 'r'
    string scol '4,3,4'
  else_if ifilt==5
    string filt 'i'
    string scol '5,4,5'
  end_if
  error goto nextfilt
  open input {root}.{filt}
  close input

  rd 151 /1m/cal/flat{filt}

  if inter>-10
    call sphot {filt} inter 0 151
  end_if
  fit* stn=/1m/cal/stan.stn dat={root}{filt} scol={scol} airmax=airmax |
    out={root}{filt} batch hard ps={root}{filt}
  $sed 's/root/{root}{filt}/' /1m/cal/fitstar.fig >{root}{filt}.fig
  $fig2dev -L gif {root}{filt}.fig {root}{filt}.gif
  printf '<img src={root}{filt}.gif>' >>/1m/{root}/{root}.html
  nextfilt:
end_do
printf '</body> </html>' >>/1m/{root}/{root}.html

end
