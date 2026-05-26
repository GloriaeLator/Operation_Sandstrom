# OPERATION SANDSTROME

## Intro
In the PS, I was given ELF-x86_64 binary, *.bin files in the examples and challenge folders. The Aim was to reverse the VM, ISA, and implement a debugger to debug our code in the specific ISA.

This writeup is for the Phase I part of finding the ISA. It is divided into following sections: 

1. **Problem Statement**
2. **probe_01**
3. **probe_02**
4. **probe_03 04 00**
5. **05 07 08 0B**
6. **Final list**


## 1. Problem Statement 

It was given that the VM takes instructions in 4 bytes and word size was also 4 bytes, and was implemented with SF and ZF in addition to R0-R7. 

Given this information, the intial thought process was to start to read the *.bin files but as it was all-binary code, it could be read in normal text-editors like vim or nano. 

As the PS mentioned to use `hexdump`, for the initial few reading and editing, I used `hexdump`, and reversed it with `xxd -r file_name`, but this method wasn't as efficient and was really lengthly to implement even a single change. 

So, I searched up for other alternatives, and found out about the HEX EDITOR extension of VSCode, this was used from that point, adjusting to display 4 bytes every column. 

From observing the outputs of the VM, for all the given example .bin files, and the number of instructions that were present, I concluded that, it would be better not to start with the probe_00. Instead chose to begin with the probe_01

Instructions are referred in this format `ii aa bb cc`

## 2. probe_01

Not only probe_01, but with all the binaries given, the last line of the program was with the instruction `0D 00 00 00` , I thought it must be the exit code, but wasn't sure if the `aa`, `bb`, `cc` part carried any weight to it. On the first observation, it felt `0D 02 00 00` should be program halt and should error, but when I changed the `aa` from `00` to `02`, nothing happened, I tried this with `bb` and `cc` values, none of the change seemed to affect the exit of the program, so I concluded that `0D` is ***exit*** instruction and doesn't take any agruements. 

The first instruction of Probe_01 was `03 00 05 00`, and it printed 5 to 4 in newlines. So I suspected it must do something to do with the 5 numbers it started printing with. so I changed `bb` from `05` to `06`, and it seemed to print from 6 to 1
So, with this evidence, I concluded that `03` might be concerned with data storage for loop. 

When `bb cc` was set to `00 01`, it printed from 256 to 1, so it stores value `0xccbb`, when the value of `aa` was changed to something other than `00` like `07`, it started the loop from a very large number but remained same for `06` and `07`, it seemed `aa` was also a input term from the instruction. So I concluded this must be the ***mov_imm16*** instruction, with agruements register_no `aa`, and imm16 value `0xccbb` as to be assigned. 

The very next instruction in the list was the `10` instruction, applying similar methods as above, by changing the value of `aa` it seems to print different reg values, while trailing `bb cc` values are taken as input

The `01` instruction was easily found as `sub` operation as changing the reg 01 changed how much the reg 00 decreased each time in loop 
 
Similarly, the `06` instruction was concluded as `cmp` instruction after a lot of trail and error, with printing each reg value after the command, then it was found the loop executed when the `aa` reg value and `bb` reg value were not equal, this subsequently gave a hint on the action of `0E` instruction which, jumped to `0xccbb` address in the memory when called, and `aa` value didn't matter. so `0E` was `jmp_neq` instruction. 


## 3. probe_02

This single bin took more time to infer than every other part of solution combined. Initially, on first look it seemed that the `0A` and `04` instructions had to do something with the memory, but could identify how they function, so I decided to write a custom binary with the instructions known so far. This part is where I went wrong, initially I allocated the reg 01 the value 01, then allocated reg 02 the value 02, when I accesses the memory it seemed that when I called the function it went to the memory address 02. 

So I concluded that this is some-sort of modified mem_write / mem_read that allows you to write to addresses in the stack,  i thought `bb` is a address in stack, though it wasnt able to address all the `0xFFFF` bytes, it could do upto `0xFF`, I assumed the stack access is limited, I even made the dbg implemented with a index_pointer to mark the start of the stack, and started read/write from there. (then went on to find other instructions, believing its true)

