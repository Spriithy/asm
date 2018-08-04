#include "jit.h"
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

static jit_t jit;
static cpu_t cpu;

void jit_init()
{
    jit.data_seg = buf_alloc(1024);
    jit.data_ptr = 0;
    jit.debug = 0;
}

void jit_set_debug(int debug)
{
    jit.debug = debug;
}

#define jit_errorf(fmt, ...)                                                               \
    {                                                                                      \
        char*       sfmt;                                                                  \
        static char buf[1024];                                                             \
        asprintf(&sfmt, "\e[31merror\e[0m :: jit(%zu): %s", vector_length(jit.code), fmt); \
        snprintf(buf, sizeof(buf), sfmt, ##__VA_ARGS__);                                   \
        printf("%s\n", buf);                                                               \
        free(sfmt);                                                                        \
    }

void jit_rr(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t rs2, int off)
{
    vector_push(jit.code, (instr_t){ 0, 0, 0, RR(op, rd, rs1, rs2, off), NULL });
}

void jit_ri16(uint32_t op, uint32_t rd, uint32_t rs1, int16_t imm16)
{
    vector_push(jit.code, (instr_t){ 0, 0, 0, RI16(op, rd, rs1, imm16), NULL });
}

void jit_jump(uint32_t op, uint32_t rs1, uint32_t rs2, char* label)
{
    vector_push(jit.code, (instr_t){ op, rs1, rs2, 0, label });
}

void jit_label(char* name)
{
    vector_iter(label_t, label, jit.labels)
    {
        if (strcmp(name, label->name) == 0) {
            jit_errorf("label redefinition: '%s'. Previous definition was here 0x%X", label->name, label->addr);
            jit.error++;
            return;
        }
    }

    vector_iter(sym_t, sym, jit.data_syms)
    {
        if (strcmp(name, sym->name) == 0) {
            jit_errorf("definition of label '%s' overrides previous data symbol declaration here 0x%X", sym->name, sym->addr);
            jit.error++;
            return;
        }
    }

    vector_push(jit.labels, (label_t){ name, vector_length(jit.code) });
}

void jit_data(char* name, uint8_t* data, size_t data_size)
{
    vector_iter(sym_t, sym, jit.data_syms)
    {
        if (strcmp(name, sym->name) == 0) {
            jit_errorf("data symbol redefinition: '%s'. Previous definition was here 0x%X", sym->name, sym->addr);
            jit.error++;
            return;
        }
    }

    jit.data_ptr = buf_memcpy(jit.data_seg, jit.data_ptr, data, data_size);
    vector_push(jit.data_syms, (sym_t){ name, jit.data_ptr - data_size });
}

void jit_nop(void)
{
    jit_rr(0, 0, 0, 0, 0);
}

void jit_int(void)
{
    jit_rr(0x01, 0, 0, 0, 0);
}

void jit_set_breakpoint(void)
{
    jit_rr(0x02, 0, 0, 0, 0);
}

void jit_lb(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x04, rd, rs1, 0, off);
}

void jit_lbu(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x05, rd, rs1, 0, off);
}

void jit_lh(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x06, rd, rs1, 0, off);
}

void jit_lhu(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x07, rd, rs1, 0, off);
}

void jit_lui(uint32_t rd, int16_t imm16)
{
    jit_ri16(0x08, rd, 0, imm16);
}

void jit_lw(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x09, rd, rs1, 0, off);
}

void jit_lwu(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x0a, rd, rs1, 0, off);
}

void jit_ld(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x0b, rd, rs1, 0, off);
}

void jit_la(uint32_t rd, char* name)
{
    jit_jump(0xff, rd, 0, name);
}

void jit_li(uint32_t rd, uint32_t imm)
{
    if ((imm >> 16) == 0) {
        jit_ori(rd, 0, imm);
        return;
    }

    jit_lui(1, imm >> 16);
    jit_ori(rd, 1, imm & 0xffff);
}

void jit_sb(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x0c, rd, rs1, 0, off);
}

void jit_sh(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x0d, rd, rs1, 0, off);
}

void jit_sw(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x0e, rd, rs1, 0, off);
}

void jit_sd(uint32_t rd, uint32_t rs1, int off)
{
    jit_rr(0x0f, rd, rs1, 0, off);
}

void jit_mov(uint32_t rd, uint32_t rs1)
{
    jit_add(rd, rs1, 0);
}

void jit_mfhi(uint32_t rd)
{
    jit_rr(0x11, rd, 0, 0, 0);
}

void jit_mthi(uint32_t rs1)
{
    jit_rr(0x12, 0, rs1, 0, 0);
}

void jit_mflo(uint32_t rd)
{
    jit_rr(0x13, rd, 0, 0, 0);
}

void jit_mtlo(uint32_t rs1)
{
    jit_rr(0x14, 0, rs1, 0, 0);
}

void jit_slt(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x15, rd, rs1, rs2, 0);
}

void jit_sltu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x16, rd, rs1, rs2, 0);
}

void jit_slti(uint32_t rd, uint32_t rs1, int16_t imm16)
{
    jit_ri16(0x17, rd, rs1, imm16);
}

void jit_sltiu(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x18, rd, rs1, imm16);
}

