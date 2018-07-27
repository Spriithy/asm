#include "emu.h"
#include "breakpoint.h"
#include <stdio.h>
#include <stdlib.h>

#define R emu.reg
#define GP R[28]
#define SP R[29]
#define FP R[30]
#define RA R[31]

#define PUSH(X) *(uint64_t*)(SP -= 8) = (X);
#define PUSHW(X) *(uint32_t*)(SP -= 4) = (X);

#define POP(X)                \
    {                         \
        (X) = *(uint64_t*)SP; \
        SP += 8;              \
    }
#define POPW(X)               \
    {                         \
        (X) = *(uint32_t*)SP; \
        SP += 4;              \
    }

#define setip(addr) emu.ip = (uint32_t*)(addr)

emulator_t emu;

static void swi(uint32_t icode)
{
    switch (icode) {
    case 0x00: /* exit */
        free(emu.code);
        exit(R[4]);
    case 5:
        putchar((int)R[4]);
        break;
    }
}

static void save_frame()
{
    PUSH(RA);
    PUSH(FP);
    RA = (uint64_t)emu.ip;
    FP = SP;
    setip(emu.code + I24_imm - 1);

    PUSH(R[16]);
    PUSH(R[17]);
    PUSH(R[18]);
    PUSH(R[19]);
    PUSH(R[20]);
    PUSH(R[21]);
    PUSH(R[22]);
    PUSH(R[23]);
}

