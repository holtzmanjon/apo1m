	program slitmask

	parameter (maxobject=100)
	character file*80, rastring*32, decstring*32
	integer id(maxobject)
	real ra(maxobject), dec(maxobject), epoch(maxobject)

	print '(1x,''Enter filename with potential targets: '');
	read '(a)', file
	l = index(file,' ')-1
	open(1,file=file(1:l),status='old')

	n=1
	ramean=0
	decmean=0
	ramin=1e10
	ramax=-1e10
	decmin=1e10
	decmax=-1e10
1	read(1,*,end=99) id(n), rastring, decstring, epoch(n)
	l = index(rastring,' ') - 1
        call hms(rastring(1:l),ra(n))
	l = index(decstring,' ') - 1
        call hms(decstring(1:l),dec(n))
	ramean=ramean+ra(n)
	decmean=decmean+ra(n)
	ramin= min(ramin,ra(n))
	ramax= max(ramax,ra(n))
	decmin= min(decmin,dec(n))
	decmax= max(decmax,dec(n))
	n=n+1
	goto 1

99	n=n-1
	racen=ramean/n
	deccen=decmean/n

	call device(x11)
	call tsetup

	call setlim(ramax,decmin,ramin,decmax)
	call box(1,2)

	call plotobj(id,ra,dec,n)
	call drawmask(racen,deccen)

	print *, "enter c key to change center, p for new pa"
	call mongohairs(ichr,x,y)

	if (char(ichr) .eq. 'c') then
	else if (char(ichr) .eq. 'p') then
 	end if

	stop
	end
	
	

