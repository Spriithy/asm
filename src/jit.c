#include "jit.h"
#include "disasm.h"
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

#define jit_debug(...)       \
    if (jit.debug) {         \
        printf(__VA_ARGS__); \
    }

#define jit_error(text) \
    printf("\e[31merror\e[0m :: jit(%zu): %s", jit.code_size, text)

#define jit_warning(text) \
    printf("warning :: jit(%zu): %s\n", jit.code_size, text)

#define jit_errorf(fmt, ...)                                                     \
    {                                                                            \
        char*       sfmt;                                                        \
        static char buf[1024];                                                   \
        asprintf(&sfmt, "\e[31merror\e[0m :: jit(%zu): %s", jit.code_size, fmt); \
        snprintf(buf, sizeof(buf), sfmt, ##__VA_ARGS__);                         \
        printf("%s\n", buf);                                                     \
        free(sfmt);                                                              \
    }

static jit_t jit;

static void jit_gen(void);

void jit_init(void)
{
    cpu_init(&jit.cpu);
    jit_set_debug(0);
    jit.text_buf = buf_alloc(1024);
    jit.text_size = 0;
    jit.data_buf = buf_alloc(1024);
    jit.data_size = 0;
    jit.error = 0;
}

void jit_run(void)
{
    jit_gen();
    if (jit.error == 0) {
        cpu_exec(&jit.cpu);
    }
}

void jit_set_debug(int debug)
{
    jit.debug = debug;
    jit.cpu.debug = debug;
}

void jit_data(char* name, uint8_t* data, size_t data_size)
{
    vector_iter(sym_t, sym, jit.data_syms)
    {
        if (strcmp(sym->name, name) == 0) {
            jit_errorf("data symbol '%s' redefinition", name);
            return;
        }
    }

    jit_debug("referenced data symbol '%s'", name);

    sym_t sym = (sym_t){ name, jit.data_size };
    vector_push(jit.data_syms, sym);
    jit.data_size = buf_write(jit.data_buf, jit.data_size, data, data_size);
    //jit.data_size = ((jit.data_size + 63) / 64) * 64;
}

void jit_label(char* name)
{
    vector_iter(sym_t, sym, jit.data_syms)
    {
        if (strcmp(sym->name, name) == 0) {
            jit_errorf("data symbol '%s' redefinition as label", name);
            return;
        }
    }

    vector_iter(sym_t, sym, jit.text_syms)
    {
        if (strcmp(sym->name, name) == 0) {
            jit_errorf("label '%s' redefinition", name);
            return;
        }
    }

    sym_t sym = (sym_t){ name, jit.code_size * 4 };
    vector_push(jit.text_syms, sym);
}

void jit_basic(uint32_t op)
{
    instr_t instr = (instr_t){ .instr = OP32(op) };
    vector_push(jit.code, instr);
    jit.code_size++;
}

void jit_rr(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t rs2, int off)
{
    instr_t instr = (instr_t){ .instr = RR(op, rd, rs1, rs2, off) };
    vector_push(jit.code, instr);
    jit.code_size++;
}

void jit_ri16(uint32_t op, uint32_t rd, uint32_t rs1, int16_t imm16)
{
    instr_t instr = (instr_t){ .instr = RI16(op, rd, rs1, imm16) };
    vector_push(jit.code, instr);
    jit.code_size++;
}

void jit_la(uint32_t rd, char* sym)
{
    instr_t instr = (instr_t){ .op = 0xff, .rd = rd, .sym = sym };
    vector_push(jit.code, instr);
    jit.code_size += 2;
}

void jit_li(uint32_t rd, uint32_t imm)
{
    if (imm >> 16 == 0) {
        jit_ri16(0x21, rd, 0, imm);
        return;
    }

    jit_ri16(0x08, 1, 0, imm >> 16);
    jit_ri16(0x21, rd, 1, imm & 0xffff);
}

void jit_jump(uint32_t op, uint32_t rs1, uint32_t rs2, char* sym)
{
    instr_t instr = (instr_t){ .op = op, .rs1 = rs1, .rs2 = rs2, .sym = sym };
    vector_push(jit.code, instr);
    jit.code_size++;
}

static size_t gen(uint32_t instr)
{
    return jit.text_size = buf_write_uint32(jit.text_buf, jit.text_size, instr);
}

static void jit_gen(void)
{
    jit_utils();

    // debug symbols
    if (jit.debug) {
        jit.cpu.text_syms = malloc(jit.code_size * sizeof(char*));
        if (jit.cpu.text_syms == NULL) {
            perror("jit_run.debug_symbols");
            exit(-1);
        }
        vector_iter(sym_t, label, jit.text_syms)
        {
            jit.cpu.text_syms[label->addr] = label->name;
        }
    }

    // gen text segment
    vector_iter(instr_t, instr, jit.code)
    {
        if (instr->sym != NULL) {
            int found = 0;
            vector_iter(sym_t, label, jit.text_syms)
            {
                if (strcmp(instr->sym, label->name) == 0) {
                    found++;
                    switch (instr->op) {
                    case 0x3a: /* call */
                    case 0x3c: /* j */
                        gen(I24(instr->op, label->addr));
                        break;
                    case 0x3d: /* jr */
                    case 0x3e: /* je */
                    case 0x3f: /* jne */
                        gen(RI16(instr->op, instr->rs1, instr->rs2, label->addr));
                        break;
                    case 0xff: /* la */
                        if (label->addr >> 16 != 0) {
                            gen(RI16(0x08, at, 0, label->addr >> 16));
                            gen(RI16(0x21, instr->rs1, at, label->addr & 0xffff));
                        } else {
                            gen(0x0);
                            gen(RI16(0x21, instr->rs1, at, label->addr & 0xffff));
                        }
                        break;
                    }
                    break;
                }
            }

            if (!found) {
                if (instr->op == 0xff) {
                    vector_iter(sym_t, sym, jit.data_syms)
                    {
                        if (strcmp(instr->sym, sym->name) == 0) {
                            uint64_t addr = jit.code_size * 4 + sym->addr;
                            if (sym->addr >> 16 != 0) {
                                gen(RI16(0x08, at, 0, addr >> 16));
                                gen(RI16(0x21, instr->rs1, at, addr & 0xffff));
                            } else {
                                gen(0x0);
                                gen(RI16(0x21, instr->rs1, at, addr & 0xffff));
                            }
                            found++;
                            break;
                        }
                    }
                }
            }

            if (!found) {
                jit_errorf("unknown symbol '%s' referenced in instruction 0x%X", instr->sym, instr->op);
            }
        } else {
            gen(instr->instr);
        }
    }

    disasm(&jit.cpu, stdout, (uint32_t*)jit.text_buf->bytes, jit.text_size / 4);

    cpu_text(&jit.cpu, jit.text_buf->bytes, jit.text_size);
    cpu_data(&jit.cpu, jit.data_buf->bytes, jit.data_size);

    vector_free(jit.text_syms);
    vector_free(jit.data_syms);
    buf_free(&jit.text_buf);
    buf_free(&jit.data_buf);
}