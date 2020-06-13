#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND "python files/scripts/test.py 2>&1"

void main()
{

    FILE *script;
    char script_buffer[256];

    char command[200] = COMMAND;

    printf("%s\n\n\n", command);

    script = popen(COMMAND, "r");

    if (script == NULL)
    {
        printf("c mamo\n");
    }

    fread(script_buffer, 1, 512, script);

    printf("%s\n", script_buffer);

    //system ("python files/scripts/test.py arg1 arg2 > script_result");
}