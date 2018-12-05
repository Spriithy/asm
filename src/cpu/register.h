#ifndef REGISTER_H_
#define REGISTER_H_

#include "opcode.h"

enum r8 {
    AL,
    AH,
    CL = 4,
    CH,
    DL = 8,
    DH,
    BL = 12,
    BH,
};

enum r16 {
    AX = 0,
    CX = 2,
    DX = 4,
    BX = 6,
    SP = 8,
    BP = 10,
    SI = 12,
    DI = 14,
    IP = 16,
};

enum r32 {
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI,
    EIP,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
};

enum r64 {
    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7,
};

enum operand_size {
    BYTE,
    WORD,
    DWORD,
    QWORD,
};

char* r8_str(enum r8 reg);
char* r16_str(enum r16 reg);
char* r32_str(enum r32 reg);
char* r64_str(enum r64 reg);

#endif /* REGISTER_H_ */
