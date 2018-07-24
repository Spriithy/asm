NAME=emu-64
CC=clang
CLFAGS=-g -std=c11 -Wall -Wextra -Werror

SRC=$(wildcard *.c) $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ=$(patsubst %.c,%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
	rm -f $(OBJ)

%.o: %.c
	clang $(CFLAGS) -c -o $@ $<

fclean: clean
	rm -f $(NAME)

re: fclean all
