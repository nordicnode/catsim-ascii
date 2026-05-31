CC=gcc
CFLAGS=-std=c17 -O2 -march=native -Wall -Wextra -lncursesw -lm -pthread
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

catsim: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(OBJ) catsim

.PHONY: clean
