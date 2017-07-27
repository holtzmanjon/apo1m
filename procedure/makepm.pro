$'rm' ../images/{root}/{root}pm.dat
yy=2000+ifix[{root}/10000]
mm=ifix[({root}-(yy-2000)*10000)/100]
dd={root}-(yy-2000)*10000-(mm*100)
string ppp '%i %i %i' yy mm dd
$/home/tcomm/1m/tcomm/posrecen {ppp} < ../images/{root}/{root}pm.out > ../images/{root}/{root}pm.dat
$/home/tcomm/1m/tcomm/posrecen {ppp} < ../images/{root}/{root}{pmfile}.out > ../images/{root}/{root}{pmfile}.dat

send 'inst 1'
ver n
end

