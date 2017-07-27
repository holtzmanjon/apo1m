do i=1,nim
  dc=mod[i-1,ndim]*nc[210]
  dr=ifix[(i-1)/ndim]*nr[210]
  hjd $110+i
  hjd={110+i:hjd}-2440000
  date=1100+ifix[hjd-11876+28]
  obsnum={110+i:obsnum}
  string hjd '%i6.6 %i3.3 %f9.3' date obsnum hjd
  !string hjd '%f12.3' hjd
  r=rs+10
  !c=(cs+ce)/2.
  c=cs
  tvplot text=hjd c=r+dr,c+dc
end_do
end
