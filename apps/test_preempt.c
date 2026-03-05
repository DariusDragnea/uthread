#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sem.h>
#include <uthread.h>

// When preempt is enabled, this test should terminate.
// When preempt is disabled, this test should run forever.

sem_t sem;

static void thread2(void *arg)
{
    (void)arg;
    for (int i = 0; i < 100000000; i++) {
        if (i % 1000000 == 0) {
            printf("thread2: %d\n", i);
        }
    }
    sem_down(sem);
    printf("thread2 finished\n");
}

static void thread1(void *arg)
{
    (void)arg;
    uthread_create(thread2, NULL);
    int i = 0;
    while (1) {
        sem_down(sem);
        i++;
        if (i % 1000000 == 0) {
            printf("thread1: %d\n", i);
        }
        sem_up(sem);
    }
}

int main(void)
{
    sem = sem_create(1);
    uthread_run(true, thread1, NULL);
    sem_destroy(sem);
    return 0;
}
