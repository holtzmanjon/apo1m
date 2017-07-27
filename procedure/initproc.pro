! Prepare standards and flat field procedure files
if photom>=1
  printf 'string top /home/1m/' >../images/{root}/stan.pro
  printf 'setdir im dir=\{top\}/{root}' >>../images/{root}/stan.pro
  printf 'setdir da dir=\{top\}/{root}' >>../images/{root}/stan.pro
  printf 'setdir ph dir=\{top\}/{root}' >>../images/{root}/stan.pro
  printf 'string root '{root}'' >>../images/{root}/stan.pro
  printf 'idet=%i' scidet >>../images/{root}/stan.pro
  printf 'call getdet %i' scidet >>../images/{root}/stan.pro
end_if
if photom==1
  printf 'sdss=0' >>../images/{root}/stan.pro
  if1=1 if2=5
else_if photom==2
  printf 'sdss=1' >>../images/{root}/stan.pro
  if1=6 if2=10
else 
  if1=0 if2=0
end_if
do ifilt=if1,if2
  string f 'filt%i2.2' ifilt
  if photom>=1
    printf 'string filt {{f}}' >>../images/{root}/stan.pro
    printf 'call getflat 201 \{root\}' >>../images/{root}/stan.pro
    printf 'call sphot {{f}} -2 0 201 0 0' >>../images/{root}/stan.pro
  end_if
end_do
if photom>=1
  printf 'end' >>../images/{root}/stan.pro
end_if

end
