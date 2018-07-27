#include "src/gen.h"
#include "src/run/cpu.h"
#include <stdio.h>
#include <stdlib.h>

extern cpu_t    cpu;
extern asmgen_t asmgen;

int main(void)
{
#define EXIT 0
#define PRINT 5

    label("_start");
    {
        addi(4, 0, 0); // argc
        addi(5, 0, 0); // argv
        call("main");
        mov(4, 2);
        int_(EXIT);
    }

    label("putc");
    {
        int_(PRINT);
        ret();
    }

    label("__print_as_int");
    {
        addi(4, 4, '0');
        call("putc");
        ret();
    }

    label("print");
    {
        set_breakpoint();
        mov(16, 4); // n

        slt(8, 4, 0); // n < 0 ?
        je(8, 0, "__print_L1");
        addi(4, 0, '-');
        call("putc"); // putc('-')
        addi(8, 0, 1);
        sub(8, 0, 8);
        mul(16, 8); // n = -n
        mflo(16);

        label("__print_L1");
        jne(16, 0, "__print_L2");
        addi(4, 0, '0'); // if (n == 0)
        call("putc"); // putc('0')

        label("__print_L2");
        addi(8, 0, 10);
        div_(16, 8); // n / 10
        mfhi(18); // n%10
        mflo(17); // n/10

        je(17, 0, "__print_L3");
        mov(4, 17);
        call("print"); // print(n / 10)

        label("__print_L3");
        mov(4, 18);
        call("__print_as_int"); // putc(n%10+'0')
        ret();
    }

    label("main");
    {
        addi(4, 0, 0x1A2B);
        call("print");

        addi(4, 0, '\n');
        call("putc");

        addi(2, 0, EXIT_SUCCESS);
        ret();
    }

    gen();

    if (!asmgen.error) {
        exec();
    }

    return 0;
}
