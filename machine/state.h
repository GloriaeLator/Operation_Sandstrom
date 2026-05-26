#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <string.h>

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

typedef enum
{
    FAULT_INVALID_OPCODE,
    FAULT_INVALID_REGISTER,
    FAULT_PC_OUT_OF_BOUNDS,
    FAULT_STACK_OVERFLOW,
    FAULT_STACK_UNDERFLOW,
    FAULT_MEMORY_OUT_OF_BOUNDS,
    FAULT_UNALIGNED_ACCESS,
    FAULT_BREAKPOINT_VIOLATION

} fault_t;

void init_state(state_ *state);
int exec(state_ *state);
void fault(state_ *state, fault_t type);
const char *fault_string(fault_t type);

int ret(state_ *state);
void sub(state_ *state, uint8_t x, uint8_t y);
int jmp_eq(state_ *state, uint16_t addr);
void mov_imm16(state_ *state, uint8_t reg_no, uint16_t value);
void mem_read(state_ *state,  uint8_t aa, uint8_t bb);
uint32_t pop(state_ *state);
void cmp(state_ *state, uint8_t x, uint8_t y);
void mov(state_ *state, uint8_t aa, uint8_t bb);
void push(state_ *state, uint32_t value);
int jmp(state_ *state, uint16_t addr);
void mem_write(state_ *state,  uint8_t aa, uint8_t bb);
void xor(state_ *state, uint8_t aa, uint8_t bb);
int call(state_ *state, uint16_t addr);
int jmp_neq(state_ *state, uint16_t addr);
void add(state_ *state, uint8_t x, uint8_t y);
void print(state_ *state, uint8_t reg_no);

#endif 