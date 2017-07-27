	character file*64

	print '(1x,''Enter output file name for script: ''$)'
	read '(a)', file
	l = index(file,' ')
	open(7,file=file(1:l),status='unknown')

	ntot = 0

C   ~Proper coefs for az=205 alt=52
C     xa : -amount star moves in X pixels for 0.05 x tilt
C     xb : -amount star moves in Y pixels for 0.05 x tilt
C     ya : -amount star moves in X pixels for 0.05 y tilt
C     yb : -amount star moves in Y pixels for 0.05 y tilt
	xa = 15
        xb = -260
        ya = -170
        yb = -45

	xa = -85
        xb = -142
        ya = -144
        yb = 86
	scale = 0.81

C      alt,az: 120,77
        xa = -201
        xb = -20
        ya = 21
        yb = -206
        scale = 1.

	print '(1x,''Enter amount star moves in X,Y for 0.05 change in x tilt: '')'
	read *, xa, xb
	print '(1x,''Enter amount star moves in X,Y for 0.05 change in y tilt: '')'
	read *, ya, yb

        xa = xa*scale/0.05
        xb = xb*scale/0.05
	ya = ya*scale/0.05
	yb = yb*scale/0.05

	x0 = 1.45
	y0 = 0.3
	dt = 0.02
	dt = 0.1

	print '(1x,''Enter current xtilt, ytilt: ''$)'
	read *,  x0, y0

	print *, 'This will generate a script for a 3x3 tilt grid'
	print '(1x,''Enter center xtilt, ytilt, and delta tilt: ''$)'
	read *, xs, ys, dt

	xd = 0
	yd = 0
        do ix=-1,1
          xtilt = xs+ix*dt
	  write(7,'(''xtilt '',f8.3)') xtilt
          do iy=-1,1
            ytilt = ys+iy*dt
            x = (xtilt-x0)*xa + (ytilt-y0)*ya
            y = (xtilt-x0)*xb + (ytilt-y0)*yb
	print *, xtilt, ytilt, x, y
	    write(7,'(''ytilt '',f8.3)') ytilt
C	    write(7,'(''qm '',2f10.3)') x-xd, y-yd
	    write(7,'(''goffset '',2f10.3)') x-xd, y-yd
            write(7,'(''sleep 5'')')
	    xd = xd + x-xd
	    yd = yd + y-yd
C            write(7,'(''exp 5'')')
            write(7,'(''gexp 3'')')
            ntot = ntot + 1
          end do
	end do
        xtilt = x0
	write(7,'(''xtilt '',f8.3)') xtilt
        ytilt = y0
	write(7,'(''ytilt '',f8.3)') ytilt
	write(7,'(''goffset '',2f10.3)') 0-xd, 0-yd
        xd = 0
        yd = 0

	print *, 'For manual offset from ', x0, y0
	print '(1x,''Enter desired xtilt, ytilt (CTRL-C to quit): ''$)'
        read *, xtilt, ytilt
        x = (xtilt-x0)*xa + (ytilt-y0)*ya
        y = (xtilt-x0)*xb + (ytilt-y0)*yb
	print *, x, y

	print *, 'Total observations: ', ntot

	stop
	end
