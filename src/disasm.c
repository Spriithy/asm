#include "disasm.h"
#include "common.h"
#include "decode.h"
#include "run/cpu.h"

extern cpu_t cpu;

#define OP DECODE_OP(code[i])
#define RD DECODE_RD(code[i])
#define RS1 DECODE_RS1(code[i])
#define RS2 DECODE_RS2(code[i])
#define OFFSET DECODE_OFFSET(code[i])
#define I16 DECODE_I16(code[i])
#define I24 DECODE_I24(code[i])

char* reg_name(int r)
{
    static char* names[] = {
        [0] = "%zero",
        [1] = "%at",
        [2] = "%v0",
        [3] = "%v1",
        [4] = "%a0",
        [5] = "%a1",
        [6] = "%a2",
        [7] = "%a3",
        [8] = "%t0",
        [9] = "%t1",
        [10] = "%t2",
        [11] = "%t3",
        [12] = "%t4",
        [13] = "%t5",
        [14] = "%t6",
        [15] = "%t7",
        [16] = "%s0",
        [17] = "%s1",
        [18] = "%s2",
        [19] = "%s3",
        [20] = "%s4",
        [21] = "%s5",
        [22] = "%s6",
        [23] = "%s7",
        [24] = "%t8",
        [25] = "%t9",
        [26] = "%k0",
        [27] = "%k1",
        [28] = "%gp",
        [29] = "%sp",
        [30] = "%fp",
        [31] = "%ra",
    };
    return names[r];
}

