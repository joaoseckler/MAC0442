#define _GNU_SOURCE

#include <float.h>
#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

#include "scheduler.h"

#define RR_QUANTUM 0.3;
#define EMPTY_QUEUE (n == 1 || rear == (front + 1) % n)

void* thread_routine(void* arg)
{
    struct pr *p = (struct pr*)arg;
    int dummy = 0;
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 1e7; // 1 ns is 1e-9

    while (1) {
        pthread_mutex_lock(&p->mutex);
        dummy = dummy << 1; // Operação aritmética!!!
        p->n_cpu = sched_getcpu();
        if (d && p->print) {
            p->print = 0;
            fprintf(stderr, "Processo %s começou a usar a CPU %d\n",
                    p->name, p->n_cpu);
        }
        pthread_mutex_unlock(&p->mutex);
        nanosleep(&t, NULL);
        pthread_testcancel();
    }
    pthread_exit(0);
}


void fcfs(struct pr* prv, int n, FILE* fp)
{
    struct timespec t, start, now;
    float wait, dummy, elapsed = 0;
    int contextchange = 0;
    struct pr *running = NULL;

    struct pr** queue = malloc(sizeof(struct pr*) * n);
    int front = 0, rear = 1, i = 0;

    clock_gettime(CLOCK_REALTIME, &start);

#ifdef DEADLINES
    FILE* fp_deadlines = fopen("deadlines", "w");
    int made_deadline = 0;
#endif

    while (i < n || running || !EMPTY_QUEUE) {
        wait = FLT_MAX;
        if (i < n)
            wait = prv[i].t0 - elapsed;
        if (running == NULL || wait < running->remaining) {
            /* Próximo evento é a chegada de um processo */
            t.tv_sec = (time_t)wait;
            t.tv_nsec = (long)(modff(wait, &dummy) * 1e9);
            nanosleep(&t, NULL);

            clock_gettime(CLOCK_REALTIME, &now);
            timediff(&start, &now, &t);
            elapsed = t.tv_sec + t.tv_nsec * 1e-9;

            if (running) {
                running->remaining -= wait;
                simple_enqueue(queue, prv + i, &rear, n);
            } else {
                running = prv + i;
                pthread_create(&running->thread, NULL, thread_routine,
                    (void*)(prv + running->id));
            }
            if (d)
                fprintf(stderr, "Chegada de processo: '%s %d %d %d'\n",
                        prv[i].name, (int)(prv[i].t0),
                        (int)(prv[i].dt),
                        (int)(prv[i].deadline));
            i++;
        } else {
            /* Próximo evento é o término de um processo */

            t.tv_sec = (time_t)running->remaining;
            t.tv_nsec = (long)(modff(running->remaining, &dummy) * 1e9);
            nanosleep(&t, NULL);

            clock_gettime(CLOCK_REALTIME, &now);
            timediff(&start, &now, &t);
            elapsed = t.tv_sec + t.tv_nsec * 1e-9;

            pthread_cancel(running->thread);
            pthread_join(running->thread, NULL);
            fprintf(fp, "%s %f %f\n", running->name, elapsed, elapsed - running->t0);
            if (d)
                fprintf(stderr, "Processo %s encerrou. Liberou a CPU %d\n",
                        running->name, running->n_cpu);
#ifdef DEADLINES
            if (elapsed < running->deadline)
                made_deadline++;
#endif

            if (!EMPTY_QUEUE) {
                front = (front + 1) % n;
                running = queue[front];
                pthread_create(&running->thread, NULL, thread_routine, (void*)(prv + running->id));
                running->created = 1;
                contextchange++;
                if (d)
                    fprintf(stderr, "%d-ésima mudança de contexto\n", contextchange);
            } else
                running = NULL;
        }
    }
    fprintf(fp, "%d\n", contextchange);
    free(queue);
#ifdef DEADLINES
    fprintf(fp_deadlines, "%f\n", (float)made_deadline/(float)n);
#endif /* DEADLINES */
}

