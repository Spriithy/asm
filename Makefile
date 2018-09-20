NAME=emu-64
CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-variable -std=c11 -g 

SHARED_SRC=$(wildcard shared/*.c) $(wildcard shared/**/*.c)
SHARED_OBJ=$(patsubst %.c, %.o, $(SHARED_SRC))

EMU_SRC=$(wildcard emu/*.c) $(wildcard emu/**/*.c)
EMU_OBJ=$(patsubst %.c, %.o, $(EMU_SRC))

AS_SRC=$(wildcard as/*.c) $(wildcard as/**/*.c)
AS_OBJ=$(patsubst %.c, %.o, $(AS_SRC))

DISAS_SRC=$(wildcard disas/*.c) $(wildcard disas/**/*.c)
DISAS_OBJ=$(patsubst %.c, %.o, $(DISAS_SRC))

all: emu as disas

emu: $(SHARED_OBJ) $(EMU_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(EMU_OBJ) -o $(NAME)
	rm -f $(SHARED_OBJ) $(EMU_OBJ)

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
