#include "src/fly_gen.h"
#include "src/run/cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern cpu_t    cpu;
extern asmgen_t asmgen;

int main(int argc, char** argv)
{
    cpu.debug = 0;

    if (argc > 0) {
        for (int i = 0; i < argc; i++) {
            cpu.debug = strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0;
        }
    }

#define EXIT 0
#define PRINT 5

    load_gen_utils();

    label("_start");
    {
        addi(a0, zero, 0); // argc
        addi(a1, zero, 0); // argv
        call("main");
        mov(a0, EXIT);
        mov(a1, v0);
        set_breakpoint();
        int_();
    }

    label("putc");
    {
        mov(a1, a0);
        addi(a0, zero, PRINT);
        int_();
        ret();
    }

    label("__print_as_int");
    {
        addi(a0, a0, '0');
        call("putc");
        ret();
    }

    label("print");
    {
        mov(s0, a0); // n

        slt(t0, a0, zero); // n < zero ?
        je(t0, zero, "print.L1");
        addi(a0, zero, '-');
        call("putc"); // putc('-')
        addi(t0, zero, 1);
        sub(t0, zero, t0);
        mul(s0, t0); // n = -n
        mflo(s0);

        label("print.L1");
        jne(s0, zero, "print.L2");
        addi(a0, zero, '0'); // if (n == zero)
        call("putc"); // putc('zero')

        label("print.L2");
        addi(t0, zero, 10);
        div_(s0, t0); // n / 10
        mfhi(s2); // n%10
        mflo(s1); // n/10

        je(s1, zero, "print.L3");
        mov(a0, s1);
        call("print"); // print(n / 10)

        label("print.L3");
        mov(a0, s2);
        call("__print_as_int"); // putc(n%10+'zero')
        ret();
    }

    label("ifact");
    {
        mov(s0, a0);
        jne(s0, zero, "ifact.L1");
        addi(v0, zero, 1);
        ret();

        label("ifact.L1");
        addi(a0, s0, -1);
        call("ifact");

        mul(s0, v0);
        mflo(v0);
        ret();
    }

    label("main");
    {
        addi(s0, zero, 0x9);

        mov(a0, s0);
        call("print");

        addi(a0, zero, '\n');
        call("putc");

        mov(a0, s0);
        call("ifact");

        mov(a0, v0);
        call("print");
        addi(a0, zero, '\n');
        call("putc");

        addi(v0, zero, EXIT_SUCCESS);
        ret();
    }

    gen();

    if (!asmgen.error) {
        exec();
    }

    return zero;
}
