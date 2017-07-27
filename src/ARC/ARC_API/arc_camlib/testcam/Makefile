ROOT_DIR = /home/observer/arc_camlib

LOAD_PCI_LOD = 1    # The current pci.lod file is burned into the eprom
                    # Set this to 1 if there is a new pci.lod file

LOAD_UTIL_LOD = 0   # Set this to load the util.lod file


LOD_FILE_DIR = "${ROOT_DIR}/lod_files/E2V4240_LN2"

CC = gcc
CFLAGS = -O2 -Wall

LDFLAGS =   $(CFLAGS)

SHELL = /bin/sh


#DSP_LIB_PATH = ../DSPlibrary
DSP_LIB_PATH = ${ROOT_DIR}/astropciAPI_LIB/src
DSP_INC_PATH = ${ROOT_DIR}/astropciAPI_LIB/src
CFITSIO_PATH = ${ROOT_DIR}/cfitsio
MAKE = make


all: testcam

testcam: testcam.o
	${CC} ${CFLAGS} -o testcam testcam.o -L${DSP_LIB_PATH} -L${CFITSIO_PATH} -lastropciAPI -lcfitsio -lm


testcam.o: testcam.c
	${CC} ${CFLAGS} -c testcam.c -I${DSP_INC_PATH} -I${CFITSIO_PATH} \
             '-DLOD_FILE_DIR=${LOD_FILE_DIR}' -DLOAD_PCI_LOD=${LOAD_PCI_LOD} \
             -DLOAD_UTIL_LOD=${LOAD_UTIL_LOD}

clean:
	rm *.o;


