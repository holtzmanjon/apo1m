parameter tinst image string=lcomment

if tinst==0|tinst==2
  string lroot '{root}g'
else
  string lroot '{root}'
end_if

if image<10
  string lfile '{lroot}.%i1' image
else_if image<100
  string lfile '{lroot}.%i2' image
else_if image<1000
  string lfile '{lroot}.%i3' image
else
  string lfile '{lroot}.%i4' image
end_if

$mkdir /home/tcomm/images/{lroot}
$chmod 777 /home/tcomm/images/{lroot}
printf '{lcomment}' >/home/tcomm/images/{lroot}/{lfile}.comment

end
