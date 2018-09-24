/*
 *   IO utility procedures
 */

    .text
.proc putc
        mov  %a1, %a0
        addi %a0, %zero, 0x05
        int
        ret

.proc print
        mov  %s0, %a0             // n
        slt  %t0, %a0, %zero      // n < zero ?
        je   %t0, %zero, print.L1
        addi %a0, %zero, '-'
        call putc                 
        nor  %s0, %s0, %zero
        addi %s0, %s0, 1          // n = -n
    print.L1:
        jne  %s0, %zero, print.L2
        addi %a0, %zero, '0'      // if (n == zero)
        call putc                 // putc('0')
        ret
    print.L2:
        addi %t0, %zero, 10
        div  %s1, %s0, %t0        // s1 = n / 10
        mfhi %s2                  // n%10
        je   %s1, %zero, print.L3
        mov  %a0, %s1
        call print                // print(n / 10)
    print.L3:
        mov  %a0, %s2
        call print_as_int       // putc(n%10+'zero')
        ret

.proc .static print_as_int
        addi %a0, %a0, '0'
        call putc
        ret
