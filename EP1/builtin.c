#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "builtin.h"

void make_dir(const char* name)
{
    // Creates directory "name" with mode 775
    mkdir(name, 0775);
}

void kill_9(unsigned int pid)
{
    // Sends a signal 9 to a process pid
    kill(pid, 9);
}

void ln_s(char* command)
{
    // Creates a soft link named "link" from "file"
    char* token;
    char* file;
    char* link;

    token = strtok(command, " ");
    file = token;

    while (token != NULL) {
        link = token;
        token = strtok(NULL, " ");
    }

    symlink(file, link);
}

void cd(char* path)
{
    chdir(path);
}
