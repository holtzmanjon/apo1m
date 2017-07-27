INC_DIR		=	-I../ \
			-I../../tifflib/linux/include

LIB_DIR		=	-L../../tifflib/linux/lib

CPP_FILES	=	../CTiffFile.cpp \
			../DllMain.cpp

OUT_FILE	=	libCTiffFile.so


all: m64 m32

m64:
	g++ -fpic -c -Wall $(INC_DIR) $(CPP_FILES)
	g++ -shared -o x64/$(OUT_FILE) $(LIB_DIR)/x64 *.o -ltiff
	rm *.o

m32:
	g++ -m32 -fpic -c -Wall $(INC_DIR) $(CPP_FILES)
	g++ -m32 -shared -o x32/$(OUT_FILE) $(LIB_DIR)/x32 *.o -ltiff
	rm *.o

clean: clean_m64 clean_m32

clean_m64:
	rm x64/$(OUT_FILE)

clean_m32:
	rm x32/$(OUT_FILE)