static void restore_frame()
{
    POP(R[23]);
    POP(R[22]);
    POP(R[21]);
    POP(R[20]);
    POP(R[19]);
    POP(R[18]);
    POP(R[17]);
    POP(R[16]);

    setip(RA);
    POP(FP);
    POP(RA);
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
    emu.ip = emu.code;
    GP = FP = SP = (uint64_t)emu.mem + sizeof(emu.mem);

cpu_loop:
    if (DEBUG)
        printf("0x%-6X 0x%-10x ", (int)(emu.ip - emu.code), *emu.ip);

    switch (OP) {
    case 0x00: /* nop */
        if (DEBUG)
            puts("nop");
        break;

    case 0x01: /* int $imm */
        if (DEBUG)
            printf("int   0x%X\n", RI16_imm);
        swi(RI16_imm);
        break;

    case 0x02: /* int %rs1 */
        if (DEBUG)
            printf("intr  $r%d\n", RS1);
        swi(R[RS1]);
        break;

    case 0x03: /* breakpoint */
        if (DEBUG) {
            printf("breakpoint\n");
            breakpoint();

            emu.ip++, emu.cycles++;
            R[0] = 0x0; // $r0 is hard wired to 0
            goto cpu_loop;
        }
        break;

    case 0x04: /* lb %RD, offset(%RS1) */
        if (DEBUG) {
            if (OFFSET)
                printf("lb    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lb    $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        R[RD] = *(int8_t*)addr;
        break;

    case 0x05: /* lbu %RD, offset(%RS1) */
        if (DEBUG) {
            if (OFFSET)
                printf("lbu   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                printf("lbu   $r%d, $r%d\n", RD, RS1);
        }
        addr = R[RS1] + OFFSET;
        R[RD] = *(uint8_t*)addr;
        break;

    case 0x06: /* lh %RD, offset(%RS1) */
        if (DEBUG) {
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
        if (DEBUG) {
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
        if (DEBUG)
            printf("lui   $r%d, 0x%X\n", RD, RI16_imm);
        R[RD] = (RI16_imm << 16) | (R[RD] & 0xfffff);
        break;

    case 0x09: /* lw %RD, offset(%RS1) */
        if (DEBUG) {
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
        if (DEBUG) {
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
        if (DEBUG) {
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
        if (DEBUG) {
            if (OFFSET)
                printf("sb    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                printf("sb    $r%d, $r%d\n", RS1, RD);
        }
        addr = R[RD] + OFFSET;
        *(uint8_t*)addr = R[RS1] & 0xff;
        break;

    case 0x0d: /* sh %RS1, offset(%RD) */
        if (DEBUG) {
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
        if (DEBUG) {
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
        if (DEBUG) {
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
        if (DEBUG)
            printf("mov   $r%d, $r%d\n", RD, RS1);
        R[RD] = R[RS1];
        break;

    case 0x11: /* mfhi %rd */
        if (DEBUG)
            printf("mfhi  $r%d\n", RD);
        R[RD] = emu.hi;
        break;

    case 0x12: /* mthi %rs1 */
        if (DEBUG)
            printf("mthi  $r%d\n", RS1);
        emu.hi = R[RS1];
        break;

    case 0x13: /* mflo %rd */
        if (DEBUG)
            printf("mflo  $r%d\n", RD);
        R[RD] = emu.lo;
        break;

    case 0x14: /* mtlo %rs1 */
        if (DEBUG)
            printf("mtlo  $r%d\n", RS1);
        emu.lo = R[RS1];
        break;

    case 0x15: /* slt $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("slt   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = ((int64_t)R[RS1] < (int64_t)R[RS2]);
        break;

    case 0x16: /* sltu $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("sltu  $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = (R[RS1] < R[RS2]);
        break;

    case 0x17: /* slti $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("slti  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = ((int64_t)R[RS1] < (int16_t)RI16_imm);
        break;

    case 0x18: /* sltiu $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("sltiu $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = (R[RS1] < RI16_imm);
        break;

    case 0x19: /* eq $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("eq    $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = (R[RS1] == R[RS2]);
        break;

    case 0x1a: /* eqi $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("eqi   $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = ((int64_t)R[RS1] == (int16_t)RI16_imm);
        break;

    case 0x1b: /* eqiu $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("eqiu  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = (R[RS1] == RI16_imm);
        break;

    case 0x20: /* or $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("or    $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] | R[RS2];
        break;

    case 0x21: /* ori $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("ori   $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] | RI16_imm;
        break;

    case 0x22: /* and $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("and   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] & R[RS2];
        break;

    case 0x23: /* andi $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("andi  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] & RI16_imm;
        break;

    case 0x24: /* xor $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("xor   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] ^ R[RS2];
        break;

    case 0x25: /* xori $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("xori  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] ^ RI16_imm;
        break;

    case 0x26: /* not $rd, $rs1 */
        if (DEBUG)
            printf("not   $r%d, $r%d\n", RD, RS1);
        R[RD] = ~R[RS1];
        break;

    case 0x27: /* nor $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("nor   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = ~(R[RS1] | R[RS2]);
        break;

    case 0x28: /* shl $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("shl   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] << R[RS2];
        break;

    case 0x29: /* shli $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("shli  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] << RI16_imm;
        break;

    case 0x2a: /* shr $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("shr   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] >> R[RS2];
        break;

    case 0x2b: /* shri $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("shri  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] >> RI16_imm;
        break;

    case 0x2c: /* add $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("add   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2d: /* addi $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("addi  $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = (int64_t)R[RS1] + (int16_t)RI16_imm;
        break;

    case 0x2e: /* addiu $rd, $rs1, #imm16 */
        if (DEBUG)
            printf("addiu $r%d, $r%d, 0x%X\n", RD, RS1, RI16_imm);
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2f: /* sub $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("sub   $r%d, $r%d, 0x%X\n", RD, RS1, RS2);
        R[RD] = (int64_t)R[RS1] - (int16_t)RI16_imm;
        break;

    case 0x30: /* subu $rd, $rs1, $rs2 */
        if (DEBUG)
            printf("subu  $r%d, $r%d, 0x%X\n", RD, RS1, RS2);
        R[RD] = R[RS1] - RI16_imm;
        break;

    case 0x31: /* mul $rs1, $rs2 */
        if (DEBUG)
            printf("mul   $r%d, $r%d\n", RS1, RS2);
        hilo_mul(R[RS1], R[RS2]);
        break;

    case 0x32: /* mulu $rs1, $rs2 */
        if (DEBUG)
            printf("mulu  $r%d, $r%d\n", RS1, RS2);
        hilo_umul(R[RS1], R[RS2]);
        break;

    case 0x33: /* div $rs1, $rs2 */
        if (DEBUG)
            printf("div   $r%d, $r%d\n", RS1, RS2);
        emu.hi = (int64_t)R[RS1] % (int64_t)R[RS2];
        emu.lo = (int64_t)R[RS1] / (int64_t)R[RS2];
        break;

    case 0x34: /* divu $rs1, $rs2 */
        if (DEBUG)
            printf("divu  $r%d, $r%d\n", RS1, RS2);
        emu.hi = R[RS1] % R[RS2];
        emu.lo = R[RS1] / R[RS2];
        break;

    case 0x36: /* pushw $rd */
        if (DEBUG)
            printf("pushw $r%d\n", RS1);
        PUSHW(R[RS1]);
        break;

    case 0x37: /* push $rs1 */
        if (DEBUG)
            printf("push  $r%d\n", RS1);
        PUSH(R[RS1]);
        break;

    case 0x38: /* popw $rd */
        if (DEBUG)
            printf("popw  $r%d\n", RD);
        POPW(R[RD]);
        break;

    case 0x39: /* pop $rd */
        if (DEBUG)
            printf("pop   $r%d\n", RD);
        POP(R[RD]);
        break;

    case 0x3a: /* call label */
#if DEBUG
        printf("call  0x%X<%s>\n", I24_imm, emu.labels[I24_imm]);
#endif
        save_frame();
        break;

    case 0x3b: /* ret */
        if (DEBUG)
            printf("ret\n");
        restore_frame();
        break;

    case 0x3c: /* j label */
#if DEBUG
        printf("j     0x%X<%s>\n", I24_imm, emu.labels[I24_imm]);
#endif
        setip(emu.code + I24_imm - 1);
        break;

    case 0x3d: /* jr $rs1 */
#if DEBUG
        printf("jr    $r%d<%s>\n", RD, emu.labels[R[RD]]);
#endif
        setip(R[RD]);
        break;

    case 0x3e: /* je $rs1, $rs2, label */
#if DEBUG
        printf("je    $r%d $r%d, 0x%X<%s>\n", RD, RS1, RI16_imm, emu.labels[RI16_imm]);
#endif
        if (R[RD] == R[RS1])
            setip(emu.code + RI16_imm - 1);
        break;

    case 0x3f: /* jne $rs1, $rs2, label */
#if DEBUG
        printf("jne   $r%d $r%d, 0x%X<%s>\n", RD, RS1, RI16_imm, emu.labels[RI16_imm]);
#endif
        if (R[RD] != R[RS1])
            setip(emu.code + RI16_imm - 1);
        break;
    }

    emu.ip++, emu.cycles++;
    R[0] = 0x0; // $r0 is hard wired to 0

#if DEBUG
    if (emu.step_mode) {
        emu.step_mode = 0;
        breakpoint();
    }
#endif

    goto cpu_loop;
}
