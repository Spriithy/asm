.include "io.asm"
.include "fact.asm"

        .text
.proc main
        addi %s0, %zero, 9
        mov  %a0, %s0
        call print
        addi %a0, %zero, 0x0a
        call putc
        mov  %a0, %s0
        call fact
        mov  %a0, %v0
        call print
        addi %a0, %zero, '\n'
        call putc
        addi %v0, %zero, 0
        ret
