#ifndef EMU64_MATHS_MATHS_H
#define EMU64_MATHS_MATHS_H

#include <stdint.h>

void mult64to128(int64_t rs1, int64_t rs2, uint64_t* hi, uint64_t* lo, int sign);

#endif // maths.h