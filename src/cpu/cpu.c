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

static void mov_m8_imm8(u32 addr, u8 imm8)
{
    MEM_BYTE_U[addr] = imm8;
}

static void mov_m8_gpr8(u32 addr, enum r8 src)
{
    MEM_BYTE_U[addr] = REG_BYTE_U[src];
}

static void mov_gpr8_m8(enum r8 dst, u32 addr)
{
    REG_BYTE_U[dst] = MEM_BYTE_U[addr];
}

static void mov_gpr16_imm16(enum r16 reg, u16 imm16)
{
    REG_WORD_U[reg] = imm16;
}

static void mov_gpr16(enum r16 dst, enum r16 src)
{
    REG_WORD_U[dst] = REG_WORD_U[src];
}

static void mov_m16_imm16(u32 addr, u16 imm16)
{
    *(u16*)&MEM_BYTE_U[addr] = imm16;
}

static void mov_m16_gpr16(u32 addr, enum r16 src)
{
    *(u16*)&MEM_BYTE_U[addr] = REG_WORD_U[src];
}

static void mov_gpr16_m16(enum r16 dst, u32 addr)
{
    REG_WORD_U[dst] = *(u16*)&MEM_BYTE_U[addr];
}

static void mov_gpr32_imm32(enum r32 reg, u32 imm32)
{
    REG_DWORD_U[reg] = imm32;
}

static void mov_gpr32(enum r32 dst, enum r32 src)
{
    REG_DWORD_U[dst] = REG_DWORD_U[src];
}

static void mov_m32_imm32(u32 addr, u32 imm32)
{
    *(u32*)&MEM_BYTE_U[addr] = imm32;
}

static void mov_m32_gpr32(u32 addr, enum r32 src)
{
    *(u32*)&MEM_BYTE_U[addr] = REG_DWORD_U[src];
}

static void mov_gpr32_m32(enum r32 dst, u32 addr)
{
    REG_DWORD_U[dst] = *(u32*)&MEM_BYTE_U[addr];
}

static void mov_xmm_imm64(enum r64 reg, u64 imm64)
{
    REG_QWORD_U[reg] = imm64;
}

static void mov_xmm(enum r64 dst, enum r64 src)
{
    REG_QWORD_U[dst] = REG_QWORD_U[src];
}

static void mov_m64_imm64(u32 addr, u64 imm64)
{
    *(u64*)&MEM_BYTE_U[addr] = imm64;
}

static void mov_m64_xmm(u32 addr, enum r64 src)
{
    *(u64*)&MEM_BYTE_U[addr] = REG_QWORD_U[src];
}

static void mov_xmm_m64(enum r64 dst, u32 addr)
{
    REG_QWORD_U[dst] = *(u64*)&MEM_BYTE_U[addr];
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
    i16 offs;
    u16 imm16;
    u32 imm32, addr;
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
            mov_gpr8(r0, r1);
            goto next;

        case WORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            mov_gpr16(r0, r1);
            goto next;

        case DWORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            mov_gpr32(r0, r1);
            goto next;

        case QWORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            mov_xmm(r0, r1);
            goto next;

        default:
            exceptionf("invalid dest operand size in MOV instruction: %d", operand_size);
        }

    case MOV_RM:
        tmp = fetch_u8();
        r0 = decode_operand(tmp);
        switch (operand_size = decode_operand_size(tmp)) {
        case BYTE:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r1] + sizeof(u8) * offs;
            mov_gpr8_m8(r0, addr);
            goto next;

        case WORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r1] + sizeof(u16) * offs;
            mov_gpr16_m16(r0, addr);
            goto next;

        case DWORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r1] + sizeof(u32) * offs;
            mov_gpr32_m32(r0, addr);
            goto next;

        case QWORD:
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r1] + sizeof(u64) * offs;
            mov_xmm_m64(r0, addr);
            goto next;

        default:
            exceptionf("invalid dest operand size in MOV instruction: %d", operand_size);
        }

    case MOV_MI:
        tmp = fetch_u8();
        r0 = decode_operand(tmp);
        switch (operand_size = decode_operand_size(tmp)) {
        case BYTE:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u8) * offs;
            imm8 = fetch_u8();
            mov_m8_imm8(addr, imm8);
            goto next;

        case WORD:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u16) * offs;
            imm16 = fetch_u16();
            mov_m16_imm16(addr, r0);
            goto next;

        case DWORD:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u32) * offs;
            imm32 = fetch_u32();
            mov_m32_imm32(addr, imm32);
            goto next;

        case QWORD:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u64) * offs;
            imm64 = fetch_u64();
            mov_m64_imm64(addr, imm64);
            goto next;

        default:
            exceptionf("invalid immediate operand size in MOV instruction: %d", operand_size);
        }

    case MOV_MR:
        tmp = fetch_u8();
        r0 = decode_operand(tmp);
        switch (operand_size = decode_operand_size(tmp)) {
        case BYTE:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u8) * offs;
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            mov_m8_gpr8(addr, r1);
            goto next;

        case WORD:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u16) * offs;
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            mov_m16_gpr16(addr, r1);
            goto next;

        case DWORD:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u32) * offs;
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            mov_m32_gpr32(addr, r1);
            goto next;

        case QWORD:
            offs = (i16)fetch_u16();
            addr = REG_DWORD_U[r0] + sizeof(u64) * offs;
            tmp = fetch_u8();
            r1 = decode_operand(tmp);
            mov_m64_xmm(addr, r1);
            goto next;

        default:
            exceptionf("invalid immediate operand size in MOV instruction: %d", operand_size);
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
