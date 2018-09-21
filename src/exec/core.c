#include "core.h"
#include "../shared/disasm.h"
#include "../shared/icode.h"
#include "maths.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void breakpoint(core_t* core);

enum {
    EXC_BAD_ACCESS = 0x10,
    EXC_PC_OUT_OF_BOUNDS,
    EXC_ZERO_DIV,
};

enum {
    UNSIGNED,
    SIGNED,
};

enum {
    ACCESS_BYTE = 1,
    ACCESS_HALFWORD = 2,
    ACCESS_WORD = 4,
    ACCESS_DOUBLEWORD = 8,
};

#define R core->reg
#define GP R[28]
#define SP R[29]
#define FP R[30]
#define RA R[31]
#define PC core->pc

#define RDATA_SIZE (0x10000)

static inline int decode_op(uint32_t word)
{
    return (int)(word & 0x3f);
}

#define rd R[decode_rd(*ip)]
static inline int decode_rd(uint32_t word)
{
    return (int)((word >> 6) & 0x1f);
}

#define rs1 R[decode_rs1(*ip)]
static inline int decode_rs1(uint32_t word)
{
    return (int)((word >> 11) & 0x1f);
}

#define rs2 R[decode_rs2(*ip)]
static inline int decode_rs2(uint32_t word)
{
    return (int)((word >> 16) & 0x1f);
}

static inline int decode_offset(uint32_t word)
{
    return ((int)word >> 21) ^ (1 << 10) - (1 << 10);
}

static inline unsigned int decode_imm16_unsigned(uint32_t word)
{
    return word >> 16;
}

static inline int decode_imm16_signed(uint32_t word)
{
    return (int)word >> 16;
}

static inline int decode_imm24(uint32_t word)
{
    return (int)word >> 8;
}

static inline void error(char* message)
{
    printf("[*] %s\n", message);
}

static inline void errorf(char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char* message;
    vasprintf(&message, fmt, args);
    error(message);
    free(message);

    va_end(args);
}

#ifdef exception
#undef exception
#endif
#define exception(excode) __exception(core, excode)
static inline void __exception(core_t* core, int excode)
{
    errorf("exception (code=0x%x addr=0x%x)", excode, core->pc);
    exit(-1);
}

#define exception_if(cond, excode) __exception_if(core, cond, excode)
static inline void __exception_if(core_t* core, int cond, int excode)
{
    if (cond) {
        exception(excode);
    }
}

#define map_address(v_addr) __map_address(core, v_addr)
static inline void* __map_address(core_t* core, uint64_t v_addr)
{
    return (void*)(core->mem + v_addr);
}

#define unmap_address(p_addr) __unmap_address(core, p_addr)
static inline uint64_t __unmap_address(core_t* core, void* p_addr)
{
    return (uint64_t)(p_addr - (void*)core->mem);
}

#define check_memory_readable(v_addr, access_length) __check_memory_readable(core, v_addr, access_length)
static void __check_memory_readable(core_t* core, uint64_t v_addr, int access_length)
{
    int base_readable = v_addr >= core->text_size;
    int end_readable = v_addr + access_length <= sizeof(core->mem);
    exception_if(!(base_readable && end_readable), EXC_BAD_ACCESS);
}

#define check_memory_writable(v_addr, access_length) __check_memory_writable(core, v_addr, access_length)
static void __check_memory_writable(core_t* core, uint64_t v_addr, int access_length)
{
    int base_writable = v_addr >= core->text_size && v_addr < (core->text_size + core->data_size);

    uint64_t v_end = v_addr + access_length;
    int      end_writable = v_end < (core->text_size + core->data_size);

    exception_if(!(base_writable && end_writable), EXC_BAD_ACCESS);
}

#define load_memory(v_addr, access_length, sign) __load_memory(core, v_addr, access_length, sign)
static uint64_t __load_memory(core_t* core, uint64_t v_addr, int access_length, int sign)
{
    check_memory_readable(v_addr, access_length);
    void* p_addr = map_address(v_addr);

    switch (access_length) {
    case ACCESS_BYTE:
        return sign == UNSIGNED ? *(uint8_t*)p_addr : (int64_t)(*(int8_t*)p_addr);
    case ACCESS_HALFWORD:
        return sign == UNSIGNED ? *(uint16_t*)p_addr : (int64_t)(*(int16_t*)p_addr);
    case ACCESS_WORD:
        return sign == UNSIGNED ? *(uint32_t*)p_addr : (int64_t)(*(int32_t*)p_addr);
    case ACCESS_DOUBLEWORD:
        return sign == UNSIGNED ? *(uint64_t*)p_addr : *(int64_t*)p_addr;
    }

    return -1;
}

