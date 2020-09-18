#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "builtin.h"

#define MAX_WORDS 30

void split(char *, char **);

int main()
{
    char* command;
    char* arg;
    char* ptrv[MAX_WORDS];
    int wstatus, err;
    pid_t child;

    using_history();

    while ((command = readline(">> "))) {
        add_history(command);

        if (!strncmp(command, "mkdir", 5)) {
            arg = &(command[6]);
            make_dir(arg);
        } else if (!strncmp(command, "kill -9", 7)) {
            arg = &(command[8]);
            kill_9(atoi(arg));
        } else if (!strncmp(command, "ln -s", 5)) {
            arg = &(command[6]);
            ln_s(arg);
        }

        else {
            split(command, ptrv);
            if ((child = fork())) {
                wait(&wstatus);
            } else {
                if ((err = execvp(command, ptrv)) == -1) {
                    printf("Não foi possível executar o comando\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        free(command);
    }
    printf("\n");

    return EXIT_SUCCESS;
}


void split(char* string, char ** ptrv)
{
    int k = 0, flag = 1;
    for (int i = 0; string[i] != '\0' && k < MAX_WORDS; i++) {
        if (string[i] == ' ') {
            string[i] = '\0';
            flag = 1;
        }
        else if (flag) {
            ptrv[k] = string + i;
            flag = 0;
            k++;
        }
    }
    ptrv[k] = NULL;
}
