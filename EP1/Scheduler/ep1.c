#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler.h"

int main(int argc, char* argv[])
{
    /******* Treat arguments *******************************************/
    int escalonador = atoi(argv[1]);
    char* infile = argv[2];
    char* outfile = argv[3];
    d = 0;

    if (argc >= 5 && strcmp(argv[4], "d") == 0)
        d = 1;
    /*******************************************************************/

    int nprocesses = 0;
    size_t m = 0;
    char* line = NULL;
    FILE* fp = fopen(infile, "r");

    if (fp == NULL) {
        printf("erro ao abrir arquivo %s\n", infile);
        exit(EXIT_FAILURE);
    }

    for (char c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n')
            nprocesses++;

    struct pr* prv = malloc(nprocesses * sizeof(struct pr));
    mutexv = malloc(nprocesses * sizeof(pthread_mutex_t));
    indices = malloc(nprocesses * sizeof(int));
    n_cpu = malloc(nprocesses * sizeof(int));

    rewind(fp);

    int i = 0;
    while ((getline(&line, &m, fp) != -1)) {
        prv[i].name = malloc(30 * sizeof(char));
        sscanf(line, "%s %f %f %f\n", prv[i].name,
            &prv[i].t0, &prv[i].dt, &prv[i].deadline);
        prv[i].t0 *= SECOND;
        prv[i].dt *= SECOND;
        prv[i].deadline *= SECOND;
        prv[i].remaining = prv[i].dt;
        prv[i].id = i;
        prv[i].created = 0;
        pthread_mutex_init(mutexv + i, NULL);
        indices[i] = i;
        i++;
        free(line);
        line = NULL;
    }
    free(line);

    fclose(fp);
    fp = fopen(outfile, "w");
    if (fp == NULL) {
        printf("erro ao abrir arquivo %s\n", outfile);
        exit(EXIT_FAILURE);
    }

    void (*sched[3])(struct pr*, int, FILE*, int) = { fcfs, srtn, rr };
    sched[escalonador - 1](prv, nprocesses, fp, d);

    /****************** Free allocated memory ***************************/

    fclose(fp);
    for (int i = 0; i < nprocesses; i++) {
        free(prv[i].name);
        pthread_mutex_destroy(mutexv + i);
    }
    free(prv);
    free(indices);
    free(mutexv);
    free(n_cpu);
}
