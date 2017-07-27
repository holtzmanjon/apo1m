INC_DIR		=	-I../	\
			-I../../ArcAPIClientServerCommon \
			-I../../CArcDevice \
			-I../../../3.0/CFitsFile \
			-I../../../3.0/CTiffFile \
			-I../../../3.0/CImage \
			-I../../../3.0/cfitsio/linux \
			-I../../../3.0/tifflib/linux/include

CPP_FILES	=	../CArcAPIClient.cpp		\
			../DllMain.cpp

OUT_FILE	=	libCArcAPIClient.so


all: m64 m32

m64:
	g++ -fPIC -c -Wall $(INC_DIR) $(CPP_FILES)
	g++ -shared -o x64/$(OUT_FILE) *.o
	rm *.o

m32:
	g++ -m32 -fPIC -c -Wall $(INC_DIR) $(CPP_FILES)
	g++ -m32 -shared -o x32/$(OUT_FILE) *.o
	rm *.o

clean: clean_m64 clean_m32

clean_m64:
	rm x64/$(OUT_FILE)

clean_m32:
	rm x32/$(OUT_FILE)

clean_backups:
	rm ../*.*~

