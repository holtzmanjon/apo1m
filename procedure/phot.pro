parameter image

tv $image
printf 'Mark your star(s) using the C key on the display, then E to exit'
mark new circ=10/scale
aperstar $image star=10/scale sky=15/scale,20/scale gain=gain ronoise=rn
print phot brief
end
