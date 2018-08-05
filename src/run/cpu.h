#ifndef EMU64_H
#define EMU64_H

#include "../decode.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    // Base
    uint8_t*  text;
    size_t    text_size;
    uint8_t*  data;
    size_t    data_size;
    uint8_t*  mem;
    size_t    mem_size;
    uint64_t  reg[32];
    uint32_t* ip;
    uint64_t  hi, lo;
    uint64_t  cycles;

    // Debug
    int    debug;
    int    step_mode;
    char** text_syms;
} cpu_t;

void cpu_init(cpu_t* cpu);
void cpu_text(cpu_t* cpu, uint8_t* text, size_t text_size);
void cpu_data(cpu_t* cpu, uint8_t* data, size_t data_size);
void cpu_exec(cpu_t* cpu);

#endif // cpu.h
