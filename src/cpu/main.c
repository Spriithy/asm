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

void print_stack()
{
    for (int i = 0; i < 32; i += sizeof(u32)) {
        printf("%08x\n", *(u32*)&cpu.data[cpu.gpr[ESP] + i]);
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
    // mov %edx, 0xfa132c7f

    *ip++ = MOV_RI;
    *ip++ = encode_r16(SI);
    *ip++ = 0x12;
    *ip++ = 0x13;
    // mov %si, 0x1312

    *ip++ = MOV_MR;
    *ip++ = encode_r32(ESI);
    *ip++ = 0x00;
    *ip++ = 0x00;
    *ip++ = encode_r32(EDX);
    // mov [%esi], %edx

    *ip++ = MOV_RM;
    *ip++ = encode_r16(AX);
    *ip++ = encode_r32(ESI);
    *ip++ = 0x01;
    *ip++ = 0x00;
    // mov %ax, [%esi + 1]

    *ip++ = PUSH;
    *ip++ = encode_r32(EAX);
    // push %eax

    *ip++ = POP;
    *ip++ = encode_r32(ECX);
    // pop %ecx

    *ip++ = HALT;

    exec();

    print_regs();

    return 0;
}
