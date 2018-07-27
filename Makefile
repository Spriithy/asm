NAME=emu-64
CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-variable -std=c11 -g 

SRC=$(wildcard *.c) $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ=$(patsubst %.c,%.o, $(SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(SRC) -o $(NAME)
	rm -f $(OBJ)

#%.o: %.c
#	$(CC) $(CFLAGS) -c -o $@ $<

fclean: clean
	rm -f $(NAME)

re: fclean all
