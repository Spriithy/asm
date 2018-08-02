#ifndef EMU64_COMPILER_NODE_H
#define EMU64_COMPILER_NODE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t op, rs1, rs2;
    uint32_t instr;
    char*    label;
} instr_t;

typedef struct {
    char*    name;
    uint32_t addr;
    uint8_t* data;
    size_t   data_size;
} sym_t;

#endif // node.h
