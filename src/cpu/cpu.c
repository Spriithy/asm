#include "cpu.h"
#include "opcode.h"
#include <stdio.h>

#define REG_BYTE rw_byte(cpu.gpr)
#define REG_BYTE_U rw_byte_u(cpu.gpr)
#define REG_WORD rw_word(cpu.gpr)
#define REG_WORD_U rw_word_u(cpu.gpr)
#define REG_DWORD rw_dword(cpu.gpr)
#define REG_DWORD_U cpu.gpr
#define REG_QWORD rw_qword(cpu.xmm)
#define REG_QWORD_U cpu.xmm

#define REG_INDIRECT_BYTE_U(r, o) MEM_BYTE_U[REG_DWORD_U[(r)] + (o)]
#define REG_INDIRECT_WORD_U(r, o) MEM_WORD_U[REG_DWORD_U[(r)] + (o)]
#define REG_INDIRECT_DWORD_U(r, o) MEM_DWORD_U[REG_DWORD_U[(r)] + (o)]
#define REG_INDIRECT_QWORD_U(r, o) MEM_QWORD_U[REG_DWORD_U[(r)] + (o)]

#define MEM_BYTE rw_byte(cpu.data)
#define MEM_BYTE_U rw_byte_u(cpu.data)
#define MEM_WORD rw_word(cpu.data)
#define MEM_WORD_U rw_word_u(cpu.data)
#define MEM_DWORD rw_dword(cpu.data)
#define MEM_DWORD_U rw_dword_u(cpu.data)
#define MEM_QWORD rw_qword(cpu.data)
#define MEM_QWORD_U rw_qword_u(cpu.data)

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

static void check_write(dword_u virt_addr)
{
    byte_u* phys_addr = &cpu.data[virt_addr];
    if (phys_addr <= cpu.rosector || virt_addr >= MEMORY_SIZE) {
        exceptionf("write-protected sector (addr=%p)", virt_addr);
        return;
    }
}

static void check_read(dword_u virt_addr)
{
    byte_u* phys_addr = &cpu.data[virt_addr];
    if (phys_addr < cpu.data || virt_addr >= MEMORY_SIZE) {
        exceptionf("read-protected sector (addr=%p)", virt_addr);
        return;
    }
}

static byte fetch_byte()
{
    byte* eip = (byte*)&REG_DWORD_U[EIP];
    check_read(*eip);
    return MEM_BYTE_U[*eip++];
}

static word fetch_word()
{
    word* eip = (word*)&REG_DWORD_U[EIP];
    check_read(*eip);
    return MEM_WORD_U[*eip++];
}

static dword fetch_dword()
{
    dword* eip = (dword*)&REG_DWORD_U[EIP];
    check_read(*eip);
    return MEM_DWORD_U[*eip++];
}

static qword fetch_qword()
{
    qword* eip = (qword*)&REG_DWORD_U[EIP];
    check_read(*eip);
    return MEM_QWORD_U[*eip++];
}

static void interupt(int icode)
{
    printf("     int 0x%02d\n", icode);
}

