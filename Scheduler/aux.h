#include <pthread.h>
#include <time.h>

/* one mutex for each process */
extern pthread_mutex_t * mutexv;
/* list of indices of the above array */
extern int * indices;

struct pr {
  char * name;
  pthread_t thread;
  int t0; // Initial time
  int dt; // Duration of process
  int deadline;
  float remaining; // Time remaining for the process to complete
  int id; // integer identifier for the process
  int created; // Is the thread created already or not
};

void fcfs(struct pr *prv, int n, FILE *fp, int d);
void srtn(struct pr *prv, int n, FILE *fp, int d);
void rr(struct pr *prv, int n, FILE *fp, int d);

void *thread_routine (void *arg);

/* Where a is before b */
void timediff(struct timespec *a, struct timespec *b, struct timespec *result);

/* Enqueue item in queue. Queue is ordered by its items integer field dt
 * Returns the position in which item was enqueued (0 is the first position) */
int srtn_enqueue(struct pr ** queue, struct pr * item,
                 int front, int *rear, int n);
void rr_enqueue(struct pr ** queue, struct pr * item, int *rear, int n);

