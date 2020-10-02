#include "aux.h"


void *thread_routine (void *arg)
{
  int i = * (int * ) arg, dummy = 0;
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = 1e7; // 1 ns is 1e-9

  while(1) {
    pthread_mutex_lock(mutexv + i);
    nanosleep(&t, NULL);
    dummy = dummy << 1; // Operação aritmética!!!
    pthread_mutex_unlock(mutexv + i);
    pthread_testcancel();
  }
  pthread_exit(0);
}

void fcfs(struct pr *prv, int n, FILE *fp, int d)
{
  struct timespec t0, dt, tr, start, now, elapsed;
  int contextchange = 0;

  clock_gettime(CLOCK_REALTIME, &start);
  elapsed.tv_sec = 0;
  elapsed.tv_nsec = 0;

  for (int i = 0; i < n; i++) {
    t0.tv_nsec = 0;
    t0.tv_sec = (time_t) prv[i].t0;
    dt.tv_nsec = 0;
    dt.tv_sec = (time_t) prv[i].dt;

    if (t0.tv_sec > elapsed.tv_sec) {
      timediff(&elapsed, &t0, &t0);
      nanosleep(&t0, NULL);
    }
    else
      contextchange++;

    clock_gettime(CLOCK_REALTIME, &now);
    timediff(&start, &now, &t0);

    pthread_create(&prv[i].thread, NULL, thread_routine, (void *) (indices + i));
    nanosleep(&dt, NULL);
    pthread_cancel(prv[i].thread);
    pthread_join(prv[i].thread, NULL);

    clock_gettime(CLOCK_REALTIME, &now);
    timediff(&start, &now, &elapsed);
    timediff(&t0, &elapsed, &tr);

    fprintf(fp, "%s %ld,%.3ld %ld,%.3ld\n", prv[i].name,
        elapsed.tv_sec, elapsed.tv_nsec/1000000,
        tr.tv_sec, tr.tv_nsec/100000);

  }
  fprintf(fp, "%d\n", contextchange);
}


void timediff(struct timespec *a, struct timespec *b, struct timespec *result)
{
  result->tv_sec = b->tv_sec - a->tv_sec;

  if (a->tv_nsec > b->tv_nsec) {
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    result->tv_sec--;
  }
  else
    result->tv_nsec = b->tv_nsec - a->tv_nsec;
}
