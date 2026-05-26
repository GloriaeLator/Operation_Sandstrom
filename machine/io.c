#include "io.h"
#include <stdio.h>

void handle_step(state_ *state)
{
    if (state->pc >= state->file_size)
    {
        fault(state, FAULT_PC_OUT_OF_BOUNDS);
        return;
    }

    if (state->breakpoint_map[state->pc])
    {
        printf("stopped at breakpoint 0x%04X\n", state->pc);
        return;
    }

    int status = exec(state);

    if (status == 0)
    {
        printf("program halted\n");
    }
    else if (status == 2)
    {
        fault(state,FAULT_INVALID_OPCODE);
    }
}

void handle_cont(state_ *state, int skip_count)
{
    int hit_count = 0;

    while (1)
    {
        if (state->pc >= state->file_size)
        {
            fault(state, FAULT_PC_OUT_OF_BOUNDS);
            return;
        }

        if (state->breakpoint_map[state->pc])
        {
            if (hit_count < skip_count)
            {
                hit_count++;

                int status = exec(state);

                if (status == 0)
                {
                    printf("program halted\n");
                    return;
                }

                if (status == 2)
                {
                    fault(state, FAULT_INVALID_OPCODE);
                    return;
                }

                continue;
            }

            printf("hit breakpoint at 0x%04X\n", state->pc);
            return;
        }

        int status = exec(state);

        if (status == 0)
        {
            printf("program halted\n");
            return;
        }

        if (status == 2)
        {
            fault(state, FAULT_INVALID_OPCODE);
            return;
        }
    }
}

void handle_run(state_ *state)
{
    state->pc = 0;
    handle_cont(state,0);
}

void handle_mem(state_ *state,uint16_t start, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {   
        if (start + i > 65535)
        {   
            printf("cant access memory beyond limit");
            return;
        }

        if (i % 4 == 0)
            printf("\n%04X: ", start + i);

        printf("%02X ", state->memory[start + i]);
    }

    printf("\n");
}

void handle_regs(state_ *state)
{
    for (int i = 0; i < 8; i++)
        printf("R%d: %08X\n", i, state->R[i]);

    printf("PC: %04X\n", state->pc);
    printf("SP: %04X\n", state->sp);

    printf("ZF: %u\n", state->zf);
    printf("SF: %u\n", state->sf);
}

void handle_break(state_ *state, uint16_t addr)
{
    if (addr % 4 != 0)
    {
        printf("breakpoints must be 4-byte aligned\n");
        return;
    }

    if (addr >= state->file_size)
    {
        printf("breakpoint outside program\n");
        return;
    }

    if (state->breakpoint_map[addr])
    {
        printf("breakpoint already exists at 0x%04X\n", addr);
        return;
    }

    state->breakpoint_map[addr] = 1;

    printf("breakpoint set at 0x%04X\n", addr);
}

void handle_unbreak(state_ *state, uint16_t addr)
{
    if (state->breakpoint_map[addr])
    {   
        state->breakpoint_map[addr] = 0;
        printf("breakpoint removed from 0x%04X\n", addr);
        return;
    }

    printf("no breakpoint at 0x%04X\n", addr);
}

void handle_load(state_ *state, const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        printf("failed to open '%s'\n", filename);
        return;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        printf("fseek failed\n");
        fclose(fp);
        return;
    }

    long file_size = ftell(fp);

    if (file_size < 0)
    {
        printf("ftell failed\n");
        fclose(fp);
        return;
    }

    rewind(fp);

    if (file_size == 0)
    {
        printf("empty file\n");
        fclose(fp);
        return;
    }

    if (file_size % 4 != 0)
    {
        printf("file size must be multiple of 4 bytes\n");
        fclose(fp);
        return;
    }

    if (file_size > 65536)
    {
        printf("program too large for RAM\n");
        fclose(fp);
        return;
    }

    // loading file, only if no errors

    init_state(state);
    state->file_size = file_size;
    state->sp = state->file_size;
    size_t bytes_read = fread(state->memory, 1, file_size, fp);
    fclose(fp);

    if (bytes_read != (size_t)file_size)
    {
        printf("failed to read entire file\n");
        return;
    }

    printf("loaded %ld bytes (%ld instructions)\n",
           file_size,
           file_size / 4);
}

int parse(state_ *state)
{
    printf("debugger> ");
    fflush(stdout);
    char line[MAX_LINE];
    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        printf("io error, no input read\n");
        return 0;
    }
    line[strcspn(line, "\n")] = '\0';
    if (strncmp(line, "load ", 5) == 0)
    {
        handle_load(state,line + 5);
    }
    else if (strcmp(line, "run") == 0)
    {
        handle_run(state);
    }
    else if (strcmp(line, "step") == 0)
    {
        handle_step(state);
    }
    else if (strncmp(line, "cont", 4) == 0)
    {
        int skip_count = 0;
    
        if (sscanf(line, "cont %d", &skip_count) != 1)
            skip_count = 0;
    
        handle_cont(state, skip_count);
    }
    else if (strncmp(line, "break ", 6) == 0)
    {
        uint16_t addr;
        if (sscanf(line + 6, "%x", &addr) == 1)
            handle_break(state,addr);
        else
            printf("invalid address\n");
    }
    else if (strncmp(line, "unbreak ", 8) == 0)
    {
        uint16_t addr;
        if (sscanf(line + 8, "%x", &addr) == 1)
            handle_unbreak(state,addr);
        else
            printf("invalid address\n");
    }
    else if (strcmp(line, "regs") == 0)
    {
        handle_regs(state);
    }
    else if (strncmp(line, "mem ", 4) == 0)
    {
        uint16_t start, len;
        if (sscanf(line + 4, "%u %u", &start, &len) == 2)
            handle_mem(state,start, len);
        else
            printf("usage: mem <start> <len>\n");
    }
    else if (strcmp(line, "quit") == 0)
    {
        printf("quiting\n");
        return 0;
    }
    else
    {
        printf("unknown command\n");
    }
}