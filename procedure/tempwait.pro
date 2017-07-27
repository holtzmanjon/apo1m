parameter ttarget dtime
if dtime==0
  dtime=1e10
end_if
call time
t1=localtim
rtemp:
call readccd
call time
dft=localtim-t1
if (dft<dtime)
 if ccdtemp>ttarget
  $sleep 60
  goto rtemp
 end_if
end_if
end

