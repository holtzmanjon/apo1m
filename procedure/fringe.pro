call altaz 210 70
call altaz 355 60
send 'setfilt sdssi'
send 'OBJECT FRINGE'
call readccd
call search 60 0 0 300 1
printf '%i4 %i4 {root}' incval incval+25-1 |
        >>../images/{root}/FRINGE.sdssi

send 'setfilt sdssz'
send 'OBJECT FRINGE'
call readccd
call search 60 0 0 300 1
printf '%i4 %i4 {root}' incval incval+25-1 |
        >>../images/{root}/FRINGE.sdssz


end

