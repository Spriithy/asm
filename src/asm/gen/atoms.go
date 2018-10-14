package gen

import (
	"fmt"

	"github.com/Spriithy/asm/src/asm"
)

type Register = int

type Instr struct {
	Code      int
	Registers [3]Register
	Immediate int
	Label     Label
	Procedure string
}

type Label struct {
	File      *asm.File
	Procedure *Procedure
	Name      string
	Offset    int
	set       bool
}

type Procedure struct {
	File   *asm.File
	Name   string
	Static bool
	Labels map[string]Label
	Code   []Instr
}

func MakeProcedure(file *asm.File, static bool, name string) *Procedure {
	return &Procedure{
		File:   file,
		Name:   name,
		Static: static,
		Labels: make(map[string]Label),
		Code:   make([]Instr, 0),
	}
}

func (p *Procedure) Close() error {
	for _, instr := range p.Code {
		switch instr.Code {
		case asm.J, asm.JE, asm.JNE:
			if !instr.Label.set {
				return fmt.Errorf("use of unknown label '%s' in procedure '%s'", instr.Label.Name, p.Name)
			}
		}
	}

	return nil
}

func (p *Procedure) Label(name string) Label {
	if label, ok := p.Labels[name]; ok {
		return label
	}

	label := Label{
		File:      p.File,
		Procedure: p,
		Name:      name,
	}
	p.Labels[name] = label
	return label
}

func (l Label) Set() {
	if l.set {
		fmt.Println("label redefinition")
		return
	}
	l.Offset = len(l.Procedure.Code)
}

func (p *Procedure) BaseInstr(code int) {
	p.Code = append(p.Code, Instr{Code: code})
}

func (p *Procedure) RInstr(code int, rd, rs1, rs2 Register, offset int) {
	p.Code = append(p.Code, Instr{
		Code:      code,
		Registers: [3]Register{rd, rs1, rs2},
		Immediate: offset,
	})
}

func (p *Procedure) ImmediateInstr(code int, rd, rs1 Register, immediate int) {
	p.Code = append(p.Code, Instr{
		Code:      code,
		Registers: [3]Register{rd, rs1},
		Immediate: immediate,
	})
}

func (p *Procedure) JInstr(code int, rd, rs1 Register, label Label) {
	p.Code = append(p.Code, Instr{
		Code:      code,
		Registers: [3]Register{rd, rs1},
		Label:     label,
	})
}

func (p *Procedure) CallInstr(procedure string) {
	p.Code = append(p.Code, Instr{
		Code:      asm.CALL,
		Procedure: procedure,
	})
}

func (p *Procedure) Nop() {
	p.BaseInstr(asm.NOP)
}

func (p *Procedure) Interrupt(code int) {
	p.Move(p.A0(), code)
	p.BaseInstr(asm.INT)
}

func (p *Procedure) Breakpoint() {
	p.BaseInstr(asm.BREAKPOINT)
}

func (p *Procedure) LoadByte(into, from Register, offset int) {
	p.RInstr(asm.LB, into, from, p.ZERO(), offset)
}

func (p *Procedure) LoadByteUnsigned(into, from Register, offset int) {
	p.RInstr(asm.LBU, into, from, p.ZERO(), offset)
}

func (p *Procedure) LoadHalfWord(into, from Register, offset int) {
	p.RInstr(asm.LH, into, from, p.ZERO(), offset)
}

func (p *Procedure) LoadHalfWordUnsigned(into, from Register, offset int) {
	p.RInstr(asm.LHU, into, from, p.ZERO(), offset)
}

func (p *Procedure) LoadUpperImmediate(into Register, value int) {
	p.ImmediateInstr(asm.LUI, into, p.ZERO(), value)
}

func (p *Procedure) LoadWord(into, from Register, offset int) {
	p.RInstr(asm.LW, into, from, p.ZERO(), offset)
}

func (p *Procedure) LoadWordUnsigned(into, from Register, offset int) {
	p.RInstr(asm.LWU, into, from, p.ZERO(), offset)
}

func (p *Procedure) LoadDoubleWord(into, from Register, offset int) {
	p.RInstr(asm.LB, into, from, p.ZERO(), offset)
}

func (p *Procedure) StoreByte(to, from Register, offset int) {
	p.RInstr(asm.SB, to, from, p.ZERO(), offset)
}

func (p *Procedure) StoreHalfWord(to, from Register, offset int) {
	p.RInstr(asm.SH, to, from, p.ZERO(), offset)
}

func (p *Procedure) StoreWord(to, from Register, offset int) {
	p.RInstr(asm.SW, to, from, p.ZERO(), offset)
}

