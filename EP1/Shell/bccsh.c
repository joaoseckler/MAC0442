#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "aux.h"
#include "builtin.h"

int main()
{
    char *command, *arg, *user, cwd[MAX_CWD_LENGTH], prompt[MAX_PROMPT_LENGTH];
    char* ptrv[MAX_WORDS];
    int wstatus, err;
    pid_t child;

    struct sigaction act1, act2;
    act1.sa_handler = handler;
    act2.sa_handler = handler_parent;
    act1.sa_flags = act2.sa_flags = 0;
    sigemptyset(&(act1.sa_mask));
    sigemptyset(&(act2.sa_mask));
    sigaction(SIGINT, &act1, NULL);

    using_history();

    user = getlogin();
    getcwd(cwd, MAX_CWD_LENGTH);
    snprintf(prompt, MAX_PROMPT_LENGTH, "{%s@%s} ", user, cwd);

    while ((command = readline(prompt))) {
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
        } else if (!strncmp(command, "cd", 2)) {
            arg = &(command[3]);
            cd(arg);
        }

        else {
            split(command, ptrv);
            if ((child = fork())) {
                sigaction(SIGINT, &act2, NULL);
                wait(&wstatus);
                sigaction(SIGINT, &act1, NULL);
            } else {
                if ((err = execvp(command, ptrv)) == -1) {
                    printf("Não foi possível executar o comando\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        user = getlogin();
        getcwd(cwd, MAX_CWD_LENGTH);
        snprintf(prompt, MAX_PROMPT_LENGTH, "{%s@%s} ", user, cwd);
        free(command);
    }
    printf("\n");

    return EXIT_SUCCESS;
}
