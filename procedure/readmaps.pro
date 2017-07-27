do ifoc=1,11
  call readmap ifoc
  copy $100+ifoc 201
end_do
END
