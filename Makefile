NAME=asm
CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-variable -std=c11 -g 

SHARED_SRC=$(wildcard src/shared/*.c) $(wildcard src/shared/**/*.c)
SHARED_OBJ=$(patsubst %.c, %.o, $(SHARED_SRC))

EXEC_SRC=$(wildcard src/exec/*.c) $(wildcard src/exec/**/*.c)
EXEC_OBJ=$(patsubst %.c, %.o, $(EXEC_SRC))

ASM_SRC=$(wildcard src/asm/*.c) $(wildcard src/asm/**/*.c)
ASM_OBJ=$(patsubst %.c, %.o, $(ASM_SRC))

DISASM_SRC=$(wildcard src/disasm/*.c) $(wildcard src/disasm/**/*.c)
DISASM_OBJ=$(patsubst %.c, %.o, $(DISASM_SRC))

all: exec asm disasm

exec: $(SHARED_OBJ) $(EXEC_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(EXEC_OBJ) -o $(NAME)-exec
	rm -f $(SHARED_OBJ) $(EXEC_OBJ)

asm: $(SHARED_OBJ) $(ASM_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(ASM_OBJ) -o $(NAME)
	rm -f $(SHARED_OBJ) $(ASM_OBJ)

disasm: $(SHARED_OBJ) $(DISASM_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(DISASM_OBJ) -o dis$(NAME)
	rm -f $(SHARED_OBJ) $(DISASM_OBJ)

clean:
	rm -f $(SHARED_OBJ) $(EMU_OBJ) $(AS_OBJ) $(DISAS_OBJ)

fclean: clean
	rm -f $(NAME)
	rm -f dis$(NAME)
	rm -f $(NAME)-exec

re: fclean all
