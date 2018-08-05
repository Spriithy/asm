#ifndef EMU64_JIT_H
#define EMU64_JIT_H

#include "buf.h"
#include "run/cpu.h"
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
    instr_t* code;
    size_t   code_size;
    buf_t*   data_buf;
    size_t   data_size;
    sym_t*   data_syms;
    buf_t*   text_buf;
    size_t   text_size;
    sym_t*   text_syms;
    size_t   error;
    int      debug;
    cpu_t    cpu;
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
    int zero = 0;                                                                               \
    int at = 1;                                                                                 \
    int v0 = 2, v1 = 3;                                                                         \
    int a0 = 4, a1 = 5, a2 = 6, a3 = 7;                                                         \
    int t0 = 8, t1 = 9, t2 = 10, t3 = 11, t4 = 12, t5 = 13, t6 = 14, t7 = 15, t8 = 24, t9 = 25; \
    int s0 = 16, s1 = 17, s2 = 18, s3 = 19, s4 = 20, s5 = 21, s6 = 22, s7 = 23;                 \
    int k0 = 26, k1 = 27;                                                                       \
    int gp = 28, sp = 29, fp = 30;                                                              \
    int ra = 31

#endif // jit.h
