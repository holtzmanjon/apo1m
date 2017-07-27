  send 'guidefac 0.' 

  gtime=1 gupdate=1
  string com 'newguide %2f8.1 %i4 %i8 %i4' hc-1 hr-1 gsize gtime*1000 gupdate
  send '{com}'
  $sleep 30
  call readccd
  string comment 'gtime: %f8.2  gupdate: %i3  gmag: %f8.2' gtime gupdate gbright
  call writelog 1 incval '{comment}'
  send 'exp 150'

  gupdate=1 gtime=3
  string com 'newguide %2f8.1 %i4 %i8 %i4' hc-1 hr-1 gsize gtime*1000 gupdate
  send '{com}'
  $sleep 30
  call readccd
  string comment 'gtime: %f8.2  gupdate: %i3  gmag: %f8.2' gtime gupdate gbright
  call writelog 1 incval '{comment}'
  send 'exp 150'

  gupdate=1 gtime=5
  string com 'newguide %2f8.1 %i4 %i8 %i4' hc-1 hr-1 gsize gtime*1000 gupdate
  send '{com}'
  $sleep 30
  call readccd
  string comment 'gtime: %f8.2  gupdate: %i3  gmag: %f8.2' gtime gupdate gbright
  call writelog 1 incval '{comment}'
  send 'exp 150'

  send 'guidefac 1.'
end
