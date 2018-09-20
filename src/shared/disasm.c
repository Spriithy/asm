#include "disasm.h"
#include "../emu/exec.h"
#include "decode.h"
#include "intern.h"
#include <string.h>

#define OP DECODE_OP(code[i])
#define RD DECODE_RD(code[i])
#define RS1 DECODE_RS1(code[i])
#define RS2 DECODE_RS2(code[i])
#define OFFSET DECODE_OFFSET((int)code[i])
#define I16 DECODE_I16(code[i])
#define I24 DECODE_I24(code[i])

char* instr_nop;
char* instr_int;
char* instr_breakpoint;
char* instr_lb;
char* instr_lbu;
char* instr_lh;
char* instr_lhu;
char* instr_lui;
char* instr_lw;
char* instr_lwu;
char* instr_ld;
char* instr_la;
char* instr_sb;
char* instr_sh;
char* instr_sw;
char* instr_sd;
char* instr_mov;
char* instr_mfhi;
char* instr_mthi;
char* instr_mflo;
char* instr_mtlo;
char* instr_slt;
char* instr_sltu;
char* instr_slti;
char* instr_sltiu;
char* instr_eq;
char* instr_eqi;
char* instr_eqiu;
char* instr_or;
char* instr_ori;
char* instr_and;
char* instr_andi;
char* instr_xor;
char* instr_xori;
char* instr_nor;
char* instr_shl;
char* instr_shli;
char* instr_shr;
char* instr_shri;
char* instr_add;
char* instr_addi;
char* instr_addu;
char* instr_addiu;
char* instr_sub;
char* instr_subu;
char* instr_mul;
char* instr_mulu;
char* instr_div;
char* instr_divu;
char* instr_mod;
char* instr_modu;
char* instr_pushw;
char* instr_push;
char* instr_popw;
char* instr_pop;
char* instr_call;
char* instr_ret;
char* instr_j;
char* instr_jr;
char* instr_je;
char* instr_jne;

static int __interned = 0;

#define INSTR(name) instr_##name = intern(#name)

void intern_instr_names()
{
    INSTR(nop);
    INSTR(int);
    INSTR(breakpoint);
    INSTR(lb);
    INSTR(lbu);
    INSTR(lh);
    INSTR(lhu);
    INSTR(lui);
    INSTR(lw);
    INSTR(lwu);
    INSTR(ld);
    INSTR(la);
    INSTR(sb);
    INSTR(sh);
    INSTR(sw);
    INSTR(sd);
    INSTR(mov);
    INSTR(mfhi);
    INSTR(mthi);
    INSTR(mflo);
    INSTR(mtlo);
    INSTR(slt);
    INSTR(sltu);
    INSTR(slti);
    INSTR(sltiu);
    INSTR(eq);
    INSTR(eqi);
    INSTR(eqiu);
    INSTR(or);
    INSTR(ori);
    INSTR(and);
    INSTR(andi);
    INSTR(xor);
    INSTR(xori);
    INSTR(nor);
    INSTR(shl);
    INSTR(shli);
    INSTR(shr);
    INSTR(shri);
    INSTR(add);
    INSTR(addi);
    INSTR(addiu);
    INSTR(sub);
    INSTR(subu);
    INSTR(mul);
    INSTR(mulu);
    INSTR(div);
    INSTR(divu);
    INSTR(mod);
    INSTR(modu);
    INSTR(pushw);
    INSTR(push);
    INSTR(popw);
    INSTR(pop);
    INSTR(call);
    INSTR(ret);
    INSTR(j);
    INSTR(jr);
    INSTR(je);
    INSTR(jne);

    __interned = 1;
}

