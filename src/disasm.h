#ifndef EMU64_DISASM_H
#define EMU64_DISASM_H

#include "exec/exec.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

extern char* instr_nop;
extern char* instr_int;
extern char* instr_breakpoint;
extern char* instr_lb;
extern char* instr_lbu;
extern char* instr_lh;
extern char* instr_lhu;
extern char* instr_lui;
extern char* instr_lw;
extern char* instr_lwu;
extern char* instr_ld;
extern char* instr_la;
extern char* instr_sb;
extern char* instr_sh;
extern char* instr_sw;
extern char* instr_sd;
extern char* instr_mov;
extern char* instr_mfhi;
extern char* instr_mthi;
extern char* instr_mflo;
extern char* instr_mtlo;
extern char* instr_slt;
extern char* instr_sltu;
extern char* instr_slti;
extern char* instr_sltiu;
extern char* instr_eq;
extern char* instr_eqi;
extern char* instr_eqiu;
extern char* instr_or;
extern char* instr_ori;
extern char* instr_and;
extern char* instr_andi;
extern char* instr_xor;
extern char* instr_xori;
extern char* instr_nor;
extern char* instr_shl;
extern char* instr_shli;
extern char* instr_shr;
extern char* instr_shri;
extern char* instr_add;
extern char* instr_addi;
extern char* instr_addu;
extern char* instr_addiu;
extern char* instr_sub;
extern char* instr_subu;
extern char* instr_mul;
extern char* instr_mulu;
extern char* instr_div;
extern char* instr_divu;
extern char* instr_mod;
extern char* instr_modu;
extern char* instr_pushw;
extern char* instr_push;
extern char* instr_popw;
extern char* instr_pop;
extern char* instr_call;
extern char* instr_ret;
extern char* instr_j;
extern char* instr_jr;
extern char* instr_je;
extern char* instr_jne;

void intern_instr_names();

int instr_opcode(char* instr);

int   reg_index(char* reg);
char* reg_name(int reg);
void  disasm(core_t* core, FILE* f, uint32_t* code, size_t code_size);

#endif // disasm.h
