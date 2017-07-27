parameter iread
ireadbuf=iread
n=ifix[ireadbuf/100]
if (mod[ireadbuf,100]==0)
  n=n-1
end_if
ireadbuf=ireadbuf-n*100
end
