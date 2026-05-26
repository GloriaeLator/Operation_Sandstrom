# OPERATION SANDSTORM

## Challenge 1

The given main_corrupted.bin, one output differs from expected output; To find a one byte change to fix this. 

The pattern that is printed is same a_n = 7 + 3n

but the pattern is broken where `91 350 97` instead of `91 94 97`

Since this is repetive algorthim, there shouldn't be a error in the program logic itself, Also observe that the `350 - 94 = 256 = 0x0100`, which seems like there has been some error in getting information from somewhere. 

```
./dbg main*u*
initializing debugger
loaded 908 bytes (227 instructions)
debugger> break 0x44
breakpoint set at 0x0044
debugger> cont 29
7
10
13
...
88
91
hit breakpoint at 0x0044
debugger> unbreak 44
debugger> step
debugger> regs
R0: 0000001D
R1: 0000005E
R2: 00000010
R3: 000001FC
R4: 0000023C
R5: 0000006C
R6: 00000064
R7: 00000040
PC: 0048
SP: 038C
ZF: 0
SF: 0

0x0048: 04 04 04 00 

0x023C: 5E 01 00 00 
```
NOTE: 0x00 00 01 5E = 350 and 0x00 00 00 5E = 94

HENCE, the one byte to change is byte 0x023D from 01 to 00