int instr_opcode(char* instr)
{
    if (!__interned) {
        intern_instr_names();
    }

    if (instr == instr_nop)
        return 0x00;
    if (instr == instr_int)
        return 0x01;
    if (instr == instr_breakpoint)
        return 0x02;

    if (instr == instr_lb)
        return 0x04;
    if (instr == instr_lbu)
        return 0x05;
    if (instr == instr_lh)
        return 0x06;
    if (instr == instr_lhu)
        return 0x07;
    if (instr == instr_lui)
        return 0x08;
    if (instr == instr_lw)
        return 0x09;
    if (instr == instr_lwu)
        return 0x0a;
    if (instr == instr_ld)
        return 0x0b;

    if (instr == instr_sb)
        return 0x0c;
    if (instr == instr_sh)
        return 0x0d;
    if (instr == instr_sw)
        return 0x0e;
    if (instr == instr_sd)
        return 0x0f;

    if (instr == instr_mov)
        return instr_opcode(instr_add);
    if (instr == instr_mfhi)
        return 0x11;
    if (instr == instr_mthi)
        return 0x12;
    if (instr == instr_mflo)
        return 0x13;
    if (instr == instr_mtlo)
        return 0x14;

    if (instr == instr_slt)
        return 0x15;
    if (instr == instr_slti)
        return 0x16;
    if (instr == instr_sltiu)
        return 0x17;
    if (instr == instr_eq)
        return 0x18;
    if (instr == instr_eqi)
        return 0x19;
    if (instr == instr_eqiu)
        return 0x1a;

    if (instr == instr_or)
        return 0x20;
    if (instr == instr_ori)
        return 0x21;
    if (instr == instr_and)
        return 0x22;
    if (instr == instr_andi)
        return 0x23;
    if (instr == instr_xor)
        return 0x24;
    if (instr == instr_xori)
        return 0x25;
    if (instr == instr_nor)
        return 0x26;
    if (instr == instr_shl)
        return 0x27;
    if (instr == instr_shli)
        return 0x28;
    if (instr == instr_shr)
        return 0x29;
    if (instr == instr_shri)
        return 0x2a;
    if (instr == instr_add)
        return 0x2b;
    if (instr == instr_addi)
        return 0x2c;
    if (instr == instr_addiu)
        return 0x2d;
    if (instr == instr_sub)
        return 0x2e;
    if (instr == instr_subu)
        return 0x2f;
    if (instr == instr_mul)
        return 0x30;
    if (instr == instr_mulu)
        return 0x31;
    if (instr == instr_div)
        return 0x32;
    if (instr == instr_divu)
        return 0x33;

    if (instr == instr_pushw)
        return 0x36;
    if (instr == instr_push)
        return 0x37;
    if (instr == instr_popw)
        return 0x38;
    if (instr == instr_pop)
        return 0x39;
    if (instr == instr_call)
        return 0x3a;
    if (instr == instr_ret)
        return 0x3b;
    if (instr == instr_j)
        return 0x3c;
    if (instr == instr_jr)
        return 0x3d;
    if (instr == instr_je)
        return 0x3e;
    if (instr == instr_jne)
        return 0x3f;

    return 0x00;
}

char* reg_name(int reg)
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
    return names[reg];
}

int reg_index(char* reg)
{
    if (strcmp(reg, "%zero") == 0)
        return 0;
    if (strcmp(reg, "%at") == 0)
        return 1;
    if (strcmp(reg, "%v0") == 0)
        return 2;
    if (strcmp(reg, "%v1") == 0)
        return 3;
    if (strcmp(reg, "%a0") == 0)
        return 4;
    if (strcmp(reg, "%a1") == 0)
        return 5;
    if (strcmp(reg, "%a2") == 0)
        return 6;
    if (strcmp(reg, "%a3") == 0)
        return 7;
    if (strcmp(reg, "%t0") == 0)
        return 8;
    if (strcmp(reg, "%t1") == 0)
        return 9;
    if (strcmp(reg, "%t2") == 0)
        return 10;
    if (strcmp(reg, "%t3") == 0)
        return 11;
    if (strcmp(reg, "%t4") == 0)
        return 12;
    if (strcmp(reg, "%t5") == 0)
        return 13;
    if (strcmp(reg, "%t6") == 0)
        return 14;
    if (strcmp(reg, "%t7") == 0)
        return 15;
    if (strcmp(reg, "%s0") == 0)
        return 16;
    if (strcmp(reg, "%s1") == 0)
        return 17;
    if (strcmp(reg, "%s2") == 0)
        return 18;
    if (strcmp(reg, "%s3") == 0)
        return 19;
    if (strcmp(reg, "%s4") == 0)
        return 20;
    if (strcmp(reg, "%s5") == 0)
        return 21;
    if (strcmp(reg, "%s6") == 0)
        return 22;
    if (strcmp(reg, "%s7") == 0)
        return 23;
    if (strcmp(reg, "%s8") == 0)
        return 24;
    if (strcmp(reg, "%s9") == 0)
        return 25;
    if (strcmp(reg, "%k0") == 0)
        return 26;
    if (strcmp(reg, "%k1") == 0)
        return 27;
    if (strcmp(reg, "%gp") == 0)
        return 28;
    if (strcmp(reg, "%sp") == 0)
        return 29;
    if (strcmp(reg, "%fp") == 0)
        return 30;
    if (strcmp(reg, "%ra") == 0)
        return 31;

    return -1;
}

