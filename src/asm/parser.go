package asm

import (
	"fmt"
	"path/filepath"
	"strings"
)

type Symbol struct {
	File   *File
	Line   int
	Col    int
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
		p.errorf("redefinition of exported symbol '%s'. Previous definition was in '%s' on line %d, column %d.", sym.Name, orig.File.ShortPath(), orig.Line, orig.Col)
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

func (p *FileParser) errorf(fmtStr string, args ...interface{}) {
	file := filepath.Base(p.File.Path)
	pfx := fmt.Sprintf("%s ~ line %d, column %d\n  => ", file, p.Token.Line, p.Token.Col)
	fmt.Printf("error: "+pfx+fmtStr+"\n", args...)
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

var codeOf = map[string]uint32{
	"nop":        0x00,
	"int":        0x01,
	"breakpoint": 0x02,
	"lb":         0x04,
	"lbu":        0x05,
	"lh":         0x06,
	"lhu":        0x07,
	"lui":        0x08,
	"lw":         0x09,
	"lwu":        0x0a,
	"ld":         0x0b,
	"la":         0xff, // la corner case
	"sb":         0x0c,
	"sh":         0x0d,
	"sw":         0x0e,
	"sd":         0x0f,
	"mov":        0x2b, // mov = add
	"mfhi":       0x11,
	"mthi":       0x12,
	"mflo":       0x13,
	"mtlo":       0x14,
	"slt":        0x15,
	"sltu":       0x16,
	"slti":       0x17,
	"sltiu":      0x18,
	"eq":         0x19,
	"equ":        0x19, // equ = eq
	"eqi":        0x1a,
	"eqiu":       0x1b,
	"or":         0x20,
	"ori":        0x21,
	"and":        0x22,
	"andi":       0x23,
	"xor":        0x24,
	"xori":       0x25,
	"nor":        0x26,
	"not":        0x26, // not = nor
	"shl":        0x27,
	"shli":       0x28,
	"shr":        0x29,
	"shri":       0x2a,
	"add":        0x2b,
	"addu":       0x2b, // addu = add
	"addi":       0x2c,
	"addiu":      0x2d,
	"sub":        0x2e,
	"subu":       0x2f,
	"mul":        0x30,
	"mulu":       0x31,
	"div":        0x32,
	"divu":       0x33,
	"pushw":      0x36,
	"push":       0x37,
	"popw":       0x38,
	"pop":        0x39,
	"call":       0x3a,
	"ret":        0x3b,
	"j":          0x3c,
	"jr":         0x3d,
	"je":         0x3e,
	"jne":        0x3f,
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

		if code <= codeOf["sb"] {
			p.emitR(tok, code, rd, rs1, 0, int(offset))
		} else {
			p.emitR(tok, code, rs1, rd, 0, int(offset))
		}

	case "lui":
		code = codeOf["lui"]
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
			p.emitR(tok, codeOf["mflo"], 0, rs1, 0, 0)
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
			p.emitR(tok, codeOf["mfhi"], 0, rs1, 0, 0)
		} else {
			p.emitR(tok, code, 0, rs1, rs2, 0)
		}

	case "mov":
		p.expect(TokReg)
		rd = p.regVal()
		p.expect(',')
		p.expect(TokReg)
		rs1 = p.regVal()
		p.emitR(tok, codeOf["add"], rd, rs1, 0, 0)

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
		for line := p.Token.Line; p.Token.Line == line; p.forward() {
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
			Line:   p.Token.Line,
			Col:    p.Token.Col,
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
			line, col := p.Token.Line, p.Token.Col
			if p.match(':') {
				p.localSymbol(Symbol{
					File:   p.File,
					Line:   line,
					Col:    col,
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
		p.errorf("unexpected token %s", kindName(p.Token.Kind))
	}
}

func (p *FileParser) emitBase(tok *Token, code uint32) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File:     p.File,
		Where:    tok,
		Compiled: base(code),
		Code:     code,
	})
	p.File.CodeSize++
}

func (p *FileParser) emitR(tok *Token, code, rd, rs1, rs2 uint32, offs int) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File:     p.File,
		Where:    tok,
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
		Where:    tok,
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
		File:  p.File,
		Where: tok,
		Code:  code,
		Src1:  rs1,
		Src2:  rs2,
		Sym:   sym,
	})
	p.File.CodeSize++
}

func (p *FileParser) emitLa(tok *Token, rd uint32, sym string) {
	p.FileSet.Code = append(p.FileSet.Code, Instr{
		File:  p.File,
		Where: tok,
		Code:  codeOf["la"],
		Dest:  rd,
		Sym:   sym,
	})
	p.File.CodeSize += 2
}

func (p *FileParser) realOffset() int {
	delta := 0
	for _, x := range p.FileSet.Code {
		if x.Code == codeOf["la"] {
			delta++
		}
	}
	return 4 * (len(p.FileSet.Code) + delta)
}
