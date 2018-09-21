#include "src/shared/icode.h"
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
        RI16(_ADDI, 4, 0, 0), // argc
        RI16(_ADDI, 5, 0, 0), // argv
        OP32(_BREAKPOINT), // breakpoint
        //I24(0x3a, 0xffff), // call main
        RI16(_ADDI, 2, 0, 1),
        RR(_ADD, 5, 2, 0, 0), // mov a1, v0
        RI16(_ADDI, 4, 0, 0x0a), // addi a0, zero, EXIT
        OP32(_INTERRUPT), // int
    };

    FILE* f = fopen("raw.esf.bin", "wb+");
    if (f == NULL)
        exit(-1);

    size_t hdr_magic = 0x6865787944414e4f;
    size_t data_size = 0;
    size_t text_size = sizeof(code);
    //char   has_debug_syms = 0;

    //fwrite(&has_debug_syms, sizeof(char), 1, f);
    fwrite(&hdr_magic, sizeof(size_t), 1, f);
    fwrite(&text_size, sizeof(size_t), 1, f);
    fwrite(&data_size, sizeof(size_t), 1, f);
    fwrite(code, sizeof(code[0]), text_size / sizeof(code[0]), f);

    fclose(f);

    /*
        84 10 80 ff 8c 10 80 ff 2c 01 00 00 6c 01 00 00
        02 00 00 00 3a ff ff 00 6b 11 00 00 2c 01 0a 00
        01 00 00 00                                    
    */
}