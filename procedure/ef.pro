rd 1 ../observe/980113/980113.052
rd 2 ../observe/980113/980113.054
window 1 box=1
window 2 box=1
fixhead 1 origin
fixhead 2 origin
create 11 n=512 sr=1 sc=1
copy 12 11
 
focus={1:focus}
fits 11 float=focus focus
add 11 1
wd 11 ./post
 
focus={2:focus}
fits 12 float=focus focus
add 12 2
wd 12 ./pre
 
end