void jit_eq(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x19, rd, rs1, rs2, 0);
}

void jit_eqi(uint32_t rd, uint32_t rs1, int16_t imm16)
{
    jit_ri16(0x1a, rd, rs1, imm16);
}

void jit_eqiu(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x1b, rd, rs1, imm16);
}

void jit_or(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x20, rd, rs1, rs2, 0);
}

void jit_ori(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x21, rd, rs1, imm16);
}

void jit_and(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x22, rd, rs1, rs2, 0);
}

void jit_andi(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x23, rd, rs1, imm16);
}

void jit_xor(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x24, rd, rs1, rs2, 0);
}

void jit_xori(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x25, rd, rs1, imm16);
}

void jit_not(uint32_t rd, uint32_t rs1)
{
    jit_nor(rd, rs1, 0);
}

void jit_neg(uint32_t rd, uint32_t rs1)
{
    jit_sub(rd, 0, rs1);
}

void jit_nor(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x26, rd, rs1, rs2, 0);
}

void jit_shl(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x27, rd, rs1, rs2, 0);
}

void jit_shli(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x28, rd, rs1, imm16);
}

void jit_shr(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x29, rd, rs1, rs2, 0);
}

void jit_shri(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x2a, rd, rs1, imm16);
}

void jit_add(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x2b, rd, rs1, rs2, 0);
}

void jit_addi(uint32_t rd, uint32_t rs1, int16_t imm16)
{
    jit_ri16(0x2c, rd, rs1, imm16);
}

void jit_addiu(uint32_t rd, uint32_t rs1, uint16_t imm16)
{
    jit_ri16(0x2d, rd, rs1, imm16);
}

void jit_sub(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x2e, rd, rs1, rs2, 0);
}

void jit_subu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x2f, rd, rs1, rs2, 0);
}

void jit_mul(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x30, 0, rs1, rs2, 0);
    jit_mflo(rd);
}

void jit_mulu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x31, 0, rs1, rs2, 0);
    jit_mflo(rd);
}

void jit_div(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x32, 0, rs1, rs2, 0);
    jit_mflo(rd);
}

void jit_divu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x33, 0, rs1, rs2, 0);
    jit_mflo(rd);
}

void jit_mod(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x32, 0, rs1, rs2, 0);
    jit_mfhi(rd);
}

void jit_modu(uint32_t rd, uint32_t rs1, uint32_t rs2)
{
    jit_rr(0x33, 0, rs1, rs2, 0);
    jit_mfhi(rd);
}

void jit_pushw(uint32_t rs1)
{
    jit_rr(0x36, 0, rs1, 0, 0);
}

void jit_push(uint32_t rs1)
{
    jit_rr(0x37, 0, rs1, 0, 0);
}

void jit_popw(uint32_t rd)
{
    jit_rr(0x38, rd, 0, 0, 0);
}

void jit_pop(uint32_t rd)
{
    jit_rr(0x39, rd, 0, 0, 0);
}

void jit_call(char* label)
{
    jit_jump(0x3a, 0, 0, label);
}

void jit_ret(void)
{
    jit_rr(0x3b, 0, 0, 0, 0);
}

void jit_j(char* label)
{
    jit_jump(0x3c, 0, 0, label);
}

void jit_jr(uint32_t rs1)
{
    jit_rr(0x3d, rs1, 0, 0, 0);
}

void jit_je(uint32_t rs1, uint32_t rs2, char* label)
{
    jit_jump(0x3e, rs1, rs2, label);
}

void jit_jne(uint32_t rs1, uint32_t rs2, char* label)
{
    jit_jump(0x3f, rs1, rs2, label);
}

void jit_run()
{
    jit_utils();

    if (jit.debug) {
        cpu.text_syms = malloc(vector_length(jit.code) * sizeof(*cpu.text_syms));
        if (cpu.text_syms == NULL) {
            perror("jit_run.setup_symbols");
            exit(-1);
        }

        vector_iter(label_t, label, jit.labels)
        {
            cpu.text_syms[label->addr] = label->name;
        }
    }

    size_t addr = 0;
    int    mapped;

    cpu.data = jit.data_seg->bytes;

    // generate text segment
    addr = 0;
    vector_iter(instr_t, ir, jit.code)
    {
        if (ir->label) {
            int found = 0;
            vector_iter(label_t, label, jit.labels)
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
                        jit_errorf("unknown symbol '%s' referenced in instruction 0x%X", ir->label, ir->op);
                        jit.error++;
                    }

                    found++;
                    break;
                }
            }

            if (ir->op == 0xff) {
                vector_iter(sym_t, sym, jit.data_syms)
                {
                    if (strcmp(ir->label, sym->name) == 0) {
                        mapped = sizeof(cpu.text) + sym->addr;
                        cpu.text[addr++] = RI16(0x08, at, 0, mapped >> 16);
                        cpu.text[addr++] = RI16(0x21, ir->rs1, at, mapped);
                        found++;
                        break;
                    }
                }
            }

            if (!found) {
                jit_errorf("reference to undefined symbol '%s'", ir->label);
                jit.error++;
            }
        } else {
            cpu.text[addr++] = ir->instr;
        }
    }

    cpu.debug = jit.debug;
    cpu_exec(&cpu);
}
