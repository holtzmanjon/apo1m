iix=0
do ix=-1,1
  iix=iix+1
  iiy=0
  do iy=-1,1
    iiy=iiy+1
    string var 'xt%i1%i1' iix iiy
    xtilt={var}
    string var 'yt%i1%i1' iix iiy
    ytilt={var}
    x=ix*size-3/8*size/2
    y=iy*size-7/8*size/2
    string foctext '(%f5.2,%f4.2) ' xtilt ytilt
    tvplot p=y,x text=foctext
  end_do
end_do
end
