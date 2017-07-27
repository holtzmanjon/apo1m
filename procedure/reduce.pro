! procedure to reduce frame in buf using flat in buffer flat and shutter
!  shading frame in buffer dt
parameter frame fsbias fflat fdt origin fpixarea ffringe fdark

  call read frame
 
! Correct for superbias
  if fsbias>0
    sub $ireadbuf $fsbias
    string comment 'Corrected for superbias using {fsbias:object}'
    fits $ireadbuf char=comment '{comment}'
  end_if

! Correct for dark
  if fdark>0
    t={ireadbuf:exposure}
    copy 110 $fdark
    mul 110 c=t
    sub $ireadbuf 110
  end_if

! Correct for shutter shading
  if fdt>0
    copy 110 $fdt
    t={ireadbuf:exptime}
    add 110 c=t
    div 110 c=t
    div $ireadbuf 110
    string comment 'Corrected for shutter shading using {fdt:object}'
    fits $ireadbuf char=comment '{comment}'
  end_if

! Divide by flat field
  if fflat>0
    div $ireadbuf $fflat
    string comment 'Flattened using {fflat:object}'
    fits $ireadbuf char=comment '{comment}'
  end_if

! Fringe subtraction
  if ffringe>0
    daosky $ireadbuf box=10
    ifringe=2
    if ifringe==1
      copy 110 $ffringe
      mul 110 c=sky
      sub $ireadbuf 110 
    else
      error goto nofringe
      unmask
      clip $ireadbuf maskonly max=sky+5*skysig
      ! Do fit only over box 10 but subtract fit from entire image
      ! fit for fringe + linear slope of sky across frame
      copy 110 $ireadbuf
      copy 111 $ireadbuf
      sub 111 111
      add 111 c=1
      copy 112 111
      add 111 111 dc=1
      add 112 112 dr=1
      sub 111 c=nc[111]/2
      sub 112 c=nr[112]/2
      lincomb 110 buf=ffringe,111,112 load sub box=10 mask
      copy 110 $ffringe
      mul 110 c=l1
      sub $ireadbuf 110
printf '{ireadbuf:object} {ireadbuf:date-obs} {ireadbuf:obsnum} %2f12.3' |
      l0 l1 >>./fringe.dat
    end_if
    string comment 'Fringe subtracted using {ffringe:object}, %f12.3' sky
    fits $ireadbuf char=comment '{comment}'
    nofringe:
  end_if

! Pixel area correction
  if fpixarea>0
   mul $ireadbuf $fpixarea
   string comment 'Corrected for nonuniform pixel areas using {fpixarea:object}'
   fits $ireadbuf char=comment '{comment}'
  end_if

! Trim and reset origin if requested
  create 99 box=11
  sr0=sr[99] er0=sr0+nr[99]-1
  sc0=sc[99] ec0=sc0+nc[99]-1
  srow=max[sr0,sr[ireadbuf]] scol=max[sc0,sc[ireadbuf]] 
  erow=min[er0,er[ireadbuf]] ecol=min[ec0,ec[ireadbuf]]
  box 13 sr=srow sc=scol nr=erow-srow+1 nc=ecol-scol+1

  if origin==1
    window $ireadbuf box=13
    string comment 'Trimmed to [%i4:%i4,%i4:%i4]' |
      sc[ireadbuf] sc[ireadbuf]+nc[ireadbuf]-1 |
      sr[ireadbuf] sr[ireadbuf]+nr[ireadbuf]-1
    fits $ireadbuf char=comment '{comment}'
!    fixhead $ireadbuf origin
!    string comment 'Origin reset to (1,1)'
!    fits $ireadbuf char=comment '{comment}'
  end_if

end
