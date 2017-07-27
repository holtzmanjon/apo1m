!center of image 1 is r1,c1, inner edge r2,c2, outer edge r3,c3
!center of image 2 is r4,c4, inner edge r5,c5, outer edge r6,c6
ds1=2*sqrt[(r2-r1)^2+(c2-c1)^2]
dm1=2*sqrt[(r3-r1)^2+(c3-c1)^2]
ds2=2*sqrt[(r5-r4)^2+(c5-c4)^2]
dm2=2*sqrt[(r6-r4)^2+(c6-c4)^2]
 
printf 'Ratio of outer diameters: %f12.3' dm2/dm1
printf 'Ratio of inner diameters: %f12.3' ds2/ds1
 
! adjust ds2 to make outer diameters scale
ds2=ds2*dm1/dm2
printf 'Adjusted ratio of inner diameters: %f12.3' ds2/ds1
 
 
f=6.06
k=0.45
dm=dm1*15
 
tsa=dm/2/(1-k)*(2/(1+ds2/ds1)-1)
 
printf 'TSA: %f12.3 microns, at circle of least confusion: %f12.3' tsa tsa/4
printf 'angular image radius at CLC: %f12.3 arcsec' tsa/4/15*0.81
 
 
END
