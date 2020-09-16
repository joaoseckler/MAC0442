#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtin.h"

int main()
{
    char* command;
    char* arg;

    using_history();

    while ((command = readline(">> "))) {
        if (!strcmp(command, "/usr/bin/du -hs .")) {
            printf("Executando du...\n");
        } else if (!strcmp(command, "/usr/bin/traceroute www.google.com.br")) {
            printf("Executando traceroute...\n");
        } else if (!strcmp(command, "./ep1 <args>")) {
            printf("Executando ep1...\n");
        } else if (!strncmp(command, "mkdir", 5)) {
            arg = &(command[6]);
            make_dir(arg);
        } else if (!strncmp(command, "kill -9", 7)) {
            arg = &(command[8]);
            kill_9(atoi(arg));
        } else if (!strncmp(command, "ln -s", 5)) {
            arg = &(command[6]);
            ln_s(arg);
        } else {
            printf("Comando desconhecido\n");
        }

        add_history(command);

        free(command);
    }

    printf("\n");

    return EXIT_SUCCESS;
}
