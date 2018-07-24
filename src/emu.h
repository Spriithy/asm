#ifndef EMU64_H
#define EMU64_H

#include <stdint.h>

#define Mb (1024 * 1024)

#define R emu.reg

#define OP (int)(*emu.ip & 0xff)
#define RD (int)((*emu.ip >> 8) & 0x1f)
#define RS1 (int)((*emu.ip >> 13) & 0x1f)
#define RS2 (int)((*emu.ip >> 18) & 0x1f)
#define OFFSET (int)(sext(*emu.ip >> 23, 9))
#define RI16_imm (uint16_t)(*emu.ip >> 16)

typedef struct {
    uint8_t   mem[64 * Mb];
    uint64_t  reg[32];
    uint64_t  cycles;
    uint32_t* code;
    uint32_t* ip;
    int       debug;
} emulator_t;

void exec(void);

#endif // emu.h