Later I found out the debugger logic is somewhat wrong, because when I input the value of `bb` to be greater than `09` it faulted with error stack underflow, I'm not sure why this happened but it got me confused, I tried some standard cases to test my various hypothesis, everything assumed writing to stack. Then I figured out that it was doing something similar to bitshifting, with `bb` value, like `reg[aa] << (8*bb)` I believed in this because my test cases were 01 02 03 for regs 01 02 03. 
then when I tried this for reg 01 and reg 02 it worked but at reg 03, it gave a value thrice of expected. I was confused, none of the hypothesis showed any correlaction, it was all random. then I identified that its similar to `reg[aa] << (8*reg[bb])`, then too the stack assumption was present. At `0x54`, I wrote a reduntent line `ff ff ff ff` after the exit. the assigned the reg 01 = 0x54, then did mem-read, Eureka moment, I found out the error in my assumption, it writes to raw memory replacing instructions in the memory, itself. now everything was justified. later it was implemented in memory byte array, instead of bitshifts mental model. 

## 4. probe_03 04 00

Initially I assumed the `0C` instruction was a normal arithematic instruction, but after the `0C` instruction it executed code beyond the exit code, so it must be something like `call`. This assumption was confirmed based on the trail and error.Folowing this `00` was found out to be the `ret` instruction, as removing it caused error of PC, out bounds, 

Then the value of reg 02 was also changed after the call. the intial guess itself was `add` the values incremented and checking the reg values, and testing on custom .bin, it was clear that it was `add` two registers instruction. 

Here there was more print functions in the instructions than the actual output, it seemed like skiping few instructins, when i deleted few reduntent print functions, the values printed differed, and also rearranging instructions causes output change, so `02` wasn't normal skip instruction, address dependent, also there was a cmp, before its call, so on further trail, it was concluded as `jmp_eq` instruction. 

I tried similar while finding the `jmp_eq` instruction, it was jumping to addr `0x10` and skipping the first exit and terminating on the second exit. it was simple as i already knew, the method to find the value of instruction

The print function, was especially usefull as it was helping me read the changes in the registers, time to time. 

## 5. 05 07 08 0B

I'll attach a snippet of the custom .bin file which was really useful for finding the instructions not given, as well as for testing the given ones

```
00000000: 0900 3400  ..4.  initial jump to actual code
00000004: 0300 0000  .... |
00000008: 0301 0000  .... |
0000000c: 0302 0000  .... |-----> intial assignment 
00000010: 0303 0000  .... |
00000014: 0304 ffff  .... |
00000018: 0000 0000  .... 
0000001c: 1000 0000  .... |
00000020: 1001 0000  ..X. |
00000024: 1002 0000  .... |-----> print the regs
00000028: 1003 0000  .... |
0000002c: 1004 0000  .... |
00000030: 0000 0000  ....
00000034: 0c00 0400  ....  call to assign
00000038: iiaa bbcc  .... | 
0000003c: iiaa bbcc  .... |----> instructions to be tested
00000040: iiaa bbcc  .... |
00000044: 0c00 1c00  ....  call to print
00000048: iiaa bbcc  ..T. |----> instructions to be tested
0000004c: 0c00 1c00  ....  call to print
00000050: 0d00 0000  ....  exit
00000054: ffff ffff  ....
```

Initially I tried `05` as it was first, but what ever I did it was, giving an stack underflow, so after many attemps of trying, no progress was made.

I decided to change to `07`, it was relatively simple, especially using this premade template, I saw registers values moving from one to another, so it became `mov` instruction. 

Similar to `05` nothing changed after running the command, no errors even, so I was suspicious of it, being memory related. but memory is not getting correpted after trying after some time, so this time, I thought it might genuinely something related to the stack and separate from the instructions, so i tried a couple of instructions with `05` and `08` in different combinations, and found out that, `08` is push to stack from reg[aa]. and `05` is pop the stack value to reg[aa]. I also noticed that when I push to stack inside a `call` instruction, the stack gets corrupted, so there was no need to implement a checkpoint/fault mechanism for this in the debugger.

I anticipated a xor operation, in the list, `0B` was the last unknown instruction, so I was expecting xor operation, I check for `01^02` , `02^03` `03^01` and `FFFF^01`, it all matched as expected, so this was concluded as `xor`. 

## 6. Final list 

The final list of the instructions and their opcode is

```
00 ret 
01 sub
02 jmp_eq
03 mov_imm16
04 mem_read
05 pop
06 cmp
07 mov
08 push
09 jmp
0A mem_write
0B xor
0C call
0D exit
0E jmp_neq
0F add
10 print
```
The convention of SF and ZF register were taken are :
```
SF = ( a < b ) ? 1 : 0 ; // when a - b is operated
ZF = ( a == b ) ; // when cmp(a, b) is operated
```