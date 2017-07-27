      program readusno

  100 n = IARGC()
      IF (n.ne.1) THEN
        print '(1x,''Enter file name: ''$)'
	read '(a)', file
      else
        CALL getarg(1,file)
      end if
      l = index(file,' ') - 1

      open(1,file=file(1:l),status='unknown')

      read(1,*) rah, decd
      do i=1,4
        read(1,*)
      end do

      gbright=99
1     read(1,101,end=99) grah, gram, gras, cgdecd, gdecm, gdecs, gmag
101   format(*,*,*,a,*,*,*)

      read(cgdecd,*) gdecd
      gdecd = abs(gdecd)
      sign = 1
      if (index(cgdecd,'-') .gt. 0) sign = -1

      if (gmag .lt. gbright .and. gmag .lt. 9) then
        grah = grah + gram / 60 + gras/3600
        gdecd = sign*(gdecd+gdecm/60+gdecs/3600)

        gddec=(gdecd-decd)*3600
        gdra=(grah-rah)*15*3600*cos(decd*3.14159/180.)
        print *, rah,grah,decd,gdecd,gddec,gdra
        call guidepos(gdra,gddec,gpa,x,y)
	print *, gpa, x, y, gmag

        if (x .gt. 30 .and. x .lt. 160 .and. y .gt. 30 .and. y .lt. 140) then
          gbright=gmag 
          gx=x 
          gy=y 
          dra=gdra 
          ddec=gddec 
          pa=gpa
        end if
      end if

      stop
      end

      subroutine guidepos(gdra, gddec, gpa, x, y)

