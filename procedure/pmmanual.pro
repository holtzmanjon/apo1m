parameter im

call read im
tv $ireadbuf

printf 'mark star'
mark
string com 'guideinst 3 %f8.2 %f8.2' c-256 r-256
send '{com}'

call pmwrite

end
