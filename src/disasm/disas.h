#ifndef DISAS_H
#define DISAS_H

#include "../shared/func.h"
#include <stddef.h>
#include <stdio.h>

typedef struct {
    char* input_file;
    char* output_file;
} config_t;

typedef struct {
    size_t   text_size;
    uint8_t* text;
    size_t   nfunc;
    func_t*  funcs;
} disas_t;

int load_file(disas_t* disas, FILE* input_file);

#endif // disas/disas.h
