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

    ori(1, 0, 0xABCD);
    lui(1, 0x1234);
    push(1);
    shli(1, 1, 32);
    popw(2);
    or_(1, 1, 2);
    int_(RDUMP);
    mov(5, EXIT_SUCCESS);
    int_(EXIT);

    emu.code = gen();

    exec();

    return 0;
}
