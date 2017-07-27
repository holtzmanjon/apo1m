# The following two variables must be changed if there are
# any changes to the ARC .lod files; the LOD_FILE_DIR should
# point to the full path of the lod file location.

#LOAD_PCI_LOD = 1    # The current pci.lod file is burned into the eprom
                    # Set this to 1 if there is a new pci.lod file
LOAD_PCI_LOD = 0    # The current pci.lod file is burned into the eprom
LOD_FILE_DIR = "./lod_files/E2V4240_LN2"

CC = gcc
CFLAGS = -O2 -Wall

LDFLAGS =   $(CFLAGS)

SHELL = /bin/sh

DSP_LIB_PATH = ./astropciAPI_LIB/LINUX
CFITSIO_PATH = ../cfitsio
DRIVER_PATH = ./astropciV1.7
TESTCAM_PATH = ./testcam
LCCD_PATH = ./lccd
MAKE = make


#all: driv DSPlib tcam

all: driv DSPlib lc

lc: 
	cd ${LCCD_PATH}; ${MAKE}; cd ..;

tcam: 
	cd ${TESTCAM_PATH}; ${MAKE}; cd ..;

driv:
	cd ${DRIVER_PATH}; ${MAKE} all; ${MAKE} all; cd ..;

DSPlib:
	cd ${DSP_LIB_PATH}; ${MAKE}; cd ..;

clean:
	cd ${DSP_LIB_PATH}; rm *.o *.a; cd ..;
	cd ${CFITSIO_PATH}; rm *.o *.a; cd ..;
	cd ${TESTCAM_PATH}; rm *.o; cd ..;


