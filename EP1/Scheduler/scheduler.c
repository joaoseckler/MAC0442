#define _GNU_SOURCE

#include <float.h>
#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

#include "scheduler.h"

pthread_mutex_t* mutexv = NULL;
int* n_cpu = NULL;
int* indices = NULL;

#define RR_QUANTUM 0.3 * SECOND;
#define EMPTY_QUEUE (n == 1 || rear == (front + 1) % n)

void* thread_routine(void* arg)
{
    int i = *(int*)arg, dummy = 0;
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 1e7; // 1 ns is 1e-9

    n_cpu[i] = sched_getcpu();

    while (1) {
        pthread_mutex_lock(mutexv + i);
        dummy = dummy << 1; // Operação aritmética!!!
        n_cpu[i] = sched_getcpu();
        pthread_mutex_unlock(mutexv + i);
        nanosleep(&t, NULL);
        pthread_testcancel();
    }
    pthread_exit(0);
}

void fcfs(struct pr* prv, int n, FILE* fp, int d)
{
    struct timespec t, start, now;
    float wait, dummy, elapsed = 0;
    int contextchange = 0;

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < n; i++) {
        wait = prv[i].t0 - elapsed;
        if (wait > 0) {
            t.tv_sec = (time_t)wait;
            t.tv_nsec = (long)(modff(wait, &dummy) * 1e9);
            nanosleep(&t, NULL);
        } else if (elapsed != 0)
            contextchange++;

        if (d) {
            fprintf(stderr, "Chegada de processo: '%s %d %d %d'\n", prv[i].name, (int)prv[i].t0, (int)prv[i].dt, (int)prv[i].deadline);
        }

        clock_gettime(CLOCK_REALTIME, &now);
        timediff(&start, &now, &t);
        elapsed = t.tv_sec + t.tv_nsec * 1e-9;

        pthread_create(&prv[i].thread, NULL, thread_routine, (void*)(indices + i));

        if (d) {
            fprintf(stderr, "Processo %s começou a usar a CPU %d\n", prv[i].name, n_cpu);
        }

        t.tv_sec = (time_t)prv[i].dt;
        t.tv_nsec = (long)(modff(prv[i].dt, &dummy) * 1e9);
        nanosleep(&t, NULL);
        pthread_cancel(prv[i].thread);

        if (d) {
            fprintf(stderr, "Processo %s encerrou. Liberou a CPU %d\n", prv[i].name, n_cpu);
        }

        pthread_join(prv[i].thread, NULL);

        clock_gettime(CLOCK_REALTIME, &now);
        timediff(&start, &now, &t);
        elapsed = t.tv_sec + t.tv_nsec * 1e-9;
        fprintf(fp, "%s %f %f\n", prv[i].name, elapsed / SECOND, (elapsed - prv[i].t0) / SECOND);
        if (d) {
            fprintf(stderr, "Finalizado o processo %s. Escrevendo '%s %f %f' no arquivo de saída.\n",
                prv[i].name, prv[i].name, elapsed / SECOND, (elapsed - prv[i].t0) / SECOND);
        }
    }
    fprintf(fp, "%d\n", contextchange);
    if (d) {
        fprintf(stderr, "Quantidade de mudanças de contexto: %d\n", contextchange);
    }
}

