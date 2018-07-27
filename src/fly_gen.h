#ifndef EMU64_GEN_H
#define EMU64_GEN_H

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
} asmgen_t;

void gen();

void data(char* name, uint8_t* data, size_t data_size);
void label(char* name);

void nop(void);
void int_(void);
void set_breakpoint(void);
void lb(uint32_t rd, uint32_t rs1, int off);
void lbu(uint32_t rd, uint32_t rs1, int off);
void lh(uint32_t rd, uint32_t rs1, int off);
void lhu(uint32_t rd, uint32_t rs1, int off);
void lui(uint32_t rd, int16_t imm16);
void lw(uint32_t rd, uint32_t rs1, int off);
void lwu(uint32_t rd, uint32_t rs1, int off);
void ld(uint32_t rd, uint32_t rs1, int off);
void la(uint32_t rd, char* name);
void sb(uint32_t rd, uint32_t rs1, int off);
void sh(uint32_t rd, uint32_t rs1, int off);
void sw(uint32_t rd, uint32_t rs1, int off);
void sd(uint32_t rd, uint32_t rs1, int off);
void mov(uint32_t rd, uint32_t rs1);
void mfhi(uint32_t rd);
void mthi(uint32_t rs1);
void mflo(uint32_t rd);
void mtlo(uint32_t rs1);
void slt(uint32_t rd, uint32_t rs1, uint32_t rs2);
void sltu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void slti(uint32_t rd, uint32_t rs1, int16_t imm16);
void sltiu(uint32_t rd, uint32_t rs1, uint16_t imm16);
void eq(uint32_t rd, uint32_t rs1, uint32_t rs2);
void eqi(uint32_t rd, uint32_t rs1, int16_t imm16);
void eqiu(uint32_t rd, uint32_t rs1, uint16_t imm16);
void or_(uint32_t rd, uint32_t rs1, uint32_t rs2);
void ori(uint32_t rd, uint32_t rs1, uint16_t imm16);
void and_(uint32_t rd, uint32_t rs1, uint32_t rs2);
void andi(uint32_t rd, uint32_t rs1, uint16_t imm16);
void xor_(uint32_t rd, uint32_t rs1, uint32_t rs2);
void xori(uint32_t rd, uint32_t rs1, uint16_t imm16);
void not(uint32_t rd, uint32_t rs1);
void nor(uint32_t rd, uint32_t rs1, uint32_t rs2);
void shl(uint32_t rd, uint32_t rs1, uint32_t rs2);
void shli(uint32_t rd, uint32_t rs1, uint16_t imm16);
void shr(uint32_t rd, uint32_t rs1, uint32_t rs2);
void shri(uint32_t rd, uint32_t rs1, uint16_t imm16);
void add(uint32_t rd, uint32_t rs1, uint32_t rs2);
void addi(uint32_t rd, uint32_t rs1, int16_t imm16);
void addiu(uint32_t rd, uint32_t rs1, uint16_t imm16);
void sub(uint32_t rd, uint32_t rs1, uint32_t rs2);
void subu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void mul(uint32_t rs1, uint32_t rs2);
void mulu(uint32_t rs1, uint32_t rs2);
void div_(uint32_t rs1, uint32_t rs2);
void divu(uint32_t rs1, uint32_t rs2);
void pushw(uint32_t rs1);
void push(uint32_t rs1);
void popw(uint32_t rd);
void pop(uint32_t rd);
void call(char* label);
void ret(void);
void j(char* label);
void jr(uint32_t rs1);
void je(uint32_t rs1, uint32_t rs2, char* label);
void jne(uint32_t rs1, uint32_t rs2, char* label);

#define load_gen_utils()                                                                        \
    int zero = 0;                                                                               \
    int at = 1;                                                                                 \
    int v0 = 2, v1 = 3;                                                                         \
    int a0 = 4, a1 = 5, a2 = 6, a3 = 7;                                                         \
    int t0 = 8, t1 = 9, t2 = 10, t3 = 11, t4 = 12, t5 = 13, t6 = 14, t7 = 15, t8 = 24, t9 = 25; \
    int s0 = 16, s1 = 17, s2 = 18, s3 = 19, s4 = 20, s5 = 21, s6 = 22, s7 = 23;                 \
    int gp = 28, sp = 29, fp = 30;                                                              \
    int k0 = 26, k1 = 27;                                                                       \
    int ra = 31;

#endif // gen.h
