#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern cpu_t cpu;

extern char* reg_name(int r);

void breakpoint()
{
    static char str[64];

    printf("> ");
    scanf("%s", str);

    if (strcmp(str, "c") == 0 || strcmp(str, "carry") == 0 || (*str == '\0')) {
        return;
    } else if (strcmp(str, "cycles") == 0) {
        printf("cpu cycles: %llu\n", cpu.cycles);
    } else if (strcmp(str, "ip") == 0) {
        printf("ip: 0x%llX\n", (uint64_t)cpu.ip);
    } else if (strcmp(str, "s") == 0 || strcmp(str, "step") == 0) {
        cpu.step_mode = 1;
        return;
    } else if (strcmp(str, "op") == 0) {
        printf("0x%X\n", *cpu.ip & 0xff);
    } else if (strcmp(str, "rd") == 0 || strcmp(str, "regdump") == 0) {
        for (int i = 0; i < 32; i++) {
            printf("%s   0x%llX   (%llu)\n", reg_name(i), cpu.reg[i], cpu.reg[i]);
        }
    } else if (strcmp(str, "si") == 0 || strcmp(str, "stackinspect") == 0) {
        uint64_t* sp = (uint64_t*)cpu.reg[29];
        uint64_t* gp = (uint64_t*)cpu.reg[28] - 1;

        while (gp >= sp) {
            printf("0x%llX\n", *gp--);
        }
    } else if (strcmp(str, "q") == 0 || strcmp(str, "quit") == 0) {
        exit(0);
    } else {
        printf("%s: unknown command\n", str);
    }

    str[0] = '\0';
    breakpoint();
}
