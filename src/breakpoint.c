#include "emu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern emulator_t emu;

void breakpoint()
{
    static char str[64];

    printf("> ");
    scanf("%s", str);

    if (strcmp(str, "c") == 0 || (*str == '\0')) {
        return;
    }

    if (strcmp(str, "s") == 0) {
        emu.step_mode = 1;
        return;
    } else if (strcmp(str, "op") == 0) {
        printf("0x%X\n", *emu.ip & 0xff);
    } else if (strcmp(str, "q") == 0) {
        exit(0);
    } else if (strcmp(str, "rd") == 0) {
        for (int i = 0; i < 32; i++) {
            printf("r%-2d   0x%llX   (%llu)\n", i, emu.reg[i], emu.reg[i]);
        }
    } else if (strcmp(str, "si") == 0) {
        uint64_t* sp = (uint64_t*)emu.reg[29];
        uint64_t* gp = (uint64_t*)emu.reg[28] - 1;

        while (gp >= sp) {
            printf("0x%llX\n", *gp--);
        }
    } else {
        printf("%s: unknown command\n", str);
    }

    str[0] = '\0';
    breakpoint();
}