package asm

import (
	"fmt"
	"path/filepath"
	"strings"
)

type Symbol struct {
	File   *File
	Pos    Pos
	Name   string
	Kind   int
	Offset int
}

func (s Symbol) String() string {
	file := filepath.Base(s.File.Path)
	return fmt.Sprintf("{src: '%s', name: '%s', offs: %d}", file, s.Name, s.Offset)
}

func (s Symbol) IsProcedure() bool {
	return s.Kind&SymProcedure != 0
}

func (s Symbol) IsLabel() bool {
	return s.Kind&SymLabel != 0
}

func (s Symbol) IsStatic() bool {
	return s.Kind&SymStatic != 0
}

const (
	SymProcedure = 1
	SymLabel     = 2
	SymStatic    = 4
)

type FileParser struct {
	FileSet   *FileSet
	File      *File
	Scanner   *Scanner
	Token     *Token
	Lookahead *Token
}

func (p *FileParser) localSymbol(sym Symbol) {
	if _, ok := p.File.Locals[sym.Name]; ok {
		p.errorf("local symbol redefinition of '%s'.", sym.Name)
		return
	}
	p.File.Locals[sym.Name] = sym
}

func (p *FileParser) exportedSymbol(sym Symbol) {
	p.localSymbol(sym)
	if orig, ok := p.FileSet.Globals[sym.Name]; ok {
		p.errorf("redefinition of exported symbol '%s'. Previous definition was in '%s' on line %d, column %d.", sym.Name, orig.File.ShortPath(), orig.Pos.Line, orig.Pos.Col)
	}
	p.FileSet.Globals[sym.Name] = sym
}

func (p *FileParser) Parse() *File {
	p.forward() // Lookahead
noAdvance:
	if p.match(TokDirective) {
		switch p.text() {
		case ".include":
			if p.match(TokString) {
				dir := filepath.Dir(p.File.Path) + string(filepath.Separator)
				file := strings.Trim(p.strVal(), "\"")
				p.FileSet.Include(dir + file)
				goto noAdvance
			} else {
				p.errorf("expected include file path.")
			}
			p.Parse()

		case ".notext":
			return p.File

		case ".text":
			p.parseTextSegment()

		default:
			p.errorf("unexpected directive %s.", p.text())
			return p.Parse()
		}
	} else {
		p.errorf("expected top-level directive, got %s.", kindName(p.Lookahead.Kind))
		return p.Parse()
	}

	return p.File
}

func (p *FileParser) errorf(msg string, args ...interface{}) {
	p.File.Errorf(p.Token.Pos, msg, args...)
}

func (p *FileParser) forward() {
	p.Scanner.Tokenize()
	p.Token = p.Lookahead
	p.Lookahead = p.Scanner.Token
}

func (p *FileParser) match(kind int) bool {
	if p.Lookahead.Kind == kind {
		p.forward()
		return true
	}
	return false
}

func (p *FileParser) expect(kind int) bool {
	if !p.match(kind) {
		p.errorf("expected '%s', got '%s'", kindName(kind), kindName(p.Token.Kind))
		return false
	}
	return true
}

func (p *FileParser) regVal() uint32 {
	return uint32(p.intVal())
}

func (p *FileParser) intVal() uint64 {
	return p.Token.IntVal
}

func (p *FileParser) strVal() string {
	return p.Token.StrVal
}

func (p *FileParser) text() string {
	return strings.Trim(p.Token.Text, " \t\n\r")
}

