putc:   int 0x05
        ret

__print_as_int:
        addi %a0, %a0, '0'
        call putc
        ret

print:  mov  %s0, %a0
        slt  %t0, %a0, %zero
        je   %t0, %zero, print.L1
        addi %a0, %zero, '-'
        call putc
        addi %t0, %zero, 1
        sub  %t0, %zero, %t0
        mul  %s0, %t0
        mflo %s0
print.L1:
        jne  %s0, %zero, print.L2
        addi %a0, %zero, '0'
        call putc
print.L2:
        addi %t0, %zero, 10
        div  %s0, %t0
        mfhi %s2
        mflo %s1
        je   %s1, %zero, print.L3
        mov  %a0, %s1
        call print
print.L3:
        mov  %a0, %s2
        call __print_as_int
        ret
