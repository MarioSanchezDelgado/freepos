CC=gcc
CFLAGS=-Wall -I../
PACKAGE=`pkg-config --cflags gtk+-3.0`
LIBS=`pkg-config --libs gtk+-3.0`
OBJS=FPos_main_paEmpezar.o
EXE=freepos
OBJ_DIR=.
SRC_DIR=.
#freepos:FPos_main_paEmpezar.o 
#	$(CC) `pkg-config --cflags gtk+-3.0` -o freepos FPos_main_paEmpezar.o `pkg-config --libs gtk+-3.0`

build: $(OBJS)
	    $(CC) $(CFLAGS) $(OBJS) -o $(EXE) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	    $(CC) $(CFLAGS) -c -I. $< -o $@ $(PACKAGE)

#gcc `pkg-config --cflags gtk+-3.0` -o  freepos FPos_main_paEmpezar.c `pkg-config --libs gtk+-3.0`

