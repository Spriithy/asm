#ifndef EMU64_PROGRAM_H
#define EMU64_PROGRAM_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* bytes;
    size_t   text_size;
    size_t   data_size;
    size_t   rdata_size;
} program_t;

#endif // program.h
