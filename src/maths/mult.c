#include <stdint.h>

void mult64to128(int64_t rs1, int64_t rs2, uint64_t* hi, uint64_t* lo, int sign)
{
    uint64_t u1 = (rs1 & 0xffffffff);
    uint64_t v1 = (rs2 & 0xffffffff);
    uint64_t t = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k = (t >> 32);

    rs1 >>= 32;
    t = (rs1 * v1) + k;
    k = (t & 0xffffffff);

    uint64_t w1 = (t >> 32);

    rs2 >>= 32;
    t = (u1 * rs2) + k;
    k = (t >> 32);

    *hi = (rs1 * rs2) + w1 + k;
    *lo = (t << 32) + w3;

    if (sign) {
        if (rs1 < 0LL)
            *hi -= rs2;
        if (rs2 < 0LL)
            *hi -= rs1;
    }
}
