	pi = 3.14159

	ax = -0.11
	bx = 0.525
	ay = -0.445
	by = -0.125
	ax=0.672728
	bx=-0.014511
	ay= 0.004677
	by= 0.686407

	print 101, ax, bx, ay, by
101     format('Old ax, bx, ay, by: ', 4f10.6)
	print '(''Enter new ax, bx, ay, by: ''$)'
	read *, ax, bx, ay, by

	dra = -100
	ddec = -1011

	dra = -1578
	ddec = 257
	print 102, dra, ddec
102     format('Old dra ddec: ',2f10.0)
	print '(1x,''Enter new dra, ddec: ''$)'
	read *, dra, ddec

	print *, 'Using: '
	print *, 'ax, bx, ay, by: ', ax, bx, ay, by
	print *, 'dra, ddec: ', dra, ddec

	sx = sqrt(ax**2 + ay**2)
	sy = sqrt(bx**2 + by**2)
	print *, 'absolute values of scales: ', sx, sy

        omega1 = atan(-ay/ax)
        omega2 = atan(bx/by)

	omega = (omega1+omega2)/2.

	sx = -ax/cos(omega)
	sy = -by/cos(omega)

	yc = (ddec*cos(omega) + dra*sin(omega) ) / -sy
	xc = (ddec*sin(omega) - dra*cos(omega) ) / sx
	print *, 'omega(1) = ', omega1
	print *, 'omega(2) = ',omega2
        print *, 'omega = ', omega*(180/pi), omega
        print *, 'sx: ', -ax/cos(omega), ay/sin(omega)
        print *, 'sy: ', -by/cos(omega), -bx/sin(omega)
        print *, 'xc: ', xc
        print *, 'yc: ', yc


	print *
	print *,' Issue command: '
	print 103, sx, sy, xc, yc, omega*(180/pi)
103     format('SETINST 2 ',5f14.6)
	stop
	end

C	implicit real (a-z)

C	k = 0.21278
C	k2 = -2.18427
C	r2 = -3.16417
C	d = 4.8834
C	r1 = -12.2797
C	diam = 3.4046
C	
C
C	gamma = k2 + (r2/d -1)**2
C	term = (1/r1 - d**2*k**2*gamma/r2**3)* diam
C	eta = r2/d - 1 - k2
C	term2 =diam *d**2 * k**2 / r2**3 * (gamma + k2 +1 + 2*eta)
C
C	print *, gamma, term, term2
C
C	stop
C	end


