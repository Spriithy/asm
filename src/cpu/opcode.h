#ifndef OPCODE_H_
#define OPCODE_H_

////////////////////////////////////////////////////////////////////////////////
//
//   Specifications
//
//
//   I. Atomic values
//
//   +--------+------+--------+
//   |  name  | size | symbol |
//   +--------+------+--------+
//   |   byte |   1  |    b   |
//   |   word |   2  |    w   |
//   |  dword |   4  |    l   |
//   +--------+------+--------+
//
//
//   II. Adressing modes
//
//   R = Register
//   M = Memory
//   I = Immediate
//   O = Offset    (1 signed word)
//   A = Adress
//   t = Atomic value specifier
//
//
//   III. Registers
//
//   +-----------+-----------+------------+------------+
//   | Byte Reg. | Word Reg. | DWord Reg. | QWord Reg. |
//   +-----------+-----------+------------+------------+
//   |     AL    |     AX    |    EAX     |    XMM0    |
//   |     AH    |     CX    |    ECX     |    XMM1    |
//   |     CL    |     DX    |    EDX     |    XMM2    |
//   |     CH    |     BX    |    EBX     |    XMM3    |
//   |     DL    |     SP    |    ESP     |    XMM4    |
//   |     DH    |     BP    |    EBP     |    XMM5    |
//   |     BL    |     SI    |    ESI     |    XMM6    |
//   |     BH    |     DI    |    EDI     |    XMM7    |
//   |           |     IP    |    EIP     |            |
//   |           |   FLAGS   |    R9      |            |
//   |           |           |    R10     |            |
//   |           |           |    R11     |            |
//   |           |           |    R12     |            |
//   |           |           |    R13     |            |
//   |           |           |    R14     |            |
//   |           |           |    R15     |            |
//   |           |           |   EFLAGS   |            |
//   +-----------+-----------+------------+------------+
//
//
//
//   0                   1                   2                   3                   4
//   0         1         2         3         4         5         6         7         8
//   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16
//   +----+----+---------+----+----+---------+----+----+---------+----+----+---------+
//   | AL | AH |         | CL | CH |         | DL | DH |         | BL | BH |         |
//   +----+----+---------+----+----+---------+----+----+---------+----+----+---------+
//   |    AX   |         |    CX   |         |    DX   |         |    BX   |         |
//   +---------+---------+---------+---------+---------+---------+---------+---------+
//   |        EAX        |        ECX        |        EDX        |        EBX        |
//   +-------------------+-------------------+-------------------+-------------------+-------------------+-------------------+
//

enum byte_reg {
    AL = 0,
    AH = 1,
    CL = 4,
    CH = 5,
    DL = 8,
    DH = 9,
    BL = 12,
    BH = 13,
};

enum word_reg {
    AX = 0,
    CX = 2,
    DX = 4,
    BX = 6,
    SP = 8,
    BP = 10,
    SI = 12,
    DI = 14,
    IP = 16,
    FLAGS = 32,
};

enum dword_reg {
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI,
    EIP,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    EFLAGS,
};

enum qword_reg {
    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7,
};

enum {
    //
    // int IB
    //
    // xx ii
    //
    INTERUPT,

    //
    // int R
    //
    // xx r-
    //
    INTERUPT_R,

    //
    // bkp
    //
    // xx
    //
    BREAKPOINT,

    //
    // SPECIALIZED INSTRUCTIONS
    //
    // xx ...
    //
    SPECIALISED,

    //
    // movb RB, IB
    //
    // xx r- ii
    //
    MOVB_R_I,

    //
    // movb RB, RB
    //
    // xx r- r-
    //
    MOVB_R_R,

    //
    // movb RB, [R + O]
    //
    // xx r- r- oo oo
    //
    MOVB_R_RM,

    //
    // movb [R + O], IB
    //
    // xx r- oo oo ii
    //
    MOVB_RM_I,

    //
    // movb [R + O], RB
    //
    // xx r- oo oo r-
    //
    MOVB_RM_R,

    //
    // mov RW, IW
    //
    // xx r- ii ii
    //
    MOV_R_I,

    //
    // mov RW, RW
    //
    // xx r- r-
    //
    MOV_R_R,

    //
    // mov RW, RB
    //
    // xx r- r-
    //
    MOV_R_RB,

    //
    // mov RW, RB
    //
    // xx r- r-
    //
    MOV_R_RBU,

    //
    // mov RW, [R + O]
    //
    // xx r- r- oo oo
    //
    MOV_R_RM,

    //
    // mov [R + O], IW
    //
    // xx r- oo oo ii ii
    //
    MOV_RM_I,

    //
    // mov [R + O], RW
    //
    // xx r- oo oo r-
    //
    MOV_RM_R,

    //
    // movl RL, IL
    //
    // xx r- ii ii ii ii
    //
    MOVL_R_I,

    //
    // movl RL, RL
    //
    // xx r- r-
    //
    MOVL_R_R,

    //
    // movl RL, RW
    //
    // xx r- r-
    //
    MOVL_R_RW,

