#include "token.h"
#include "../intern.h"
#include <stdio.h>

char* token_kind_names[] = {
    "EOF",
    [TOK_REG] = "reg",
    [TOK_INT] = "int",
    [TOK_NAME] = "name",
    [TOK_CHR] = "char",
    [TOK_STR] = "string",
};

char* kind_str(tkind_t kind)
{
    char* str;
    if (kind == TOK_EOF) {
        return intern("eof");
    } else if (kind > 127) {
        return intern(token_kind_names[kind]);
    }

    asprintf(&str, "%c", kind);
    return intern(str);
}

char* tok_str(tok_t* tok)
{
    char* str;

    if (tok == NULL) {
        asprintf(&str, "token(null)");
    } else if (tok->kind == TOK_EOF) {
        asprintf(&str, "token(eof) (%zu, %zu)", tok->lno, tok->col);
    } else if (tok->kind == TOK_INT) {
        asprintf(&str, "token(%s) (%zu, %zu) '%.*s' %llu", token_kind_names[tok->kind], tok->lno, tok->col, 32, intern_range(tok->start, tok->end), tok->int_val);
    } else if (tok->kind > 127) {
        asprintf(&str, "token(%s) (%zu, %zu) '%.*s'", token_kind_names[tok->kind], tok->lno, tok->col, 32, intern_range(tok->start, tok->end));
    } else {
        asprintf(&str, "token('%c') (%zu, %zu)", tok->kind, tok->lno, tok->col);
    }

    return str;
}
