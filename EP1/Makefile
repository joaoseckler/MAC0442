CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic -std=gnu11
LDLIBS=-lreadline
LDPTH=-pthread

SRC1=bccsh.c builtin.c aux.c
OBJ1=$(SRC1:.c=.o)
H1=builtin.h aux.h

SRC2=ep1.c scheduler.c
OBJ2=$(SRC2:.c=.o)
H2=scheduler.h

EXEC1=bccsh
EXEC2=ep1

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJ1) $(H1)
	$(CC) $(CFLAGS) -o $@ $(SRC1) $(LDLIBS)

$(EXEC2): $(OBJ2) $(H2)
	$(CC) $(CFLAGS) -o $@ $(SRC2) $(LDPTH)

clean:
	rm -rf $(OBJ1) $(EXEC1) $(OBJ2) $(EXEC2)
