#ifndef EMU64_H
#define EMU64_H

#include "../common.h"
#include "../decode.h"
#include <stdint.h>

typedef struct {
    // Base
    uint32_t  text[0x80000];
    uint8_t   data[0x1e00000];
    uint64_t  reg[32];
    uint32_t* ip;
    uint64_t  hi, lo;
    uint64_t  cycles;

    // Debug
    int    debug;
    int    step_mode;
    char** text_syms;
} cpu_t;

void exec(void);

#endif // cpu.h
