	real*8 jd,jdmin, rad, decd, dra, ddec, julian, dt
	real*8 rah, ra, dec
	character name*24

	open(1,file='getephem.inp',status='old')

	read(1,'(a)')  name
	read(1,*) epoch
	read(1,*) julian
        if (epoch .gt. 0) julian = int(julian)+(12+epoch)/24.

	l = index(name,' ') -1
	open(2,file=name(1:l)//'.ephem',status='old')

	jdmin=1.e10
1	read(2,*,end=99) jd, rad, decd, dra, ddec
        if (abs(julian-jd) .lt. jdmin) then
	  jdmin=abs(julian-jd)
	  dt=(julian-jd)*24
	  dra=dra/cos(decd*3.14159/180)
C	print *, jd, rad, decd, dra, ddec, dt, julian, jdmin
	  ra=(rad+dra*dt/3600)/15
          dec=decd+ddec*dt/3600
        end if
	goto 1

99	continue
	print *, ra, ra*15
	print *, dec

	stop
	end
