#include "cpu.h"
#include "../disasm.h"
#include "../maths.h"
#include "breakpoint.h"
#include <stdio.h>
#include <stdlib.h>

#define OP DECODE_OP(*(cpu->ip))
#define RD DECODE_RD(*(cpu->ip))
#define RS1 DECODE_RS1(*(cpu->ip))
#define RS2 DECODE_RS2(*(cpu->ip))
#define OFFSET DECODE_OFFSET(*(cpu->ip))
#define I16 DECODE_I16(*(cpu->ip))
#define I24 DECODE_I24(*(cpu->ip))

#define R cpu->reg
#define GP R[28]
#define SP R[29]
#define FP R[30]
#define RA R[31]

static inline void interrupt(cpu_t* cpu)
{
    switch (R[4]) {
    case 0x05: /* putchar */
        putchar((int)R[5]);
        break;
    case 0x0a: /* exit */
        exit(R[5]);
    }
}

static inline void set_ip(cpu_t* cpu, void* addr)
{
    cpu->ip = (uint32_t*)(addr);
}

static inline uint64_t addrmap(cpu_t* cpu, int offset)
{
    return (uint64_t)cpu->text + offset;
}

static inline void push(cpu_t* cpu, uint64_t val)
{
    SP -= 8;
    *(uint64_t*)addrmap(cpu, SP) = val;
}

static inline void pushw(cpu_t* cpu, uint32_t val)
{
    SP -= 4;
    *(uint64_t*)addrmap(cpu, SP) = val;
}

static inline void pop(cpu_t* cpu, uint64_t* reg)

{
    *reg = *(uint64_t*)addrmap(cpu, SP);
    SP += 8;
}

static inline void popw(cpu_t* cpu, uint64_t* reg)

{
    *reg = *(uint64_t*)addrmap(cpu, SP);
    SP += 4;
}

static inline void save_frame(cpu_t* cpu)
{
    push(cpu, RA);
    push(cpu, FP);
    RA = (uint64_t)cpu->ip;
    FP = SP;
    set_ip(cpu, cpu->text + I24 - 1);

    push(cpu, R[16]);
    push(cpu, R[17]);
    push(cpu, R[18]);
    push(cpu, R[19]);
    push(cpu, R[20]);
    push(cpu, R[21]);
    push(cpu, R[22]);
    push(cpu, R[23]);
}

static inline void restore_frame(cpu_t* cpu)
{
    pop(cpu, &R[23]);
    pop(cpu, &R[22]);
    pop(cpu, &R[21]);
    pop(cpu, &R[20]);
    pop(cpu, &R[19]);
    pop(cpu, &R[18]);
    pop(cpu, &R[17]);
    pop(cpu, &R[16]);

    set_ip(cpu, (void*)RA);
    pop(cpu, &FP);
    pop(cpu, &RA);
}

static inline void show_disas(cpu_t* cpu)
{
    if (cpu->debug)
        disasm(stdout, cpu->ip, 1);
}

