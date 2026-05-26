#include "state.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 00
int  ret(state_ *state)
{
    uint32_t addr = pop(state);

    if (addr >= state->file_size) {
        fault(state, FAULT_PC_OUT_OF_BOUNDS);
        return 0;
    }
    if (addr % 4 != 0)
    { 
        fault(state, FAULT_UNALIGNED_ACCESS); 
        return 0; 
    }

    state->pc = (uint16_t)addr;
    return 1;
}

// 01
void sub(state_ *state, uint8_t x, uint8_t y)
{
    if (x >= 8 || y >= 8) {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }

    state->sf = (state->R[x] < state->R[y]) ? 0 : 1;
    state->R[x] = state->R[x] - state->R[y];

}

// 02
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

// 03
void mov_imm16(state_ *state, uint8_t reg_no, uint16_t value)
{
    if (reg_no >= 8) 
    {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }

    state->R[reg_no] = value;
}

// 04
void mem_read(state_ *state,  uint8_t aa, uint8_t bb)
{
    if (aa >= 8 || bb >= 8) 
    {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }
    uint32_t addr = state->R[bb];
    if(addr >= state->file_size)
    {
        state->R[aa] = 0;
        return;
    }

    if (addr > 65532) {
        fault(state, FAULT_MEMORY_OUT_OF_BOUNDS);
        return;
    }

    state->R[aa] =
          ((uint32_t)state->memory[addr + 0] <<  0)
        | ((uint32_t)state->memory[addr + 1] <<  8)
        | ((uint32_t)state->memory[addr + 2] << 16)
        | ((uint32_t)state->memory[addr + 3] << 24);
} 

// 05
uint32_t pop(state_ *state)
{
    if (state->sp <= state->file_size) {
        fault(state, FAULT_STACK_UNDERFLOW);
        return 0;
    }

    state->sp -= 4;

    return
          ((uint32_t)state->memory[state->sp + 0] <<  0)
        | ((uint32_t)state->memory[state->sp + 1] <<  8)
        | ((uint32_t)state->memory[state->sp + 2] << 16)
        | ((uint32_t)state->memory[state->sp + 3] << 24);
}

// 06
void cmp(state_ *state, uint8_t x, uint8_t y)
{
    if (x >= 8 || y >= 8) {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }

    state->zf = (state->R[x] == state->R[y]) ; 
}

// 07
void mov(state_ *state, uint8_t aa, uint8_t bb)
{
    if (aa >= 8 || bb >= 8) {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }

    state->R[aa] =  state->R[bb];
}

// 08
void push(state_ *state, uint32_t value)
{
    if (state->sp > 65532) {
        fault(state, FAULT_STACK_OVERFLOW);
        return;
    }

    state->memory[state->sp + 0] = (value >>  0) & 0xFF;
    state->memory[state->sp + 1] = (value >>  8) & 0xFF;
    state->memory[state->sp + 2] = (value >> 16) & 0xFF;
    state->memory[state->sp + 3] = (value >> 24) & 0xFF;

    state->sp += 4;
}

// 09
int jmp(state_ *state, uint16_t addr)
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

    state->pc = addr;
    return 1;
}

// 0A
void mem_write(state_ *state,  uint8_t aa, uint8_t bb)
{   
    if (aa >= 8 || bb >= 8) 
    {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }
    uint32_t val = state->R[bb];
    uint32_t addr = state->R[aa];

    if (addr >= 65533) 
    {
        fault(state, FAULT_MEMORY_OUT_OF_BOUNDS); return;
    }

    state->memory[addr+0] = (val>> 0 & 0xFF);
    state->memory[addr+1] = (val>> 8 & 0xFF);
    state->memory[addr+2] = (val>>16 & 0xFF);
    state->memory[addr+3] = (val>>24 & 0xFF);
}


// 0B
void xor(state_ *state, uint8_t aa, uint8_t bb)
{
    if (aa >= 8 || bb >= 8) 
    {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }

    state->R[aa] ^=  state->R[bb];   
}

// 0C
int call(state_ *state, uint16_t addr)
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

    push(state, state->pc+4);
    state->pc = addr;
    return 1;
}

