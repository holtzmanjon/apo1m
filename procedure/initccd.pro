if scidet==32
  send ccdinit
  send -disk
  send 'settemp -125'
  send 'qck 0'
  send 'qck 0'
  send ccdinit
  send 'qck 0'
  send +disk
end_if
send initccd
send ginitccd
send settime
call readccd
call readgccd
string root '{ccdroot}'
setdir im dir=../images/{ccdroot}/
end
