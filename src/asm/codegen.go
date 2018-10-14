package asm

import (
	"fmt"
)

type Instr struct {
	File     *File
	Pos      Pos
	Code     uint32
	Dest     uint32
	Src1     uint32
	Src2     uint32
	Imm      int
	Sym      string
	Compiled uint32
}

type CodeGen struct {
	FileSet    *FileSet
	FileOrder  []string
	OutputFile string
	Offset     int
}

func (cg *CodeGen) errorf(msg string, args ...interface{}) {
	instr := cg.fetchInstr()
	instr.File.Errorf(instr.Pos, msg, args...)
}

func (cg *CodeGen) EmitAll() {
	for _, fileName := range cg.FileOrder {
		cg.Emit(fileName)
	}
}

func (cg *CodeGen) Emit(fileName string) {
	fileEnd := cg.Offset + cg.FileSet.Files[fileName].CodeSize
	for ; cg.Offset < fileEnd; cg.Offset++ {
		switch instr := cg.fetchInstr(); instr.Code {
		case CALL:
			proc, ok := cg.fetchProcedure(instr.Sym, fileName)
			if !ok {
				cg.errorf("undefined procedure reference to '%s'.", instr.Sym)
				cg.EmitInstr(NOP)
				continue
			}
			cg.EmitInstr(i24(instr.Code, proc.Offset))

		case J, JE, JNE:
			label, ok := cg.fetchLabel(instr.Sym, fileName)
			if !ok {
				cg.errorf("undefined label reference to '%s'.", instr.Sym)
				cg.EmitInstr(NOP)
				continue
			}

			if instr.Code == J {
				cg.EmitInstr(i24(instr.Code, label.Offset))
			} else {
				cg.EmitInstr(i16(instr.Code, instr.Src1, instr.Src2, label.Offset))
			}

		case LA:
			sym, ok := cg.fetchSymbol(instr.Sym, fileName)

			if !ok {
				cg.errorf("undefined symbol reference to '%s'.", instr.Sym)
				cg.EmitInstr(NOP)
				continue
			}

			if sym.Offset>>16 != 0 {
				cg.EmitInstr(i16(LUI, regVal["%at"], 0, sym.Offset>>16))
				cg.EmitInstr(i16(ORI, instr.Src1, regVal["%at"], sym.Offset&0xffff))
			} else {
				cg.EmitInstr(NOP)
				cg.EmitInstr(i16(ORI, instr.Src1, 0, sym.Offset&0xffff))
			}

		default:
			cg.EmitInstr(instr.Compiled)
		}
	}
}

func (cg *CodeGen) EmitInstr(instr uint32) {
	fmt.Printf("0x%08x\n", instr)
}

func (cg *CodeGen) fetchProcedure(name, file string) (sym Symbol, ok bool) {
	if sym, ok = cg.FileSet.Globals[name]; ok {
	} else if sym, ok = cg.FileSet.Files[file].Locals[name]; ok {
	}

	if !sym.IsProcedure() {
		cg.errorf("undefined procedure '%s'.", name)
		ok = false
		return
	}

	return
}

func (cg *CodeGen) fetchLabel(name, file string) (sym Symbol, ok bool) {
	if sym, ok = cg.FileSet.Files[file].Locals[name]; ok {
		if !sym.IsLabel() {
			ok = false
		}
		return
	}
	return
}

func (cg *CodeGen) fetchSymbol(name, file string) (sym Symbol, ok bool) {
	if sym, ok = cg.fetchLabel(name, file); ok {
		return
	}

	if sym, ok = cg.fetchProcedure(name, file); ok {
		return
	}

	ok = false
	return
}

func (cg *CodeGen) fetchInstr() Instr {
	return cg.FileSet.Code[cg.Offset]
}

func base(code uint32) uint32 {
	return code & 0x3f
}

func r(code, rd, rs1, rs2 uint32, off int) uint32 {
	return base(code) | ((rd & 0x1f) << 6) | ((rs1 & 0x1f) << 11) | ((rs2 & 0x1f) << 16) | (uint32(off) << 21)
}

func i16(code, rd, rs1 uint32, imm int) uint32 {

	return base(code) | ((rd & 0x1f) << 6) | ((rs1 & 0x1f) << 11) | ((uint32(imm) & 0xffff) << 16)
}

func i24(code uint32, imm int) uint32 {
	return base(code) | (uint32(imm) << 8)
}