func (p *Procedure) StoreDoubleWord(to, from Register, offset int) {
	p.RInstr(asm.SD, to, from, p.ZERO(), offset)
}

func (p *Procedure) Move(into, from Register) {
	p.RInstr(asm.MOV, into, from, p.ZERO(), 0)
}

func (p *Procedure) MoveFromHI(into Register) {
	p.RInstr(asm.MFHI, into, p.ZERO(), p.ZERO(), 0)
}

func (p *Procedure) MoveToHI(from Register) {
	p.RInstr(asm.MTHI, p.ZERO(), from, p.ZERO(), 0)
}

func (p *Procedure) MoveFromLO(into Register) {
	p.RInstr(asm.MFLO, into, p.ZERO(), p.ZERO(), 0)
}

func (p *Procedure) MoveToLO(from Register) {
	p.RInstr(asm.MTLO, p.ZERO(), from, p.ZERO(), 0)
}

func (p *Procedure) SetLessThan(result, left, right Register) {
	p.RInstr(asm.SLT, result, left, right, 0)
}

func (p *Procedure) SetLessThanUnsigned(result, left, right Register) {
	p.RInstr(asm.SLTU, result, left, right, 0)
}

func (p *Procedure) SetLessThanImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.SLTI, result, left, right)
}

func (p *Procedure) SetLessThanImmediateUnsigned(result, left Register, right int) {
	p.ImmediateInstr(asm.SLTIU, result, left, right)
}

func (p *Procedure) Equals(result, left, right Register) {
	p.RInstr(asm.EQ, result, left, right, 0)
}

func (p *Procedure) EqualsUnsigned(result, left, right Register) {
	p.RInstr(asm.EQU, result, left, right, 0)
}

func (p *Procedure) EqualsImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.EQI, result, left, right)
}

func (p *Procedure) EqualsImmediateUnsigned(result, left Register, right int) {
	p.ImmediateInstr(asm.EQIU, result, left, right)
}

func (p *Procedure) Or(result, left, right Register) {
	p.RInstr(asm.OR, result, left, right, 0)
}

func (p *Procedure) OrImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.ORI, result, left, right)
}

func (p *Procedure) And(result, left, right Register) {
	p.RInstr(asm.AND, result, left, right, 0)
}

func (p *Procedure) AndImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.ANDI, result, left, right)
}

func (p *Procedure) Xor(result, left, right Register) {
	p.RInstr(asm.XOR, result, left, right, 0)
}

func (p *Procedure) XorImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.XORI, result, left, right)
}

func (p *Procedure) Nor(result, left, right Register) {
	p.RInstr(asm.NOR, result, left, right, 0)
}

func (p *Procedure) Not(result, source Register) {
	p.Nor(result, source, p.ZERO())
	p.AddImmediate(result, result, 1)
}

func (p *Procedure) ShiftLeft(result, left, right Register) {
	p.RInstr(asm.SHL, result, left, right, 0)
}

func (p *Procedure) ShiftLeftImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.SHLI, result, left, right)
}

func (p *Procedure) ShiftRight(result, left, right Register) {
	p.RInstr(asm.SHR, result, left, right, 0)
}

func (p *Procedure) ShiftRightImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.SHRI, result, left, right)
}

func (p *Procedure) Add(result, left, right Register) {
	p.RInstr(asm.ADD, result, left, right, 0)
}

func (p *Procedure) AddUnsigned(result, left, right Register) {
	p.RInstr(asm.ADDU, result, left, right, 0)
}

func (p *Procedure) AddImmediate(result, left Register, right int) {
	p.ImmediateInstr(asm.ADDI, result, left, right)
}

func (p *Procedure) AddImmediateUnsigned(result, left Register, right int) {
	p.ImmediateInstr(asm.ADDIU, result, left, right)
}

func (p *Procedure) Subtract(result, left, right Register) {
	p.RInstr(asm.SUB, result, left, right, 0)
}

func (p *Procedure) SubtractUnsigned(result, left, right Register) {
	p.RInstr(asm.SUBU, result, left, right, 0)
}

func (p *Procedure) Multiply(result, left, right Register) {
	p.RInstr(asm.MUL, p.ZERO(), left, right, 0)
	if result != p.ZERO() {
		p.MoveFromLO(result)
	}
}

func (p *Procedure) MultiplyUnsigned(result, left, right Register) {
	p.RInstr(asm.MULU, p.ZERO(), left, right, 0)
	if result != p.ZERO() {
		p.MoveFromLO(result)
	}
}

func (p *Procedure) Divide(result, left, right Register) {
	p.RInstr(asm.DIV, p.ZERO(), left, right, 0)
	if result != p.ZERO() {
		p.MoveFromLO(result)
	}
}

