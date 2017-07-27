parameter alt rot

error goto setdf
junk=dfoff
goto gdf

setdf:
dfoff=0

gdf:
error goto setgdf
junk=gdfoff
goto fadj

setgdf:
gdfoff=0

fadj:
if alt==0
 call readstat 1
end_if

! altitude dependence as cos of zenith distance, amplitude
!   40 units from zenith to alt=30
zd=90-alt
df=-80*(cosd[zd]-cosd[20])
df=0
! rotator dependence on sin of rotator angle
! rotator angle where focus in guider is same as in science camera
gref=-90   
gdf=25*(sind[rot]-sind[gref])
gdf=0
type df gdf 

!type dfoff gdfoff
!string com 'df %i' df-dfoff
!printf 'adjustment command would be: {com}'
!!send '{com}'
!!string com 'guidefoc %i' gdf-gdfoff
!string com 'guidefoc %i' gdf
!printf 'adjustment command would be: {com}'
!!send '{com}'
!
!dfoff=df
!gdfoff=gdf

END

