CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=gnu11
LDLIBS=-lreadline
LDPTH=-pthread

SRC1=bccsh.c builtin.c aux.c
OBJ1=$(SRC1:.c=.o)

SRC2=ep1.c scheduler.c
OBJ2=$(SRC2:.c=.o)

EXEC1=bccsh
EXEC2=ep1

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(EXEC2): $(OBJ2)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDPTH)

clean:
	rm -rf $(OBJ1) $(EXEC1) $(OBJ2) $(EXEC2)
