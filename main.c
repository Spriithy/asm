#include "src/emu.h"
#include "src/gen.h"
#include <stdlib.h>

extern emulator_t emu;

int main(void)
{

    lui(2, 0xAABB);
    ori(2, 2, 0xCCDD);
    sb(2, 2, 1);
    lbu(5, 1, 2);
    int_(0, 1);
    int_(0, 0);

    emu.code = gen();

    exec();

    return 0;
}