void disasm(FILE* f, uint32_t* code, size_t code_size)
{
    if (code_size == 0) {
        return;
    }

    for (size_t i = 0; i < code_size; i++) {
        switch (OP) {
        case 0x00: /* nop */
            fprintf(f, "nop\n");
            break;

        case 0x01: /* int %imm */
            fprintf(f, "int\n");
            break;

        case 0x02: /* breakpoint */
            fprintf(f, "breakpoint\n");
            break;

        case 0x04: /* lb %reg_name(RD), offset(%RS1) */
            fprintf(f, "lb    %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x05: /* lbu %reg_name(RD), offset(%RS1) */
            fprintf(f, "lbu   %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x06: /* lh %reg_name(RD), offset(%RS1) */
            fprintf(f, "lh    %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x07: /* lhu %reg_name(RD), offset(%RS1) */
            fprintf(f, "lhu   %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x08: /* lui %reg_name(RD), %imm16 */
            fprintf(f, "lui   %s, 0x%X\n", reg_name(RD), I16);
            break;

        case 0x09: /* lw %reg_name(RD), offset(%RS1) */
            fprintf(f, "lw    %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x0a: /* lwu %reg_name(RD), offset(%RS1) */
            fprintf(f, "lwu   %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x0b: /* ld %reg_name(RD), offset(%RS1) */
            fprintf(f, "ld    %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x0c: /* sb %reg_name(RS1), offset(%RD) */
            fprintf(f, "sb    %s, %d(%s)\n", reg_name(RS1), OFFSET, reg_name(RD));
            break;

        case 0x0d: /* sh %reg_name(RS1), offset(%RD) */
            fprintf(f, "sh    %s, %d(%s)\n", reg_name(RS1), OFFSET, reg_name(RD));
            break;

        case 0x0e: /* sw %reg_name(RS1), offset(%RD) */
            fprintf(f, "sw    %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x0f: /* sd %reg_name(RS1), offset(%RD) */
            fprintf(f, "sd    %s, %d(%s)\n", reg_name(RD), OFFSET, reg_name(RS1));
            break;

        case 0x10: /* mov %reg_name(RD), %RS1 */
            fprintf(f, "mov   %s, %s\n", reg_name(RD), reg_name(RS1));
            break;

        case 0x11: /* mfhi %rd */
            fprintf(f, "mfhi  %s\n", reg_name(RD));
            break;

        case 0x12: /* mthi %rs1 */
            fprintf(f, "mthi  %s\n", reg_name(RS1));
            break;

        case 0x13: /* mflo %rd */
            fprintf(f, "mflo  %s\n", reg_name(RD));
            break;

        case 0x14: /* mtlo %rs1 */
            fprintf(f, "mtlo  %s\n", reg_name(RS1));
            break;

        case 0x15: /* slt %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "slt   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x16: /* sltu %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "sltu  %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x17: /* slti %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "slti  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x18: /* sltiu %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "sltiu %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x19: /* eq %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "eq    %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x1a: /* eqi %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "eqi   %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x1b: /* eqiu %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "eqiu  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x20: /* or %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "or    %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x21: /* ori %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "ori   %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x22: /* and %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "and   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x23: /* andi %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "andi  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x24: /* xor %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "xor   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x25: /* xori %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "xori  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x26: /* not %reg_name(RD), %rs1 */
            fprintf(f, "not   %s, %s\n", reg_name(RD), reg_name(RS1));
            break;

        case 0x27: /* nor %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "nor   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x28: /* shl %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "shl   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x29: /* shli %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "shli  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x2a: /* shr %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "shr   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x2b: /* shri %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "shri  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x2c: /* add %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "add   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x2d: /* addi %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "addi  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x2e: /* addiu %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "addiu %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x2f: /* sub %rd, %rs1, %rs2 */
            fprintf(f, "sub   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x30: /* subu %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "subu  %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x31: /* mul %reg_name(RS1), %rs2 */
            fprintf(f, "mul   %s, %s\n", reg_name(RS1), reg_name(RS2));
            break;

        case 0x32: /* mulu %reg_name(RS1), %rs2 */
            fprintf(f, "mulu  %s, %s\n", reg_name(RS1), reg_name(RS2));
            break;

        case 0x33: /* div %reg_name(RS1), %rs2 */
            fprintf(f, "div   %s, %s\n", reg_name(RS1), reg_name(RS2));
            break;

        case 0x34: /* divu %reg_name(RS1), %rs2 */
            fprintf(f, "divu  %s, %s\n", reg_name(RS1), reg_name(RS2));
            break;

        case 0x36: /* pushw %rd */
            fprintf(f, "pushw %s\n", reg_name(RS1));
            break;

        case 0x37: /* push %rs1 */
            fprintf(f, "push  %s\n", reg_name(RS1));
            break;

        case 0x38: /* popw %rd */
            fprintf(f, "popw  %s\n", reg_name(RD));
            break;

        case 0x39: /* pop %rd */
            fprintf(f, "pop   %s\n", reg_name(RD));
            break;

        case 0x3a: /* call label */
            if (cpu.debug)
                fprintf(f, "call  0x%X<%s>\n", I24, cpu.text_syms[I24]);
            else
                fprintf(f, "call  0x%X\n", I24);
            break;

        case 0x3b: /* ret */
            fprintf(f, "ret\n");
            break;

        case 0x3c: /* j label */
            if (cpu.debug)
                fprintf(f, "j     0x%X<%s>\n", I24, cpu.text_syms[I24]);
            else
                fprintf(f, "j     0x%X\n", I24);
            break;

        case 0x3d: /* jr %rs1 */
            if (cpu.debug)
                fprintf(f, "jr    %s\n", reg_name(RD));
            else
                fprintf(f, "jr    %s\n", reg_name(RD));
            break;

        case 0x3e: /* je %reg_name(RS1), %reg_name(RS2), label */
            if (cpu.debug)
                fprintf(f, "je    %s %s, 0x%X<%s>\n", reg_name(RD), reg_name(RS1), I16, cpu.text_syms[I16]);
            else
                fprintf(f, "je    %s %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x3f: /* jne %reg_name(RS1), %reg_name(RS2), label */
            if (cpu.debug)
                fprintf(f, "jne   %s %s, 0x%X<%s>\n", reg_name(RD), reg_name(RS1), I16, cpu.text_syms[I16]);
            else
                fprintf(f, "jne   %s %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;
        }
    }
}
