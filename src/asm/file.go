package asm

import (
	"fmt"
	"os"
	"path/filepath"
)

type FileSet struct {
	Files   map[string]*File
	Globals map[string]Symbol
	CodeGen *CodeGen
	Code    []Instr
}

type File struct {
	Path     string
	Locals   map[string]Symbol
	CodeSize int
}

func (f *File) ShortPath() string {
	return filepath.Base(f.Path)
}

func (f *File) Errorf(pos Pos, msg string, args ...interface{}) {
	fmt.Printf("%s#%d:%d: \x1b[31merror\x1b[0m :: ", f.ShortPath(), pos.Line, pos.Col)
	fmt.Printf(msg, args...)
	fmt.Printf("\n")
}

func (f *File) Warningf(pos Pos, msg string, args ...interface{}) {
	fmt.Printf("%s#%d:%d: \x1b[33mwarning\x1b[0m :: ", f.ShortPath(), pos.Line, pos.Col)
	fmt.Printf(msg, args...)
	fmt.Printf("\n")
}

func Compile(filePath, outFile string) *FileSet {
	fs := &FileSet{
		Files:   make(map[string]*File),
		Globals: make(map[string]Symbol),
		CodeGen: &CodeGen{
			OutputFile: outFile,
		},
	}
	fs.CodeGen.FileSet = fs
	fs.Include(filePath)
	fs.CodeGen.EmitAll()
	return fs
}

func (fs *FileSet) IsAlreadyIncluded(filePath string) bool {
	_, ok := fs.Files[filePath]
	return ok
}

func (fs *FileSet) Include(filePath string) {
	absPath, err := filepath.Abs(filePath)
	if err != nil {
		fmt.Println("error: " + err.Error())
		os.Exit(-1)
	}

	if !fs.IsAlreadyIncluded(absPath) {
		fs.Files[absPath] = ParseFile(fs, absPath)
		fs.CodeGen.FileOrder = append(fs.CodeGen.FileOrder, absPath)
	}
}

func ParseFile(fs *FileSet, filePath string) *File {
	file := &File{
		Path:   filePath,
		Locals: make(map[string]Symbol),
	}
	return (&FileParser{
		FileSet: fs,
		File:    file,
		Scanner: Scan(file),
	}).Parse()
}
