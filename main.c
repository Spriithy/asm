#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define OP32(op) (op & 0x3f)
#define RR(op, rd, rs1, rs2, off) (OP32(op) | ((rd & 0x1f) << 6) | ((rs1 & 0x1f) << 11) | ((rs2 & 0x1f) << 16) | ((uint32_t)(off) << 21))
#define RI16(op, rd, rs1, imm16) (OP32(op) | ((rd & 0x1f) << 6) | ((rs1 & 0x1f) << 11) | (((uint32_t)(imm16)&0xffff) << 16))
#define I24(op, imm24) (OP32(op) | (imm24 << 8))

#define DECODE_OFFSET(x) (((x >> 21) ^ (1 << 10) - (1 << 10)))

int main(void)
{
    uint32_t code[] = {
        RR(0x04, 2, 2, 0, -1024),
        RR(0x0c, 2, 2, 0, 1023),
        RI16(0x2c, 4, 0, 0), // argc
        RI16(0x2c, 5, 0, 0), // argv
        OP32(0x02), // breakpoint
        I24(0x3a, 0xffff), // call main
        RR(0x2b, 5, 2, 0, 0), // mov a1, v0
        RI16(0x2c, 4, 0, 0x0a), // addi a0, zero, EXIT
        OP32(0x01), // int
    };

    FILE* f = fopen("raw.esf.bin", "w+");
    if (f == NULL)
        exit(-1);

    fwrite(code, sizeof(code[0]), sizeof(code) / sizeof(code[0]), f);

    fclose(f);
}