#include "gen.h"
#include "vector.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OP32(op) (op & 0x3f)

#define RR(op, rd, rs1, rs2, off) (OP32(op) \
    | ((rd & 0x1f) << 6)                    \
    | ((rs1 & 0x1f) << 11)                  \
    | ((rs2 & 0x1f) << 16)                  \
    | ((uint32_t)off << 21))

#define RI16(op, rd, rs1, imm16) (OP32(op) \
    | ((rd & 0x1f) << 6)                   \
    | ((rs1 & 0x1f) << 11)                 \
    | (((uint32_t)imm16 & 0xffff) << 16))

#define I24(op, imm24) (OP32(op) | (imm24 << 8))

typedef struct {
    uint32_t op, rs1, rs2;
    uint32_t instr;
    char*    label;
    uint32_t addr;
} instr_t;

typedef struct {
    char*    name;
    uint32_t addr;
} label_t;

static struct {
    instr_t* code;
    label_t* labels;
} asmgen;

static void rr(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t rs2, int off)
{
    vector_push(asmgen.code, (instr_t){ 0, 0, 0, RR(op, rd, rs1, rs2, off), NULL, 0 });
}

static void ri16(uint32_t op, uint32_t rd, uint32_t rs1, int16_t imm16)
{
    vector_push(asmgen.code, (instr_t){ 0, 0, 0, RI16(op, rd, rs1, imm16), NULL, 0 });
}

static void jmp(uint32_t op, uint32_t rs1, uint32_t rs2, char* label)
{
    vector_push(asmgen.code, (instr_t){ op, rs1, rs2, 0, label, vector_length(asmgen.code) });
}

void label(char* name)
{
    vector_iter(label_t, label, asmgen.labels)
    {
        if (strcmp(name, label->name) == 0) {
            printf("label redefinition: '%s'. Previous definition was here 0x%X\n", label->name, label->addr);
            exit(1);
        }
    }

    vector_push(asmgen.labels, (label_t){ name, vector_length(asmgen.labels) });
}

void nop()
{
    rr(0, 0, 0, 0, 0);
}

void int_(uint32_t rd, int16_t icode)
{
    ri16(0x01, rd, 0, icode);
}

void intr(uint32_t rd, uint32_t rs1)
{
    rr(0x02, rd, rs1, 0, 0);
}

void set_breakpoint()
{
    rr(0x03, 0, 0, 0, 0);
}

void lb(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x04, rd, rs1, 0, off);
}

void lbu(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x05, rd, rs1, 0, off);
}

void lh(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x06, rd, rs1, 0, off);
}

void lhu(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x07, rd, rs1, 0, off);
}

void lui(uint32_t rd, int16_t imm16)
{
    ri16(0x08, rd, 0, imm16);
}

void lw(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x09, rd, rs1, 0, off);
}

void lwu(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x0a, rd, rs1, 0, off);
}

void ld(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x0b, rd, rs1, 0, off);
}

void sb(uint32_t rs1, int off, uint32_t rd)
{
    rr(0x0c, rd, rs1, 0, off);
}

void sh(uint32_t rs1, int off, uint32_t rd)
{
    rr(0x0d, rd, rs1, 0, off);
}

void sw(uint32_t rs1, int off, uint32_t rd)
{
    rr(0x0e, rd, rs1, 0, off);
}

void sd(uint32_t rs1, int off, uint32_t rd)
{
    rr(0x0f, rd, rs1, 0, off);
}

void mov(uint32_t rd, uint32_t rs1)
{
    rr(0x10, rd, rs1, 0, 0);
}

void mfhi(uint32_t rd)
{
    rr(0x11, rd, 0, 0, 0);
}

void mthi(uint32_t rs1)
{
    rr(0x12, 0, rs1, 0, 0);
}

void mflo(uint32_t rd)
{
    rr(0x13, rd, 0, 0, 0);
}

void mtlo(uint32_t rs1)
{
    rr(0x14, 0, rs1, 0, 0);
}

void slt(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x15, rd, rs1, rs2, 0);
}

void sltu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x16, rd, rs1, rs2, 0);
}

void slti(uint32_t rd, uint32_t rs1, int16_t imm16)
{
    ri16(0x17, rd, rs1, imm16);
}

void sltiu(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x18, rd, rs1, imm16);
}

void eq(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x19, rd, rs1, rs2, 0);
}

void eqi(uint32_t rd, uint32_t rs1, int16_t imm16)
{
    ri16(0x1a, rd, rs1, imm16);
}

void eqiu(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x1b, rd, rs1, imm16);
}

void or_(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x20, rd, rs1, rs2, 0);
}

void ori(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x21, rd, rs1, imm16);
}

void and_(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x22, rd, rs1, rs2, 0);
}

void andi(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x23, rd, rs1, imm16);
}

void xor_(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x24, rd, rs1, rs2, 0);
}

void xori(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x25, rd, rs1, imm16);
}

void not(uint32_t rd, uint32_t rs1)
{
    rr(0x26, rd, rs1, 0, 0);
}

void nor(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x27, rd, rs1, rs2, 0);
}

void shl(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x28, rd, rs1, rs2, 0);
}

void shli(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x29, rd, rs1, imm16);
}

void shr(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x2a, rd, rs1, rs2, 0);
}

void shri(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x2b, rd, rs1, imm16);
}

void add(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x2c, rd, rs1, rs2, 0);
}

void addi(uint32_t rd, uint32_t rs1, int16_t imm16)
{
    ri16(0x2d, rd, rs1, imm16);
}

void addiu(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    ri16(0x2e, rd, rs1, imm16);
}

void sub(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x2f, rd, rs1, rs2, 0);
}

void subu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x30, rd, rs1, rs2, 0);
}

void mul(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x31, rd, rs1, rs2, 0);
}

void mulu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x32, rd, rs1, rs2, 0);
}

void div_(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x33, rd, rs1, rs2, 0);
}

void divu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    rr(0x34, rd, rs1, rs2, 0);
}

void jal(char* label)
{
    jmp(0x3c, 0, 0, label);
}

void jalr(uint32_t rs1, char* label)
{
    jmp(0x3d, rs1, 0, label);
}

void je(uint32_t rs1, uint32_t rs2, char* label)
{
    jmp(0x3e, rs1, rs2, label);
}

void jne(uint32_t rs1, uint32_t rs2, char* label)
{
    jmp(0x3f, rs1, rs2, label);
}

uint32_t* gen()
{
    uint32_t* code = malloc(vector_length(asmgen.code) * sizeof(*code));

    size_t addr = 0;

    vector_iter(instr_t, ir, asmgen.code)
    {
        if (ir->label) {
            vector_iter(label_t, label, asmgen.labels)
            {
                if (strcmp(ir->label, label->name) == 0) {
                    printf("0x%X\n", I24(ir->op, ir->addr));
                    code[addr++] = I24(ir->op, ir->addr);
                    break;
                }
            }
        } else {
            printf("0x%X\n", ir->instr);
            code[addr++] = ir->instr;
        }
    }

    return code;
}
