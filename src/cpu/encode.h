#ifndef ENCODE_H_
#define ENCODE_H_

#include "mem.h"
#include "opcode.h"
#include "register.h"

// clang-format off
#define encode_reg(r) _Generic((r),      \
                enum r8:  encode_r8,     \
                enum r16: encode_r16,    \
                enum r32: encode_r32,    \
                enum r64: encode_r64)(r)
// clang-format on

u8 encode_r8(enum r8 reg);
u8 encode_r16(enum r16 reg);
u8 encode_r32(enum r32 reg);
u8 encode_r64(enum r64 reg);

#endif /* ENCODE_H_ */
