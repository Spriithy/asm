#include "emu.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

#define R emu.reg

emulator_t emu;

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

void hilo_umul(uint64_t rs1, uint64_t rs2)
{
    uint64_t u1 = (rs1 & 0xffffffff);
    uint64_t v1 = (rs2 & 0xffffffff);
    uint64_t t = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k = (t >> 32);

    rs1 >>= 32;
    t = (rs1 * v1) + k;
    k = (t & 0xffffffff);
    uint64_t w1 = (t >> 32);

    rs2 >>= 32;
    t = (u1 * rs2) + k;
    k = (t >> 32);

    emu.hi = (rs1 * rs2) + w1 + k;
    emu.lo = (t << 32) + w3;
}

void hilo_mul(int64_t rs1, int64_t rs2)
{
    hilo_umul((uint64_t)rs1, (uint64_t)rs2);
    if (rs1 < 0LL)
        emu.hi -= rs2;
    if (rs2 < 0LL)
        emu.hi -= rs1;
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
            printf("int   0x%X\n", RI16_imm);
        swi(RI16_imm);
        break;

    case 0x02: /* int %rs1 */
        if (emu.debug)
            printf("intr  $r%d\n", RS1);
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
                printf("lb    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lb    $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        R[RD] = *(int8_t*)addr;
        break;

    case 0x05: /* lbu %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lbu   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lbu   $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        R[RD] = *(uint8_t*)addr;
        break;

    case 0x06: /* lh %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lh    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lh    $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        align16check(addr);
        R[RD] = *(int16_t*)addr;
        break;

    case 0x07: /* lhu %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lhu   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lhu   $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        align16check(addr);
        R[RD] = *(uint16_t*)addr;
        break;

    case 0x08: /* lui %RD, $imm16 */
        if (emu.debug)
            printf("lui   $r%d, 0x%X\n", RD, RI16_imm);
        R[RD] = (RI16_imm << 16) | (R[RD] & 0xfffff);
        break;

    case 0x09: /* lw %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lw    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("lw    $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RS1] + OFFSET;
        align32check(addr);
        R[RD] = *(int32_t*)addr;
        break;

    case 0x0a: /* lwu %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("lwu   $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("lwu   $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RS1] + OFFSET;
        align32check(addr);
        R[RD] = *(uint32_t*)addr;
        break;

    case 0x0b: /* ld %RD, offset(%RS1) */
        if (emu.debug) {
            if (OFFSET)
                printf("ld    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("ld    $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RS1] + OFFSET;
        align64check(addr);
        R[RD] = *(uint64_t*)addr;
        break;

    case 0x0c: /* sb %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sb    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("sb    $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RD] + OFFSET;
        *(uint8_t*)addr = R[RS1] & 0xff;
        break;

    case 0x0d: /* sh %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sh    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("sh    $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RD] + OFFSET;
        align16check(addr);
        *(uint16_t*)addr = R[RS1] & 0xffff;
        break;

    case 0x0e: /* sw %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sw    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("sw    $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RD] + OFFSET;
        align32check(addr);
        *(uint32_t*)addr = R[RS1] & 0xffffffff;
        break;

    case 0x0f: /* sd %RS1, offset(%RD) */
        if (emu.debug) {
            if (OFFSET)
                printf("sd    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("sd    $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RD] + OFFSET;
        align64check(addr);
        *(uint64_t*)addr = R[RS1];
        break;

    case 0x10: /* mov %RD, %RS1 */
        if (emu.debug)
            printf("mov   $r%d, $r%d\n", RD, RS1);
        R[RD] = R[RS1];
        break;

    case 0x11: /* mfhi %rd */
        if (emu.debug)
            printf("mfhi  $r%d\n", RD);
        R[RD] = emu.hi;
        break;

    case 0x12: /* mthi %rs1 */
        if (emu.debug)
            printf("mthi  $r%d\n", RS1);
        emu.hi = R[RS1];
        break;

    case 0x13: /* mflo %rd */
        if (emu.debug)
            printf("mflo  $r%d\n", RD);
        R[RD] = emu.lo;
        break;

    case 0x14: /* mtlo %rs1 */
        if (emu.debug)
            printf("mtlo  $r%d\n", RS1);
        emu.lo = R[RS1];
        break;

    case 0x15: /* slt $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("slt   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = ((int64_t)R[RS1] < (int64_t)R[RS2]);
        break;

    case 0x16: /* sltu $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("sltu  $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = (R[RS1] < R[RS2]);
        break;

    case 0x17: /* slti $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("slti  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = ((int64_t)R[RS1] < (int16_t)RI16_imm);
        break;

    case 0x18: /* sltiu $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("sltiu $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = (R[RS1] < RI16_imm);
        break;

    case 0x19: /* eq $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("eq    $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = (R[RS1] == R[RS2]);
        break;

    case 0x1a: /* eqi $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("eqi   $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = ((int64_t)R[RS1] == (int16_t)RI16_imm);
        break;

    case 0x1b: /* eqiu $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("eqiu  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = (R[RS1] == RI16_imm);
        break;

    case 0x20: /* or $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("or    $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] | R[RS2];
        break;

    case 0x21: /* ori $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("ori   $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] | RI16_imm;
        break;

    case 0x22: /* and $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("and   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] & R[RS2];
        break;

    case 0x23: /* andi $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("andi  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] & RI16_imm;
        break;

    case 0x24: /* xor $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("xor   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] ^ R[RS2];
        break;

    case 0x25: /* xori $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("xori  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] ^ RI16_imm;
        break;

    case 0x26: /* not $rd, $rs1 */
        if (emu.debug)
            printf("not   $r%d, $r%d\n", RD, RS1);
        R[RD] = ~R[RS1];
        break;

    case 0x27: /* nor $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("nor   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = ~(R[RS1] | R[RS2]);
        break;

    case 0x28: /* shl $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("shl   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] << R[RS2];
        break;

    case 0x29: /* shli $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("shli  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] << RI16_imm;
        break;

    case 0x2a: /* shr $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("shr   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] >> R[RS2];
        break;

    case 0x2b: /* shri $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("shri  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] >> RI16_imm;
        break;

    case 0x2c: /* add $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("add   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2d: /* addi $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("addi  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = (int64_t)R[RS1] + (int16_t)RI16_imm;
        break;

    case 0x2e: /* addiu $rd, $rs1, #imm16 */
        if (emu.debug)
            printf("addiu $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2f: /* sub $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("sub   $r%d, $r%d, 0x%X\n", RD, RS1, RS2);
        R[RD] = (int64_t)R[RS1] - (int16_t)RI16_imm;
        break;

    case 0x30: /* subu $rd, $rs1, $rs2 */
        if (emu.debug)
            printf("subu  $r%d, $r%d, 0x%X\n", RD, RS1, RS2);
        R[RD] = R[RS1] - RI16_imm;
        break;

    case 0x31: /* mul $rs1, $rs2 */
        if (emu.debug)
            printf("mul   $r%d, $r%d\n", RS1, RS2);
        hilo_mul(R[RS1], R[RS2]);
        break;

    case 0x32: /* mulu $rs1, $rs2 */
        if (emu.debug)
            printf("mulu  $r%d, $r%d\n", RS1, RS2);
        hilo_umul(R[RS1], R[RS2]);
        break;

    case 0x33: /* div $rs1, $rs2 */
        if (emu.debug)
            printf("div   $r%d, $r%d\n", RS1, RS2);
        emu.hi = (int64_t)R[RS1] % (int64_t)R[RS2];
        emu.lo = (int64_t)R[RS1] / (int64_t)R[RS2];
        break;

    case 0x34: /* divu $rs1, $rs2 */
        if (emu.debug)
            printf("divu  $r%d, $r%d\n", RS1, RS2);
        emu.hi = R[RS1] % R[RS2];
        emu.lo = R[RS1] / R[RS2];
        break;
    }
    emu.ip++, emu.cycles++;
    goto cpu_loop;
}