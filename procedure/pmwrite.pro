call readstat 1
printf '%2f12.6 %f8.1 {utc} %3f12.6 0.' ratarg dectarg epoch lst az alt |
   >>../images/{root}/{root}pm.out

end
