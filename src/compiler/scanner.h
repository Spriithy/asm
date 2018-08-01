#ifndef EMU64_COMPILER_SCANNER_H
#define EMU64_COMPILER_SCANNER_H

#include "../error.h"
#include "token.h"
#include <stddef.h>
#include <stdio.h>

typedef struct {
    FILE*     file;
    char*     file_name;
    size_t    file_size;
    char*     file_bytes;
    size_t    pos;
    size_t    lno;
    size_t    col;
    tok_t*    tok;
    error_t** err_list;
} scanner_t;

scanner_t* scanner_init(char* file_name);
void       scanner_delete(scanner_t* scan);
void       scanner_tok(scanner_t* scan);

#endif // scanner.h
