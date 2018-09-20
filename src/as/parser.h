#ifndef EMU64_COMPILER_PARSER_H
#define EMU64_COMPILER_PARSER_H

#include "scanner.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    union {
        char* label_name;
        struct {
            char*    sym_name;
            uint8_t* sym_data;
            size_t   sym_data_size;
        };
        struct {
            char* instr;
            int   rd;
            int   rs1;
            int   rs2;
            int   imm;
            char* label;
        };
    };
} ast_t;

typedef struct {
    char*      file_name;
    scanner_t* scan;
    int        debug;
    tok_t*     tok;
    tok_t*     prev;
    int        seg;
} parser_t;

parser_t* parser_init(char* file_name);
void      parser_set_debug(parser_t* pars, int debug);
void      parser_delete(parser_t* parser);

void parse(parser_t* pars);

#endif // parser.h
