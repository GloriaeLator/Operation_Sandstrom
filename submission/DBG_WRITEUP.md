# OPERATION SANDSTORM

## Intro

This is a write-up for the PS Operation Sandstorm, this covers the part of the debugger

The debugger program was written with C, and contained multiple files, namely `io.c io.h state.h engine.c main.c`

The Write-up is divided into the following sections :

1. **Phase II**
2. **Instruction**
3. **Memory**
4. **IO**
5. **Fault handling**
6. **Example**
7. **Final Thoughts**

## 1. Phase II

The Task was basically to build a debugger for this ISA, which is interactive, and independent of the `vm` binary. This can be accomplished by recreating the VM environment itself inside the dbg, making it work exactly like the VM, a emulator with breakpoints and debugging capabilities. 

The debugger first needs to process the opcodes and execute them, for that we need a data structure to store all the properties of the VM, for which a struct is defined containing all the essential information necessary. 

```
typedef struct{         
    uint32_t R[8];         
    uint16_t pc;           
    uint16_t sp;
    uint8_t memory[65536];
    uint8_t breakpoint_map[65536];
    uint16_t file_size;         
    uint8_t zf;
    uint8_t sf;
} state_;
```

The specifications of the VM mentioned in the documentation of PS, was used with minimal modifications to implement the above struct. Next is the instruction parsing and execution. 

## 2. Instruction

The Instruction was obtained was extracting the next 4 four bytes from pc, then pc is incremented by 4. (this alone later caused some issues). the instruction obtained is split as 
```
ii aa bb cc ---> single instruction (32 bits)

ii --> opcode
aa --> second byte
bb --> third byte
cc --> forth byte
ccbb ---> imm16 value.
```
Then based on the opcode matched in the switch-case statement, corresponding exec function is called for example:  for `00`, `ret` function is executed. The default switch case comes out to be `fault` acc to the docs of the PS.

Almost all functions are easy to implement, while the default case and `0D` (exit) case require special handling, so they return different exit_code, and further processing is done in the caller function. 

But there is some error that occurs in jump statements, like `jmp jmp_neq jmp_eq call ret`. if pc is incremented before opcode parsing, then the pc is ahead of the current instruction, that is being executed, this is fine but slightly inconvienent, so it is better to post-increment the pc, there was some error, like the pc being incremented even after the changing address, this can be prevented by using return statement instead of break. 

```
int jmp_eq(state_ *state, uint16_t addr)
{

    if (addr >= state->file_size) 
    { 
        fault(state, FAULT_PC_OUT_OF_BOUNDS); 
        return 0; 
    }
    if (addr % 4 != 0)
    { 
        fault(state, FAULT_UNALIGNED_ACCESS); 
        return 0; 
    }
    if (state->zf != 0)
    {    
        state->pc = addr;
        return 1;
    }
    return 0;
}

...
case 0x02: 
    if (jmp_eq(state, imm16))
        return 1;
    break;
...

```

## 3. Memory

Memory management did cause some great issues and set-backs to the speed of completion, this was mainly due the fact I assumed all the memory is written in stack, though it is unsafe to write in memory directly where instructions are present, but the VM has it implemented. 

The `state->file_size` was introduced so that instead of a hard-coded address where the stack starts, the stack here starts after the file is allocated, this is all fit in the same memory. Though making it slightly different VM which might have had a sepatate stack, since the `mem_read` beyond file_size return 0 as output; this case has been handled separately.

## 4. IO

Another part of the debugger is the IO, part this part is basically the heart of the VM, it contains two classes of functions, one is the parse function, which reads input and calls the handler functions accordingly. the handler functions parse the command further and from the information that we get they start the program, the handler functions include `handle_run` `handle_mem` `handle_regs` `handle_step` `handle_cont` `handle_break` `handle_unbreak`

The `run`, `cont`, and `step` have similar implementation of calling the execute functio which gets the instructions and calls the operand functions. 

The `mem` function prints the memory from the start byte to end byte, this happens by reading the memory of the state struct. Similarly the `regs` prints the registers. 

the `load` requires the reading the file specifies though, this part of the code was copied from online sources and modified, as I was not well-versed with this topic. Do note that the loading the file occurs only after all the errors check are done. 

The `quit` command just return into the caller function with a specifically different exit code. 

The `break` and `unbreak` was implemented using a hash map, as a array would be slow on lookup, there isn't much space constraints (improvement: use array of bitfield instead of uint8_t to save space)

## 5. Fault Handling

Halfway through, I realized it is really tedious to maintain proper and standard error messages to prompt to the user, so  I implemented the fault function  

```
void fault(state_ *state, fault_t type);
const char *fault_string(fault_t type);
```
These were especially usefull, to make the fault handling easier. 

## 6. Examples

There is still might be some minor edge cases that might be left out; I tried to make it error free as possible, This debugger was utilized and tested in the given examples and few test.bin s and the outputs were same as the ./vm

```
./dbg probe_01.bin
initializing debugger
loaded 32 bytes (8 instructions)
debugger> mem 0 32

0000: 03 00 05 00 
0004: 03 02 00 00 
0008: 10 00 00 00 
000C: 03 01 01 00 
0010: 01 00 01 00 
0014: 06 00 02 00 
0018: 0E 00 08 00 
001C: 0D 00 00 00 
debugger> break 10
breakpoint set at 0x0010
debugger> run
5
hit breakpoint at 0x0010
debugger> unbreak 10
breakpoint removed from 0x0010
debugger> regs
R0: 00000005
R1: 00000001
R2: 00000000
R3: 00000000
R4: 00000000
R5: 00000000
R6: 00000000
R7: 00000000
PC: 0010
SP: 0020
ZF: 0
SF: 0
debugger> step
debugger> regs
R0: 00000004
R1: 00000001
R2: 00000000
R3: 00000000
R4: 00000000
R5: 00000000
R6: 00000000
R7: 00000000
PC: 0014
SP: 0020
ZF: 0
SF: 1
debugger> break 10
breakpoint set at 0x0010
debugger> cont 2
4
3
hit breakpoint at 0x0010
```
