#ifndef EMU64_JIT_H
#define EMU64_JIT_H

#include "../emu/exec.h"
#include "../shared/buf.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t op, rd, rs1, rs2;
    uint32_t instr;
    char*    sym;
} instr_t;

typedef struct {
    char*  name;
    size_t addr;
} sym_t;

typedef struct {
    instr_t*  code;
    size_t    code_size;
    buf_t*    data_buf;
    size_t    data_size;
    sym_t*    data_syms;
    buf_t*    text_buf;
    size_t    text_size;
    sym_t*    text_syms;
    size_t    error;
    int       debug;
    program_t prog;
    core_t    core;
} jit_t;

void jit_init(void);
void jit_run(void);
void jit_set_debug(int debug);

void jit_data(char* name, uint8_t* data, size_t data_size);
void jit_label(char* name);

void jit_basic(uint32_t op);
void jit_rr(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t rs2, int off);
void jit_ri16(uint32_t op, uint32_t rd, uint32_t rs1, int16_t imm16);
void jit_jump(uint32_t op, uint32_t rs1, uint32_t rs2, char* sym);
void jit_la(uint32_t rd, char* sym);

#define jit_utils()                                                                             \
    int ZERO = 0;                                                                               \
    int AT = 1;                                                                                 \
    int V0 = 2, V1 = 3;                                                                         \
    int A0 = 4, A1 = 5, A2 = 6, A3 = 7;                                                         \
    int T0 = 8, T1 = 9, T2 = 10, T3 = 11, T4 = 12, T5 = 13, T6 = 14, T7 = 15, T8 = 24, T9 = 25; \
    int S0 = 16, S1 = 17, S2 = 18, S3 = 19, S4 = 20, S5 = 21, S6 = 22, S7 = 23;                 \
    int K0 = 26, K1 = 27;                                                                       \
    int GP = 28, SP = 29, FP = 30;                                                              \
    int RA = 31

#endif // jit.h
