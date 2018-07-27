#include "disasm.h"
#include "common.h"
#include "decode.h"

#if DEBUG
#include "run/cpu.h"

extern cpu_t cpu;

#define R cpu.reg

#endif

#define OP DECODE_OP(code[i])
#define RD DECODE_RD(code[i])
#define RS1 DECODE_RS1(code[i])
#define RS2 DECODE_RS2(code[i])
#define OFFSET DECODE_OFFSET(code[i])
#define I16 DECODE_I16(code[i])
#define I24 DECODE_I24(code[i])

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

        case 0x01: /* int $imm */
            fprintf(f, "int   0x%X\n", I16);
            break;

        case 0x02: /* int %rs1 */
            fprintf(f, "intr  $r%d\n", RS1);
            break;

        case 0x03: /* breakpoint */
            fprintf(f, "breakpoint\n");
            break;

        case 0x04: /* lb %RD, offset(%RS1) */
            if (OFFSET)
                fprintf(f, "lb    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                fprintf(f, "lb    $r%d, $r%d\n", RD, RS1);
            break;

        case 0x05: /* lbu %RD, offset(%RS1) */
            if (OFFSET)
                fprintf(f, "lbu   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                fprintf(f, "lbu   $r%d, $r%d\n", RD, RS1);
            break;

        case 0x06: /* lh %RD, offset(%RS1) */
            if (OFFSET)
                fprintf(f, "lh    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                fprintf(f, "lh    $r%d, $r%d\n", RD, RS1);
            break;

        case 0x07: /* lhu %RD, offset(%RS1) */
            if (OFFSET)
                fprintf(f, "lhu   $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                fprintf(f, "lhu   $r%d, $r%d\n", RD, RS1);
            break;

        case 0x08: /* lui %RD, $imm16 */
            fprintf(f, "lui   $r%d, 0x%X\n", RD, I16);
            break;

        case 0x09: /* lw %RD, offset(%RS1) */
            if (OFFSET)
                fprintf(f, "lw    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                fprintf(f, "lw    $r%d, $r%d\n", RS1, RD);
            break;

        case 0x0a: /* lwu %RD, offset(%RS1) */
            if (OFFSET)
                fprintf(f, "lwu   $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                fprintf(f, "lwu   $r%d, $r%d\n", RS1, RD);
            break;

        case 0x0b: /* ld %RD, offset(%RS1) */
            if (OFFSET)
                fprintf(f, "ld    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                fprintf(f, "ld    $r%d, $r%d\n", RS1, RD);
            break;

        case 0x0c: /* sb %RS1, offset(%RD) */
            if (OFFSET)
                fprintf(f, "sb    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                fprintf(f, "sb    $r%d, $r%d\n", RS1, RD);
            break;

        case 0x0d: /* sh %RS1, offset(%RD) */
            if (OFFSET)
                fprintf(f, "sh    $r%d, %d($r%d)\n", RS1, OFFSET, RD);
            else
                fprintf(f, "sh    $r%d, $r%d\n", RS1, RD);
            break;

        case 0x0e: /* sw %RS1, offset(%RD) */
            if (OFFSET)
                fprintf(f, "sw    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                fprintf(f, "sw    $r%d, $r%d\n", RD, RS1);
            break;

        case 0x0f: /* sd %RS1, offset(%RD) */
            if (OFFSET)
                fprintf(f, "sd    $r%d, %d($r%d)\n", RD, OFFSET, RS1);
            else
                fprintf(f, "sd    $r%d, $r%d\n", RD, RS1);
            break;

        case 0x10: /* mov %RD, %RS1 */
            fprintf(f, "mov   $r%d, $r%d\n", RD, RS1);
            break;

        case 0x11: /* mfhi %rd */
            fprintf(f, "mfhi  $r%d\n", RD);
            break;

        case 0x12: /* mthi %rs1 */
            fprintf(f, "mthi  $r%d\n", RS1);
            break;

        case 0x13: /* mflo %rd */
            fprintf(f, "mflo  $r%d\n", RD);
            break;

        case 0x14: /* mtlo %rs1 */
            fprintf(f, "mtlo  $r%d\n", RS1);
            break;

        case 0x15: /* slt $rd, $rs1, $rs2 */
            fprintf(f, "slt   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x16: /* sltu $rd, $rs1, $rs2 */
            fprintf(f, "sltu  $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x17: /* slti $rd, $rs1, #imm16 */
            fprintf(f, "slti  $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x18: /* sltiu $rd, $rs1, #imm16 */
            fprintf(f, "sltiu $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x19: /* eq $rd, $rs1, $rs2 */
            fprintf(f, "eq    $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x1a: /* eqi $rd, $rs1, #imm16 */
            fprintf(f, "eqi   $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x1b: /* eqiu $rd, $rs1, #imm16 */
            fprintf(f, "eqiu  $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x20: /* or $rd, $rs1, $rs2 */
            fprintf(f, "or    $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x21: /* ori $rd, $rs1, #imm16 */
            fprintf(f, "ori   $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x22: /* and $rd, $rs1, $rs2 */
            fprintf(f, "and   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x23: /* andi $rd, $rs1, #imm16 */
            fprintf(f, "andi  $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x24: /* xor $rd, $rs1, $rs2 */
            fprintf(f, "xor   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x25: /* xori $rd, $rs1, #imm16 */
            fprintf(f, "xori  $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x26: /* not $rd, $rs1 */
            fprintf(f, "not   $r%d, $r%d\n", RD, RS1);
            break;

        case 0x27: /* nor $rd, $rs1, $rs2 */
            fprintf(f, "nor   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x28: /* shl $rd, $rs1, $rs2 */
            fprintf(f, "shl   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x29: /* shli $rd, $rs1, #imm16 */
            fprintf(f, "shli  $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x2a: /* shr $rd, $rs1, $rs2 */
            fprintf(f, "shr   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x2b: /* shri $rd, $rs1, #imm16 */
            fprintf(f, "shri  $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x2c: /* add $rd, $rs1, $rs2 */
            fprintf(f, "add   $r%d, $r%d, $r%d\n", RD, RS1, RS2);
            break;

        case 0x2d: /* addi $rd, $rs1, #imm16 */
            fprintf(f, "addi  $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x2e: /* addiu $rd, $rs1, #imm16 */
            fprintf(f, "addiu $r%d, $r%d, 0x%X\n", RD, RS1, I16);
            break;

        case 0x2f: /* sub $rd, $rs1, $rs2 */
            fprintf(f, "sub   $r%d, $r%d, 0x%X\n", RD, RS1, RS2);
            break;

        case 0x30: /* subu $rd, $rs1, $rs2 */
            fprintf(f, "subu  $r%d, $r%d, 0x%X\n", RD, RS1, RS2);
            break;

        case 0x31: /* mul $rs1, $rs2 */
            fprintf(f, "mul   $r%d, $r%d\n", RS1, RS2);
            break;

        case 0x32: /* mulu $rs1, $rs2 */
            fprintf(f, "mulu  $r%d, $r%d\n", RS1, RS2);
            break;

        case 0x33: /* div $rs1, $rs2 */
            fprintf(f, "div   $r%d, $r%d\n", RS1, RS2);
            break;

        case 0x34: /* divu $rs1, $rs2 */
            fprintf(f, "divu  $r%d, $r%d\n", RS1, RS2);
            break;

        case 0x36: /* pushw $rd */
            fprintf(f, "pushw $r%d\n", RS1);
            break;

        case 0x37: /* push $rs1 */
            fprintf(f, "push  $r%d\n", RS1);
            break;

        case 0x38: /* popw $rd */
            fprintf(f, "popw  $r%d\n", RD);
            break;

        case 0x39: /* pop $rd */
            fprintf(f, "pop   $r%d\n", RD);
            break;

        case 0x3a: /* call label */
#if DEBUG
            fprintf(f, "call  0x%X<%s>\n", I24, cpu.labels[I24]);
#else
            fprintf(f, "call  0x%X\n", I24);
#endif
            break;

        case 0x3b: /* ret */
            fprintf(f, "ret\n");
            break;

        case 0x3c: /* j label */
#if DEBUG
            fprintf(f, "j     0x%X<%s>\n", I24, cpu.labels[I24]);
#else
            fprintf(f, "j     0x%X\n", I24);
#endif
            break;

        case 0x3d: /* jr $rs1 */
#if DEBUG
            fprintf(f, "jr    $r%d<%s>\n", RD, cpu.labels[R[RD]]);
#else
            fprintf(f, "jr    $r%d\n", RD);
#endif
            break;

        case 0x3e: /* je $rs1, $rs2, label */
#if DEBUG
            fprintf(f, "je    $r%d $r%d, 0x%X<%s>\n", RD, RS1, I16, cpu.labels[I16]);
#else
            fprintf(f, "je    $r%d $r%d, 0x%X\n", RD, RS1, I16);
#endif
            break;

        case 0x3f: /* jne $rs1, $rs2, label */
#if DEBUG
            fprintf(f, "jne   $r%d $r%d, 0x%X<%s>\n", RD, RS1, I16, cpu.labels[I16]);
#else
            fprintf(f, "jne   $r%d $r%d, 0x%X\n", RD, RS1, I16);
#endif
            break;
        }
    }
}
