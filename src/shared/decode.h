#ifndef EMU64_DECODE_H
#define EMU64_DECODE_H

#define DECODE_OP(x) (int)(x & 0x3f)
#define DECODE_RD(x) (int)((x >> 6) & 0x1f)
#define DECODE_RS1(x) (int)((x >> 11) & 0x1f)
#define DECODE_RS2(x) (int)((x >> 16) & 0x1f)
#define DECODE_OFFSET(x) (((x >> 21) ^ (1 << 10) - (1 << 10)))
#define DECODE_I16(x) (uint16_t)(x >> 16)
#define DECODE_I24(x) (uint32_t)(x >> 8)

#endif // decode.h
