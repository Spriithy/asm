package asm

const (
	NOP = iota
	INT
	BREAKPOINT
	LB
	LBU
	LH
	LHU
	LUI
	LW
	LWU
	LD
	SB
	SH
	SW
	SD
	MFHI = 0x11 + iota
	MTHI
	MFLO
	MTLO
	SLT
	SLTU
	SLTI
	SLTIU
	EQ
	EQI
	EQIU
	OR = 0x20 + iota
	ORI
	AND
	ANDI
	XOR
	XORI
	NOR
	SHL
	SHLI
	SHR
	SHRI
	ADD
	ADDI
	ADDIU
	SUB
	SUBU
	MUL
	MULU
	DIV
	DIVU
	PUSHW = 0x36 + iota
	PUSH
	POPW
	POP
	CALL
	RET
	J
	JR
	JE
	JNE

	LA   = 0xff
	MOV  = ADD
	EQU  = EQ
	NOT  = NOR
	ADDU = ADD
)

var codeOf = map[string]uint32{
	"nop":        NOP,
	"int":        INT,
	"breakpoint": BREAKPOINT,
	"lb":         LB,
	"lbu":        LBU,
	"lh":         LH,
	"lhu":        LHU,
	"lui":        LUI,
	"lw":         LW,
	"lwu":        LWU,
	"ld":         LD,
	"la":         LA,
	"sb":         SB,
	"sh":         SH,
	"sw":         SW,
	"sd":         SD,
	"mov":        MOV,
	"mfhi":       MFHI,
	"mthi":       MTHI,
	"mflo":       MFLO,
	"mtlo":       MTLO,
	"slt":        SLT,
	"sltu":       SLTU,
	"slti":       SLTI,
	"sltiu":      SLTIU,
	"eq":         EQ,
	"equ":        EQU,
	"eqi":        EQI,
	"eqiu":       EQIU,
	"or":         OR,
	"ori":        ORI,
	"and":        AND,
	"andi":       ANDI,
	"xor":        XOR,
	"xori":       XORI,
	"nor":        NOR,
	"not":        NOT,
	"shl":        SHL,
	"shli":       SHLI,
	"shr":        SHR,
	"shri":       SHRI,
	"add":        ADD,
	"addu":       ADDU,
	"addi":       ADDI,
	"addiu":      ADDIU,
	"sub":        SUB,
	"subu":       SUBU,
	"mul":        MUL,
	"mulu":       MULU,
	"div":        DIV,
	"divu":       DIVU,
	"pushw":      PUSHW,
	"push":       PUSH,
	"popw":       POPW,
	"pop":        POP,
	"call":       CALL,
	"ret":        RET,
	"j":          J,
	"jr":         JR,
	"je":         JE,
	"jne":        JNE,
}
