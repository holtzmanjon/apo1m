MACHINE=linux
CC=cc
CFLAGS=-c
slalib_c: addet.o afin.o airmas.o altaz.o amp.o ampqk.o aop.o \
aoppa.o aoppat.o aopqk.o av2m.o bear.o caf2r.o caldj.o \
calyd.o cc2s.o cc62s.o cd2tf.o cldj.o clyd.o cr2af.o \
cr2tf.o cs2c.o cs2c6.o ctf2d.o ctf2r.o daf2r.o dafin.o \
dat.o dav2m.o dbear.o dbjin.o dc62s.o dcc2s.o dcmpf.o \
dcs2c.o dd2tf.o de2h.o deuler.o dfltin.o dh2e.o dimxv.o \
djcal.o djcl.o dm2av.o dmat.o dmoon.o dmxm.o dmxv.o \
dr2af.o dr2tf.o drange.o dranrm.o ds2c6.o ds2tp.o dsep.o \
dt.o dtf2d.o dtf2r.o dtp2s.o dtp2v.o dtprd.o dtps2c.o \
dtpv2c.o dtt.o dv2tp.o dvdv.o dvn.o dvxv.o e2h.o \
earth.o ecleq.o ecmat.o ecor.o eg50.o epb.o epb2d.o \
epco.o epj.o epj2d.o eqecl.o eqeqx.o eqgal.o etrms.o \
euler.o evp.o fitxy.o fk425.o fk45z.o fk524.o fk54z.o \
flotin.o galeq.o galsup.o ge50.o geoc.o gmst.o gmsta.o \
gresid.o h2e.o imxv.o intin.o invf.o kbj.o m2av.o \
map.o mappa.o mapqk.o mapqkz.o moon.o mxm.o mxv.o \
nut.o nutc.o oap.o oapqk.o obs.o pa.o pcd.o pda2h.o \
pdq2h.o planet.o pm.o prebn.o prec.o preces.o precl.o \
prenut.o pvobs.o pxy.o random.o range.o ranorm.o rcc.o \
rdplan.o refco.o refro.o refv.o refz.o rverot.o rvgalc.o \
rvlg.o rvlsrd.o rvlsrk.o s2tp.o sep.o smat.o subet.o \
supgal.o svd.o svdcov.o svdsol.o tp2s.o tp2v.o tprd.o \
tps2c.o tpv2c.o unpcd.o v2tp.o vdv.o vn.o vxv.o \
wait.o xy2xy.o zd.o 
	ar rv slalib_c.$(MACHINE).a *.o
a: a.o
	cc -o /home/avalon2/holtz/bin/a a.c slalib_c.osf1axp.a -lm -lc

