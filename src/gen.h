#ifndef EMU64_GEN_H
#define EMU64_GEN_H

#include <stdint.h>

uint32_t* gen();

void label(char* name);
void nop();
void int_(uint32_t rd, int16_t icode);
void intr(uint32_t rd, uint32_t rs1);
void set_breakpoint();
void lb(uint32_t rd, uint32_t rs1, int off);
void lbu(uint32_t rd, uint32_t rs1, int off);
void lh(uint32_t rd, uint32_t rs1, int off);
void lhu(uint32_t rd, uint32_t rs1, int off);
void lui(uint32_t rd, int16_t imm16);
void lw(uint32_t rd, uint32_t rs1, int off);
void lwu(uint32_t rd, uint32_t rs1, int off);
void ld(uint32_t rd, uint32_t rs1, int off);
void sb(uint32_t rs1, int off, uint32_t rd);
void sh(uint32_t rs1, int off, uint32_t rd);
void sw(uint32_t rs1, int off, uint32_t rd);
void sd(uint32_t rs1, int off, uint32_t rd);
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
void mul(uint32_t rd, uint32_t rs1, uint32_t rs2);
void mulu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void div_(uint32_t rd, uint32_t rs1, uint32_t rs2);
void divu(uint32_t rd, uint32_t rs1, uint32_t rs2);
void jal(char* label);
void jalr(uint32_t rs1, char* label);
void je(uint32_t rs1, uint32_t rs2, char* label);
void jne(uint32_t rs1, uint32_t rs2, char* label);

#endif // gen.h
