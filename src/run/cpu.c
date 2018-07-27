#include "cpu.h"
#include "../disasm.h"
#include "breakpoint.h"
#include <stdio.h>
#include <stdlib.h>

#define OP DECODE_OP(*cpu.ip)
#define RD DECODE_RD(*cpu.ip)
#define RS1 DECODE_RS1(*cpu.ip)
#define RS2 DECODE_RS2(*cpu.ip)
#define OFFSET DECODE_OFFSET(*cpu.ip)
#define I16 DECODE_I16(*cpu.ip)
#define I24 DECODE_I24(*cpu.ip)

#define R cpu.reg
#define GP R[28]
#define SP R[29]
#define FP R[30]
#define RA R[31]

#define PUSH(X) *(uint64_t*)(SP -= 8) = (X)
#define PUSHW(X) *(uint32_t*)(SP -= 4) = (X)

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

#define setip(addr) cpu.ip = (uint32_t*)(addr)

cpu_t cpu;

static void swi(uint32_t icode)
{
    switch (icode) {
    case 0x00: /* exit */
        free(cpu.code);
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
    RA = (uint64_t)cpu.ip;
    FP = SP;
    setip(cpu.code + I24 - 1);

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

    cpu.hi = (rs1 * rs2) + w1 + k;
    cpu.lo = (t << 32) + w3;
}

void hilo_mul(int64_t rs1, int64_t rs2)
{
    hilo_umul((uint64_t)rs1, (uint64_t)rs2);
    if (rs1 < 0LL)
        cpu.hi -= rs2;
    if (rs2 < 0LL)
        cpu.hi -= rs1;
}

void show_disas()
{
    if (cpu.debug)
        disasm(stdout, cpu.ip, 1);
}

void exec()
{
    uint64_t addr;

    cpu.cycles = 0;
    cpu.ip = cpu.code;
    GP = FP = SP = (uint64_t)cpu.mem + sizeof(cpu.mem);

cpu_loop:
    if (cpu.debug)
        printf("0x%-6X 0x%-10x ", (int)(cpu.ip - cpu.code), *cpu.ip);

    switch (OP) {
    case 0x00: /* nop */
        show_disas();
        break;

    case 0x01: /* int $imm */
        show_disas();
        swi(I16);
        break;

    case 0x02: /* int %rs1 */
        show_disas();
        swi(R[RS1]);
        break;

    case 0x03: /* breakpoint */
        show_disas();
        if (cpu.debug) {
            breakpoint();

            cpu.ip++, cpu.cycles++;
            R[0] = 0x0; // $r0 is hard wired to 0
            goto cpu_loop;
        }
        break;

    case 0x04: /* lb %RD, offset(%RS1) */
        show_disas();
        addr = R[RS1] + OFFSET;
        R[RD] = *(int8_t*)addr;
        break;

    case 0x05: /* lbu %RD, offset(%RS1) */
        show_disas();
        addr = R[RS1] + OFFSET;
        R[RD] = *(uint8_t*)addr;
        break;

    case 0x06: /* lh %RD, offset(%RS1) */
        show_disas();
        addr = R[RS1] + OFFSET;
        align16check(addr);
        R[RD] = *(int16_t*)addr;
        break;

    case 0x07: /* lhu %RD, offset(%RS1) */
        show_disas();
        addr = R[RS1] + OFFSET;
        align16check(addr);
        R[RD] = *(uint16_t*)addr;
        break;

    case 0x08: /* lui %RD, $imm16 */
        show_disas();
        R[RD] = (I16 << 16) | (R[RD] & 0xfffff);
        break;

    case 0x09: /* lw %RD, offset(%RS1) */
        show_disas();
        addr = R[RS1] + OFFSET;
        align32check(addr);
        R[RD] = *(int32_t*)addr;
        break;

    case 0x0a: /* lwu %RD, offset(%RS1) */
        show_disas();
        addr = R[RS1] + OFFSET;
        align32check(addr);
        R[RD] = *(uint32_t*)addr;
        break;

    case 0x0b: /* ld %RD, offset(%RS1) */
        show_disas();
        addr = R[RS1] + OFFSET;
        align64check(addr);
        R[RD] = *(uint64_t*)addr;
        break;

    case 0x0c: /* sb %RS1, offset(%RD) */
        show_disas();
        addr = R[RD] + OFFSET;
        *(uint8_t*)addr = R[RS1] & 0xff;
        break;

    case 0x0d: /* sh %RS1, offset(%RD) */
        show_disas();
        addr = R[RD] + OFFSET;
        align16check(addr);
        *(uint16_t*)addr = R[RS1] & 0xffff;
        break;

    case 0x0e: /* sw %RS1, offset(%RD) */
        show_disas();
        addr = R[RD] + OFFSET;
        align32check(addr);
        *(uint32_t*)addr = R[RS1] & 0xffffffff;
        break;

    case 0x0f: /* sd %RS1, offset(%RD) */
        show_disas();
        addr = R[RD] + OFFSET;
        align64check(addr);
        *(uint64_t*)addr = R[RS1];
        break;

    case 0x10: /* mov %RD, %RS1 */
        show_disas();
        R[RD] = R[RS1];
        break;

    case 0x11: /* mfhi %rd */
        show_disas();
        R[RD] = cpu.hi;
        break;

    case 0x12: /* mthi %rs1 */
        show_disas();
        cpu.hi = R[RS1];
        break;

    case 0x13: /* mflo %rd */
        show_disas();
        R[RD] = cpu.lo;
        break;

    case 0x14: /* mtlo %rs1 */
        show_disas();
        cpu.lo = R[RS1];
        break;

    case 0x15: /* slt $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = ((int64_t)R[RS1] < (int64_t)R[RS2]);
        break;

    case 0x16: /* sltu $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = (R[RS1] < R[RS2]);
        break;

    case 0x17: /* slti $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = ((int64_t)R[RS1] < (int16_t)I16);
        break;

    case 0x18: /* sltiu $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = (R[RS1] < I16);
        break;

    case 0x19: /* eq $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = (R[RS1] == R[RS2]);
        break;

    case 0x1a: /* eqi $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = ((int64_t)R[RS1] == (int16_t)I16);
        break;

    case 0x1b: /* eqiu $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = (R[RS1] == I16);
        break;

    case 0x20: /* or $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = R[RS1] | R[RS2];
        break;

    case 0x21: /* ori $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = R[RS1] | I16;
        break;

    case 0x22: /* and $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = R[RS1] & R[RS2];
        break;

    case 0x23: /* andi $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = R[RS1] & I16;
        break;

    case 0x24: /* xor $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = R[RS1] ^ R[RS2];
        break;

    case 0x25: /* xori $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = R[RS1] ^ I16;
        break;

    case 0x26: /* not $rd, $rs1 */
        show_disas();
        R[RD] = ~R[RS1];
        break;

    case 0x27: /* nor $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = ~(R[RS1] | R[RS2]);
        break;

    case 0x28: /* shl $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = R[RS1] << R[RS2];
        break;

    case 0x29: /* shli $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = R[RS1] << I16;
        break;

    case 0x2a: /* shr $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = R[RS1] >> R[RS2];
        break;

    case 0x2b: /* shri $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = R[RS1] >> I16;
        break;

    case 0x2c: /* add $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2d: /* addi $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = (int64_t)R[RS1] + (int16_t)I16;
        break;

    case 0x2e: /* addiu $rd, $rs1, #imm16 */
        show_disas();
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2f: /* sub $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = (int64_t)R[RS1] - (int16_t)I16;
        break;

    case 0x30: /* subu $rd, $rs1, $rs2 */
        show_disas();
        R[RD] = R[RS1] - I16;
        break;

    case 0x31: /* mul $rs1, $rs2 */
        show_disas();
        hilo_mul(R[RS1], R[RS2]);
        break;

    case 0x32: /* mulu $rs1, $rs2 */
        show_disas();
        hilo_umul(R[RS1], R[RS2]);
        break;

    case 0x33: /* div $rs1, $rs2 */
        show_disas();
        cpu.hi = (int64_t)R[RS1] % (int64_t)R[RS2];
        cpu.lo = (int64_t)R[RS1] / (int64_t)R[RS2];
        break;

    case 0x34: /* divu $rs1, $rs2 */
        show_disas();
        cpu.hi = R[RS1] % R[RS2];
        cpu.lo = R[RS1] / R[RS2];
        break;

    case 0x36: /* pushw $rd */
        show_disas();
        PUSHW(R[RS1]);
        break;

    case 0x37: /* push $rs1 */
        show_disas();
        PUSH(R[RS1]);
        break;

    case 0x38: /* popw $rd */
        show_disas();
        POPW(R[RD]);
        break;

    case 0x39: /* pop $rd */
        show_disas();
        POP(R[RD]);
        break;

    case 0x3a: /* call label */
        show_disas();
        save_frame();
        break;

    case 0x3b: /* ret */
        show_disas();
        restore_frame();
        break;

    case 0x3c: /* j label */
        show_disas();
        setip(cpu.code + I24 - 1);
        break;

    case 0x3d: /* jr $rs1 */
        show_disas();
        setip(R[RD]);
        break;

    case 0x3e: /* je $rs1, $rs2, label */
        show_disas();
        if (R[RD] == R[RS1])
            setip(cpu.code + I16 - 1);
        break;

    case 0x3f: /* jne $rs1, $rs2, label */
        show_disas();
        if (R[RD] != R[RS1])
            setip(cpu.code + I16 - 1);
        break;
    }

    cpu.ip++, cpu.cycles++;
    R[0] = 0x0; // $r0 is hard wired to 0

    if (cpu.debug && cpu.step_mode) {
        cpu.step_mode = 0;
        breakpoint();
    }

    goto cpu_loop;
}
