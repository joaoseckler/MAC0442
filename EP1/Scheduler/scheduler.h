#pragma once

#include <pthread.h>
#include <stdio.h>
#include <time.h>

/* one mutex for each process */
extern pthread_mutex_t* mutexv;
/* list of indices of the above array */
extern int* indices;
extern int* n_cpu;
extern struct pr* prv;
extern int d;

#define SECOND .1

struct pr {
    char* name;
    pthread_t thread;
    float t0; // Initial time
    float dt; // Duration of process
    float deadline;
    float remaining; // Time remaining for the process to complete
    int id; // integer identifier for the process
    int created; // Is the thread created already or not
};

void fcfs(struct pr* prv, int n, FILE* fp, int d);
void srtn(struct pr* prv, int n, FILE* fp, int d);
void rr(struct pr* prv, int n, FILE* fp, int d);

void* thread_routine(void* arg);

/* Where a is before b */
void timediff(struct timespec* a, struct timespec* b, struct timespec* result);

/* Enqueue item in queue. Queue is ordered by its items integer field dt
 * Returns the position in which item was enqueued (0 is the first position) */
int srtn_enqueue(struct pr** queue, struct pr* item,
    int front, int* rear, int n);
void rr_enqueue(struct pr** queue, struct pr* item, int* rear, int n);
