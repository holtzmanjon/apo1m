do i=1,nfilt
  string var 'nexp%i2.2' i
  string evar 'exp%i2.2' i
  if i>6
    {var}=1
    {evar}=0
  else
    {var}=0
    {evar}=0
  end_if
end_do
END
