#include "parser.h"
#include "../disasm.h"
#include "../intern.h"
#include "../jit.h"
#include "../vector.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

enum {
    PARSED_EOF,
    PARSED_OK,
    PARSED_ERR,
};

#define fwd() __fwd(pars)
static int __fwd(parser_t*);

parser_t* parser_init(char* file_name)
{
    parser_t* pars = malloc(sizeof(*pars));
    if (pars == NULL) {
        perror("parser_init");
        exit(-1);
    }

    pars->file_name = file_name;
    pars->scan = scanner_init(file_name);

    jit_utils();

    jit_label("__start");
    {
        jit_addi(a0, zero, 0); // argc
        jit_addi(a1, zero, 0); // argv
        jit_call("main");
        jit_mov(a1, v0);
        jit_addi(a0, zero, 0x0a);
        jit_int();
    }

    intern_instr_names();

    fwd();

    return pars;
}

void parser_set_debug(parser_t* pars, int debug)
{
    pars->debug = debug;
    scanner_set_debug(pars->scan, debug);
}

#define parser_debug(...)    \
    if (pars->debug) {       \
        printf(__VA_ARGS__); \
    }

#define parser_error(text) \
    printf("\e[31merror\e[0m :: %s(%zu:%zu): %s\n", pars->file_name, pars->prev->lno, pars->prev->col, text)

#define parser_warning(text) \
    printf("warning :: %s(%zu:%zu): %s\n", pars->file_name, pars->prev->lno, pars->prev->col, text)

