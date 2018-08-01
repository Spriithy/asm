#include "scanner.h"
#include "../disasm.h"
#include "../intern.h"
#include "../vector.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

scanner_t* scanner_init(char* file_name)
{
    scanner_t* scan = malloc(sizeof(*scan));
    if (scan == NULL) {
        perror("scanner_init");
        exit(-1);
    }

    scan->file_name = file_name;
    scan->file = fopen(scan->file_name, "r");

    if (scan->file == NULL) {
        perror("scanner");
        exit(-1);
    }

    fseek(scan->file, 0, SEEK_END);
    scan->file_size = ftell(scan->file);
    fseek(scan->file, 0, SEEK_SET);

    scan->file_bytes = malloc(scan->file_size);
    if (scan->file_bytes == NULL) {
        perror("scanner");
        exit(-1);
    }

    int    c;
    size_t pos = 0;
    while ((c = fgetc(scan->file)) != EOF) {
        scan->file_bytes[pos++] = (char)c;
    }

    scan->file_bytes[pos] = '\0';
    fseek(scan->file, 0, SEEK_SET);

    scan->pos = 0;
    scan->lno = 1;
    scan->col = 1;

    return scan;
}

void scanner_delete(scanner_t* scan)
{
    fclose(scan->file);

    vector_iter(error_t*, err, scan->err_list)
    {
        error_free(err);
    }
}

#define pos() __pos(scan)
static char* __pos(scanner_t* scan)
{
    return &scan->file_bytes[scan->pos];
}

#define chr() __chr(scan)
static int __chr(scanner_t* scan)
{
    return *pos();
}

#define fwd() __fwd(scan)
static void __fwd(scanner_t* scan)
{
    fgetc(scan->file);
    scan->pos++;
    scan->col++;

    if (scan->tok != NULL) {
        scan->tok->end++;
    }

    if (chr() == '\n') {
        scan->lno++;
        scan->col = 0;
    }
}

#define match(c) __match(scan, (c))
static int __match(scanner_t* scan, int c)
{
    if (chr() == c) {
        fwd();
        return 1;
    }

    return 0;
}

#define fmatch(f) __fmatch(scan, (f))
static int __fmatch(scanner_t* scan, int (*func)(int))
{
    if (func(chr())) {
        fwd();
        return 1;
    }

    return 0;
}

static int is_name_start(int c)
{
    return isalpha(c) || c == '_' || c == '.';
}

static int is_name_part(int c)
{
    return isalpha(c) || isdigit(c) || c == '_' || c == '.';
}

static void make_tok(scanner_t* scan)
{
    scan->tok = malloc(sizeof(*scan->tok));
    *scan->tok = (tok_t){
        .lno = scan->lno,
        .col = scan->col,
        .start = pos(),
        .end = pos()
    };
}

#define scanner_errorf(fmt, ...)                                                                                     \
    {                                                                                                                \
        char* sfmt;                                                                                                  \
        asprintf(&sfmt, "%s ~ line %zu, column %zu\n\t=> %s", scan->file_name, scan->tok->lno, scan->tok->col, fmt); \
                                                                                                                     \
        error_t* err = errorf(sfmt, ##__VA_ARGS__);                                                                  \
        vector_push(scan->err_list, err);                                                                            \
                                                                                                                     \
        free(sfmt);                                                                                                  \
    }

static void scan_name(scanner_t* scan)
{
    scan->tok->kind = TOK_NAME;
    for (; is_name_part(chr());) {
        fwd();
    }
}

static int char_ival[] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10,
    ['A'] = 10,
    ['b'] = 11,
    ['B'] = 11,
    ['c'] = 12,
    ['C'] = 12,
    ['d'] = 13,
    ['D'] = 13,
    ['e'] = 14,
    ['E'] = 14,
    ['f'] = 15,
    ['F'] = 15,
};

static void scan_num(scanner_t* scan)
{
    scan->tok->kind = TOK_INT;
    size_t len = 0;

    int base = 10;
    if (match('0')) {
        if (match('x') || match('X')) {
            base = 16;
        } else if (match('b') || match('B')) {
            base = 2;
        } else {
            base = 8;
        }
    }

    for (;;) {
        if (match('_')) {
            continue;
        }

        int digit = char_ival[(unsigned char)chr()];

        if (digit == 0 && chr() != '0') {
            break;
        }

        if (digit >= base) {
            scanner_errorf("digit '%c' out of range for base %d", chr(), base);
            digit = 0;
        }

        if (scan->tok->int_val > (ULLONG_MAX - digit) / base) {
            scanner_errorf("integer literal overflow");
            while (fmatch(isdigit)) {
            }

            scan->tok->int_val = 0;
            break;
        }
        scan->tok->int_val = scan->tok->int_val * base + digit;
        fwd();
    }

    if (pos() == scan->tok->start) {
        scanner_errorf("expected base %d digit, got '%c'", base, chr());
    }
}

static void scan_reg(scanner_t* scan)
{
    scan->tok->kind = TOK_REG;
    for (; is_name_part(chr());) {
        fwd();
    }

    char* reg_str = intern_range(scan->tok->start, scan->tok->end);
    int   reg = 1 + reg_index(reg_str);

    if (reg == -1) {
        scanner_errorf("unknown register '%s'", reg_str);
    }

    scan->tok->int_val = reg - 1;
}

static void discard_spaces(scanner_t* scan)
{
    while (fmatch(isspace)) {
    }
}

static int comment(scanner_t* scan)
{
    int b = 0;
    discard_spaces(scan);
    if (match(';')) {
        while (!match('\n')) {
            fwd();
        }
        b = 1;
    }
    return b;
}

void scanner_tok(scanner_t* scan)
{
    scan->tok = NULL;

    while (comment(scan)) {
    }

    make_tok(scan);

    if (feof(scan->file)) {
        scan->tok->kind = TOK_EOF;
        return;
    }

    if (is_name_start(chr())) {
        scan_name(scan);
        return;
    }

    if (isdigit(chr())) {
        scan_num(scan);
        return;
    }

    if (match('%')) {
        scan_reg(scan);
        return;
    }

    scan->tok->kind = chr();
    fwd();
}
