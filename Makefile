CC=gcc
CFLAGS=-I.  -Wall
DEPS = checkpoint.h utils.h
OBJ = checkpoint.o utils.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

checkpoint: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

all : checkpoint

clean :
	rm checkpoint *.o