    //
    // movl RL, RW
    //
    // xx r- r-
    //
    MOVL_R_RWU,

    //
    // movl RL, [R + O]
    //
    // xx r- r- oo oo
    //
    MOVL_R_RM,

    //
    // movl [R + O], IL
    //
    // xx r- oo oo ii ii ii ii
    //
    MOVL_RM_I,

    //
    // movl [R + O], RL
    //
    // xx r- oo oo r-
    //
    MOVL_RM_R,

    //
    // movq RQ, IQ
    //
    // xx r- ii ii ii ii ii ii ii ii
    //
    MOVQ_R_I,

    //
    // movq RQ, RQ
    //
    // xx r- r-
    //
    MOVQ_R_R,

    //
    // movq RQ, RL
    //
    // xx r- r-
    //
    MOVQ_R_RL,

    //
    // movq RQ, RL
    //
    // xx r- r-
    //
    MOVQ_R_RLU,

    //
    // movq RQ, [R + O]
    //
    // xx r- r- oo oo
    //
    MOVQ_R_RM,

    //
    // movq [R + O], IQ
    //
    // xx r- oo oo ii ii ii ii ii ii ii ii
    //
    MOVQ_RM_I,

    //
    // movq [R + O], RQ
    //
    // xx r- oo oo r-
    //
    MOVQ_RM_R,

    //
    // push RW
    //
    // xx r-
    //
    PUSH,

    //
    // pushl RL
    //
    // xx r-
    //
    PUSHL,

    //
    // pushq RQ
    //
    // xx r-
    //
    PUSHQ,

    //
    // pop RW
    //
    // xx r-
    //
    POP,

    //
    // popl RL
    //
    // xx r-
    //
    POPL,

    //
    // popq RQ
    //
    // xx r-
    //
    POPQ,

    //
    // cmpb RB, RB
    //
    // xx r- r-
    //
    CMPB_R_R,

    //
    // cmpb RB, IB
    //
    // xx r- ii
    //
    CMPB_R_IB,

    //
    // cmp RW, RW
    //
    // xx r- r-
    //
    CMP_R_R,

    //
    // cmp RW, IW
    //
    // xx r- ii ii
    //
    CMP_R_I,

    //
    // cmpl RL, RL
    //
    // xx r- r-
    //
    CMPL_R_R,

    //
    // cmpl RL, IL
    //
    // xx r- ii ii ii ii
    //
    CMPL_R_I,

    //
    // cmpq RQ, RQ
    //
    // xx r- r-
    //
    CMPQ_R_R,

    //
    // cmpq RQ, IQ
    //
    // xx r- ii ii ii ii ii ii ii ii
    //
    CMPQ_R_I,

    JMP,
    JO,
    JNO,
    JS,
    JNS,
    JZ,
    JNZ,
    JA,
    JAE,
    JNAE,
    JL,
    JLE,
    JNLE,
    JCXZ,

    CALL_O,
    CALL_O_R,

    CALL_A,
    CALL_A_R,

    RET,
    RET_R,

    INC_RB,
    INC_RMB,
    INC_R,
    INC_RM,
    INC_RL,
    INC_RML,

    DEC_RB,
    DEC_RMB,
    DEC_R,
    DEC_RM,
    DEC_RL,
    DEC_RML,

    ADD_R_I,
    ADD_R_R,
    ADD_R_RM,
    ADD_RM_I,
    ADD_RM_R,
};

////////////////////////////////////////////////////////////////////////////////
//
// SPECIALISED INSTRUCTIONS
//

enum {
    //
    // movb al, [R + O]
    //
    // 03 xx r- oo oo
    //
    MOV_AL_RM,

    //
    // movb [R + O], al
    //
    // 03 xx r- oo oo
    //
    MOV_RM_AL,

    //
    // mov ax, [R + O]
    //
    // 03 xx r- oo oo
    //
    MOV_AX_RM,

    //
    // mov [R + O], ax
    //
    // 03 xx r- oo oo
    //
    MOV_RM_AX,

    //
    // movl eax, [R + O]
    //
    // 03 xx r- oo oo
    //
    MOV_EAX_RM,

    //
    // movl [R + O], eax
    //
    // 03 xx r- oo oo
    //
    MOV_RM_EAX,

    //
    // movq xmm0, [R + O]
    //
    // 03 xx r- oo oo
    //
    MOV_XMM0_RM,

    //
    // movq [R + O], rax
    //
    // 03 xx r- oo oo
    //
    MOV_RM_XMM0,

    PUSH_AX,
    POP_AX,

    PUSH_EAX,
    POP_EAX,

    PUSH_RAX,
    POP_RAX,

    PUSH_EFLAGS,
    POP_EFLAGS,

    CMP_AL_R,
    CMP_AL_I,
    CMP_AX_R,
    CMP_AX_I,
    CMP_EAX_R,
    CMP_EAX_I,
    CMP_RAX_R,
    CMP_RAX_I,

};

#endif // OPCODE_H_
