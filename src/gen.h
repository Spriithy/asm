#ifndef EMU64_GEN_H
#define EMU64_GEN_H

#include <stdint.h>

#define OP32(op) (op & 0xff)

#define RR(op, rd, rs1, rs2, off) (OP32(op) \
    | ((rd & 0x1f) << 8)                    \
    | ((rs1 & 0x1f) << 13)                  \
    | ((rs2 & 0x1f) << 18)                  \
    | (off << 23))

#define RI16(op, rd, imm16) (OP32(op) \
    | ((rd & 0x1f) << 8)              \
    | ((imm16 & 0xffff) << 16))

#define NOP() OP32(0x00)
#define INT(rd, icode) RI16(0x01, rd, (icode)&0xffff)
#define INTR(rd, rs1) RR(0x02, rd, rs1, 0, 0)
#define BRKPT() OP32(0x03)
#define LB(rd, rs1, off) RR(0x04, rd, rs1, 0, off)
#define LBU(rd, rs1, off) RR(0x05, rd, rs1, 0, off)
#define LH(rd, rs1, off) RR(0x06, rd, rs1, 0, off)
#define LHU(rd, rs1, off) RR(0x07, rd, rs1, 0, off)
#define LUI(rd, imm16) RI16(0x08, rd, imm16)
#define LW(rd, rs1, off) RR(0x09, rd, rs1, 0, off)
#define LWU(rd, rs1, off) RR(0x0a, rd, rs1, 0, off)
#define LD(rd, rs1, off) RR(0x0b, rd, rs1, 0, off)
#define SB(rs1, rd, off) RR(0x0c, rd, rs1, 0, off)
#define SH(rs1, rd, off) RR(0x0d, rd, rs1, 0, off)
#define SW(rs1, rd, off) RR(0x0e, rd, rs1, 0, off)
#define SD(rs1, rd, off) RR(0x0f, rd, rs1, 0, off)
#define MOV(rd, rs1) RR(0x10, rd, rs1, 0, 0)

#define OR(rd, rs1, rs2) RR(0x20, rd, rs1, rs2, 0)
#define ORI(rd, imm16) RI16(0x21, rd, imm16)

#endif // end.h
