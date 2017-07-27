parameter itime nframe string=cname

printf 'call getdet %i3' scidet >>../images/{root}/biasdark.pro
call readccd
if itime==0
  send 'object BIAS'
  string com 'mexp 0 %i3' nframe
  send '{com}'
  printf 'call median %i3 %i3 0 0 sbias 1' incval incval+nframe-1 |
      >>../images/{root}/biasdark.pro
  printf 'wd 101 ../images/{root}/{cname}' >>../images/{root}/biasdark.pro
else
  string com 'mdark %i4 %i3' itime nframe
  send '{com}'
  printf 'call median %i3 %i3 0 1 sbias 1' incval incval+nframe-1 |
      >>../images/{root}/biasdark.pro
  printf 'wd 101 ../images/{root}/{cname}' >>../images/{root}/biasdark.pro
end_if

end

