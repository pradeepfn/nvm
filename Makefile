CC = gcc
CFLAGS = -Wall

TARGET = checkpoint

all : $(TARGET)

$(TARGET) : checkpoint.c checkpoint.h
	$(CC) $(CFLAGS) -g -o $(TARGET)  checkpoint.h checkpoint.c

clean :
	rm $(TARGET)