void disasm(core_t* core, FILE* f, uint32_t* code, size_t code_size)
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

        case 0x26: /* nor %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "nor   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x27: /* shl %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "shl   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x28: /* shli %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "shli  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x29: /* shr %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "shr   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x2a: /* shri %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "shri  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x2b: /* add %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "add   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x2c: /* addi %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "addi  %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x2d: /* addiu %reg_name(RD), %reg_name(RS1), #imm16 */
            fprintf(f, "addiu %s, %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x2e: /* sub %rd, %rs1, %rs2 */
            fprintf(f, "sub   %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x2f: /* subu %reg_name(RD), %reg_name(RS1), %rs2 */
            fprintf(f, "subu  %s, %s, %s\n", reg_name(RD), reg_name(RS1), reg_name(RS2));
            break;

        case 0x30: /* mul %reg_name(RS1), %rs2 */
            fprintf(f, "mul   %s, %s\n", reg_name(RS1), reg_name(RS2));
            break;

        case 0x31: /* mulu %reg_name(RS1), %rs2 */
            fprintf(f, "mulu  %s, %s\n", reg_name(RS1), reg_name(RS2));
            break;

        case 0x32: /* div %reg_name(RS1), %rs2 */
            fprintf(f, "div   %s, %s\n", reg_name(RS1), reg_name(RS2));
            break;

        case 0x33: /* divu %reg_name(RS1), %rs2 */
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
            if (core != NULL && core->debug && core->syms != NULL)
                fprintf(f, "call  0x%X<%s>\n", I24, core->syms[I24]);
            else
                fprintf(f, "call  0x%X\n", I24);
            break;

        case 0x3b: /* ret */
            fprintf(f, "ret\n");
            break;

        case 0x3c: /* j label */
            if (core != NULL && core->debug && core->syms != NULL)
                fprintf(f, "j     0x%X<%s>\n", I24, core->syms[I24]);
            else
                fprintf(f, "j     0x%X\n", I24);
            break;

        case 0x3d: /* jr %rs1 */
            if (core != NULL && core->debug)
                fprintf(f, "jr    %s\n", reg_name(RD));
            else
                fprintf(f, "jr    %s\n", reg_name(RD));
            break;

        case 0x3e: /* je %reg_name(RS1), %reg_name(RS2), label */
            if (core != NULL && core->debug && core->syms != NULL)
                fprintf(f, "je    %s %s, 0x%X<%s>\n", reg_name(RD), reg_name(RS1), I16, core->syms[I16]);
            else
                fprintf(f, "je    %s %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;

        case 0x3f: /* jne %reg_name(RS1), %reg_name(RS2), label */
            if (core != NULL && core->debug && core->syms != NULL)
                fprintf(f, "jne   %s %s, 0x%X<%s>\n", reg_name(RD), reg_name(RS1), I16, core->syms[I16]);
            else
                fprintf(f, "jne   %s %s, 0x%X\n", reg_name(RD), reg_name(RS1), I16);
            break;
        }
    }
}
