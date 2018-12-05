#include "cpu.h"
#include "encode.h"
#include "register.h"
#include <stdio.h>

void print_text()
{
    for (int i = 0; i < 16; i++) {
        printf("0x%02x ", cpu.data[i]);
        if ((i + 1) % sizeof(u32) == 0) {
            printf("\n");
        }
    }
}

void print_regs()
{
    for (int i = 0; i < 16; i++) {
        printf("%4s 0x%08x\n", r32_str(i), cpu.gpr[i]);
    }

    for (int i = 0; i < 8; i++) {
        printf("xmm%d 0x%016llx\n", i, cpu.xmm[i]);
    }
}

int main(void)
{
    u8* ip = cpu.data;

    *ip++ = MOV_RI;
    *ip++ = encode_r32(EDX);
    *ip++ = 0x7f;
    *ip++ = 0x2c;
    *ip++ = 0x13;
    *ip++ = 0xfa;
    *ip++ = NOP;
    *ip++ = MOV_RI;
    *ip++ = encode_r16(DX);
    *ip++ = 0x00;
    *ip++ = 0x00;
    *ip++ = NOP;
    *ip++ = MOV_RI;
    *ip++ = encode_r8(DH);
    *ip++ = 0xff;
    *ip++ = MOV_RR;
    *ip++ = encode_r64(XMM0);
    *ip++ = encode_r32(EDX);
    *ip++ = HALT;

    exec();

    print_regs();

    return 0;
}
