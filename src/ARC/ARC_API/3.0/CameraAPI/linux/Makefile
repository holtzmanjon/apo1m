INC_DIR		=	-I../					\
			-I../../../3.0/CArcDevice		\
			-I../../../3.0/CArcAPIClient		\
			-I../../../3.0/CDeinterlace		\
			-I../../../3.0/CFitsFile		\
			-I../../../3.0/CTiffFile		\
			-I../../../3.0/CDisplay			\
			-I../../../3.0/CImage			\
			-I../../../3.0/cfitsio/linux		\
			-I/opt/jdk1.6.0_22/include		\
			-I/opt/jdk1.6.0_22/include/linux 	\
			-I../../../3.0/tifflib/linux/include

INC_DIR_X64	=	-I../../../3.0/xpa-2.1.12/x64

LIB_DIR_X64	=	-L../../../3.0/CArcDevice/linux/x64	\
			-L../../../3.0/CArcAPIClient/linux/x64	\
			-L../../../3.0/CDeinterlace/linux/x64	\
			-L../../../3.0/CFitsFile/linux/x64	\
			-L../../../3.0/CTiffFile/linux/x64	\
			-L../../../3.0/CDisplay/linux/x64	\
			-L../../../3.0/CImage/linux/x64		\
			-L../../../3.0/cfitsio/linux/x64	\
			-L../../../3.0/xpa-2.1.12/x64		\
			-L../../../3.0/tifflib/linux/lib/x64
#			-L../../Release/x64

INC_DIR_X32	=	-I../../../3.0/xpa-2.1.12/x32

LIB_DIR_X32	=	-L../../../3.0/CArcDevice/linux/x32	\
			-L../../../3.0/CArcAPIClient/linux/x32	\
			-L../../../3.0/CDeinterlace/linux/x32	\
			-L../../../3.0/CFitsFile/linux/x32	\
			-L../../../3.0/CTiffFile/linux/x32	\
			-L../../../3.0/CDisplay/linux/x32	\
			-L../../../3.0/CImage/linux/x32		\
			-L../../../3.0/cfitsio/linux/x32	\
			-L../../../3.0/xpa-2.1.12/x32		\
			-L../../../3.0/tifflib/linux/lib/x32
#			-L../../Release/x32

LIB_FILES	=	-lCArcDevice			\
			-lCArcAPIClient			\
			-lCDeinterlace			\
			-lCFitsFile			\
			-lCTiffFile			\
			-lCDisplay			\
			-lCImage			\
			-lcfitsio			\
			-ltiff				\
			-lm -lnsl

CPP_FILES	=	../CameraAPI.cpp		\
			../DllMain.cpp

OUT_FILE	=	libCameraAPI.so


all: m64 m32

m64:
	g++ -fpic -c -Wall $(INC_DIR) $(INC_DIR_X64) $(CPP_FILES)
	g++ -shared -o x64/$(OUT_FILE) $(LIB_DIR_X64) *.o $(LIB_FILES)
	rm *.o

m32:
	g++ -m32 -fpic -c -Wall $(INC_DIR) $(INC_DIR_X32) $(CPP_FILES)
	g++ -m32 -shared -o x32/$(OUT_FILE) $(LIB_DIR_X32) *.o $(LIB_FILES)
	rm *.o

clean: clean_m64 clean_m32

clean_m64:
	rm x64/$(OUT_FILE)

clean_m32:
	rm x32/$(OUT_FILE)