void exec(cpu_t* cpu)
{
    uint64_t addr;

    cpu->cycles = 0;
    cpu->ip = cpu->text;
    GP = FP = SP = (uint64_t)(cpu->data - (uint8_t*)cpu->text) + sizeof(cpu->data);

cpu_loop:
    if (cpu->debug)
        printf("0x%-6X 0x%-10x ", (int)(cpu->ip - cpu->text), *cpu->ip);

    switch (OP) {
    case 0x00: /* nop */
        show_disas(cpu);
        break;

    case 0x01: /* int */
        show_disas(cpu);
        interrupt(cpu);
        break;

    case 0x02: /* breakpoint */
        show_disas(cpu);
        if (cpu->debug) {
            breakpoint();
            cpu->ip++, cpu->cycles++;
            R[0] = 0x0; // $r0 is hard wired to 0
            goto cpu_loop;
        }
        break;

    case 0x04: /* lb %RD, offset(%RS1) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RS1] + OFFSET);
        R[RD] = *(int8_t*)addr;
        break;

    case 0x05: /* lbu %RD, offset(%RS1) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RS1] + OFFSET);
        R[RD] = *(uint8_t*)addr;
        break;

    case 0x06: /* lh %RD, offset(%RS1) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RS1] + OFFSET);
        R[RD] = *(int16_t*)addr;
        break;

    case 0x07: /* lhu %RD, offset(%RS1) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RS1] + OFFSET);
        R[RD] = *(uint16_t*)addr;
        break;

    case 0x08: /* lui %RD, $imm16 */
        show_disas(cpu);
        R[RD] = (I16 << 16);
        break;

    case 0x09: /* lw %RD, offset(%RS1) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RS1] + OFFSET);
        R[RD] = *(int32_t*)addr;
        break;

    case 0x0a: /* lwu %RD, offset(%RS1) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RS1] + OFFSET);
        R[RD] = *(uint32_t*)addr;
        break;

    case 0x0b: /* ld %RD, offset(%RS1) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RS1] + OFFSET);
        R[RD] = *(uint64_t*)addr;
        break;

    case 0x0c: /* sb %RS1, offset(%RD) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RD] + OFFSET);
        *(uint8_t*)addr = R[RS1] & 0xff;
        break;

    case 0x0d: /* sh %RS1, offset(%RD) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RD] + OFFSET);
        *(uint16_t*)addr = R[RS1] & 0xffff;
        break;

    case 0x0e: /* sw %RS1, offset(%RD) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RD] + OFFSET);
        *(uint32_t*)addr = R[RS1] & 0xffffffff;
        break;

    case 0x0f: /* sd %RS1, offset(%RD) */
        show_disas(cpu);
        addr = addrmap(cpu, R[RD] + OFFSET);
        *(uint64_t*)addr = R[RS1];
        break;

    case 0x10: /* mov %RD, %RS1 */
        show_disas(cpu);
        R[RD] = R[RS1];
        break;

    case 0x11: /* mfhi %rd */
        show_disas(cpu);
        R[RD] = cpu->hi;
        break;

    case 0x12: /* mthi %rs1 */
        show_disas(cpu);
        cpu->hi = R[RS1];
        break;

    case 0x13: /* mflo %rd */
        show_disas(cpu);
        R[RD] = cpu->lo;
        break;

    case 0x14: /* mtlo %rs1 */
        show_disas(cpu);
        cpu->lo = R[RS1];
        break;

    case 0x15: /* slt $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = ((int64_t)R[RS1] < (int64_t)R[RS2]);
        break;

    case 0x16: /* sltu $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = (R[RS1] < R[RS2]);
        break;

    case 0x17: /* slti $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = ((int64_t)R[RS1] < (int16_t)I16);
        break;

    case 0x18: /* sltiu $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = (R[RS1] < I16);
        break;

    case 0x19: /* eq $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = (R[RS1] == R[RS2]);
        break;

    case 0x1a: /* eqi $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = ((int64_t)R[RS1] == (int16_t)I16);
        break;

    case 0x1b: /* eqiu $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = (R[RS1] == I16);
        break;

    case 0x20: /* or $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = R[RS1] | R[RS2];
        break;

    case 0x21: /* ori $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = R[RS1] | I16;
        break;

    case 0x22: /* and $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = R[RS1] & R[RS2];
        break;

    case 0x23: /* andi $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = R[RS1] & I16;
        break;

    case 0x24: /* xor $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = R[RS1] ^ R[RS2];
        break;

    case 0x25: /* xori $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = R[RS1] ^ I16;
        break;

    case 0x27: /* nor $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = ~(R[RS1] | R[RS2]);
        break;

    case 0x28: /* shl $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = R[RS1] << R[RS2];
        break;

    case 0x29: /* shli $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = R[RS1] << I16;
        break;

    case 0x2a: /* shr $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = R[RS1] >> R[RS2];
        break;

    case 0x2b: /* shri $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = R[RS1] >> I16;
        break;

    case 0x2c: /* add $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2d: /* addi $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = (int64_t)R[RS1] + (int16_t)I16;
        break;

    case 0x2e: /* addiu $rd, $rs1, #imm16 */
        show_disas(cpu);
        R[RD] = R[RS1] + R[RS2];
        break;

    case 0x2f: /* sub $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = (int64_t)R[RS1] - (int16_t)I16;
        break;

    case 0x30: /* subu $rd, $rs1, $rs2 */
        show_disas(cpu);
        R[RD] = R[RS1] - I16;
        break;

    case 0x31: /* mul $rs1, $rs2 */
        show_disas(cpu);
        mult64to128(R[RS1], R[RS2], &cpu->hi, &cpu->lo, 0);
        break;

    case 0x32: /* mulu $rs1, $rs2 */
        show_disas(cpu);
        mult64to128(R[RS1], R[RS2], &cpu->hi, &cpu->lo, 1);
        break;

    case 0x33: /* div $rs1, $rs2 */
        show_disas(cpu);
        cpu->hi = (int64_t)R[RS1] % (int64_t)R[RS2];
        cpu->lo = (int64_t)R[RS1] / (int64_t)R[RS2];
        break;

    case 0x34: /* divu $rs1, $rs2 */
        show_disas(cpu);
        cpu->hi = R[RS1] % R[RS2];
        cpu->lo = R[RS1] / R[RS2];
        break;

    case 0x36: /* pushw $rd */
        show_disas(cpu);
        pushw(cpu, R[RS1]);
        break;

    case 0x37: /* push $rs1 */
        show_disas(cpu);
        push(cpu, R[RS1]);
        break;

    case 0x38: /* popw $rd */
        show_disas(cpu);
        popw(cpu, &R[RD]);
        break;

    case 0x39: /* pop $rd */
        show_disas(cpu);
        pop(cpu, &R[RD]);
        break;

    case 0x3a: /* call label */
        show_disas(cpu);
        save_frame(cpu);
        break;

    case 0x3b: /* ret */
        show_disas(cpu);
        restore_frame(cpu);
        break;

    case 0x3c: /* j label */
        show_disas(cpu);
        set_ip(cpu, cpu->text + I24 - 1);
        break;

    case 0x3d: /* jr $rs1 */
        show_disas(cpu);
        set_ip(cpu, cpu->text + R[RD] - 1);
        break;

    case 0x3e: /* je $rs1, $rs2, label */
        show_disas(cpu);
        if (R[RD] == R[RS1])
            set_ip(cpu, cpu->text + I16 - 1);
        break;

    case 0x3f: /* jne $rs1, $rs2, label */
        show_disas(cpu);
        if (R[RD] != R[RS1])
            set_ip(cpu, cpu->text + I16 - 1);
        break;
    }

    cpu->ip++, cpu->cycles++;
    R[0] = 0x0; // $r0 is hard wired to 0

    if (cpu->debug && cpu->step_mode) {
        cpu->step_mode = 0;
        breakpoint();
    }

    goto cpu_loop;
}
