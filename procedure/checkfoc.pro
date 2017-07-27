! Do we need to focus?
call time
call readstat
! call ccdtemp
dt=uttim-tfoc
dtemp=abs[auxtemp-foctemp]
if nfocus==0|dofoc>0|(uttim>nextf)|(dt>dtfoc)|(dtemp>dtempfoc)
  printf 'FOCUS: %6f10.2' uttim tfoc dtfoc auxtemp foctemp dtempfoc
  call getdate
  printf '{date} FOCUS: %6f10.2' uttim tfoc dtfoc auxtemp foctemp dtempfoc >>./focus.log
! send 'pa 0'
  fwhm=10
  oldfoc=focus
  if nfocus==0
    coarse=1
  else_if scidet==17
    coarse=4
  else
    coarse=0
  end_if
  ! If first time, do full focus run
  ! If not first time, set current object to 0 to force reacquisition
  !   of next object with attendant refocus
  if nfocus<=1000
    !call altaz 150 70
    call altaz az 70
    call coord 1 0 0 0
    call getfoc focus 1 0 0 coarse
    focerr1=focerr
    if scidet~=17
      call getfoc focus -2 2 0 4
    end_if
  else
    focerr1=0
    focerr=0
    curobj=0
  end_if
  if (focerr==0&focerr1==0)|(daytest==1)
    call time
    nfocus=nfocus+1
    if nfocus==1
      nextf=uttim+0.5
    else_if nfocus<4
      nextf=uttim+0.5
    else
      slope=(focus-oldfoc)/(uttim-tfoc)
      if (abs[slope]>0)
        nextf=uttim+max[0.5,50/abs[slope]]
      else
        nextf=100
      end_if
      nextf=min[uttim+2,nextf]
    end_if
    tfoc=uttim
    foctemp=auxtemp
  end_if
  printf 'FOCUS nextf: %f10.2' nextf
  curobj=0
end_if

end

