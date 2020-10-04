#include <readline/readline.h>
#include <stdio.h>

#include "aux.h"

void split(char* string, char** ptrv)
{
    int k = 0, flag = 1;
    for (int i = 0; string[i] != '\0' && k < MAX_WORDS; i++) {
        if (string[i] == ' ') {
            string[i] = '\0';
            flag = 1;
        } else if (flag) {
            ptrv[k] = string + i;
            flag = 0;
            k++;
        }
    }
    ptrv[k] = NULL;
}

void handler()
{
    printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void handler_parent()
{
    printf("\n");
}
