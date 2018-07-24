#include "src/emu.h"
#include "src/gen.h"
#include <stdlib.h>

extern emulator_t emu;

void gen(uint32_t x)
{
    static int count = 0;
    emu.code[count++] = x;
    emu.code[count] = 0;
}

int main(void)
{
    emu.code = malloc(1024);

    gen(LUI(2, 0xAABB));
    gen(ORI(2, 0xCCDD));
    gen(SB(2, 1, 2));
    gen(LBU(5, 1, 2));
    gen(MOV(1, 0));
    gen(INT(0, 0));

    exec();

    return 0;
}
