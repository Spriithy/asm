#ifndef EMU64_EXEC_H
#define EMU64_EXEC_H

#include "program.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t pc, hi, lo;
    uint64_t reg[32];
    uint8_t  mem[0x1000000]; // 16 Mb
    uint8_t* text;
    size_t   text_size;
    uint8_t* data;
    size_t   data_size;
    uint8_t* rdata;
    size_t   rdata_size;

    int    debug;
    int    step;
    char** syms;
} core_t;

void core_load(core_t* core, program_t* prog);
void core_exec(core_t* core);

#endif // exec.h
