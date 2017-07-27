parameter ha zd decd phi

!parang1=arcsind[cosd[latitude]*sind[ha]/sind[zd]]

cp=cosd[phi]
sqsz=cp*sind[ha]
cqsz=sind[phi]*cosd[decd]-cp*sind[decd]*cosd[ha]
parang=arctan2[sqsz,cqsz]*180/pi
end

