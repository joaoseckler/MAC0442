#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

extern pthread_mutex_t * mutexv;
extern int * indices;

struct pr {
  char * name;
  pthread_t thread;
  int t0;
  int dt;
  int deadline;
};


void fcfs(struct pr *prv, int n, FILE *fp, int d);
void *thread_routine (void *arg);
void timediff(struct timespec *a, struct timespec *b, struct timespec *result);
/* Where a is before b */
