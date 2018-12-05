#include "encode.h"

u8 encode_r8(enum r8 reg)
{
    return BYTE | (reg << 2);
}

u8 encode_r16(enum r16 reg)
{
    return WORD | (reg << 2);
}

u8 encode_r32(enum r32 reg)
{
    return DWORD | (reg << 2);
}

u8 encode_r64(enum r64 reg)
{
    return QWORD | (reg << 2);
}
