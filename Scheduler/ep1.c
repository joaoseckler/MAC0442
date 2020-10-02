#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

pthread_mutex_t * mutexv;
int * indices;

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



int main(int argc, char * argv[])
{

  /******* Treat arguments *******************************************/

  int escalonador = atoi(argv[1]);
  char * infile = argv[2];
  char * outfile = argv[3];
  int d = 0;
  if (argc >= 5 && strcmp(argv[4], "d") == 0 )
    d = 1;
  /*******************************************************************/


  int nprocesses = 0;
  size_t  m = 0;
  char * line = NULL;
  FILE * fp = fopen(infile, "r");

  if (fp == NULL) {
    printf("erro ao abrir arquivo %s\n", infile);
    exit(EXIT_FAILURE);
  }

  while ((getline(&line, &m, fp) != -1))
    nprocesses++;

  struct pr * prv = malloc(nprocesses * sizeof(struct pr));
  mutexv = malloc(nprocesses * sizeof(pthread_mutex_t));
  indices = malloc(nprocesses * sizeof(int));

  rewind(fp);

  int i = 0;
  while ((getline(&line, &m, fp) != -1)) {
    prv[i].name = malloc(30 * sizeof(char));
    sscanf(line, "%s %d %d %d\n", prv[i].name,
        &prv[i].t0, &prv[i].dt, &prv[i].deadline);
    pthread_mutex_init(mutexv + i, NULL);
    /* pthread_mutex_lock(mutexv + i); */
    indices[i] = i;
    i++;
  }

  fclose(fp);
  fp = fopen(outfile, "w");
  if (fp == NULL) {
    printf("erro ao abrir arquivo %s\n", outfile);
    exit(EXIT_FAILURE);
  }

  switch (escalonador) {
    case 1:
      fcfs(prv, nprocesses, fp, d);
      break;
    default:
      printf("Número escolhido para o escalonador não é válido");
  }

  /****************** Free allocated memory ***************************/

  fclose(fp);
  for (int i = 0; i < nprocesses; i++) {
    free(prv[i].name);
    pthread_mutex_destroy(mutexv + i);
  }
  free(prv);
  free(indices);
  free(mutexv);

}

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