func (p *Procedure) DivideUnsigned(result, left, right Register) {
	p.RInstr(asm.DIVU, p.ZERO(), left, right, 0)
	if result != p.ZERO() {
		p.MoveFromLO(result)
	}
}

func (p *Procedure) Modulus(result, left, right Register) {
	p.RInstr(asm.DIV, p.ZERO(), left, right, 0)
	if result != p.ZERO() {
		p.MoveFromHI(result)
	}
}

func (p *Procedure) ModulusUnsigned(result, left, right Register) {
	p.RInstr(asm.DIVU, p.ZERO(), left, right, 0)
	if result != p.ZERO() {
		p.MoveFromHI(result)
	}
}

func (p *Procedure) PushWord(source Register) {
	p.RInstr(asm.PUSHW, p.ZERO(), source, p.ZERO(), 0)
}

func (p *Procedure) Push(source Register) {
	p.RInstr(asm.PUSH, p.ZERO(), source, p.ZERO(), 0)
}

func (p *Procedure) PopWord(into Register) {
	p.RInstr(asm.PUSHW, into, p.ZERO(), p.ZERO(), 0)
}

func (p *Procedure) Pop(into Register) {
	p.RInstr(asm.PUSH, into, p.ZERO(), p.ZERO(), 0)
}

func (p *Procedure) Call(procedure string) {
	p.CallInstr(procedure)
}

func (p *Procedure) Return() {
	p.BaseInstr(asm.RET)
}

func (p *Procedure) Jump(label Label) {
	p.JInstr(asm.J, p.ZERO(), p.ZERO(), label)
}

func (p *Procedure) JumpRegister(source Register) {
	p.RInstr(asm.JR, p.ZERO(), source, p.ZERO(), 0)
}

func (p *Procedure) JumpEquals(left, right Register, label Label) {
	p.JInstr(asm.JE, left, right, label)
}

func (p *Procedure) JumpNotEquals(left, right Register, label Label) {
	p.JInstr(asm.JNE, left, right, label)
}

func (p *Procedure) JumpZero(source Register, label Label) {
	p.JumpEquals(source, p.ZERO(), label)
}

func (p *Procedure) JumpNotZero(source Register, label Label) {
	p.JumpNotEquals(source, p.ZERO(), label)
}

func (p *Procedure) Reg(n int) Register {
	if n < 0 || n >= 32 {
		return 0
	}
	return n
}

func (p *Procedure) ZERO() Register {
	return p.Reg(0)
}

func (p *Procedure) AT0() Register {
	return p.Reg(1)
}

func (p *Procedure) AT1() Register {
	return p.Reg(26)
}

func (p *Procedure) AT2() Register {
	return p.Reg(27)
}

func (p *Procedure) V0() Register {
	return p.Reg(2)
}

func (p *Procedure) V1() Register {
	return p.Reg(3)
}

func (p *Procedure) A0() Register {
	return p.Reg(4)
}

func (p *Procedure) A1() Register {
	return p.Reg(5)
}

func (p *Procedure) A2() Register {
	return p.Reg(6)
}

func (p *Procedure) A3() Register {
	return p.Reg(7)
}

func (p *Procedure) T0() Register {
	return p.Reg(8)
}

func (p *Procedure) T1() Register {
	return p.Reg(9)
}

func (p *Procedure) T2() Register {
	return p.Reg(10)
}

func (p *Procedure) T3() Register {
	return p.Reg(11)
}

func (p *Procedure) T4() Register {
	return p.Reg(12)
}

func (p *Procedure) T5() Register {
	return p.Reg(13)
}

func (p *Procedure) T6() Register {
	return p.Reg(14)
}

func (p *Procedure) T7() Register {
	return p.Reg(15)
}

func (p *Procedure) T8() Register {
	return p.Reg(24)
}

func (p *Procedure) T9() Register {
	return p.Reg(25)
}

func (p *Procedure) S0() Register {
	return p.Reg(16)
}

func (p *Procedure) S1() Register {
	return p.Reg(17)
}

func (p *Procedure) S2() Register {
	return p.Reg(18)
}

func (p *Procedure) S3() Register {
	return p.Reg(19)
}

func (p *Procedure) S4() Register {
	return p.Reg(20)
}

func (p *Procedure) S5() Register {
	return p.Reg(21)
}

func (p *Procedure) S6() Register {
	return p.Reg(22)
}

func (p *Procedure) S7() Register {
	return p.Reg(23)
}

func (p *Procedure) GP() Register {
	return p.Reg(28)
}

func (p *Procedure) SP() Register {
	return p.Reg(29)
}

func (p *Procedure) FP() Register {
	return p.Reg(30)
}

func (p *Procedure) RA() Register {
	return p.Reg(31)
}
