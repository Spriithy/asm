#include "src/emu.h"
#include "src/gen.h"
#include <stdlib.h>

extern emulator_t emu;

int main(void)
{
#define SP 29
#define EXIT 0
#define RDUMP 1
#define PRINT 5

    emu.debug = 1;

    label("_start");
    {
        addi(4, 0, 0); // argc
        addi(5, 0, 0); // argv
        call("main");
        mov(4, 2);
        int_(EXIT);
    }

    label("main");
    {
        ori(1, 0, 0xABCD);
        lui(1, 0x1234);
        push(1);
        shli(1, 1, 32);
        popw(2);
        or_(1, 1, 2);
        int_(RDUMP);
        addi(2, 0, EXIT_SUCCESS);
        ret();
    }

    emu.code = gen();

    exec();

    return 0;
}
