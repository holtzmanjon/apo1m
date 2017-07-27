parameter flatbuf string=date

if {date}<020400
  tbuf=0
  error goto noflat
  open flat /home/1m/cal/flat{filt}.fits
  close flat
  tbuf=flatbuf
  rd $flatbuf /home/1m/cal/flat{filt} 
  noflat:
else
  tbuf=0
  error goto noflat2
  open flat /home/1m/cal/flat{filt}apr02.fits
  close flat
  tbuf=flatbuf
  rd $flatbuf /home/1m/cal/flat{filt}apr02 
  noflat2:
end_if

flatbuf=tbuf

end
