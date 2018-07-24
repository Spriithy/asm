#include "emu.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

emulator_t emu;

#define sext(imm, n) ((imm ^ (1U << (n - 1)) - (1U << (n - 1))))

static void swi(uint32_t icode)
{
    switch (icode) {
    case 0x00: /* exit */
        free(emu.code);
        exit(R[1]);
    case 0x01: /* reg dump */
        for (int i = 0; i < 32; i++) {
            printf("r%-2d   0x%llX   (%llu)\n", i, R[i], R[i]);
        }
        break;
    }
}

static inline void align16check(uint64_t addr)
{
    if (addr & 0x1) {
        puts("Address error: 16-bit address is not aligned");
        exit(1);
    }
}

static inline void align32check(uint64_t addr)
{
    if (addr & 0x3) {
        puts("Address error: 32-bit address is not aligned");
        exit(1);
    }
}
static inline void align64check(uint64_t addr)
{
    if (addr & 0x7) {
        puts("Address error: 64-bit address is not aligned");
        exit(1);
    }
}

void exec()
{
    uint64_t addr;
    emu.cycles = 0;
    emu.debug = 1;
    emu.ip = emu.code;
    R[1] = (uint64_t)emu.mem;

cpu_loop:
    printf("%p 0x%-10x ", emu.ip, *emu.ip);
    switch (OP) {
    case 0x00: /* nop */
        if (emu.debug)
            puts("nop");
        break;
    case 0x01: /* int $imm */
        if (emu.debug)
            printf("int  0x%X\n", RI16_imm);
        swi(RI16_imm);
        break;
    case 0x02: /* int %rs1 */
        if (emu.debug)
            printf("intr $r%d\n", RS1);
        swi(R[RS1]);
        break;
    case 0x03: /* brkpt */
        if (emu.debug)
            printf("brkpt\n");
        breakpoint();
        break;
    case 0x04: /* lb %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lb   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lb   $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        R[RD] = *(int8_t*)addr;
        break;
    case 0x05: /* lbu %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lbu  $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lbu  $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        R[RD] = *(uint8_t*)addr;
        break;
    case 0x06: /* lh %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lh $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lh $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        align16check(addr);
        R[RD] = *(int16_t*)addr;
        break;
    case 0x07: /* lhu %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lhu $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lhu $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        align16check(addr);
        R[RD] = *(uint16_t*)addr;
        break;
    case 0x08: /* lui %RD, $imm16 */
        if (emu.debug)
            printf("lui  $r%d, 0x%X\n", RD, RI16_imm);
        R[RD] = (RI16_imm << 16) | (R[RD] & 0xfffff);
        break;
    case 0x09: /* lw %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lw   $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("lw   $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RS1] + OFFSET;
        align32check(addr);
        R[RD] = *(int32_t*)addr;
        break;
    case 0x0a: /* lwu %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lwu  $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("lwu  $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RS1] + OFFSET;
        align32check(addr);
        R[RD] = *(uint32_t*)addr;
        break;
    case 0x0b: /* ld %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("ld   $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("ld   $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RS1] + OFFSET;
        align64check(addr);
        R[RD] = *(uint64_t*)addr;
        break;
    case 0x0c: /* sb %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sb   $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("sb   $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RD] + OFFSET;
        *(uint8_t*)addr = R[RS1] & 0xff;
        break;
    case 0x0d: /* sh %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sh   $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("sh   $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RD] + OFFSET;
        align16check(addr);
        *(uint16_t*)addr = R[RS1] & 0xffff;
        break;
    case 0x0e: /* sw %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sw   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("sw   $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RD] + OFFSET;
        align32check(addr);
        *(uint32_t*)addr = R[RS1] & 0xffffffff;
        break;
    case 0x0f: /* sd %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sd   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("sd   $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RD] + OFFSET;
        align64check(addr);
        *(uint64_t*)addr = R[RS1];
        break;
    case 0x10: /* mov %RD, %RS1 */
        if (emu.debug)
            printf("mov  $r%d, $r%d\n", RD, RS1);
        R[RD] = R[RS1];
        break;

    case 0x21: /* ori %RD, $imm16 */
        if (emu.debug)
            printf("ori  $r%d, 0x%X\n", RD, RI16_imm);
        R[RD] |= RI16_imm;
        break;
    }
    emu.ip++, emu.cycles++;
    goto cpu_loop;
}