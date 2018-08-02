putc:   mov  %a1, %a0
        addi %a0, %zero, 0x05
        int
        ret

__print_as_int:
        addi %a0, %a0, 48         ; '0'
        call putc
        ret

print:  mov  %s0, %a0 ; n
        slt  %t0, %a0, %zero      ; n < zero ?
        je   %t0, %zero, print.L1
        addi %a0, %zero, 45
        call putc                 ; putc('-')
        nor  %s0, %s0, %zero
        addi %s0, %s0, 1          ; n = -n
print.L1:
        jne  %s0, %zero, print.L2
        addi %a0, %zero, 48       ; if (n == zero)
        call putc ; putc('zero')
print.L2:
        addi %t0, %zero, 10
        div  %s1, %s0, %t0        ; s1 = n / 10
        mfhi %s2                  ; n%10

        je   %s1, %zero, print.L3
        mov  %a0, %s1
        call print                ; print(n / 10)
print.L3:
        mov  %a0, %s2
        call __print_as_int       ; putc(n%10+'zero')
        ret

fact:   mov  %s0, %a0             ; factorial function
        jne  %s0, %zero, fact.L1
        addi %v0, %zero, 1
        ret
fact.L1:
        addi %a0, %s0, -1
        call fact
        mul  %s0, %v0
        mflo %v0
        ret

main:
        addi %s0, %zero, 10

        mov  %a0, %s0
        call print

        addi %a0, %zero, 0x0a
        call putc

        mov  %a0, %s0
        call fact

        mov  %a0, %v0
        call print
        addi %a0, %zero, 0x0a    ; \n
        call putc

        addi %v0, %zero, 0
        ret
