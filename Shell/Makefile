CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c11
RL=-lreadline
TARGET=bccsh

$(TARGET): $(TARGET).c $(TARGET).h
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(RL)

clang: $(TARGET).c $(TARGET).h
	clang $(CFLAGS) -o $(TARGET) $(TARGET).c $(RL)

clean:
	rm -f *.o *.out $(TARGET)
