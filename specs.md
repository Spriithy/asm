# Specs

  Register       Name       Desc.              Saved
------------------------------------------------------
  x0             zero       always zero          —
  x1             ra         return address     Caller
  x2             sp         stack pointer      Callee
  x3             gp         global pointer       —
  x4             tp         thread pointer       —
  x5-7           t0-2       temporaries        Caller
  x8             s0/fp      frame pointer      Callee
  x9             s1         saved reg.         Callee
  x10-11         a0-1       f. args/ret        Caller
  x12-17         a2-7       f. args            Caller
  x18-27         s2-11      saved regs.        Callee
  x28-31         t3-6       temporaries        Caller
------------------------------------------------------
  f0-7           ft0-7      FP temporaries     Caller
  f8-9           fs0-1      FP saved regs.     Callee
  f10-11         fa0-1      FP args/ret        Caller
  f12-17         fa2-7      FP args            Caller
  f18-27         fs2-11     FP saved regs.     Callee
  f28-f31        ft8-11     FP temporaries     Caller


  Name        Size (bytes)
----------------------------
  char             1
  short            2
  int              4
  long             4
  long long        8
  void*            4
  float            4
  double           8
  long double     16

NB:
  - char / unsigned short    0 extended in registers
  - signed char / short      sign extended


# Instruction Set

opcode = I & 0x7f
rd     = (I >> 7) & 0x1f

* R-type

                 28 27     22 21   16 15    11 10         3 2     0
 +------------+----+---------+-------+--------+------------+------+
 | funct7          |   rs2   |  rs1  |   rd   |   opcode   | size |
 +------------+----+---------+-------+--------+------------+------+

* I-type

  31                  20 19     15 14  12 11      7 6           0
 +----------------------+---------+------+---------+-------------+
 | imm                  | rs1     |funct3| rd      | opcode      |
 +----------------------+---------+------+---------+-------------+

* S-type

  31        25 24     20 19     15 14  12 11      7 6           0
 +------------+---------+---------+------+---------+-------------+
 | imm        | rs2     | rs1     |funct3| imm     | opcode      |
 +------------+---------+---------+------+---------+-------------+

* U-type

  31                                      11      7 6           0
 +---------------------------------------+---------+-------------+
 | imm                                   | rd      | opcode      |
 +---------------------------------------+---------+-------------+

 Listing
--------------------------------------------------------------------------------

00  —              —
01 int             interupt
02 intr            interupt reg.
03 brkpt           breakpoint
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
10 mov
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
26 not
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
36 
37 
38 push
39 pop
3a 
3b 
3c jal
3d jalr
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