#ifndef EMU64_DISASM_H
#define EMU64_DISASM_H

#include "run/cpu.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int   reg_index(char* reg);
char* reg_name(int reg);
void  disasm(cpu_t* cpu, FILE* f, uint32_t* code, size_t code_size);

#endif // disasm.h
