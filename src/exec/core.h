#ifndef EXEC_CORE_H
#define EXEC_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    _Bool  debug;
    size_t memory_size;
    char*  input_file;
} config_t;

extern config_t config;

typedef struct {
    uint8_t*  mem;
    uint32_t* seg_text;
    uint8_t*  seg_data;
    size_t    text_size;
    size_t    data_size;

    uint64_t pc, hi, lo;
    uint64_t reg[32];

    int step;
} core_t;

int  load_file(core_t* core, FILE* input_file);
void core_exec(core_t* core);

#endif // exec/core.h
