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
  float remaining;
  int id;
  int created;
};

void fcfs(struct pr *prv, int n, FILE *fp, int d);
void srtn(struct pr *prv, int n, FILE *fp, int d);

void *thread_routine (void *arg);

/* Where a is before b */
void timediff(struct timespec *a, struct timespec *b, struct timespec *result);

/* Enqueue item in queue. Queue is ordered by its items integer field dt
 * Returns the position in which item was enqueued (0 is the first position) */
int enqueue(struct pr ** queue, struct pr * item, int front, int *rear, int n);
