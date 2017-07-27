INC_DIR		=	-I../

CPP_FILES	=	../CArcDevice.cpp	\
			../ArcOSDefs.cpp	\
			../CArcPCIBase.cpp	\
			../CArcPCI.cpp		\
			../CArcPCIe.cpp		\
			../CArcTools.cpp	\
			../CLog.cpp		\
			../CStringList.cpp	\
			../TempCtrl.cpp		\
			../DllMain.cpp

LIB_DIR		=	-L../../Release

OUT_FILE	=	libCArcDevice.so

COMP_FLAGS	=	-fPIC -std=c++0x -c -Wall

LINK_FLAGS	=	-shared

all: m64 m32

m64:
	g++ $(COMP_FLAGS) $(INC_DIR) $(CPP_FILES)
	g++ $(LINK_FLAGS) -o x64/$(OUT_FILE) *.o $(LIB_DIR)/x64 $(LIBS)
	rm *.o

m32:
	g++ -m32 $(COMP_FLAGS) $(INC_DIR) $(CPP_FILES)
	g++ -m32 $(LINK_FLAGS) -o x32/$(OUT_FILE) *.o $(LIB_DIR)/x32 $(LIBS)
	rm *.o

clean: clean_m64 clean_m32

clean_m64:
	rm x64/$(OUT_FILE)

clean_m32:
	rm x32/$(OUT_FILE)

clean_backups:
	rm ../*.*~

