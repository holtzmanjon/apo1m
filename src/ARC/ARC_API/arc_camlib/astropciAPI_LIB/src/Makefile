CC	   =	gcc

SRC_PATH   =	../src/
SRC	   =	$(SRC_PATH)DSPCommand.c $(SRC_PATH)LoadDspFile.c \
		$(SRC_PATH)FitsFile.c $(SRC_PATH)Memory.c \
		$(SRC_PATH)Temperature.c $(SRC_PATH)Deinterlace.c

CFLAGS	   =	-Wall
LIB 	   =	libastropciAPI.a

OBJ 	   = 	$(LIB)($(SRC_PATH)LoadDspFile.o) \
		$(LIB)($(SRC_PATH)Temperature.o) \
		$(LIB)($(SRC_PATH)DSPCommand.o) \
		$(LIB)($(SRC_PATH)Memory.o) \
		$(LIB)($(SRC_PATH)Deinterlace.o) \
#	$(LIB)($(SRC_PATH)FitsFile.o)

all: $(LIB)

$(LIB): $(OBJ)

DSPCommand.o: DSPCommand.c
	$(CC) -c $(SRC_PATH)DSPCommand.c 

LoadDspFile.o: LoadDspFile.c
	$(CC) -c $(SRC_PATH)LoadDspFile.c

FitsFile.o: FitsFile.c
	$(CC) -c $(SRC_PATH)FitsFile.c

Memory.o: Memory.c
	$(CC) -c $(SRC_PATH)Memory.c

Temperature.o: Temperature.c
	$(CC) -c $(SRC_PATH)Temperature.c

Deinterlace.o: Deinterlace.c
	$(CC) -c $(SRC_PATH)Deinterlace.c

clean:
	rm $(LIB)
