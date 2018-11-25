CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-variable -pedantic -std=c11 -g 
LDFLAGS=

SRC=$(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))

all: asm clean

asm: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o asm

clean:
	rm -f $(OBJ)

fclean: clean

re: fclean all
