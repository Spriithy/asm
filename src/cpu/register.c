#include "register.h"

char* r8_str(enum r8 reg)
{
    switch (reg) {
    case AL:
        return "al";
    case AH:
        return "ah";
    case CL:
        return "cl";
    case CH:
        return "ch";
    case DL:
        return "dl";
    case DH:
        return "dh";
    case BL:
        return "bl";
    case BH:
        return "bh";
    }
}

char* r16_str(enum r16 reg)
{
    switch (reg) {
    case AX:
        return "ax";
    case CX:
        return "cx";
    case DX:
        return "dx";
    case BX:
        return "bx";
    case SP:
        return "sp";
    case BP:
        return "bp";
    case SI:
        return "si";
    case DI:
        return "di";
    case IP:
        return "ip";
    }
}

char* r32_str(enum r32 reg)
{
    switch (reg) {
    case EAX:
        return "eax";
    case ECX:
        return "ecx";
    case EDX:
        return "edx";
    case EBX:
        return "ebx";
    case ESP:
        return "esp";
    case EBP:
        return "ebp";
    case ESI:
        return "esi";
    case EDI:
        return "edi";
    case EIP:
        return "eip";
    case R9:
        return "r9";
    case R10:
        return "r10";
    case R11:
        return "r11";
    case R12:
        return "r12";
    case R13:
        return "r13";
    case R14:
        return "r14";
    case R15:
        return "r15";
    }
}

char* r64_str(enum r64 reg)
{
    switch (reg) {
    case XMM0:
        return "xmm0";
    case XMM1:
        return "xmm1";
    case XMM2:
        return "xmm2";
    case XMM3:
        return "xmm3";
    case XMM4:
        return "xmm4";
    case XMM5:
        return "xmm5";
    case XMM6:
        return "xmm6";
    case XMM7:
        return "xmm7";
    }
}
