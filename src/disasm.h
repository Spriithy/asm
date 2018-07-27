#ifndef EMU64_DISASM_H
#define EMU64_DISASM_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void disasm(FILE* f, uint32_t* code, size_t code_size);

#endif // disasm.h