#define parser_errorf(fmt, ...)                                                                                         \
    {                                                                                                                   \
        char*       sfmt;                                                                                               \
        static char buf[1024];                                                                                          \
        asprintf(&sfmt, "\e[31merror\e[0m :: %s(%zu:%zu): %s", pars->file_name, pars->prev->lno, pars->prev->col, fmt); \
        snprintf(buf, sizeof(buf), sfmt, ##__VA_ARGS__);                                                                \
        printf("%s\n", buf);                                                                                            \
        free(sfmt);                                                                                                     \
    }

#define tok_is(kind) __tok_is(pars, kind)
static int __tok_is(parser_t* pars, tkind_t kind)
{
    return pars->tok->kind == kind;
}

static int __fwd(parser_t* pars)
{
    scanner_tok(pars->scan);
    pars->prev = pars->tok;
    pars->tok = pars->scan->tok;
    if (tok_is(TOK_EOF)) {
        return PARSED_EOF;
    }
    return PARSED_OK;
}

#define match(kind) __match(pars, kind)
static int __match(parser_t* pars, tkind_t kind)
{
    parser_debug(">>> match('%s', {%s})\n", kind_str(kind), tok_str(pars->tok));
    if (tok_is(kind)) {
        fwd();
        return 1;
    }
    parser_debug(">>> NO MATCH\n");
    return 0;
}

#define expect(kind) __expect(pars, kind)
static void __expect(parser_t* pars, tkind_t kind)
{
    parser_debug(">>> expect('%s', {%s})\n", kind_str(kind), tok_str(pars->tok));
    if (!match(kind)) {
        parser_errorf("expected '%s', got '%s'", kind_str(kind), kind_str(pars->tok->kind));
        return;
    }
}

#define intval() __intval(pars)
static uint64_t __intval(parser_t* pars)
{
    return pars->prev->int_val;
}

#define strval() __strval(pars)
static char* __strval(parser_t* pars)
{
    return pars->prev->str_val;
}

#define srcstr() __srcstr(pars)
static char* __srcstr(parser_t* pars)
{
    return intern_range(pars->prev->start, pars->prev->end);
}

#define name_is(x) (strcmp(name, instr_##x) == 0)
#define parse_instr(name) __parse_instr(pars, name)
void __parse_instr(parser_t* pars, char* name)
{
    char* label;
    int   op, rd, rs1, rs2, offset, imm;

    if (name_is(nop)) {
    } else if (name_is(int) || name_is(breakpoint) || name_is(ret)) {
        op = instr_opcode(name);
        jit_rr(op, 0, 0, 0, 0);
    } else if (name_is(lb) || name_is(lbu)
        || name_is(lh) || name_is(lhu)
        || name_is(lw) || name_is(lwu)
        || name_is(ld)
        || name_is(sb) || name_is(sh)
        || name_is(sw) || name_is(sd)) {

        op = instr_opcode(name);
        expect(TOK_REG);
        rd = intval();
        expect(',');
        offset = 1;
        if (match('-')) {
            offset = -1;
        }
        expect(TOK_INT);
        offset *= intval();
        if (offset < 0) {
            expect('(');
        }
        expect(TOK_REG);
        rs1 = intval();
        if (offset < 0) {
            expect(')');
        }
        if (op <= instr_opcode(instr_sb)) {
            jit_rr(op, rd, rs1, 0, offset);
        } else {
            jit_rr(op, rs1, rd, 0, offset);
        }
    } else if (name_is(lui)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rd = intval();
        expect(',');
        imm = 1;
        if (match('-')) {
            imm = -1;
        }
        expect(TOK_INT);
        imm *= intval();
        jit_ri16(op, rd, 0, imm);
    } else if (name_is(mfhi) || name_is(mflo) || name_is(popw) || name_is(pop)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rd = intval();
        jit_rr(op, rd, 0, 0, 0);
    } else if (name_is(mthi) || name_is(mtlo)
        || name_is(pushw) || name_is(push) || name_is(jr)) {

        op = instr_opcode(name);
        expect(TOK_REG);
        rs1 = intval();
        jit_rr(op, 0, rs1, 0, 0);
    } else if (name_is(or) || name_is(and) || name_is(xor) || name_is(nor)
        || name_is(shl) || name_is(shr) || name_is(add) || name_is(sub)
        || name_is(subu) || name_is(eq) || name_is(slt) || name_is(sltu)) {

        op = instr_opcode(name);
        expect(TOK_REG);
        rd = intval();
        expect(',');
        expect(TOK_REG);
        rs1 = intval();
        expect(',');
        expect(TOK_REG);
        rs2 = intval();
        jit_rr(op, rd, rs1, rs2, 0);
    } else if (name_is(mul) || name_is(mulu) || name_is(div) || name_is(divu)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rs1 = intval();
        expect(',');
        expect(TOK_REG);
        rs2 = intval();
        if (match(',')) {
            expect(TOK_REG);
            rd = intval();
            jit_rr(op, 0, rs2, rd, 0);
            jit_mflo(rs1);
        } else {
            jit_rr(op, 0, rs1, rs2, 0);
        }
    } else if (name_is(mod) || name_is(modu)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rd = intval();
        expect(',');
        expect(TOK_REG);
        rs1 = intval();
        expect(',');
        expect(TOK_REG);
        rs2 = intval();

        name_is(mod) ? jit_mod(rd, rs1, rs2) : jit_modu(rd, rs1, rs2);
    } else if (name_is(mov)) {
        expect(TOK_REG);
        rd = intval();
        expect(',');
        expect(TOK_REG);
        rs1 = intval();
        jit_mov(rd, rs1);
    } else if (name_is(slti) || name_is(sltiu) || name_is(eqi) || name_is(eqiu)
        || name_is(ori) || name_is(andi) || name_is(xori) || name_is(shli)
        || name_is(shri) || name_is(addi) || name_is(addiu)) {

        op = instr_opcode(name);
        expect(TOK_REG);
        rd = intval();
        expect(',');
        expect(TOK_REG);
        rs1 = intval();
        expect(',');
        imm = 1;
        if (match('-')) {
            imm = -1;
        }
        expect(TOK_INT);
        imm *= intval();
        jit_ri16(op, rd, rs1, imm & 0xffff);
    } else if (name_is(call) || name_is(j)) {
        op = instr_opcode(name);
        expect(TOK_NAME);
        label = srcstr();
        jit_jump(op, 0, 0, label);
    } else if (name_is(je) || name_is(jne)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rs1 = intval();
        expect(',');
        expect(TOK_REG);
        rs2 = intval();
        expect(',');
        expect(TOK_NAME);
        label = srcstr();
        jit_jump(op, rs1, rs2, label);
    } else {
        parser_errorf("unknown instruction '%s'", name);
    }
}

#define parse_data() __parse_data(pars)
static void __parse_data(parser_t* pars)
{
    while (match(TOK_NAME)) {
        char* name = srcstr();

        if (strcmp(name, ".text") == 0) {
            return;
        }

        if (match(TOK_NAME)) {
            char* align = srcstr();
            int   el_size = 4;
            if (strcmp(align, ".byte") == 0 || strcmp(align, ".b") == 0) {
                el_size = 1;
            } else if (strcmp(align, ".hword") == 0 || strcmp(align, ".hw") == 0) {
                el_size = 2;
            } else if (strcmp(align, ".word") == 0 || strcmp(align, ".w") == 0) {
                el_size = 4;
            } else if (strcmp(align, ".dword") == 0 || strcmp(align, ".dw") == 0) {
                el_size = 8;
            } else {
                parser_errorf("expected alignment specifier, got '%s'", kind_str(pars->tok->kind));
            }

            buf_t*   buf = buf_alloc(256);
            size_t   size = 0;
            uint64_t val = 0;
            switch (el_size) {
            case 1:
                if (match(TOK_INT)) {
                    val = intval();
                    if (val > 0xff) {
                        parser_warning("integer constant overflows byte");
                        val &= 0xff;
                    }
                    size = buf_write(buf, size, val);
                    while (match(',')) {
                        expect(TOK_INT);
                        val = intval();
                        if (val > 0xff) {
                            parser_warning("integer constant overflows byte");
                            val &= 0xff;
                        }
                        size = buf_write(buf, size, val);
                    }
                } else if (match(TOK_STR)) {
                    buf_write_str(buf, 0, strval());
                }
                break;
            case 2:
                expect(TOK_INT);
                val = intval();
                if (val > 0xffff) {
                    parser_warning("integer constant overflows hword");
                    val &= 0xffff;
                }
                size = buf_write_uint16(buf, size, val);
                while (match(',')) {
                    expect(TOK_INT);
                    val = intval();
                    if (val > 0xffff) {
                        parser_warning("integer constant overflows hword");
                        val &= 0xffff;
                    }
                    size = buf_write_uint16(buf, size, val);
                }
                break;
            case 4:
                expect(TOK_INT);
                val = intval();
                if (val > 0xffffffff) {
                    parser_warning("integer constant overflows word");
                    val &= 0xffffffff;
                }
                size = buf_write_uint32(buf, size, val);
                while (match(',')) {
                    expect(TOK_INT);
                    val = intval();
                    if (val > 0xffffffff) {
                        parser_warning("integer constant overflows word");
                        val &= 0xffffffff;
                    }
                    size = buf_write_uint32(buf, size, val);
                }
                break;
            case 8:
                expect(TOK_INT);
                val = intval();
                size = buf_write_uint64(buf, size, val);
                while (match(',')) {
                    expect(TOK_INT);
                    val = intval();
                    size = buf_write_uint64(buf, size, val);
                }
                break;
            }
            parser_debug("matched data declaration: '%s' (%zu bytes)\n", name, buf->cap);
            jit_data(name, buf->bytes, size);
            buf_free(&buf);
        }
    }
}

void parse(parser_t* pars)
{
    for (;;) {
        if (match(TOK_EOF)) {
            parser_debug("matched EOF\n");
            return;
        }

        if (match(TOK_NAME)) {
            char* name = srcstr();

            if (strcmp(name, ".data") == 0) {
                parse_data();
                continue;
            }

            if (match(':')) {
                parser_debug("matched label: '%s'\n", name);
                jit_label(name);
                continue;
            }

            parse_instr(name);
            continue;
        }
    }
}

void parser_delete(parser_t* pars)
{
    scanner_delete(pars->scan);
}