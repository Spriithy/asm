#ifndef EMU64_H
#define EMU64_H

#include "../common.h"
#include <stdint.h>

#define Mb (1024 * 1024)

#define sext(imm, n) ((imm ^ (1U << (n - 1)) - (1U << (n - 1))))

#define OP (int)(*cpu.ip & 0x3f)
#define RD (int)((*cpu.ip >> 6) & 0x1f)
#define RS1 (int)((*cpu.ip >> 11) & 0x1f)
#define RS2 (int)((*cpu.ip >> 16) & 0x1f)
#define OFFSET (int)(sext(*cpu.ip >> 21, 11))
#define RI16_imm (uint16_t)(*cpu.ip >> 16)
#define I24_imm (uint32_t)(*cpu.ip >> 8)

typedef struct {
    uint8_t   mem[64 * Mb];
    uint64_t  reg[32];
    uint64_t  hi, lo;
    uint64_t  cycles;
    uint32_t* code;
    uint32_t* ip;
#if DEBUG
    int    step_mode;
    char** labels;
#endif
} cpu_t;

void exec(void);

#endif // cpu.h
