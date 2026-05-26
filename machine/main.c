#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include "io.h"

int main(int argc, char **argv)
{
    state_ state;
    
    init_state(&state);
    printf("initializing debugger\n");

    if (argc >= 2)
    {
        handle_load(&state, argv[1]);
    }

    while (1)
    {
        if (!parse(&state))
            break;
    }

    return 0;
}