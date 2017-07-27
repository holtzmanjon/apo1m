parameter start end

if (end==0)
  end=start
end_if

err call getroot
r={root}

do iftp=start,end

  string file '{root}c.%i3.3' iftp
  string exten '.fits'
  string name '{file}{exten}'
  $scp holtz@ganymede.nmsu.edu:/1m/{root}/{name} .

end_do
end
