# Specs

  Register       Name       Desc.              Saved
------------------------------------------------------
  r0             zero       always zero          —
  r1             at0        asm-temp             —
  r2-3           v0-1       return values        —
  r4-7           a0-3       call args            —
  r8-15,24,25    t0-9       temps                —
  r16-23         s0-7       saved regs          Yes 
  r26-27         at1-2      asm-temp             —
  r28            gp         global ptr          Yes
  r29            sp         stack ptr           Yes
  r30            fp         frame ptr           Yes
  r31            ra         return addr         Yes

  Name        Size (bytes)
----------------------------
  char             1
  short            2
  int              4
  long             4
  long long        8
  void*            8
  float            4
  double           8
  long double     16

NB:
  - char / unsigned short    0 extended in registers
  - signed char / short      sign extended

  Instructions Listing
--------------------------------------------------------------------------------

00  —              —
01 int             interupt
02 breakpoint
03 
04 lb
05 lbu
06 lh
07 lhu
08 lui
09 lw
0a lwu
0b ld
0c sb
0d sh
0e sw
0f sd
10 mov            (P)
11 mfhi
12 mthi           
13 mflo
14 mtlo   
15 slt
16 sltu
17 slti
18 sltiu
19 eq             $d = ($s == $t)
1a eqi            $d = ($s == #imm)
1b eqiu           
1c 
1d 
1e 
1f 
20 or
21 ori
22 and
23 andi
24 xor   
25 xori
26 not           not x y = nor x y 0   (neg => not x, x add x, x, 1)
27 nor
28 shl
29 shli
2a shr
2b shri
2c add
2d addi
2e addiu
2f sub
30 subu
31 mul
32 mulu
33 div
34 divu
35 
36 pushw
37 push
38 popw
39 pop
3a call
3b ret
3c j
3d jr
3e je
3f jne
40 
41 
42 
43 
44 
45 
46 
47 
48 
49 
4a 
4b 