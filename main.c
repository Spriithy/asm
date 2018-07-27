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

#define EXIT 0x0a
#define PRINT 0x05

    load_gen_utils();

    uint64_t x = 9;
    data("x", (uint8_t*)&x, sizeof(x));

    label("_start");
    {
        addi(a0, zero, 0); // argc
        addi(a1, zero, 0); // argv
        call("main");
        mov(a1, v0);
        addi(a0, zero, EXIT);
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
        nor(s0, s0, 0);
        addi(s0, s0, 1); // n = -n

        label("print.L1");
        jne(s0, zero, "print.L2");
        addi(a0, zero, '0'); // if (n == zero)
        call("putc"); // putc('zero')

        label("print.L2");
        addi(t0, zero, 10);
        div_(s1, s0, t0); // s1 = n / 10
        mfhi(s2); // n%10

        je(s1, zero, "print.L3");
        mov(a0, s1);
        call("print"); // print(n / 10)

        label("print.L3");
        mov(a0, s2);
        call("__print_as_int"); // putc(n%10+'zero')
        ret();
    }

    label("fact");
    {
        mov(s0, a0);
        jne(s0, zero, "fact.L1");
        li(v0, 1);
        ret();

        label("fact.L1");
        addi(a0, s0, -1);
        call("fact");

        mul(v0, s0, v0);
        ret();
    }

    label("main");
    {
        la(t0, "x");
        ld(s0, t0, 0);

        mov(a0, s0);
        call("print");

        addi(a0, zero, '\n');
        call("putc");

        mov(a0, s0);
        call("fact");

        mov(a0, v0);
        call("print");
        addi(a0, zero, '\n');
        call("putc");

        la(t0, "x");
        sd(t0, s0, 0);

        addi(v0, zero, EXIT_SUCCESS);
        ret();
    }

    gen();

    if (!asmgen.error) {
        exec();
    }

    return zero;
}
