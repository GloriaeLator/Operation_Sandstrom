#ifndef IO_H
#define  IO_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"

#define MAX_LINE 256

void handle_load(state_ *state,const char *arg);
void handle_run(state_ *state);
void handle_step(state_ *state);
void handle_cont(state_ *state, int skip_count);
void handle_break(state_ *state,uint16_t addr);
void handle_unbreak(state_ *state,uint16_t addr);
void handle_regs(state_ *state);
void handle_mem(state_ *state, uint16_t start, uint16_t len);

int parse(state_ *state);
 
#endif