INC_DIR		=	-I../ \
			-I../../xpa-2.1.12/x64 \
			-I../../xpa-2.1.12/x32

LIB_DIR		=	-L../../xpa-2.1.12

CPP_FILES	=	../CDisplay.cpp \
			../DllMain.cpp

OUT_FILE	=	libCDisplay.so


all: m64 m32

m64:
	g++ -fpic -c -Wall $(INC_DIR) $(CPP_FILES)
	g++ -shared -o x64/$(OUT_FILE) $(LIB_DIR)/x64 *.o -lxpa
	rm *.o

m32:
	g++ -m32 -fpic -c -Wall $(INC_DIR) $(CPP_FILES)
	g++ -m32 -shared -o x32/$(OUT_FILE) $(LIB_DIR)/x32 *.o -lxpa
	rm *.o

clean: clean_m64 clean_m32

clean_m64:
	rm x64/$(OUT_FILE)

clean_m32:
	rm x32/$(OUT_FILE)