void srtn(struct pr* prv, int n, FILE* fp)
{
    struct timespec t, start, now;
    float dummy, wait, elapsed = 0;
    int contextchange = 0;
    struct pr *running = NULL, *swap_dummy;

    struct pr** queue = malloc(sizeof(struct pr*) * n);
    int front = 0, rear = 1, i = 0;

    clock_gettime(CLOCK_REALTIME, &start);

#ifdef DEADLINES
    FILE* fp_deadlines = fopen("deadlines", "w");
    int made_deadline = 0;
#endif /* DEADLINES */

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

            if (d)
                fprintf(stderr, "Chegada de processo: '%s %d %d %d'\n",
                        prv[i].name, (int)(prv[i].t0),
                        (int)(prv[i].dt),
                        (int)(prv[i].deadline));

            if (running) {
                if (srtn_enqueue(queue, prv + i, front, &rear, n) == front + 1 && running->remaining > (float)prv[i].dt) {

                    /* Preempção !!! */
                    pthread_mutex_lock(&running->mutex);
                    if (d)
                        fprintf(stderr, "Processo %s deixou de usar a CPU %d\n",
                                running->name, running->n_cpu);

                    running->print = 1;
                    swap_dummy = running;
                    running = queue[front + 1];
                    queue[front + 1] = swap_dummy;

                    pthread_create(&running->thread, NULL, thread_routine,
                        (void*)(prv + running->id));
                    running->created = 1;
                    contextchange++;
                    if (d)
                        fprintf(stderr, "%d-ésima mudança de contexto\n", contextchange);
                }
            } else {
                running = prv + i;
                pthread_create(&running->thread, NULL, thread_routine,
                    (void*)(prv + running->id));
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
            fprintf(fp, "%s %f %f\n", running->name, (elapsed),
                (elapsed - running->t0));
            if (d)
                fprintf(stderr, "Processo %s encerrou. Liberou a CPU %d\n",
                        running->name, running->n_cpu);
#ifdef DEADLINES
            if (elapsed < running->deadline)
                made_deadline++;
#endif

            if (!EMPTY_QUEUE) { /* if queue not empty */
                front = (front + 1) % n;
                running = queue[front];
                if (running->created) {
                    pthread_mutex_unlock(&running->mutex);
                } else {
                    pthread_create(&running->thread, NULL, thread_routine,
                        (void*)(prv + running->id));
                    running->created = 1;
                }
                contextchange++;
                if (d)
                    fprintf(stderr, "%d-ésima mudança de contexto\n", contextchange);
            } else
                running = NULL;
        }
    }
    fprintf(fp, "%d\n", contextchange);
    free(queue);
#ifdef DEADLINES
    fprintf(fp_deadlines, "%f\n", (float)made_deadline/(float)n);
#endif /* DEADLINES */
}

void rr(struct pr* prv, int n, FILE* fp)
{
    struct timespec t, start, now;
    float dummy, wait, elapsed = 0, qremaining = RR_QUANTUM;
    int contextchange = 0;
    struct pr* running = NULL;

    struct pr** queue = malloc(sizeof(struct pr*) * n);
    int front = 0, rear = 1, i = 0;

    clock_gettime(CLOCK_REALTIME, &start);

#ifdef DEADLINES
    FILE* fp_deadlines = fopen("deadlines", "w");
    int made_deadline = 0;
#endif /* DEADLINES */

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
            if (d)
                fprintf(stderr, "Chegada de processo: '%s %d %d %d'\n",
                        prv[i].name, (int)(prv[i].t0),
                        (int)(prv[i].dt),
                        (int)(prv[i].deadline));
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
                    (void*)(prv + running->id));
                running->created = 1;

            } else {
                simple_enqueue(queue, prv + i, &rear, n);
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
            fprintf(fp, "%s %f %f\n", running->name, elapsed,
                (elapsed - running->t0));
            if (d)
                fprintf(stderr, "Processo %s encerrou. Liberou a CPU %d\n",
                        running->name, running->n_cpu);
#ifdef DEADLINES
            printf("elapsed: %f, name: %s, deadline: %f\n", elapsed, running->name, running->deadline);
            if (elapsed < running->deadline)
                made_deadline++;
#endif

            if (!EMPTY_QUEUE) {
                front = (front + 1) % n;
                running = queue[front];
                if (running->created) {
                    pthread_mutex_unlock(&running->mutex);
                } else {
                    pthread_create(&running->thread, NULL, thread_routine,
                        (void*)(prv + running->id));
                    running->created = 1;
                }
                contextchange++;
                if (d)
                    fprintf(stderr, "%d-ésima mudança de contexto\n", contextchange);
            } else
                running = NULL;

        } else { /* O próximo evento é o quantum acabar */
            t.tv_sec = (time_t)qremaining;
            t.tv_nsec = (long)(modff(qremaining, &dummy) * 1e9);
            nanosleep(&t, NULL);
            running->remaining -= qremaining;
            qremaining = RR_QUANTUM;

            clock_gettime(CLOCK_REALTIME, &now);
            timediff(&start, &now, &t);
            elapsed = t.tv_sec + t.tv_nsec * 1e-9;

            /* Preempção !!! */
            pthread_mutex_lock(&running->mutex);
            running->print = 1;
            if (d)
                fprintf(stderr, "Processo %s deixou de usar a CPU %d\n",
                        running->name, running->n_cpu);

            queue[rear] = running;
            running = queue[(front + 1) % n];
            rear = (rear + 1) % n;
            front = (front + 1) % n;

            if (running->created) {
                pthread_mutex_unlock(&running->mutex);
            } else {
                pthread_create(&running->thread, NULL, thread_routine,
                    (void*)(prv + running->id));
                running->created = 1;
            }
            contextchange++;
            if (d)
                fprintf(stderr, "%d-ésima mudança de contexto\n", contextchange);
        }
    }

    fprintf(fp, "%d\n", contextchange);

#ifdef DEADLINES
    fprintf(fp_deadlines, "%f\n", (float)made_deadline/(float)n);
#endif /* DEADLINES */

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

void simple_enqueue(struct pr** queue, struct pr* item, int* rear, int n)
{
    queue[*rear] = item;
    *rear = (*rear + 1) % n;
}