#define store_memory(v_addr, access_length, data) __store_memory(core, v_addr, access_length, data)
static void __store_memory(core_t* core, uint64_t v_addr, int access_length, uint64_t data)
{
    check_memory_writable(v_addr, access_length);
    void* p_addr = map_address(v_addr);

    switch (access_length) {
    case ACCESS_BYTE:
        *(uint8_t*)p_addr = data & 0xff;
        break;
    case ACCESS_HALFWORD:
        *(uint16_t*)p_addr = data & 0xffff;
        break;
    case ACCESS_WORD:
        *(uint32_t*)p_addr = data & 0xffffffff;
        break;
    case ACCESS_DOUBLEWORD:
        *(uint64_t*)p_addr = data;
        break;
    }
}

#define push(val) __push(core, val)
static inline void __push(core_t* core, uint64_t val)
{
    SP -= 8;
    store_memory(SP, ACCESS_DOUBLEWORD, val);
}

#define pushw(val) __pushw(core, val)
static inline void __pushw(core_t* core, uint32_t val)
{
    SP -= 4;
    store_memory(SP, ACCESS_WORD, val);
}

#define pop(reg) __pop(core, reg)
static inline void __pop(core_t* core, uint64_t* reg)
{
    *reg = load_memory(SP, ACCESS_DOUBLEWORD, UNSIGNED);
    SP += 8;
}

#define popw(reg) __popw(core, reg)
static inline void __popw(core_t* core, uint64_t* reg)
{
    *reg = load_memory(SP, ACCESS_WORD, UNSIGNED);
    SP += 4;
}

#define add_pc(offset) ___pc(core, PC + offset)
#define set_pc(npc) __set_pc(core, npc)
static inline void __set_pc(core_t* core, uint64_t npc)
{
    PC = npc;
}

static inline void save_frame(core_t* core)
{
    push(RA);
    push(FP);
    RA = PC;
    FP = SP;
    set_pc(decode_imm24(*(uint32_t*)map_address(PC)) - 4);

    push(R[16]);
    push(R[17]);
    push(R[18]);
    push(R[19]);
    push(R[20]);
    push(R[21]);
    push(R[22]);
    push(R[23]);
}

static inline void restore_frame(core_t* core)
{
    pop(&R[23]);
    pop(&R[22]);
    pop(&R[21]);
    pop(&R[20]);
    pop(&R[19]);
    pop(&R[18]);
    pop(&R[17]);
    pop(&R[16]);

    set_pc(RA);
    pop(&FP);
    pop(&RA);
}

static inline void show_disas(core_t* core)
{
    if (config.debug) {
        printf("0x%-12llX ", PC);
        disasm(NULL, stdout, (uint32_t*)map_address(PC), 1);
    }
}

static inline void interrupt(core_t* core)
{
    switch (R[4]) {
    case 0x04: /* getchar */
        R[2] = getchar();
        break;
    case 0x05: /* putchar */
        putchar((int)R[5]);
        break;
    case 0x0a: /* exit */
        exit(R[5]);
    }
}

