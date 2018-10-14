CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-variable -std=c11 -g 

SHARED_SRC=$(wildcard src/shared/*.c) $(wildcard src/shared/**/*.c)
SHARED_OBJ=$(patsubst %.c, %.o, $(SHARED_SRC))

EXEC_SRC=$(wildcard src/exec/*.c) $(wildcard src/exec/**/*.c)
EXEC_OBJ=$(patsubst %.c, %.o, $(EXEC_SRC))

DISASM_SRC=$(wildcard src/disasm/*.c) $(wildcard src/disasm/**/*.c)
DISASM_OBJ=$(patsubst %.c, %.o, $(DISASM_SRC))

all: exec disasm clean

exec: $(SHARED_OBJ) $(EXEC_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(EXEC_OBJ) -o asm-exec

disasm: $(SHARED_OBJ) $(DISASM_OBJ)
	$(CC) $(CFLAGS) $(SHARED_OBJ) $(DISASM_OBJ) -o disasm

clean:
	rm -f $(SHARED_OBJ) $(EXEC_OBJ) $(DISASM_OBJ)

fclean: clean
	rm -f disasm
	rm -f asm-exec

re: fclean all
