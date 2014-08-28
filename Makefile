CC = gcc
CFLAGS = -Wall

TARGET = checkpoint

all : $(TARGET)

$(TARGET) : sampleapp.c  checkpoint.c checkpoint.h
	$(CC) $(CFLAGS) -g -o $(TARGET) sampleapp.c  checkpoint.h checkpoint.c

clean :
	rm $(TARGET)