void exec()
{
    // Decode utils
    int   op, reg1, reg2, offs, addr;
    byte  imm8;
    word  imm16;
    dword imm32;
    qword imm64;

    // Registers
    qword_u* xmm0;
    dword_u *esp, *eax;
    word_u*  ax;
    byte_u * al, *ah;

    xmm0 = &REG_QWORD_U[XMM0];
    esp = &REG_DWORD_U[ESP];
    eax = &REG_DWORD_U[EAX];
    ax = &REG_WORD_U[AX];
    al = &REG_BYTE_U[AL];
    ah = &REG_BYTE_U[AH];

    switch (op = fetch_byte()) {

    case INTERUPT:
        imm8 = fetch_byte();
        interupt(imm8);
        break;

    case INTERUPT_R:
        reg1 = fetch_byte();
        imm8 = REG_BYTE[reg1];
        interupt(imm8);
        break;

    case BREAKPOINT:
        break;

    case SPECIALISED:

        ////////////////////////////////////////////////////////////////////////
        //
        // Specialised
        //

        switch (op = fetch_byte()) {
        case MOV_AL_RM:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_read(addr);
            *al = MEM_BYTE_U[addr];
            break;

        case MOV_RM_AL:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_write(addr);
            MEM_BYTE_U[addr] = *al;
            break;

        case MOV_AX_RM:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_read(addr);
            *ax = MEM_WORD_U[addr];
            break;

        case MOV_RM_AX:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_write(addr);
            MEM_WORD_U[addr] = *ax;
            break;

        case MOV_EAX_RM:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_read(addr);
            *eax = MEM_DWORD_U[addr];
            break;

        case MOV_RM_EAX:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_write(addr);
            MEM_DWORD_U[addr] = *eax;
            break;

        case MOV_XMM0_RM:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_read(addr);
            *xmm0 = MEM_QWORD_U[addr];
            break;

        case MOV_RM_XMM0:
            reg1 = fetch_byte();
            offs = fetch_word();
            addr = REG_DWORD_U[reg1] + offs;
            check_write(addr);
            MEM_QWORD_U[addr] = *xmm0;
            break;
        }
        break;

        //
        // End Specialised
        //
        ////////////////////////////////////////////////////////////////////////

    case MOVB_R_I:
        reg1 = fetch_byte();
        imm8 = fetch_byte();
        REG_BYTE[reg1] = imm8;
        break;

    case MOVB_R_R:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_BYTE[reg1] = REG_BYTE[reg2];
        break;

    case MOVB_R_RM:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        offs = fetch_word();
        addr = REG_DWORD_U[reg2] + offs;
        check_read(addr);
        REG_BYTE[reg1] = MEM_BYTE_U[addr];
        break;

    case MOVB_RM_I:
        reg1 = fetch_byte();
        offs = fetch_word();
        imm8 = fetch_byte();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_BYTE_U[addr] = imm8;
        break;

    case MOVB_RM_R:
        reg1 = fetch_byte();
        offs = fetch_word();
        reg2 = fetch_byte();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_BYTE_U[addr] = REG_BYTE[reg2];
        break;

    case MOV_R_I:
        reg1 = fetch_byte();
        imm16 = fetch_word();
        REG_WORD[reg1] = imm16;
        break;

    case MOV_R_R:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_WORD[reg1] = REG_WORD[reg2];
        break;

    case MOV_R_RB:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_WORD[reg1] = REG_BYTE[reg2];
        break;

    case MOV_R_RBU:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_WORD_U[reg1] = REG_BYTE_U[reg2];
        break;

    case MOV_R_RM:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        offs = fetch_word();
        addr = REG_DWORD_U[reg2] + offs;
        check_read(addr);
        REG_WORD[reg1] = MEM_WORD_U[addr];
        break;

    case MOV_RM_I:
        reg1 = fetch_byte();
        offs = fetch_word();
        imm16 = fetch_word();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_WORD_U[addr] = imm16;
        break;

    case MOV_RM_R:
        reg1 = fetch_byte();
        offs = fetch_word();
        reg2 = fetch_byte();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_WORD_U[addr] = REG_WORD[reg2];
        break;

    case MOVL_R_I:
        reg1 = fetch_byte();
        imm32 = fetch_dword();
        REG_DWORD[reg1] = imm32;
        break;

    case MOVL_R_R:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_DWORD[reg1] = REG_DWORD[reg2];
        break;

    case MOVL_R_RW:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_DWORD[reg1] = (dword)REG_WORD[reg2];
        break;

    case MOVL_R_RWU:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_DWORD_U[reg1] = REG_WORD_U[reg2];
        break;

    case MOVL_R_RM:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        offs = fetch_word();
        addr = REG_DWORD_U[reg2] + offs;
        check_read(addr);
        REG_DWORD[reg1] = MEM_DWORD_U[addr];
        break;

    case MOVL_RM_I:
        reg1 = fetch_byte();
        offs = fetch_word();
        imm32 = fetch_dword();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_DWORD[addr] = imm32;
        break;

    case MOVL_RM_R:
        reg1 = fetch_byte();
        offs = fetch_word();
        reg2 = fetch_byte();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_DWORD_U[addr] = REG_DWORD[reg2];
        break;

    case MOVQ_R_I:
        reg1 = fetch_byte();
        imm64 = fetch_qword();
        REG_QWORD_U[reg1] = imm64;
        break;

    case MOVQ_R_R:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_QWORD_U[reg1] = REG_QWORD_U[reg2];
        break;

    case MOVQ_R_RL:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_QWORD[reg1] = (qword)REG_DWORD[reg2];
        break;

    case MOVQ_R_RLU:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        REG_QWORD_U[reg1] = REG_DWORD_U[reg2];
        break;

    case MOVQ_R_RM:
        reg1 = fetch_byte();
        reg2 = fetch_byte();
        offs = fetch_word();
        addr = REG_DWORD_U[reg2] + offs;
        check_read(addr);
        REG_QWORD_U[reg1] = MEM_QWORD_U[addr];
        break;

    case MOVQ_RM_I:
        reg1 = fetch_byte();
        offs = fetch_word();
        imm64 = fetch_dword();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_QWORD_U[addr] = imm64;
        break;

    case MOVQ_RM_R:
        reg1 = fetch_byte();
        offs = fetch_word();
        reg2 = fetch_byte();
        addr = REG_DWORD_U[reg1] + offs;
        check_write(addr);
        MEM_QWORD_U[addr] = REG_QWORD[reg2];
        break;

    case PUSH:
        reg1 = fetch_byte();
        check_write(*esp);
        MEM_DWORD_U[*esp] = REG_WORD_U[reg1];
        *esp += sizeof(word);
        break;

    case PUSHL:
        reg1 = fetch_byte();
        check_write(*esp);
        MEM_DWORD_U[*esp] = REG_DWORD_U[reg1];
        *esp += sizeof(dword);
        break;

    case PUSHQ:
        reg1 = fetch_byte();
        check_write(*esp);
        MEM_QWORD_U[*esp] = REG_QWORD_U[reg1];
        *esp += sizeof(qword);
        break;

    case POP:
        reg1 = fetch_byte();
        check_read(*esp);
        *esp -= sizeof(word);
        REG_WORD_U[reg1] = MEM_WORD_U[--*esp];
        break;

    case POPL:
        reg1 = fetch_byte();
        check_read(*esp);
        *esp -= sizeof(dword);
        REG_DWORD_U[reg1] = MEM_DWORD_U[*esp];
        break;

    case POPQ:
        reg1 = fetch_byte();
        check_read(*esp);
        *esp -= sizeof(qword);
        REG_QWORD_U[reg1] = MEM_QWORD_U[*esp];
        break;

    case CMPB_R_R:
        reg1 = fetch_byte();
        reg2 = fetch_byte();

        break;
    }
}
