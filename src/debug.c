#include "emu.h"
#include <stdio.h>
#include <unistd.h>

extern emulator_t emu;

void breakpoint()
{
    return;
    for (int i = 0; i < 32; i++) {
        if (i % 8 == 0)
            printf("\n");
        printf("0x%02X ", emu.mem[i]);
    }
    printf("\n");
}