#include "parser.h"
#include "../disasm.h"
#include "../intern.h"
#include "../jit.h"
#include "../vector.h"
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

#define parser_errorf(fmt, ...)                                                                              \
    {                                                                                                        \
        char*       sfmt;                                                                                    \
        static char buf[1024];                                                                               \
        asprintf(&sfmt, "error :: %s(%zu:%zu): %s", pars->file_name, pars->prev->lno, pars->prev->col, fmt); \
        snprintf(buf, sizeof(buf), sfmt, ##__VA_ARGS__);                                                     \
        printf("%s\n", buf);                                                                                 \
        free(sfmt);                                                                                          \
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
    if (tok_is(kind)) {
        fwd();
        return 1;
    }

    return 0;
}

#define expect(kind) __expect(pars, kind)
static void __expect(parser_t* pars, tkind_t kind)
{
    if (!match(kind)) {
        parser_errorf("expected '%s', got '%s'", kind_str(kind), kind_str(pars->tok->kind));
    }
}

#define ival() __ival(pars)
static int __ival(parser_t* pars)
{
    return pars->prev->int_val;
}

#define strval() __strval(pars)
static char* __strval(parser_t* pars)
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
        rd = ival();
        expect(',');
        offset = 1;
        if (match('-')) {
            offset = -1;
        }
        expect(TOK_INT);
        offset *= ival();
        if (offset < 0) {
            expect('(');
        }
        expect(TOK_REG);
        rs1 = ival();
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
        rd = ival();
        expect(',');
        imm = 1;
        if (match('-')) {
            imm = -1;
        }
        expect(TOK_INT);
        imm *= ival();
        jit_ri16(op, rd, 0, imm);
    } else if (name_is(mfhi) || name_is(mflo) || name_is(popw) || name_is(pop)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rd = ival();
        jit_rr(op, rd, 0, 0, 0);
    } else if (name_is(mthi) || name_is(mtlo)
        || name_is(pushw) || name_is(push) || name_is(jr)) {

        op = instr_opcode(name);
        expect(TOK_REG);
        rs1 = ival();
        jit_rr(op, 0, rs1, 0, 0);
    } else if (name_is(or) || name_is(and) || name_is(xor) || name_is(nor)
        || name_is(shl) || name_is(shr) || name_is(add) || name_is(sub)
        || name_is(subu) || name_is(eq) || name_is(slt) || name_is(sltu)) {

        op = instr_opcode(name);
        expect(TOK_REG);
        rd = ival();
        expect(',');
        expect(TOK_REG);
        rs1 = ival();
        expect(',');
        expect(TOK_REG);
        rs2 = ival();
        jit_rr(op, rd, rs1, rs2, 0);
    } else if (name_is(mul) || name_is(mulu) || name_is(div) || name_is(divu)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rs1 = ival();
        expect(',');
        expect(TOK_REG);
        rs2 = ival();
        if (match(',')) {
            expect(TOK_REG);
            rd = ival();
            jit_rr(op, 0, rs2, rd, 0);
            jit_mflo(rs1);
        } else {
            jit_rr(op, 0, rs1, rs2, 0);
        }
    } else if (name_is(mod) || name_is(modu)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rd = ival();
        expect(',');
        expect(TOK_REG);
        rs1 = ival();
        expect(',');
        expect(TOK_REG);
        rs2 = ival();

        name_is(mod) ? jit_mod(rd, rs1, rs2) : jit_modu(rd, rs1, rs2);
    } else if (name_is(mov)) {
        expect(TOK_REG);
        rd = ival();
        expect(',');
        expect(TOK_REG);
        rs1 = ival();
        jit_mov(rd, rs1);
    } else if (name_is(slti) || name_is(sltiu) || name_is(eqi) || name_is(eqiu)
        || name_is(ori) || name_is(andi) || name_is(xori) || name_is(shli)
        || name_is(shri) || name_is(addi) || name_is(addiu)) {

        op = instr_opcode(name);
        expect(TOK_REG);
        rd = ival();
        expect(',');
        expect(TOK_REG);
        rs1 = ival();
        expect(',');
        imm = 1;
        if (match('-')) {
            imm = -1;
        }
        expect(TOK_INT);
        imm *= ival();
        jit_ri16(op, rd, rs1, imm);
    } else if (name_is(call) || name_is(j)) {
        op = instr_opcode(name);
        expect(TOK_NAME);
        label = strval();
        jit_jump(op, 0, 0, label);
    } else if (name_is(je) || name_is(jne)) {
        op = instr_opcode(name);
        expect(TOK_REG);
        rs1 = ival();
        expect(',');
        expect(TOK_REG);
        rs2 = ival();
        expect(',');
        expect(TOK_NAME);
        label = strval();
        jit_jump(op, rs1, rs2, label);
    }
}

void parse(parser_t* pars)
{
    for (;;) {
        if (match(TOK_EOF)) {
            return;
        }

        if (match(TOK_NAME)) {
            char* name = strval();

            if (match(':')) {
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