void srtn(struct pr* prv, int n, FILE* fp, int d)
{
    struct timespec t, start, now;
    float dummy, wait, elapsed = 0;
    int contextchange = 0;
    struct pr *running = NULL, *swap_dummy;

    struct pr** queue = malloc(sizeof(struct pr*) * n);
    int front = 0, rear = 1, i = 0;

    clock_gettime(CLOCK_REALTIME, &start);

    while (i < n || running || !EMPTY_QUEUE) {
        wait = FLT_MAX;
        if (i < n) {
            wait = (float)prv[i].t0 - elapsed;
        }
        if (running == NULL || wait < running->remaining) {
            /* O próximo evento é a chegada de um processo */

            t.tv_sec = (time_t)wait;
            t.tv_nsec = (long)(modff(wait, &dummy) * 1e9);
            nanosleep(&t, NULL);
            if (running)
                running->remaining -= wait;

            clock_gettime(CLOCK_REALTIME, &now);
            timediff(&start, &now, &t);
            elapsed = t.tv_sec + t.tv_nsec * 1e-9;
            /* We could do elapsed += wait, but that would accumulate errors */

            if (running) {
                if (srtn_enqueue(queue, prv + i, front, &rear, n) == front + 1 && running->remaining > (float)prv[i].dt) {

                    /* Preempção !!! */
                    pthread_mutex_lock(mutexv + running->id);
                    swap_dummy = running;
                    running = queue[front + 1];
                    queue[front + 1] = swap_dummy;

                    pthread_create(&running->thread, NULL, thread_routine,
                        (void*)(indices + running->id));
                    running->created = 1;
                    contextchange++;
                }
            } else {
                running = prv + i;
                pthread_create(&running->thread, NULL, thread_routine,
                    (void*)(indices + running->id));
                running->created = 1;
            }
            i++;

        } else { /* O próximo evento é um processo acabar */
            t.tv_sec = (time_t)running->remaining;
            t.tv_nsec = (long)(modff(running->remaining, &dummy) * 1e9);
            nanosleep(&t, NULL);
            clock_gettime(CLOCK_REALTIME, &now);
            timediff(&start, &now, &t);
            elapsed = t.tv_sec + t.tv_nsec * 1e-9;

            pthread_cancel(running->thread);
            pthread_join(running->thread, NULL);
            fprintf(fp, "%s %f %f\n", running->name, (elapsed) / SECOND,
                (elapsed - running->t0) / SECOND);

            if (!EMPTY_QUEUE) { /* if queue not empty */
                front = (front + 1) % n;
                running = queue[front];
                if (running->created) {
                    pthread_mutex_unlock(mutexv + running->id);
                } else {
                    pthread_create(&running->thread, NULL, thread_routine,
                        (void*)(indices + running->id));
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

void rr(struct pr* prv, int n, FILE* fp, int d)
{
    struct timespec t, start, now;
    float dummy, wait, elapsed = 0, qremaining = RR_QUANTUM;
    int contextchange = 0;
    struct pr* running = NULL;

    struct pr** queue = malloc(sizeof(struct pr*) * n);
    int front = 0, rear = 1, i = 0;

    clock_gettime(CLOCK_REALTIME, &start);

    while (i < n || running || !EMPTY_QUEUE) {
        wait = FLT_MAX;
        if (i < n) {
            wait = (float)prv[i].t0 - elapsed;
        }

        if (running == NULL || (wait < running->remaining && (EMPTY_QUEUE || wait < qremaining))) {
            /* O próximo evento é a chegada de um processo */

            t.tv_sec = (time_t)wait;
            t.tv_nsec = (long)(modff(wait, &dummy) * 1e9);
            nanosleep(&t, NULL);
            if (running) {
                running->remaining -= wait;
                if (!EMPTY_QUEUE)
                    qremaining -= wait;
            }

            clock_gettime(CLOCK_REALTIME, &now);
            timediff(&start, &now, &t);
            elapsed = t.tv_sec + t.tv_nsec * 1e-9;
            /* We could do elapsed += wait, but that would accumulate errors */

            if (running == NULL) {
                running = prv + i;
                pthread_create(&running->thread, NULL, thread_routine,
                    (void*)(indices + running->id));
                running->created = 1;

            } else {
                rr_enqueue(queue, prv + i, &rear, n);
            }
            i++;

        } else if (running->remaining < wait && (EMPTY_QUEUE || running->remaining < qremaining)) {
            /* O próximo evento é um processo acabar */
            t.tv_sec = (time_t)running->remaining;
            t.tv_nsec = (long)(modff(running->remaining, &dummy) * 1e9);
            nanosleep(&t, NULL);
            qremaining -= running->remaining;

            clock_gettime(CLOCK_REALTIME, &now);
            timediff(&start, &now, &t);
            elapsed = t.tv_sec + t.tv_nsec * 1e-9;

            pthread_cancel(running->thread);
            pthread_join(running->thread, NULL);
            fprintf(fp, "%s %f %f\n", running->name, elapsed / SECOND,
                (elapsed - running->t0) / SECOND);

            if (!EMPTY_QUEUE) {
                front = (front + 1) % n;
                running = queue[front];
                if (running->created) {
                    pthread_mutex_unlock(mutexv + running->id);
                } else {
                    pthread_create(&running->thread, NULL, thread_routine,
                        (void*)(indices + running->id));
                    running->created = 1;
                }
                contextchange++;
            } else
                running = NULL;

        } else { /* O próximo evento é o quantum acabar */
            t.tv_sec = (time_t)qremaining;
            t.tv_nsec = (long)(modff(qremaining, &dummy) * 1e9);
            nanosleep(&t, NULL);
            running->remaining -= qremaining;
            qremaining = RR_QUANTUM;

            /* Preempção !!! */
            pthread_mutex_lock(mutexv + running->id);
            queue[rear] = running;
            running = queue[(front + 1) % n];
            rear = (rear + 1) % n;
            front = (front + 1) % n;

            if (running->created) {
                pthread_mutex_unlock(mutexv + running->id);
            } else {
                pthread_create(&running->thread, NULL, thread_routine,
                    (void*)(indices + running->id));
                running->created = 1;
            }
            contextchange++;
        }
    }
    fprintf(fp, "%d\n", contextchange);
    free(queue);
}

void timediff(struct timespec* a, struct timespec* b, struct timespec* result)
{
    result->tv_sec = b->tv_sec - a->tv_sec;

    if (a->tv_nsec > b->tv_nsec) {
        result->tv_nsec = 1000000000 - a->tv_nsec + b->tv_nsec;
        result->tv_sec--;
    } else
        result->tv_nsec = b->tv_nsec - a->tv_nsec;
}

int srtn_enqueue(struct pr** queue, struct pr* item,
    int front, int* rear, int n)
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

void rr_enqueue(struct pr** queue, struct pr* item, int* rear, int n)
{
    queue[*rear] = item;
    *rear = (*rear + 1) % n;
}
