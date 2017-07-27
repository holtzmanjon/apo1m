DESTNAME = 

INSTALL  = install
RM       = rm
RANLIB   = ranlib
       
CC       = gcc

# Uncomment to build ISA interface library
#CFLAGS   = -O2 -g -c -Wall -fno-strength-reduce -DLINUX 

# Uncomment to build PPI interface library
CFLAGS   = -O2 -g -c -Wall -fno-strength-reduce -DLINUX -D_APG_PPI

INCLUDES = 
LDFLAGS  = -O2 
XLIBS    =  
MATHLIB  = -lm

LIBS     = ${XLIBS} ${MATHLIB}

LOCAL_LIBRARIES = ./libapogee.a

BINDIR   = 

LIBSRCS  =  apapi.c apdata.c apini.c aplow.c config.c apppi.c

LIBOBJS  =  apapi.o apdata.o apini.o aplow.o config.o apppi.o

all:: libapogee.a apitest

apitest: apitest.o libapogee.a
	gcc -O2 -o apitest apitest.o libapogee.a

libapogee.a: ${LIBOBJS}
	${AR} -rv libapogee.a ${LIBOBJS}
	${RANLIB} libapogee.a

apogee: apogee.o libapogee.a
	gcc -O2 -o apogee apogee.o libapogee.a

iopermiss: iopermiss.o
	gcc -O2 -o iopermiss iopermiss.o 

clean:
	-rm *.o *.a apogee apitest iopermiss
