#include "fly_gen.h"
#include "run/cpu.h"
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

asmgen_t asmgen;

extern cpu_t cpu;

static void rr(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t rs2, int off)
{
    vector_push(asmgen.code, (instr_t){ 0, 0, 0, RR(op, rd, rs1, rs2, off), NULL });
}

static void ri16(uint32_t op, uint32_t rd, uint32_t rs1, int16_t imm16)
{
    vector_push(asmgen.code, (instr_t){ 0, 0, 0, RI16(op, rd, rs1, imm16), NULL });
}

static void direct_jmp(uint32_t op, uint32_t rs1, uint32_t rs2, char* label)
{
    vector_push(asmgen.code, (instr_t){ op, rs1, rs2, 0, label });
}

void label(char* name)
{
    vector_iter(label_t, label, asmgen.labels)
    {
        if (strcmp(name, label->name) == 0) {
            printf("label redefinition: '%s'. Previous definition was here 0x%X\n", label->name, label->addr);
            asmgen.error++;
            return;
        }
    }

    vector_iter(datasym_t, sym, asmgen.data_syms)
    {
        if (strcmp(name, sym->sym.name) == 0) {
            printf("definition of label '%s' overrides previous data symbol declaration here 0x%X\n", sym->sym.name, sym->sym.addr);
            asmgen.error++;
            return;
        }
    }

    vector_push(asmgen.labels, (label_t){ name, vector_length(asmgen.code) });
}

void data(char* name, uint8_t* data, size_t data_size)
{
    vector_iter(datasym_t, sym, asmgen.data_syms)
    {
        if (strcmp(name, sym->sym.name) == 0) {
            printf("data symbol redefinition: '%s'. Previous definition was here 0x%X\n", sym->sym.name, sym->sym.addr);
            asmgen.error++;
            return;
        }
    }

    size_t offset = asmgen.data_offset + data_size & 7 ? (data_size + 7) & 8 : 0;

    if (data_size & 0x7) {
        asmgen.data_offset += (data_size + 7) & 8; // align to double word size
    }

    vector_push(asmgen.data_syms, (datasym_t){ { name, offset }, data, data_size });
}

void nop(void)
{
    rr(0, 0, 0, 0, 0);
}

void int_(void)
{
    rr(0x01, 0, 0, 0, 0);
}

void set_breakpoint(void)
{
    rr(0x02, 0, 0, 0, 0);
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

void la(uint32_t rd, char* name)
{
    direct_jmp(0xff, rd, 0, name);
}

void sb(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x0c, rd, rs1, 0, off);
}

void sh(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x0d, rd, rs1, 0, off);
}

void sw(uint32_t rd, uint32_t rs1, int off)
{
    rr(0x0e, rd, rs1, 0, off);
}

void sd(uint32_t rd, uint32_t rs1, int off)
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
    nor(rd, rs1, 0);
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

void mul(uint32_t rs1, uint32_t rs2)
{
    rr(0x31, 0, rs1, rs2, 0);
}

void mulu(uint32_t rs1, uint32_t rs2)
{
    rr(0x32, 0, rs1, rs2, 0);
}

void div_(uint32_t rs1, uint32_t rs2)
{
    rr(0x33, 0, rs1, rs2, 0);
}

void divu(uint32_t rs1, uint32_t rs2)
{
    rr(0x34, 0, rs1, rs2, 0);
}

void pushw(uint32_t rs1)
{
    rr(0x36, 0, rs1, 0, 0);
}

void push(uint32_t rs1)
{
    rr(0x37, 0, rs1, 0, 0);
}

void popw(uint32_t rd)
{
    rr(0x38, rd, 0, 0, 0);
}

void pop(uint32_t rd)
{
    rr(0x39, rd, 0, 0, 0);
}

void call(char* label)
{
    direct_jmp(0x3a, 0, 0, label);
}

void ret(void)
{
    rr(0x3b, 0, 0, 0, 0);
}

void j(char* label)
{
    direct_jmp(0x3c, 0, 0, label);
}

void jr(uint32_t rs1)
{
    rr(0x3d, rs1, 0, 0, 0);
}

void je(uint32_t rs1, uint32_t rs2, char* label)
{
    direct_jmp(0x3e, rs1, rs2, label);
}

void jne(uint32_t rs1, uint32_t rs2, char* label)
{
    direct_jmp(0x3f, rs1, rs2, label);
}

void gen()
{
    load_gen_utils();

    if (cpu.debug) {
        char** labels = malloc(vector_length(asmgen.code) * sizeof(*labels));
        vector_iter(label_t, label, asmgen.labels)
        {
            labels[label->addr] = label->name;
        }
        cpu.text_syms = labels;

        char** data_syms = malloc(vector_length(asmgen.data_syms) * sizeof(*data_syms));
        vector_iter(datasym_t, sym, asmgen.data_syms)
        {
            data_syms[sym->sym.addr] = sym->sym.name;
        }
        cpu.data_syms = data_syms;
    }

    size_t addr = 0;
    int    mapped;

    // generate static data segment
    vector_iter(datasym_t, sym, asmgen.data_syms)
    {
        memcpy(cpu.data + addr, sym->data, sym->data_size);
        addr += sym->data_size;
    }

    // generate text segment
    addr = 0;
    vector_iter(instr_t, ir, asmgen.code)
    {
        if (ir->label) {
            int found = 0;
            vector_iter(label_t, label, asmgen.labels)
            {
                if (strcmp(ir->label, label->name) == 0) {
                    switch (ir->op) {
                    case 0x3a: /* call */
                    case 0x3c: /* j */
                        cpu.text[addr++] = I24(ir->op, label->addr);
                        break;
                    case 0x3d: /* jr */
                    case 0x3e: /* je */
                    case 0x3f: /* jne */
                        cpu.text[addr++] = RI16(ir->op, ir->rs1, ir->rs2, label->addr);
                        break;
                    case 0xff: /* la */
                        mapped = label->addr;
                        if (mapped >> 16 > 0) {
                            cpu.text[addr++] = RI16(0x08, at, 0, mapped >> 16);
                        }
                        cpu.text[addr++] = RI16(0x21, ir->rs1, at, mapped);
                        break;
                    default:
                        printf("unknown symbol '%s' referenced in instruction 0x%X\n", ir->label, ir->op);
                        asmgen.error++;
                    }

                    found++;
                    break;
                }
            }

            if (ir->op == 0xff) {
                vector_iter(datasym_t, sym, asmgen.data_syms)
                {
                    if (strcmp(ir->label, sym->sym.name) == 0) {
                        mapped = sizeof(cpu.text) + sym->sym.addr;
                        cpu.text[addr++] = RI16(0x08, at, 0, mapped >> 16);
                        cpu.text[addr++] = RI16(0x21, ir->rs1, at, mapped);
                        found++;
                        break;
                    }
                }
            }

            if (!found) {
                printf("reference to undefined symbol '%s'\n", ir->label);
                asmgen.error++;
            }
        } else {
            cpu.text[addr++] = ir->instr;
        }
    }
}
