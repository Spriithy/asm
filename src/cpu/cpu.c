#include "cpu.h"
#include "opcode.h"
#include "register.h"
#include <stdbool.h>
#include <stdio.h>

#define REG_BYTE rw_i8(cpu.gpr)
#define REG_BYTE_U rw_u8(cpu.gpr)
#define REG_WORD rw_i16(cpu.gpr)
#define REG_WORD_U rw_u16(cpu.gpr)
#define REG_DWORD rw_i32(cpu.gpr)
#define REG_DWORD_U cpu.gpr
#define REG_QWORD rw_i64(cpu.xmm)
#define REG_QWORD_U cpu.xmm

#define REG_INDIRECT_BYTE_U(r, o) MEM_BYTE_U[REG_DWORD_U[(r)] + (o)]
#define REG_INDIRECT_WORD_U(r, o) MEM_WORD_U[REG_DWORD_U[(r)] + (o)]
#define REG_INDIRECT_DWORD_U(r, o) MEM_DWORD_U[REG_DWORD_U[(r)] + (o)]
#define REG_INDIRECT_QWORD_U(r, o) MEM_QWORD_U[REG_DWORD_U[(r)] + (o)]

#define MEM_BYTE rw_i8(cpu.data)
#define MEM_BYTE_U rw_u8(cpu.data)
#define MEM_WORD rw_i16(cpu.data)
#define MEM_WORD_U rw_u16(cpu.data)
#define MEM_DWORD rw_i32(cpu.data)
#define MEM_DWORD_U rw_u32(cpu.data)
#define MEM_QWORD rw_i64(cpu.data)
#define MEM_QWORD_U rw_u64(cpu.data)

#define CF 0x0001
#define PF 0x0004
#define AF 0x0010
#define ZF 0x0040
#define SF 0x0080
#define TF 0x0100
#define IF 0x0200
#define DF 0x0400
#define OF 0x0800

#define FLAG(flag) (REG_WORD[FLAGS] & flag)
#define SET_FLAG(flag) REG_WORD_U[FLAGS] |= flag
#define UNSET_FLAG(flag) REG_WORD_U[FLAGS] &= ~flag

#define SIGN_EXTEND(x, w) dword_u(dword(x << (32 - w)) >> (32 - w))

struct cpu cpu;

/*
static u8*  al = &REG_BYTE_U[AL];
static u16* ax = &REG_WORD_U[AX];
static u32* eax = &REG_DWORD_U[EAX];
static u64* xmm0 = &REG_QWORD_U[XMM0];
*/
static u32* eip = &REG_DWORD_U[EIP];

u32 bits(u32 data, u32 start, u32 len)
{
    return (data >> start) & ((1 << len) - 1);
}

u32 sign_extend(u32 data, u32 width)
{
    return (u32)((i32)(data << (32 - width)) >> (32 - width));
}

enum operand_size decode_operand_size(u8 byte)
{
    return (enum operand_size)byte & 0x03;
}

int decode_operand(u8 byte)
{
    return byte >> 2;
}

static u8 fetch_u8()
{
    return MEM_BYTE_U[(*eip)++];
}

static u16 fetch_u16()
{
    u16 val = *(u16*)&MEM_BYTE_U[*eip];
    *eip += sizeof(u16);
    return val;
}

static u32 fetch_u32()
{
    u32 val = *(u32*)&MEM_BYTE_U[*eip];
    *eip += sizeof(u32);
    return val;
}

static u64 fetch_u64()
{
    u64 val = *(u64*)&MEM_BYTE_U[*eip];
    *eip += sizeof(u64);
    return val;
}

static void mov_gpr8_imm8(enum r8 reg, u8 imm8)
{
    REG_BYTE_U[reg] = imm8;
}

static void mov_gpr8(enum r8 dst, enum r8 src)
{
    REG_BYTE_U[dst] = REG_BYTE_U[src];
}

static void mov_gpr8_gpr16(enum r8 dst, enum r16 src)
{
    REG_BYTE_U[dst] = REG_WORD_U[src];
}

static void mov_gpr8_gpr32(enum r8 dst, enum r32 src)
{
    REG_BYTE_U[dst] = REG_DWORD_U[src];
}

static void mov_gpr8_xmm(enum r8 dst, enum r64 src)
{
    REG_BYTE_U[dst] = REG_QWORD_U[src];
}

static void mov_gpr16_imm16(enum r16 reg, u16 imm16)
{
    REG_WORD_U[reg] = imm16;
}

static void mov_gpr16_gpr8(enum r16 dst, enum r8 src)
{
    REG_WORD_U[dst] = REG_BYTE_U[src];
}

static void mov_gpr16(enum r16 dst, enum r16 src)
{
    REG_WORD_U[dst] = REG_WORD_U[src];
}

static void mov_gpr16_gpr32(enum r16 dst, enum r32 src)
{
    REG_WORD_U[dst] = REG_DWORD_U[src];
}

static void mov_gpr16_xmm(enum r16 dst, enum r64 src)
{
    REG_WORD_U[dst] = REG_QWORD_U[src];
}

