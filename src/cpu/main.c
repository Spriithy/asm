#include "mem.h"
#include <stdio.h>

#define MEM_DWORD_U rw_dword_u(m)

int main(void)
{
    byte_u m[80] = { 0 };

    dword_u* eip = &MEM_DWORD_U[1];

    *eip++ = 0xa0b1c2d3;

    for (size_t i = 0; i < sizeof(m); i++) {
        printf("0x%02x ", m[i]);
        if ((i + 1) % sizeof(dword) == 0) {
            printf("\n");
        }
    }

    return 0;
}
