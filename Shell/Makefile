CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=gnu11
DEPFLAGS=-lreadline
TARGET=bccsh

$(TARGET): builtin.o $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) builtin.o $(TARGET).c $(DEPFLAGS)

builtin.o: builtin.c builtin.h
	$(CC) $(CFLAGS) -c builtin.c

clean:
	rm -f *.o *.out $(TARGET)