static void mov_gpr32_imm32(enum r32 reg, u32 imm32)
{
    REG_DWORD_U[reg] = imm32;
}

static void mov_gpr32_gpr8(enum r32 dst, enum r8 src)
{
    REG_DWORD_U[dst] = REG_BYTE_U[src];
}

static void mov_gpr32_gpr16(enum r32 dst, enum r16 src)
{
    REG_DWORD_U[dst] = REG_WORD_U[src];
}

static void mov_gpr32(enum r32 dst, enum r32 src)
{
    REG_DWORD_U[dst] = REG_DWORD_U[src];
}

static void mov_gpr32_xmm(enum r32 dst, enum r64 src)
{
    REG_DWORD_U[dst] = REG_QWORD_U[src];
}

static void mov_xmm_imm64(enum r64 reg, u64 imm64)
{
    REG_QWORD_U[reg] = imm64;
}

static void mov_xmm_gpr8(enum r64 dst, enum r8 src)
{
    REG_QWORD_U[dst] = REG_BYTE_U[src];
}

static void mov_xmm_gpr16(enum r64 dst, enum r16 src)
{
    REG_QWORD_U[dst] = REG_WORD_U[src];
}

static void mov_xmm_gpr32(enum r64 dst, enum r32 src)
{
    REG_QWORD_U[dst] = REG_DWORD_U[src];
}

static void mov_xmm(enum r64 dst, enum r64 src)
{
    REG_QWORD_U[dst] = REG_QWORD_U[src];
}

static void interupt(i8 icode)
{
    (void)icode;
}

void exec()
{
    (void)interupt;

    *eip = 0x00;

    int operand_size;
    int tmp, r0, r1;
    u8  imm8, op;
    u16 imm16 /* , offs */;
    u32 imm32 /* , addr */;
    u64 imm64;

next:
    op = fetch_u8();
    switch (op) {
    case MOV_RI:
        tmp = fetch_u8();
        switch (operand_size = decode_operand_size(tmp)) {
        case BYTE:
            imm8 = fetch_u8();
            r0 = decode_operand(tmp);
            mov_gpr8_imm8(r0, imm8);
            goto next;

        case WORD:
            imm16 = fetch_u16();
            r0 = decode_operand(tmp);
            mov_gpr16_imm16(r0, imm16);
            goto next;

        case DWORD:
            imm32 = fetch_u32();
            r0 = decode_operand(tmp);
            mov_gpr32_imm32(r0, imm32);
            goto next;

        case QWORD:
            imm64 = fetch_u64();
            r0 = decode_operand(tmp);
            mov_xmm_imm64(r0, imm64);
            goto next;

        default:
            exceptionf("invalid dest operand size in MOV instruction: %d", operand_size);
        }

    case MOV_RR:
        tmp = fetch_u8();
        r0 = decode_operand(tmp);
        switch (operand_size = decode_operand_size(tmp)) {
        case BYTE:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            switch (operand_size = decode_operand_size(tmp)) {
            case BYTE:
                mov_gpr8(r0, r1);
                goto next;

            case WORD:
                mov_gpr8_gpr16(r0, r1);
                goto next;

            case DWORD:
                mov_gpr8_gpr32(r0, r1);
                goto next;

            case QWORD:
                mov_gpr8_xmm(r0, r1);
                goto next;

            default:
                exceptionf("invalid src operand size in MOV instruction: %d", operand_size);
            }

        case WORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            switch (operand_size = decode_operand_size(tmp)) {
            case BYTE:
                mov_gpr16_gpr8(r0, r1);
                goto next;

            case WORD:
                mov_gpr16(r0, r1);
                goto next;

            case DWORD:
                mov_gpr16_gpr32(r0, r1);
                goto next;

            case QWORD:
                mov_gpr16_xmm(r0, r1);
                goto next;

            default:
                exceptionf("invalid src operand size in MOV instruction: %d", operand_size);
            }

        case DWORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            switch (operand_size = decode_operand_size(tmp)) {
            case BYTE:
                mov_gpr32_gpr8(r0, r1);
                goto next;

            case WORD:
                mov_gpr32_gpr16(r0, r1);
                goto next;

            case DWORD:
                mov_gpr32(r0, r1);
                goto next;

            case QWORD:
                mov_gpr32_xmm(r0, r1);
                goto next;

            default:
                exceptionf("invalid src operand size in MOV instruction: %d", operand_size);
            }

        case QWORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            switch (operand_size = decode_operand_size(tmp)) {
            case BYTE:
                mov_xmm_gpr8(r0, r1);
                goto next;

            case WORD:
                mov_xmm_gpr16(r0, r1);
                goto next;

            case DWORD:
                mov_xmm_gpr32(r0, r1);
                goto next;

            case QWORD:
                mov_xmm(r0, r1);
                goto next;

            default:
                exceptionf("invalid src operand size in MOV instruction: %d", operand_size);
            }

        default:
            exceptionf("invalid dest operand size in MOV instruction: %d", operand_size);
        }

    case NOP:
        goto next;

    case HALT:
        clean();
        break;
    }
}

void clean()
{
}