func (p *FileParser) instr(name string) {
	var code, rd, rs1, rs2 uint32
	var offset, imm int64

	switch tok := p.Token; name {
	case "nop":
	case "int", "breakpoint", "ret":
		code = codeOf[name]
		p.emitBase(tok, code)

	case "lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "sb", "sh", "sw", "sd":
		code = codeOf[name]
		p.expect(TokReg)
		rd = p.regVal()
		p.expect(',')
		offset = 1
		if p.match('-') {
			offset = -1
		}
		p.expect(TokInt)
		offset *= int64(p.intVal())
		p.expect('(')
		p.expect(TokReg)
		rs1 = p.regVal()
		p.expect(')')

		if code < SB {
			p.emitR(tok, code, rd, rs1, 0, int(offset))
		} else {
			p.emitR(tok, code, rs1, rd, 0, int(offset))
		}

	case "lui":
		code = LUI
		p.expect(TokReg)
		rd = p.regVal()
		p.expect(',')
		imm = 1
		if p.match('-') {
			imm = -1
		}
		p.expect(TokInt)
		imm *= int64(p.intVal())
		p.emitI(tok, code, rd, 0, int(imm))

	case "la":
		p.expect(TokReg)
		rd = p.regVal()
		p.expect(',')
		p.expect(TokName)
		p.emitLa(tok, rd, p.text())

	case "li":
		// TODO

	case "mfhi", "mflo", "popw", "pop":
		code = codeOf[name]
		p.expect(TokReg)
		rd = p.regVal()
		p.emitR(tok, code, rd, 0, 0, 0)

	case "mthi", "mtlo", "pushw", "push", "jr":
		code = codeOf[name]
		p.expect(TokReg)
		rs1 = p.regVal()
		p.emitR(tok, code, 0, rs1, 0, 0)

	case "or", "and", "xor", "nor", "shl", "shr", "add", "addu", "sub", "subu",
		"eq", "equ", "slt", "sltu":
		code = codeOf[name]
		p.expect(TokReg)
		rd = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs1 = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs2 = p.regVal()
		p.emitR(tok, code, rd, rs1, rs2, 0)

	case "mul", "mulu", "div", "divu":
		code = codeOf[name]
		p.expect(TokReg)
		rs1 = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs2 = p.regVal()
		if p.match(',') {
			p.expect(TokReg)
			rd = p.regVal()
			p.emitR(tok, code, 0, rs2, rd, 0)
			p.emitR(tok, MFLO, 0, rs1, 0, 0)
		} else {
			p.emitR(tok, code, 0, rs1, rs2, 0)
		}

	case "mod", "modu":
		code = codeOf[name]
		p.expect(TokReg)
		rs1 = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs2 = p.regVal()
		if p.match(',') {
			p.expect(TokReg)
			rd = p.regVal()
			p.emitR(tok, code, 0, rs2, rd, 0)
			p.emitR(tok, MFHI, 0, rs1, 0, 0)
		} else {
			p.emitR(tok, code, 0, rs1, rs2, 0)
		}

	case "mov":
		p.expect(TokReg)
		rd = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs1 = p.regVal()
		p.emitR(tok, ADD, rd, rs1, 0, 0)

	case "slti", "sltiu", "eqi", "eqiu", "ori", "andi", "xori", "shli", "shri",
		"addi", "addiu":
		code = codeOf[name]
		p.expect(TokReg)
		rd = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs1 = p.regVal()
		p.expect(',')
		imm = 1
		if p.match('-') {
			imm = -1
		}
		p.expect(TokInt)
		imm *= int64(p.intVal())
		p.emitI(tok, code, rd, rs1, int(imm&0xffff))

	case "call", "j":
		code = codeOf[name]
		p.expect(TokName)
		p.emitJ(tok, code, 0, 0, p.text())

	case "je", "jne":
		code = codeOf[name]
		p.expect(TokReg)
		rs1 = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs2 = p.regVal()
		p.expect(',')
		p.expect(TokName)
		p.emitJ(tok, code, rs1, rs2, p.text())

	default:
		p.errorf("unknown instruction '%s'", name)
		for line := p.Token.Pos.Line; p.Token.Pos.Line == line; p.forward() {
		}
	}
}

func (p *FileParser) parseTextSegment() {
	for p.match(TokDirective) {
		if p.text() != ".proc" {
			p.errorf("expected procedure directive (.proc), got %s.", p.text())
			continue
		}

		sym := Symbol{
			File:   p.File,
			Pos:    p.Token.Pos,
			Offset: p.realOffset(),
			Kind:   SymProcedure,
		}

		if p.match(TokDirective) {
			switch p.text() {
			case ".static":
				sym.Kind |= SymStatic

			default:
				p.errorf("unexpected directive %s.", p.text())
			}
		}

		p.expect(TokName)
		sym.Name = p.text()

		if sym.Kind&SymStatic > 0 {
			p.localSymbol(sym)
		} else {
			p.exportedSymbol(sym)
		}

		for p.match(TokName) {
			name := p.text()
			if p.match(':') {
				p.localSymbol(Symbol{
					File:   p.File,
					Pos:    p.Token.Pos,
					Name:   name,
					Offset: p.realOffset(),
					Kind:   SymLabel,
				})
			} else {
				p.instr(name)
			}
		}
	}

	if !p.match(TokEOF) {
		p.errorf("unexpected token %s.", kindName(p.Token.Kind))
	}
}

func (p *FileParser) emitBase(tok *Token, code uint32) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File:     p.File,
		Pos:      tok.Pos,
		Compiled: base(code),
		Code:     code,
	})
	p.File.CodeSize++
}

func (p *FileParser) emitR(tok *Token, code, rd, rs1, rs2 uint32, offs int) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File:     p.File,
		Pos:      tok.Pos,
		Compiled: r(code, rd, rs1, rs2, offs),
		Code:     code,
		Dest:     rd,
		Src1:     rs1,
		Src2:     rs2,
		Imm:      offs,
	})
	p.File.CodeSize++
}

func (p *FileParser) emitI(tok *Token, code, rd, rs1 uint32, imm int) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File:     p.File,
		Pos:      tok.Pos,
		Compiled: i16(code, rd, rs1, imm),
		Code:     code,
		Dest:     rd,
		Src1:     rs1,
		Imm:      imm,
	})
	p.File.CodeSize++
}

func (p *FileParser) emitJ(tok *Token, code, rs1, rs2 uint32, sym string) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File: p.File,
		Pos:  tok.Pos,
		Code: code,
		Src1: rs1,
		Src2: rs2,
		Sym:  sym,
	})
	p.File.CodeSize++
}

func (p *FileParser) emitLa(tok *Token, rd uint32, sym string) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File: p.File,
		Pos:  tok.Pos,
		Code: LA,
		Dest: rd,
		Sym:  sym,
	})
	p.File.CodeSize += 2
}

func (p *FileParser) realOffset() int {
	delta := 0
	for _, x := range p.FileSet.Code {
		if x.Code == LA {
			delta++
		}
	}
	return 4 * (len(p.FileSet.Code) + delta)
}
