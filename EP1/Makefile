CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=gnu11
LDLIBS=-pthread

SRC=ep1.c scheduler.c
OBJ=$(SRC:.c=.o)
EXEC=ep1

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

clean:
	rm -rf $(OBJ) $(EXEC)