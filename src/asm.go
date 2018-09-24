package main

import (
	"github.com/Spriithy/emu-64/src/asm"
)

func main() {
	asm.Compile("samples/main.asm", "a.out")
}
