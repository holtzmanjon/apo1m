
! Do we need to focus?

if dofoc~=0
  call time
  call readstat
  call ccdtemp
  if dofoc==1|(uttim>nextf)|((uttim-tfoc)>dtfoc)|(abs[(auxtemp-foctemp)]>dtempfoc)
    printf 'FOCUS: %i %6f10.2' dofoc uttim tfoc dtfoc auxtemp foctemp dtempfoc
    call getdate
    printf '{date} FOCUS: %i %6f10.2' dofoc uttim tfoc dtfoc auxtemp foctemp dtempfoc >>./focus.log
!   send 'pa 0'
    fwhm=10
    oldfoc=focus
    if nfocus==0
      coarse=1
    else
      coarse=0
    end_if
    ! If first time, do full focus run
    ! If not first time, set current object to 0 to force reacquisition
    !   of next object with attendant refocus
    if nfocus<=1000
      call altaz 220 70
      call coord 1 0 0 1
      call getfoc focus 1 0 0 coarse
    else
      focerr=0
      curobj=0
    end_if
    if (focerr==0)|(daytest==1)
      call time
      nfocus=nfocus+1 
      if nfocus==1
        nextf=uttim+0.5
      else_if nfocus<4
        nextf=uttim+1
      else
        slope=(focus-oldfoc)/(uttim-tfoc)
        if (abs[slope]>0)
          nextf=uttim+max[0.5,50/abs[slope]]
        else
          nextf=100
        end_if
      end_if
      tfoc=uttim
      foctemp=auxtemp
    end_if
    printf 'FOCUS nextf: %f10.2' nextf
    curobj=0
  end_if
end_if

end
