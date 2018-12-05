CC=clang
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c11 -g -flto 
LDFLAGS=-flto 

SRC=$(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))

CPU_SRC=$(wildcard src/cpu/*.c)
CPU_OBJ=$(patsubst %.c, %.o, $(CPU_SRC))

all: cpu clean

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

cpu: $(CPU_OBJ)
	$(CC) $(LDFLAGS) $(CPU_OBJ) -o $@

clean:
	rm -f $(OBJ)

re: clean all
