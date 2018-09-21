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

    size_t hdr_magic = 0x6865787944414e4f;
    size_t data_size = 0;
    size_t text_size = sizeof(code);

    fwrite(&hdr_magic, sizeof(hdr_magic), 1, f);
    fwrite(&data_size, sizeof(data_size), 1, f);
    fwrite(&text_size, sizeof(text_size), 1, f);
    fwrite(code, sizeof(code[0]), text_size / sizeof(code[0]), f);

    fclose(f);

    /*
        84 10 80 ff 8c 10 80 ff 2c 01 00 00 6c 01 00 00
        02 00 00 00 3a ff ff 00 6b 11 00 00 2c 01 0a 00
        01 00 00 00                                    
    */
}