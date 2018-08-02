#ifndef EMU64_JIT_H
#define EMU64_JIT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t op, rs1, rs2;
    uint32_t instr;
    char*    label;
} instr_t;

typedef struct {
    char*    name;
    uint32_t addr;
} sym_t;

typedef sym_t label_t;

typedef struct {
    sym_t    sym;
    uint8_t* data;
    size_t   data_size;
} datasym_t;

typedef struct {
    instr_t*   code;
    label_t*   labels;
    datasym_t* data_syms;
    size_t     data_offset;
    size_t     error;
} jit_t;

void jit_run(int debug);

void jit_rr(uint32_t op, uint32_t rd, uint32_t rs1, uint32_t rs2, int off);
void jit_ri16(uint32_t op, uint32_t rd, uint32_t rs1, int16_t imm16);
void jit_jump(uint32_t op, uint32_t rs1, uint32_t rs2, char* label);

void jit_data(char* name, uint8_t* data, size_t data_size);
void jit_label(char* name);

void jit_nop(void);
void jit_int(void);
void jit_set_breakpoint(void);
void jit_lb(uint32_t rd, uint32_t rs1, int off);
void jit_lbu(uint32_t rd, uint32_t rs1, int off);
void jit_lh(uint32_t rd, uint32_t rs1, int off);
void jit_lhu(uint32_t rd, uint32_t rs1, int off);
void jit_lui(uint32_t rd, int16_t imm16);
void jit_lw(uint32_t rd, uint32_t rs1, int off);
void jit_lwu(uint32_t rd, uint32_t rs1, int off);
void jit_ld(uint32_t rd, uint32_t rs1, int off);
void jit_la(uint32_t rd, char* name);
void jit_li(uint32_t rd, uint32_t imm);
void jit_sb(uint32_t rd, uint32_t rs1, int off);
void jit_sh(uint32_t rd, uint32_t rs1, int off);
void jit_sw(uint32_t rd, uint32_t rs1, int off);
void jit_sd(uint32_t rd, uint32_t rs1, int off);
void jit_mov(uint32_t rd, uint32_t rs1);
void jit_mfhi(uint32_t rd);
void jit_mthi(uint32_t rs1);
void jit_mflo(uint32_t rd);
void jit_mtlo(uint32_t rs1);
void jit_slt(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_sltu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_slti(uint32_t rd, uint32_t rs1, int16_t imm16);
void jit_sltiu(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_eq(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_eqi(uint32_t rd, uint32_t rs1, int16_t imm16);
void jit_eqiu(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_or(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_ori(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_and(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_andi(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_xor(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_xori(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_not(uint32_t rd, uint32_t rs1);
void jit_neg(uint32_t rd, uint32_t rs1);
void jit_nor(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_shl(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_shli(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_shr(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_shri(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_add(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_addi(uint32_t rd, uint32_t rs1, int16_t imm16);
void jit_addiu(uint32_t rd, uint32_t rs1, uint16_t imm16);
void jit_sub(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_subu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_mul(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_mulu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_div(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_divu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_mod(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_modu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jit_pushw(uint32_t rs1);
void jit_push(uint32_t rs1);
void jit_popw(uint32_t rd);
void jit_pop(uint32_t rd);
void jit_call(char* label);
void jit_ret(void);
void jit_j(char* label);
void jit_jr(uint32_t rs1);
void jit_je(uint32_t rs1, uint32_t rs2, char* label);
void jit_jne(uint32_t rs1, uint32_t rs2, char* label);

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
