#include "scanner.h"
#include "../shared/disasm.h"
#include "../shared/intern.h"
#include "../shared/vector.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define scanner_debug(...)   \
    if (scan->debug) {       \
        printf(__VA_ARGS__); \
    }

#define scanner_error(text) \
    printf("\e[31merror\e[0m :: %s(%zu:%zu): %s\n", scan->file_name, scan->tok->lno, scan->tok->col, text)

#define scanner_warning(text) \
    printf("warning :: %s(%zu:%zu): %s\n", scan->file_name, scan->tok->lno, scan->tok->col, text)

#define scanner_errorf(fmt, ...)                                                                                      \
    {                                                                                                                 \
        char*       sfmt;                                                                                             \
        static char buf[1024];                                                                                        \
        asprintf(&sfmt, "\e[31merror\e[0m :: %s(%zu:%zu): %s", scan->file_name, scan->tok->lno, scan->tok->col, fmt); \
        snprintf(buf, sizeof(buf), sfmt, ##__VA_ARGS__);                                                              \
        printf("%s\n", buf);                                                                                          \
        free(sfmt);                                                                                                   \
    }

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

void scanner_set_debug(scanner_t* scan, int debug)
{
    scan->debug = debug;
}

void scanner_delete(scanner_t* scan)
{
    fclose(scan->file);
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

#define expect(c) __expect(scan, c)
static int __expect(scanner_t* scan, int c)
{
    if (!match(c)) {
        scanner_errorf("expected '%c', got '%c'", c, chr());
        return 0;
    }

    return 1;
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

static void scan_name(scanner_t* scan)
{
    scan->tok->kind = TOK_NAME;
    for (; is_name_part(chr());) {
        fwd();
    }

    if (scan->debug) {
        printf("scanner_tok -> %s\n", tok_str(scan->tok));
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
            scanner_errorf("integer literal overflow (%zu)", scan->tok->int_val);
            while (fmatch(isdigit)) {
            }

            scan->tok->int_val = 0;
            break;
        }
        scan->tok->int_val = scan->tok->int_val * base + digit;
        fwd();
    }

    scanner_debug("  -> %s\n", tok_str(scan->tok));
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

    scanner_debug("  -> %s\n", tok_str(scan->tok));
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

static void mkstr(tok_t* tok)
{
    char*  str = intern_range(tok->start + 1, tok->end - 1);
    size_t len = strlen(str);

    tok->str_val = malloc(len);
    if (tok->str_val == NULL) {
        perror("mkstr");
        exit(-1);
    }

    char* rc = tok->str_val;
    for (char* c = str; c != '\0';) {
        switch (*c) {
        case '\\':
            switch (*++c) {
            case '0':
                *rc++ = '\0';
                break;

            case 'n':
                *rc++ = '\n';
                break;

            case 'r':
                *rc++ = '\r';
                break;

            case 't':
                *rc++ = '\t';
                break;

            case '\\':
                *rc++ = '\\';
                break;

            case '"':
                *rc++ = '"';
                break;

            case '\'':
                *rc++ = '\'';
                break;
            }
            break;

        default:
            *rc++ = *c++;
        }
    }
}

static void scan_escape(scanner_t* scan)
{
    expect('\\');
    switch (chr()) {
    case '0':
        scan->tok->int_val = '\0';
        break;

    case 'n':
        scan->tok->int_val = '\n';
        break;

    case 'r':
        scan->tok->int_val = '\r';
        break;

    case 't':
        scan->tok->int_val = '\t';
        break;

    case '\\':
        scan->tok->int_val = '\\';
        break;

    case '\'':
        scan->tok->int_val = '\'';
        break;

    case '"':
        scan->tok->int_val = '"';
        break;

    default:
        scanner_errorf("unknown escape sequence '\\%c'", chr());
    }

    fwd();
}

static void scan_string(scanner_t* scan)
{
    scan->tok->kind = TOK_STR;
    for (; !feof(scan->file);) {
        if (match('\n')) {
            scanner_error("unclosed string literal");
            return;
        }

        if (fmatch(iscntrl)) {
            scanner_error("illegal control character in string literal");
            continue;
        }

        if (match('"')) {
            return;
        }

        if (match('\\')) {
            scan_escape(scan);
        }

        fwd();
    }
    mkstr(scan->tok);
    scanner_debug("  -> %s\n", tok_str(scan->tok));
}

static void scan_char(scanner_t* scan)
{
    scan->tok->kind = TOK_INT;
    switch (chr()) {
    case '\\':
        scan_escape(scan);
        break;
    default:
        scan->tok->int_val = chr();
        fwd();
    }
    expect('\'');
    scanner_debug("  -> %s\n", tok_str(scan->tok));
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

    if (fmatch(is_name_start)) {
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

    if (match('"')) {
        scan_string(scan);
        return;
    }

    if (match('\'')) {
        scan_char(scan);
        return;
    }

    scan->tok->kind = chr();
    fwd();
}
