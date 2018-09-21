NAME=emu-64
CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-variable -std=c11 -g 

SHARED_SRC=$(wildcard src/shared/*.c) $(wildcard src/shared/**/*.c)
SHARED_OBJ=$(patsubst %.c, %.o, $(SHARED_SRC))

EXEC_SRC=$(wildcard src/exec/*.c) $(wildcard src/exec/**/*.c)
EXEC_OBJ=$(patsubst %.c, %.o, $(EXEC_SRC))

AS_SRC=$(wildcard src/as/*.c) $(wildcard src/as/**/*.c)
AS_OBJ=$(patsubst %.c, %.o, $(AS_SRC))

DISAS_SRC=$(wildcard src/disas/*.c) $(wildcard src/disas/**/*.c)
DISAS_OBJ=$(patsubst %.c, %.o, $(DISAS_SRC))

all: exec as disas

exec: $(SHARED_OBJ) $(EXEC_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(EXEC_OBJ) -o $(NAME)
	rm -f $(SHARED_OBJ) $(EXEC_OBJ)

as: $(SHARED_OBJ) $(AS_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(AS_OBJ) -o $(NAME)-as
	rm -f $(SHARED_OBJ) $(AS_OBJ)

disas: $(SHARED_OBJ) $(DISAS_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(DISAS_OBJ) -o $(NAME)-disas
	rm -f $(SHARED_OBJ) $(DISAS_OBJ)

clean:
	rm -f $(SHARED_OBJ) $(EMU_OBJ) $(AS_OBJ) $(DISAS_OBJ)

fclean: clean
	rm -f $(NAME)
	rm -f $(NAME)-as
	rm -f $(NAME)-disas

re: fclean all
