CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=gnu11
LDLIBS=-lreadline

SRC=bccsh.c builtin.c aux.c
OBJ=$(SRC:.c=.o)
EXEC=bccsh

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

clean:
	rm -rf $(OBJ) $(EXEC)
