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
