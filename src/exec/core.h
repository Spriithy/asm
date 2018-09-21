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
    uint32_t* seg_text;
    uint8_t*  seg_data;
    size_t    text_size;
    size_t    data_size;
} core_t;

int load_file(core_t* core, FILE* input_file);

#endif // exec/core.h
