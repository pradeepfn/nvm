CC=gcc
CFLAGS=-I.  -Wall -g
DEPS = checkpoint.h utils.h
OBJ = checkpoint.o utils.o 
P1 = p1.o 
P2 = p2.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

checkpoint: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

all : checkpoint

p1 : $(OBJ) $(P1)
	gcc -o $@ $^ $(CFLAGS)


p2 : $(OBJ) $(P2)
	gcc -o $@ $^ $(CFLAGS)

clean :
	rm checkpoint *.o

remove :
	rm nvm.* mmap.file* 