// 0D - normal exit

// 0E
int jmp_neq(state_ *state, uint16_t addr)
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
    if (state->zf == 0)
    {    
        state->pc = addr;
        return 1;
    }
    return 0;
}

// 0F
void add(state_ *state, uint8_t x, uint8_t y)
{
    if (x >= 8 || y >= 8) {
        fault(state, FAULT_INVALID_REGISTER);
        return;
    }

    state->R[x] += state->R[y];
}

// 10
void print(state_ *state, uint8_t reg_no) 
{
    if (reg_no >= 8) 
    { 
        fault(state, FAULT_INVALID_REGISTER); 
        return; 
    }

    printf("%u\n",(unsigned)state->R[reg_no]);
}


void init_state(state_ *state)
{
    memset(state, 0, sizeof(state_));
}

int exec(state_* state)
{
    if (state->pc > 65532)
    {
        fault(state, FAULT_PC_OUT_OF_BOUNDS);
        return 0;
    }

    uint8_t opcode = state->memory[state->pc];
    uint8_t a      = state->memory[state->pc + 1];
    uint8_t b      = state->memory[state->pc + 2];
    uint8_t c      = state->memory[state->pc + 3];
    uint16_t imm16 = b | (c << 8);

    switch(opcode)
    {
        case 0x00: 
            if (ret(state))
                return 1;
            break;
        case 0x01: 
            sub(state, a, b); 
            break;
        case 0x02: 
            if (jmp_eq(state, imm16))
                return 1;
            break;
        case 0x03: 
            mov_imm16(state, a, imm16); 
            break;
        case 0x04: 
            mem_read(state, a, b); 
            break;
        case 0x05: 
            state->R[a] = pop(state); 
            break;
        case 0x06:  
            cmp(state, a, b); 
            break;
        case 0x07: 
            mov(state,a,b); 
            break;
        case 0x08: 
            push(state,state->R[a]); 
            break;
        case 0x09: 
            if (jmp(state, imm16))
                return 1;
            break;
        case 0x0A: 
            mem_write(state, a, b); 
            break;
        case 0x0B: 
            xor(state, a, b); 
            break;
        case 0x0C: 
            if (call(state, imm16))
                return 1;
            break;
        case 0x0D: 
            return 0; 
            break;
        case 0x0E: 
            if (jmp_neq(state, imm16))
                return 1;
            break;
        case 0x0F: 
            add(state, a, b); 
            break;
        case 0x10: 
            print(state, a); 
            break;
        default: return 2;
    }

    state->pc += 4;
    return 1;
}

const char *fault_string(fault_t type)
{
    switch (type)
    {
        case FAULT_INVALID_OPCODE:
            return "invalid opcode";

        case FAULT_INVALID_REGISTER:
            return "invalid register";

        case FAULT_PC_OUT_OF_BOUNDS:
            return "pc out of bounds";

        case FAULT_STACK_OVERFLOW:
            return "stack overflow";

        case FAULT_STACK_UNDERFLOW:
            return "stack underflow";

        case FAULT_MEMORY_OUT_OF_BOUNDS:
            return "memory out of bounds";

        case FAULT_UNALIGNED_ACCESS:
            return "unaligned memory access";

        case FAULT_BREAKPOINT_VIOLATION:
            return "breakpoint violation";

        default:
            return "unknown fault";
    }
}

void fault(state_ *state, fault_t type)
{
    printf("\n");

    printf("FAULT: %s\n", fault_string(type));

    printf("PC: 0x%04X\n", state->pc);
    printf("SP: 0x%04X\n", state->sp);

    if (state->pc <= 65532)
    {
        printf(
            "INST: %02X %02X %02X %02X\n",
            state->memory[state->pc + 0],
            state->memory[state->pc + 1],
            state->memory[state->pc + 2],
            state->memory[state->pc + 3]
        );
    }

    printf("\n");

    for (int i = 0; i < 8; i++)
    {
        printf("R%d: %08X\n", i, state->R[i]);
    }

    printf("\n");

    printf("ZF: %u\n", state->zf);
    printf("SF: %u\n", state->sf);
}
