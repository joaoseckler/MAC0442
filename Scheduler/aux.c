#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "aux.h"

pthread_mutex_t * mutexv = NULL;
int * indices = NULL;

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

    fprintf(fp, "%s %ld,%.3ld %ld.%.3ld\n", prv[i].name,
        elapsed.tv_sec, elapsed.tv_nsec/1000000,
        tr.tv_sec, tr.tv_nsec/100000);

  }
  fprintf(fp, "%d\n", contextchange);
}

void srtn(struct pr *prv, int n, FILE *fp, int d)
{
  struct timespec t, start, now;
  float dummy, wait, tr, elapsed = 0;
  int contextchange = 0;
  struct pr * running = NULL, * swap_dummy;

  struct pr ** queue = malloc(sizeof(struct pr *) * n);
  int front = 0, rear = 1, i = 0;

  clock_gettime(CLOCK_REALTIME, &start);

  while (i < n || running || rear != (front + 1) % n) {
    wait = FLT_MAX;
    if (i < n) {
      wait = (float) prv[i].t0 - elapsed;
    }
    if (running == NULL || wait < running->remaining) {
      /* O próximo evento é a chegada de um processo */

      t.tv_sec = (time_t) wait;
      t.tv_nsec = (long) (modff(wait, &dummy) * 1e9);
      nanosleep(&t, NULL);
      if (running)
        running->remaining -= wait;

      clock_gettime(CLOCK_REALTIME, &now);
      timediff(&start, &now, &t);
      elapsed = t.tv_sec + t.tv_nsec*1e-9;
      /* We could do elapsed += wait, but that would accumulate errors */

      if (running) {
        if (enqueue(queue, prv + i, front, &rear, n) == front + 1 &&
            running->remaining > (float) prv[i].dt) {

          /* Preempção !!! */
          pthread_mutex_lock(mutexv + running->id);
          swap_dummy = running;
          running = queue[front + 1];
          queue[front + 1] = swap_dummy;

          pthread_create(&running->thread, NULL, thread_routine,
                         (void *) (indices + running->id));
          running->created = 1;
          contextchange++;
        }
      }
      else {
        running = prv + i;
        pthread_create(&running->thread, NULL, thread_routine,
                       (void *) (indices + running->id));
        running->created = 1;
      }
      i++;

    } else { /* O próximo evento é um processo acabar */
      t.tv_sec = (time_t) running->remaining;
      t.tv_nsec = (long) (modff(running->remaining, &dummy) * 1e9);
      nanosleep(&t, NULL);

      clock_gettime(CLOCK_REALTIME, &now);
      timediff(&start, &now, &t);
      elapsed = t.tv_sec + t.tv_nsec*1e-9;

      pthread_cancel(running->thread);
      pthread_join(running->thread, NULL);
      tr = elapsed - running->t0;
      fprintf(fp, "%s %f %f\n", running->name, elapsed, tr);

      if (rear != (front + 1) % n) { /* if queue not empty */
        front = (front + 1) % n;
        running = queue[front];
        if (running->created) {
          pthread_mutex_unlock(mutexv + running->id);
        }
        else {
          pthread_create(&running->thread, NULL, thread_routine,
                         (void *) (indices + running->id));
          running->created = 1;
        }
        contextchange++;
      } else
        running = NULL;
    }
  }
  fprintf(fp, "%d\n", contextchange);
  free(queue);
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

int enqueue(struct pr ** queue, struct pr * item, int front, int *rear, int n)
{
  int i, r;
  for (i = *rear - 1; i != front && item->dt < queue[i]->dt; i = (i - 1) % n) {
    queue[(i + 1) % n] = queue[i];
  }
  *rear = (*rear + 1) % n;
  r = (i + 1) % n;
  queue[r] = item;
  return r;
}