void core_exec(core_t* core)
{
    GP = unmap_address(core->seg_data + core->data_size);
    FP = SP = config.memory_size;

    PC = 0;
    uint32_t* ip;

core_loop:
    if (PC > core->text_size) {
        exception(EXC_PC_OUT_OF_BOUNDS);
    }
    ip = map_address(PC);
    switch (decode_op(*ip)) {
    case _NOP: /* nop */
        show_disas(core);
        break;

    case _INTERRUPT: /* int */
        show_disas(core);
        interrupt(core);
        break;

    case _BREAKPOINT: /* breakpoint */
        if (config.debug) {
            show_disas(core);
            breakpoint(core);
            PC += 4;
            R[0] = 0x0; // $r0 is hard wired to 0
            goto core_loop;
        }
        break;

    case _LB: /* lb %RD, offset(%RS1) */
        show_disas(core);
        rd = load_memory(rs1 + decode_offset(*ip), ACCESS_BYTE, SIGNED);
        break;

    case _LBU: /* lbu %RD, offset(%RS1) */
        show_disas(core);
        rd = load_memory(rs1 + decode_offset(*ip), ACCESS_BYTE, UNSIGNED);
        break;

    case _LH: /* lh %RD, offset(%RS1) */
        show_disas(core);
        rd = load_memory(rs1 + decode_offset(*ip), ACCESS_HALFWORD, SIGNED);
        break;

    case _LHU: /* lhu %RD, offset(%RS1) */
        show_disas(core);
        rd = load_memory(rs1 + decode_offset(*ip), ACCESS_BYTE, UNSIGNED);
        break;

    case _LUI: /* lui %RD, $imm16 */
        show_disas(core);
        rd = decode_imm16_unsigned(*ip) << 16;
        break;

    case _LW: /* lw %RD, offset(%RS1) */
        show_disas(core);
        rd = load_memory(rs1 + decode_offset(*ip), ACCESS_WORD, SIGNED);
        break;

    case _LWU: /* lwu %RD, offset(%RS1) */
        show_disas(core);
        rd = load_memory(rs1 + decode_offset(*ip), ACCESS_WORD, UNSIGNED);
        break;

    case _LD: /* ld %RD, offset(%RS1) */
        show_disas(core);
        rd = load_memory(rs1 + decode_offset(*ip), ACCESS_DOUBLEWORD, UNSIGNED);
        break;

    case _SB: /* sb %RS1, offset(%RD) */
        show_disas(core);
        store_memory(rd + decode_offset(*ip), ACCESS_BYTE, rs1);
        break;

    case _SH: /* sh %RS1, offset(%RD) */
        show_disas(core);
        store_memory(rd + decode_offset(*ip), ACCESS_HALFWORD, rs1);
        break;

    case _SW: /* sw %RS1, offset(%RD) */
        show_disas(core);
        store_memory(rd + decode_offset(*ip), ACCESS_WORD, rs1);
        break;

    case _SD: /* sd %RS1, offset(%RD) */
        show_disas(core);
        store_memory(rd + decode_offset(*ip), ACCESS_DOUBLEWORD, rs1);
        break;

    case _MFHI: /* mfhi %rd */
        show_disas(core);
        rd = core->hi;
        break;

    case _MTHI: /* mthi %rs1 */
        show_disas(core);
        core->hi = rs1;
        break;

    case _MFLO: /* mflo %rd */
        show_disas(core);
        rd = core->lo;
        break;

    case _MTLO: /* mtlo %rs1 */
        show_disas(core);
        core->lo = rs1;
        break;

    case _SLT: /* slt $rd, $rs1, $rs2 */
        show_disas(core);
        rd = ((int64_t)rs1 < (int64_t)rs2);
        break;

    case _SLTU: /* sltu $rd, $rs1, $rs2 */
        show_disas(core);
        rd = (rs1 < rs2);
        break;

    case _SLTI: /* slti $rd, $rs1, #imm16 */
        show_disas(core);
        rd = ((int64_t)rs1 < decode_imm16_signed(*ip));
        break;

    case _SLTIU: /* sltiu $rd, $rs1, #imm16 */
        show_disas(core);
        rd = (rs1 < decode_imm16_unsigned(*ip));
        break;

    case _EQ: /* eq $rd, $rs1, $rs2 */
        show_disas(core);
        rd = (rs1 == rs2);
        break;

    case _EQI: /* eqi $rd, $rs1, #imm16 */
        show_disas(core);
        rd = ((int64_t)rs1 == decode_imm16_signed(*ip));
        break;

    case _EQIU: /* eqiu $rd, $rs1, #imm16 */
        show_disas(core);
        rd = (rs1 == decode_imm16_unsigned(*ip));
        break;

    case _OR: /* or $rd, $rs1, $rs2 */
        show_disas(core);
        rd = rs1 | rs2;
        break;

    case _ORI: /* ori $rd, $rs1, #imm16 */
        show_disas(core);
        rd = rs1 | decode_imm16_unsigned(*ip);
        break;

    case _AND: /* and $rd, $rs1, $rs2 */
        show_disas(core);
        rd = rs1 & rs2;
        break;

    case _ANDI: /* andi $rd, $rs1, #imm16 */
        show_disas(core);
        rd = rs1 & decode_imm16_unsigned(*ip);
        break;

    case _XOR: /* xor $rd, $rs1, $rs2 */
        show_disas(core);
        rd = rs1 ^ rs2;
        break;

    case _XORI: /* xori $rd, $rs1, #imm16 */
        show_disas(core);
        rd = rs1 ^ decode_imm16_unsigned(*ip);
        break;

    case _NOR: /* nor $rd, $rs1, $rs2 */
        show_disas(core);
        rd = ~(rs1 | rs2);
        break;

    case _SHL: /* shl $rd, $rs1, $rs2 */
        show_disas(core);
        rd = rs1 << rs2;
        break;

    case _SHLI: /* shli $rd, $rs1, #imm16 */
        show_disas(core);
        rd = rs1 << decode_imm16_unsigned(*ip);
        break;

    case _SHR: /* shr $rd, $rs1, $rs2 */
        show_disas(core);
        rd = rs1 >> rs2;
        break;

    case _SHRI: /* shri $rd, $rs1, #imm16 */
        show_disas(core);
        rd = rs1 >> decode_imm16_unsigned(*ip);
        break;

    case _ADD: /* add $rd, $rs1, $rs2 */
        show_disas(core);
        rd = rs1 + rs2;
        break;

    case _ADDI: /* addi $rd, $rs1, #imm16 */
        show_disas(core);
        rd = (int64_t)rs1 + decode_imm16_signed(*ip);
        break;

    case _ADDIU: /* addiu $rd, $rs1, #imm16 */
        show_disas(core);
        rd = rs1 + rs2;
        break;

    case _SUB: /* sub $rd, $rs1, $rs2 */
        show_disas(core);
        rd = (int64_t)rs1 - decode_imm16_signed(*ip);
        break;

    case _SUBU: /* subu $rd, $rs1, $rs2 */
        show_disas(core);
        rd = rs1 - decode_imm16_unsigned(*ip);
        break;

    case _MUL: /* mul $rs1, $rs2 */
        show_disas(core);
        mult64to128(rs1, rs2, &core->hi, &core->lo, SIGNED);
        break;

    case _MULU: /* mulu $rs1, $rs2 */
        show_disas(core);
        mult64to128(rs1, rs2, &core->hi, &core->lo, UNSIGNED);
        break;

    case _DIV: /* div $rs1, $rs2 */
        show_disas(core);
        exception_if(rs2 == 0, EXC_ZERO_DIV);
        core->hi = (int64_t)rs1 % (int64_t)rs2;
        core->lo = (int64_t)rs1 / (int64_t)rs2;
        break;

    case _DIVU: /* divu $rs1, $rs2 */
        show_disas(core);
        exception_if(rs2 == 0, EXC_ZERO_DIV);
        core->hi = rs1 % rs2;
        core->lo = rs1 / rs2;
        break;

    case _PUSHW: /* pushw $rd */
        show_disas(core);
        pushw(rs1);
        break;

    case _PUSH: /* push $rs1 */
        show_disas(core);
        push(rs1);
        break;

    case _POPW: /* popw $rd */
        show_disas(core);
        popw(&rd);
        break;

    case _POP: /* pop $rd */
        show_disas(core);
        pop(&rd);
        break;

    case _CALL: /* call label */
        show_disas(core);
        save_frame(core);
        break;

    case _RET: /* ret */
        show_disas(core);
        restore_frame(core);
        break;

    case _J: /* j label */
        show_disas(core);
        set_pc(decode_imm24(*ip) - 1);
        break;

    case _JR: /* jr $rs1 */
        show_disas(core);
        set_pc(rd - 1);
        break;

    case _JE: /* je $rs1, $rs2, label */
        show_disas(core);
        if (rd == rs1)
            set_pc(decode_imm16_unsigned(*ip) - 4);
        break;

    case _JNE: /* jne $rs1, $rs2, label */
        show_disas(core);
        if (rd != rs1)
            set_pc(decode_imm16_unsigned(*ip) - 4);
        break;
    }

    PC += 4;
    R[0] = 0x0; // $r0 is hard wired to 0

    if (config.debug && core->step) {
        core->step = 0;
        breakpoint(core);
    }

    goto core_loop;
}
