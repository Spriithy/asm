#ifndef EMU64_COMPILER_TOKEN_H
#define EMU64_COMPILER_TOKEN_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TOK_EOF,
    _ = 128,
    TOK_REG, // %(r[0-31]|name)
    TOK_NAME, // [a-zA-Z_][a-zA-Z_0-9]*
    TOK_INT, // [1-9][0-9]* | 0[xX][0-9a-fA-F]+
    TOK_CHR, // '\'' . '\''
    TOK_STR, // '"' [^"]* '"'
} tkind_t;

typedef struct {
    size_t  lno;
    size_t  col;
    tkind_t kind;
    char*   start;
    char*   end;
    union {
        uint64_t int_val;
        char*    str_val;
    };
} tok_t;

char* kind_str(tkind_t kind);
char* tok_str(tok_t* token);

#endif // token.h
