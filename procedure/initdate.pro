send initccd
send ginitccd
send settime

call readccd
call readgccd
string root '{ccdroot}'
setdir im dir=../images/{ccdroot}/

send 'td 30'
end
